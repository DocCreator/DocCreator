#include "ElasticDeformation.hpp"

#include <algorithm> //std::min, std::max

#include <cassert>
#include <cmath> //std::isfinite
#include <random>

#include <iostream>//DEBUG

#include "Random.hpp"
#include "Validate.hpp"
#include <opencv2/core/version.hpp>
#if CV_VERSION_MAJOR >= 5
#include <opencv2/geometry.hpp> //minAreaRect, boundingRect, getAffineTransform, getRotationMatrix2D: moved out of imgproc in OpenCV 5
#endif

namespace dc {
  namespace ElasticDeformation {


    namespace {

      inline
      int
      getBorderModeCV(BorderReplication borderMode)
      {
	int borderModeCV = cv::BORDER_REPLICATE;
	switch (borderMode) {
	default:
	case BorderReplication::REPLICATE:
	  borderModeCV = cv::BORDER_REPLICATE;
	  break;
	case BorderReplication::REFLECT:
	  borderModeCV = cv::BORDER_REFLECT;
	  break;
	case BorderReplication::WRAP:
	  borderModeCV = cv::BORDER_WRAP;
	  break;
	case BorderReplication::REFLECT101:
	  borderModeCV = cv::BORDER_REFLECT101;
	  break;
	//B: BLACK was missing, and gave BORDER_REPLICATE: the border was
	// never black. With BORDER_CONSTANT, cv::remap() and cv::warpAffine()
	// fill it with their border value, black.
	case BorderReplication::BLACK:
	  borderModeCV = cv::BORDER_CONSTANT;
	  break;
	}
	return borderModeCV;
      }

      inline
      int getInterpolationCV(Interpolation interpolation)
      {
	int interpolationCV = cv::INTER_LINEAR;
	switch (interpolation) {
	case Interpolation::NEAREST:
	  interpolationCV = cv::INTER_NEAREST;
	  break;
	default:
	case Interpolation::BILINEAR:
	  interpolationCV = cv::INTER_LINEAR;
	  break;
	case Interpolation::AREA:
	  interpolationCV = cv::INTER_AREA;
	  break;
	case Interpolation::BICUBIC:
	  interpolationCV = cv::INTER_CUBIC;
	  break;
	case Interpolation::LANCZOS:
	  interpolationCV = cv::INTER_LANCZOS4;
	  break;
	}
	return interpolationCV;
      }

    } //anonymous namespace


    void getTransformedRects(const cv::Mat &map_x, const cv::Mat &map_y,
			     const std::vector<cv::Rect> &rects,
			     std::vector<std::vector<cv::Point2f> > &rects_pts)
    {
      assert(map_x.type() == CV_32FC1);
      assert(map_y.type() == CV_32FC1);
      assert(map_x.size() == map_y.size());

      //TODO: convert code from
      // https://stackoverflow.com/questions/41703210/inverting-a-real-valued-index-grid
      // to C++

      const int rows = map_x.rows;
      const int cols = map_x.cols;

      for (const cv::Rect &r : rects) {
	std::vector<cv::Point2f> npts;
	int x, y;
	// (x, y) -> (x+w, y)
	y = r.y;
	if (y >= 0 && y < rows) {
	  x = r.x;
	  for ( ; x <= r.x+r.width; ++x) {
	    if (x >= 0 && x < cols) {
	      // map_x.at<float>(y, x) actually stores x+dx
	      // we want x-dx that is 2*x-nx
	      const float nx = 2.f*x - map_x.at<float>(y, x);
	      const float ny = 2.f*y - map_y.at<float>(y, x);
	      npts.push_back(cv::Point2f(nx, ny));
	    }
	  }
	}
	// (x+w, y) -> (x+w, y+h)
	x = r.x + r.width;
	if (x >= 0 && x < cols) {
	  y = r.y;
	  for ( ; y <= r.y+r.height; ++y) {
	    if (y >= 0 && y < rows) {
	      const float nx = 2.f*x - map_x.at<float>(y, x);
	      const float ny = 2.f*y - map_y.at<float>(y, x);
	      npts.push_back(cv::Point2f(nx, ny));
	    }
	  }
	}
	// (x+w, y+h) -> (x, y+h)
	y = r.y + r.height;
	if (y >= 0 && y < rows) {
	  x = r.x + r.width;
	  for ( ; x >= r.x; --x) {
	    if (x >= 0 && x < cols) {
	      const float nx = 2.f*x - map_x.at<float>(y, x);
	      const float ny = 2.f*y - map_y.at<float>(y, x);
	      npts.push_back(cv::Point2f(nx, ny));
	    }
	  }
	}
	// (x, y+h) -> (x, y)
	x = r.x;
	if (x >= 0 && x < cols) {
	  y = r.y + r.height;
	  for ( ; y >= r.y; --y) {
	    if (y >= 0 && y < rows) {
	      const float nx = 2.f*x - map_x.at<float>(y, x);
	      const float ny = 2.f*y - map_y.at<float>(y, x);
	      npts.push_back(cv::Point2f(nx, ny));
	    }
	  }
	}
	rects_pts.push_back(npts);
      }

    }

