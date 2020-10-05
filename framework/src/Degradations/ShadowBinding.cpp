#define _USE_MATH_DEFINES //for M_PI with Visual

#include "ShadowBinding.hpp"

#include <cassert>
#include <cmath> //cos, M_PI
//#include <iostream>//DEBUG

#include <opencv2/imgproc/imgproc.hpp>

namespace dc {
  namespace ShadowBinding {
  
    /**
       Darken rectangle @a rect of image @a img with intensity @a intensity.
  
       @param[in, out] img
   
    */
    static void
    darkenImagePart(cv::Mat &img, const cv::Rect &rect, float intensity)
    {
      assert(img.type() == CV_8UC3 || img.type() == CV_8UC1 || img.type() == CV_8UC4);
      assert(rect.x + rect.width <= img.cols);
      assert(rect.y + rect.height <= img.rows);

      const float coeff = intensity;

      const int x0 = rect.x;
      const int x1 = rect.x + rect.width;
      const int y0 = rect.y;
      const int y1 = rect.y + rect.height;

      if (img.type() == CV_8UC3) {
	for (int y = y0; y < y1; ++y) {
	  cv::Vec3b *p = img.ptr<cv::Vec3b>(y) + x0;
	  for (int x = x0; x < x1; ++x) {
	    (*p)[0] = cv::saturate_cast<uchar>((*p)[0] * coeff);
	    (*p)[1] = cv::saturate_cast<uchar>((*p)[1] * coeff);
	    (*p)[2] = cv::saturate_cast<uchar>((*p)[2] * coeff);
	    ++p;
	  }
	}
      }
      else if (img.type() == CV_8UC4) {
	for (int y = y0; y < y1; ++y) {
	  cv::Vec4b *p = img.ptr<cv::Vec4b>(y) + x0;
	  for (int x = x0; x < x1; ++x) {
	    (*p)[0] = cv::saturate_cast<uchar>((*p)[0] * coeff);
	    (*p)[1] = cv::saturate_cast<uchar>((*p)[1] * coeff);
	    (*p)[2] = cv::saturate_cast<uchar>((*p)[2] * coeff);
	    //we do not change the alpha channel
	    ++p;
	  }
	}
      }
      else if (img.type() == CV_8UC1) {
	for (int y = y0; y < y1; ++y) {
	  uchar *p = img.ptr<uchar>(y) + x0;
	  for (int x = x0; x < x1; ++x) {
	    (*p) = cv::saturate_cast<uchar>((*p) * coeff);
	    ++p;
	  }
	}
      }

    }

    cv::Mat
    shadowBinding(const cv::Mat &img,
		  Border border,
		  int distance,
		  float intensity,
		  float angle)
    {
      cv::Mat matOut = img.clone();
      
      cv::Rect rec;

      const float theta = angle * static_cast<float>(M_PI) / 180.f;
      const float radius = static_cast<float>(distance);

      assert(fabs(intensity) <= 1.f);

      const float l0 = intensity * 100 + 1;

      for (int i = 0; i < distance; ++i) {
	constexpr float step = 1;
	switch (border) {
	case Border::LEFT:
	  rec = cv::Rect(i, 0, step, matOut.rows);
	  break;
	case Border::TOP:
	  rec = cv::Rect(0, i, matOut.cols, step);
	  break;
	case Border::RIGHT:
	  rec = cv::Rect(matOut.cols - 1 - i, 0, step, matOut.rows);
	  break;
	case Border::BOTTOM:
	default:
	  rec = cv::Rect(0, matOut.rows - 1 - i, matOut.cols, step);
	  break;
	}

	//transform i from [0; distance[ to [theta; 0[
	const float phi = theta * (1 - i / static_cast<float>(distance));
	const float c = (l0 / (l0 + radius * (1 - cosf(phi))));
	const float coeff = c * c;

	//std::cerr<<"distance="<<distance<<" intensity="<<intensity<<" angle="<<angle<<" i="<<i<<" => c="<<coeff<<"\n";

	darkenImagePart(matOut, rec, coeff);
      }

      return matOut;
    }

	//B: float casted to int ? Is it really what we want ?
    static inline constexpr int
    getDistance(Border border, float distanceRatio, int w, int h) noexcept
    {
      return (border == Border::LEFT || border == Border::RIGHT)
	? distanceRatio * w
	: distanceRatio * h;
    }

    /**
       @param[in, out] matOut.
    */
    cv::Mat
    shadowBinding2(const cv::Mat &img,
		   float distanceRatio,
		   Border border,
		   float intensity,
		   float angle)
    {
      const int distance =
	getDistance(border, distanceRatio, img.cols, img.rows);

      return shadowBinding(img, border, distance, intensity, angle);
    }


  } //namespace ShadowBinding
} //namespace dc
  
