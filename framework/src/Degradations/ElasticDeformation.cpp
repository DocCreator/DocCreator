#include "ElasticDeformation.hpp"

#include <cassert>
#include <random>

namespace dc {
  namespace ElasticDeformation {

    cv::Mat transform(const cv::Mat &img, float alpha, float sigma,
		      int borderMode, int interpolation)
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
      cv::Mat img2;
      cv::remap(img, img2, map_x, map_y, interpolation, borderMode, cv::Scalar(0, 0, 0));

      return img2;
    }


    cv::Mat transform2(const cv::Mat &img, float alpha, float sigma, float alpha_affine,
		       int borderMode, int interpolation)
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

      cv::Mat img2;
      cv::warpAffine(img, img2, M, img.size(), borderMode);

      return transform(img2, alpha, sigma, borderMode, interpolation);
    }
    
  } //namespace ElasticDeformation
} //namespace dc



