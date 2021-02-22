#include "ElasticDeformation.hpp"

#include <cassert>
#include <random>

#include <iostream>//DEBUG

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
	      const float nx = 2*x - map_x.at<float>(y, x);
	      const float ny = 2*y - map_y.at<float>(y, x);
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
	      const float nx = 2*x - map_x.at<float>(y, x);
	      const float ny = 2*y - map_y.at<float>(y, x);
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
	      const float nx = 2*x - map_x.at<float>(y, x);
	      const float ny = 2*y - map_y.at<float>(y, x);
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
	      const float nx = 2*x - map_x.at<float>(y, x);
	      const float ny = 2*y - map_y.at<float>(y, x);
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

    cv::Mat transform(const cv::Mat &img, float alpha, float sigma,
		      BorderReplication borderMode,
		      Interpolation interpolation,
		      cv::Mat &map_x, cv::Mat &map_y)
    {
      const int scale = std::min(img.cols, img.rows);
      alpha *= scale;
      sigma *= scale;

      cv::Mat dx(img.size(), CV_32FC1);
      cv::randu(dx, -1.0f, 1.0f);
      cv::Mat dy(img.size(), CV_32FC1);
      cv::randu(dy, -1.0f, 1.0f);

      cv::GaussianBlur(dx, dx, cv::Size(), sigma);
      cv::GaussianBlur(dy, dy, cv::Size(), sigma);

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
		      Interpolation interpolation)
    {
      cv::Mat map_x, map_y;
      return transform(img, alpha, sigma, borderMode, interpolation, map_x, map_y);
    }

    cv::Mat transform_rects(const cv::Mat &img,
			    const std::vector<cv::Rect> &rects,
			    std::vector<std::vector<cv::Point2f> > &rects_pts,
			    float alpha, float sigma,
			    BorderReplication borderMode,
			    Interpolation interpolation)
    {
      cv::Mat map_x, map_y;
      cv::Mat img2 = transform(img, alpha, sigma, borderMode, interpolation, map_x, map_y);

      getTransformedRects(map_x, map_y, rects, rects_pts);

      return img2;
    }


    cv::Mat transform2(const cv::Mat &img,
		       float alpha, float sigma, float alpha_affine,
		       BorderReplication borderMode,
		       Interpolation interpolation,
		       cv::Mat &map_x, cv::Mat &map_y, cv::Mat &M)
    {
      const int width = img.cols;
      const int height = img.rows;
      const cv::Point center(width/2, height/2);
      const int shift = std::min(width, height)/3;
      cv::Point2f src[3];
      src[0] = cv::Point2f(center.x+shift, center.y+shift);
      src[1] = cv::Point2f(center.x+shift, center.y-shift);
      src[2] = cv::Point2f(center.x-shift, center.y-shift);

      std::default_random_engine generator;
      std::uniform_real_distribution<float> distribution(-alpha_affine, alpha_affine);

      cv::Point2f dst[3];
      for (int i=0; i<3; ++i) 
	dst[i] = cv::Point2f(src[i].x + distribution(generator),
			     src[i].y + distribution(generator));

      M = cv::getAffineTransform(src, dst);

      const int borderModeCV = getBorderModeCV(borderMode);

      cv::Mat img2;
      cv::warpAffine(img, img2, M, img.size(), borderModeCV);

      return transform(img2, alpha, sigma, borderMode, interpolation, map_x, map_y);
    }

    cv::Mat transform2(const cv::Mat &img, float alpha, float sigma, float alpha_affine,
		       BorderReplication borderMode,
		       Interpolation interpolation)

    {
      cv::Mat map_x, map_y, M;
      return transform2(img, alpha, sigma, alpha_affine, borderMode, interpolation, map_x, map_y, M);
    }

    cv::Mat transform2_rects(const cv::Mat &img,
			     const std::vector<cv::Rect> &rects,
			     std::vector<std::vector<cv::Point2f> > &rects_pts,
			     float alpha, float sigma, float alpha_affine,
			     BorderReplication borderMode,
			     Interpolation interpolation)

    {
      cv::Mat map_x, map_y, M;
      cv::Mat res = transform2(img, alpha, sigma, alpha_affine, borderMode, interpolation, map_x, map_y, M);

      std::vector<std::vector<cv::Point2f> > rects_pts0;
      getTransformedRects(map_x, map_y, rects, rects_pts0);

      rects_pts.reserve(rects_pts0.size());
      for (const auto &v : rects_pts0) {
	std::vector<cv::Point2f> out;
	out.reserve(v.size());
	for (const auto &p : v) {
	  cv::Point2f pt(M.at<double>(0, 0)*p.x + M.at<double>(0, 1)*p.y + M.at<double>(0, 2),
			 M.at<double>(1, 0)*p.x + M.at<double>(1, 1)*p.y + M.at<double>(1, 2));
	  out.push_back(pt);
	}
	rects_pts.push_back(out);
      }

      return res;
    }


  } //namespace ElasticDeformation
} //namespace dc