    std::vector<cv::RotatedRect> getRotatedRects(const std::vector<std::vector<cv::Point2f> > &rects_pts)
    {
      std::vector<cv::RotatedRect> rotatedRects;
      rotatedRects.reserve(rects_pts.size());
      for (const auto &v : rects_pts) {
	cv::RotatedRect rr = cv::minAreaRect(v);
	rotatedRects.push_back(rr);
      }
      return rotatedRects;
    }

    /*
      @return a field of @a size holding uniform noise smoothed by a
      Gaussian of @a sigma.

      B: this was two lines -- fill the whole field, blur it -- and it was
      the whole cost of a deformation. @a sigma is multiplied by the size
      of the image by the caller, so that a deformation looks the same
      whatever resolution it is applied at; the Gaussian then has a kernel
      as wide as the image is large. OpenCV takes 8*sigma+1 taps for a
      float image, that is 3023 of them for a 4721x6564 page, and the two
      blurs were 82 of the 83 seconds such a page took.

      Nothing in the field varies faster than sigma, by construction, so
      it is drawn and smoothed on a grid f times coarser with a sigma f
      times smaller, then stretched back. A Gaussian blur of white noise
      divides its deviation by the sigma it uses, so the coarse field
      comes out f times too strong and is brought back by f.

      Measured over 25 draws at four coarsenesses: the deviation of the
      field stays within 6% of what the full size blur gives, which is
      well inside the 13% one draw differs from the next.
    */
    static cv::Mat
    smoothNoiseField(RandomEngine &rng, const cv::Size &size, float sigma)
    {
      //samples kept per sigma on the coarse grid
      constexpr float SAMPLES_PER_SIGMA = 8.f;
      //and a floor on its side, so the stretch back has something to work
      //from on a small image
      constexpr int MIN_SIDE = 4;

      int f = 1;
      if (sigma > SAMPLES_PER_SIGMA) {
	f = static_cast<int>(sigma / SAMPLES_PER_SIGMA);
	const int maxF = std::max(1, std::min(size.width, size.height) / MIN_SIDE);
	f = std::min(f, maxF);
      }

      if (f <= 1) {
	cv::Mat field(size, CV_32FC1);
	rng.fill(field, cv::RNG::UNIFORM, -1.f, 1.f);
	cv::GaussianBlur(field, field, cv::Size(), sigma);
	return field;
      }

      cv::Mat coarse(cv::Size(std::max(MIN_SIDE, size.width / f),
			      std::max(MIN_SIDE, size.height / f)),
		     CV_32FC1);
      rng.fill(coarse, cv::RNG::UNIFORM, -1.f, 1.f);
      cv::GaussianBlur(coarse, coarse, cv::Size(), sigma / f);
      coarse *= 1.f / f;

      cv::Mat field;
      cv::resize(coarse, field, size, 0, 0, cv::INTER_CUBIC);
      return field;
    }

