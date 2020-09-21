#include "ElasticDeformation.hpp"

#include <cassert>
#include <random>

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


    cv::Mat transform(const cv::Mat &img, float alpha, float sigma,
		      BorderReplication borderMode,
		      Interpolation interpolation)
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

      cv::Mat map_x(img.size(), CV_32FC1);
      cv::Mat map_y(img.size(), CV_32FC1);
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


    cv::Mat transform2(const cv::Mat &img, float alpha, float sigma, float alpha_affine,
		       BorderReplication borderMode,
		       Interpolation interpolation)

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

      cv::Mat M = cv::getAffineTransform(src, dst);

      const int borderModeCV = getBorderModeCV(borderMode);

      cv::Mat img2;
      cv::warpAffine(img, img2, M, img.size(), borderModeCV);

      return transform(img2, alpha, sigma, borderMode, interpolation);
    }
    
  } //namespace ElasticDeformation
} //namespace dc