    cv::Mat transform(const cv::Mat &img, float alpha, float sigma,
		      BorderReplication borderMode,
		      Interpolation interpolation,
		      cv::Mat &map_x, cv::Mat &map_y,
		      std::optional<uint32_t> seed)
    {
      static const char *func = "dc::ElasticDeformation::transform";
      require8UC134(img, "img", func);
      //B: sigma was not checked: 0 gave cv::GaussianBlur() neither a kernel
      // size nor a sigma (cv::Exception), and a NaN went through
      requireArgument(std::isfinite(alpha), "alpha must be finite", func);
      requireArgument(std::isfinite(sigma) && sigma > 0,
		      "sigma must be positive", func);

      const float scale = static_cast<float>(std::min(img.cols, img.rows));
      alpha *= scale;
      sigma *= scale;

      //B: cv::randu() draws from a generator shared by the whole thread
      // (cf. cv::theRNG()), which the caller cannot seed.
      RandomEngine rng = makeRandomEngine(seed);

      cv::Mat dx = smoothNoiseField(rng, img.size(), sigma);
      cv::Mat dy = smoothNoiseField(rng, img.size(), sigma);

      dx *= alpha;
      dy *= alpha;

      map_x = cv::Mat(img.size(), CV_32FC1);
      map_y = cv::Mat(img.size(), CV_32FC1);
      for (int i=0; i<img.rows; ++i) {
	float *rx = map_x.ptr<float>(i);
	float *ry = map_y.ptr<float>(i);
	const float *rdx = dx.ptr<float>(i);
	const float *rdy = dy.ptr<float>(i);
	for (int j=0; j<img.cols; ++j) {
	  rx[j] = j+rdx[j];
	  ry[j] = i+rdy[j];
	}
      }

      const int borderModeCV = getBorderModeCV(borderMode);
      const int interpolationCV = getInterpolationCV(interpolation);

      cv::Mat img2;
      cv::remap(img, img2, map_x, map_y, interpolationCV, borderModeCV, cv::Scalar(0, 0, 0));

      return img2;
    }

    cv::Mat transform(const cv::Mat &img, float alpha, float sigma,
		      BorderReplication borderMode,
		      Interpolation interpolation,
		      std::optional<uint32_t> seed)
    {
      cv::Mat map_x, map_y;
      return transform(img, alpha, sigma, borderMode, interpolation, map_x, map_y, seed);
    }

    cv::Mat transform_rects(const cv::Mat &img,
			    const std::vector<cv::Rect> &rects,
			    std::vector<std::vector<cv::Point2f> > &rects_pts,
			    float alpha, float sigma,
			    BorderReplication borderMode,
			    Interpolation interpolation,
			    std::optional<uint32_t> seed)
    {
      cv::Mat map_x, map_y;
      cv::Mat img2 = transform(img, alpha, sigma, borderMode, interpolation, map_x, map_y, seed);

      getTransformedRects(map_x, map_y, rects, rects_pts);

      return img2;
    }


    cv::Mat transform2(const cv::Mat &img,
		       float alpha, float sigma, float alphaAffine,
		       BorderReplication borderMode,
		       Interpolation interpolation,
		       cv::Mat &map_x, cv::Mat &map_y, cv::Mat &M,
		       std::optional<uint32_t> seed)
    {
      static const char *func = "dc::ElasticDeformation::transform2";
      require8UC134(img, "img", func);
      requireArgument(std::isfinite(alpha), "alpha must be finite", func);
      requireArgument(std::isfinite(sigma) && sigma > 0,
		      "sigma must be positive", func);
      requireArgument(std::isfinite(alphaAffine), "alphaAffine must be finite", func);

      const int width = img.cols;
      const int height = img.rows;
      const cv::Point center(width/2, height/2);
      const int shift = std::min(width, height)/3;
      cv::Point2f src[3];
      src[0] = cv::Point2f(static_cast<float>(center.x+shift), static_cast<float>(center.y+shift));
      src[1] = cv::Point2f(static_cast<float>(center.x+shift), static_cast<float>(center.y-shift));
      src[2] = cv::Point2f(static_cast<float>(center.x-shift), static_cast<float>(center.y-shift));

      //B: the generator used to be default constructed, that is seeded with
      // a fixed value, so this affine jitter was the same on every call.
      RandomEngine rng = makeRandomEngine(seed);

      cv::Point2f dst[3];
      for (int i=0; i<3; ++i)
	dst[i] = cv::Point2f(src[i].x + rng.uniform(-alphaAffine, alphaAffine),
			     src[i].y + rng.uniform(-alphaAffine, alphaAffine));

      M = cv::getAffineTransform(src, dst);

      const int borderModeCV = getBorderModeCV(borderMode);
      const int interpolationCV = getInterpolationCV(interpolation);

      //B: the border mode was given as the 5th parameter, which is the
      // interpolation (flags): REPLICATE gave INTER_LINEAR, REFLECT
      // INTER_CUBIC, etc., and the border was always BORDER_CONSTANT, black,
      // whatever borderMode, and interpolation was ignored here.
      cv::Mat img2;
      cv::warpAffine(img, img2, M, img.size(), interpolationCV, borderModeCV,
		     cv::Scalar(0, 0, 0));

      //B: the generator is seeded again for the elastic part: with a seed,
      // the result stays reproducible.
      return transform(img2, alpha, sigma, borderMode, interpolation, map_x, map_y, seed);
    }

    cv::Mat transform2(const cv::Mat &img, float alpha, float sigma, float alphaAffine,
		       BorderReplication borderMode,
		       Interpolation interpolation,
		       std::optional<uint32_t> seed)

    {
      cv::Mat map_x, map_y, M;
      return transform2(img, alpha, sigma, alphaAffine, borderMode, interpolation, map_x, map_y, M, seed);
    }

    cv::Mat transform2_rects(const cv::Mat &img,
			     const std::vector<cv::Rect> &rects,
			     std::vector<std::vector<cv::Point2f> > &rects_pts,
			     float alpha, float sigma, float alphaAffine,
			     BorderReplication borderMode,
			     Interpolation interpolation,
			     std::optional<uint32_t> seed)

    {
      cv::Mat map_x, map_y, M;
      cv::Mat res = transform2(img, alpha, sigma, alphaAffine, borderMode, interpolation, map_x, map_y, M, seed);

      //B: the points were moved by the elastic displacement at their
      // original position, and then by M; the image is moved by M first,
      // and then by the elastic displacement, which is looked up where M
      // took the point. The rectangles drifted away from the image moved,
      // the more the stronger the affine part.
      //As in getTransformedRects(), map_x(y, x) holds x+dx, and a point s
      // goes to s-dx, with the displacement at the pixel nearest to s.
      const int rows = map_x.rows;
      const int cols = map_x.cols;
      rects_pts.reserve(rects_pts.size() + rects.size());
      for (const cv::Rect &r : rects) {
	//the boundary of r, as in getTransformedRects()
	std::vector<cv::Point2f> boundary;
	for (int x = r.x; x <= r.x + r.width; ++x)
	  boundary.emplace_back(static_cast<float>(x), static_cast<float>(r.y));
	for (int y = r.y; y <= r.y + r.height; ++y)
	  boundary.emplace_back(static_cast<float>(r.x + r.width), static_cast<float>(y));
	for (int x = r.x + r.width; x >= r.x; --x)
	  boundary.emplace_back(static_cast<float>(x), static_cast<float>(r.y + r.height));
	for (int y = r.y + r.height; y >= r.y; --y)
	  boundary.emplace_back(static_cast<float>(r.x), static_cast<float>(y));

	std::vector<cv::Point2f> out;
	out.reserve(boundary.size());
	for (const cv::Point2f &p : boundary) {
	  const double sx = M.at<double>(0, 0)*p.x + M.at<double>(0, 1)*p.y + M.at<double>(0, 2);
	  const double sy = M.at<double>(1, 0)*p.x + M.at<double>(1, 1)*p.y + M.at<double>(1, 2);
	  const int xi = std::min(std::max(static_cast<int>(std::lround(sx)), 0), cols - 1);
	  const int yi = std::min(std::max(static_cast<int>(std::lround(sy)), 0), rows - 1);
	  const double dx = map_x.at<float>(yi, xi) - xi;
	  const double dy = map_y.at<float>(yi, xi) - yi;
	  out.emplace_back(static_cast<float>(sx - dx), static_cast<float>(sy - dy));
	}
	rects_pts.push_back(out);
      }

      return res;
    }


  } //namespace ElasticDeformation
} //namespace dc



