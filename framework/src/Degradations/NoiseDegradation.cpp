#include "NoiseDegradation.hpp"

#include <cassert>
#include <opencv2/imgproc/imgproc.hpp>

#include "ColorUtils.hpp" //dc::isGray()

namespace dc {
  namespace NoiseDegradation {

    // We need to work with signed images (as noise can be
    // negative as well as positive). We use 16 bit signed
    // images as otherwise we would lose precision.
  
    cv::Mat makeRandnNoise(const cv::Mat &image,
			   double average, double standardDeviation,
			   AddNoiseType addType=AddNoiseType::ADD_NOISE_AS_IS)
    {
      const int type = CV_MAKETYPE(CV_16S, image.channels());
      cv::Mat noise_image(image.size(), type);

      bool isImgGray = false;
      if (addType == AddNoiseType::ADD_NOISE_AS_GRAY_IF_GRAY) {
	isImgGray = dc::isGray(image);
      }
      const bool sameNoiseOnAllChannels = ((addType == AddNoiseType::ADD_NOISE_AS_GRAY
					    || (addType == AddNoiseType::ADD_NOISE_AS_GRAY_IF_GRAY && isImgGray))
					   && (image.type() != CV_8UC1));

      if (! sameNoiseOnAllChannels) {
	cv::randn(noise_image, cv::Scalar::all(average), cv::Scalar::all(standardDeviation));
      }
      else {
	cv::Mat noise_image1(image.size(), CV_16SC1);
	cv::randn(noise_image1, cv::Scalar::all(average), cv::Scalar::all(standardDeviation));
	if (image.channels() == 3) {
	  int from_to[] = {0,0,
			   0,1,
			   0,2};
	  cv::mixChannels(&noise_image1, 1, &noise_image, 1, from_to, 3);
	}
	else {
	  int from_to[] = {0,0,
			   0,1,
			   0,2,
			   0,3};
	  cv::mixChannels(&noise_image1, 1, &noise_image, 1, from_to, 4);
	}
      }
      return noise_image;
    }

    cv::Mat addGaussianNoise(const cv::Mat &image,
			     float average, float standardDeviation,
			     AddNoiseType addType)
    {
      assert(image.depth() == CV_8U);
  
      cv::Mat dst;

      const int type = CV_MAKETYPE(CV_16S, image.channels());
      cv::Mat noise_image = makeRandnNoise(image, (double)average, (double)standardDeviation, addType);
  
      cv::Mat temp_image;
      image.convertTo(temp_image, type);
      cv::add(temp_image, noise_image, temp_image);
  
      temp_image.convertTo(dst, image.type());
  
      assert(dst.type() == image.type());
      assert(dst.size() == image.size());
  
      return dst;
    }


    cv::Mat addSpeckleNoise(const cv::Mat &image,
			    float average, float standardDeviation,
			    AddNoiseType addType)
    {
      assert(image.depth() == CV_8U);
  
      cv::Mat dst;

      const int type = CV_MAKETYPE(CV_16S, image.channels());
      cv::Mat noise_image = makeRandnNoise(image, average, standardDeviation, addType);
  
      cv::Mat temp_image, temp_image2;
      image.convertTo(temp_image, type);
      cv::multiply(temp_image, noise_image, temp_image2);
      cv::add(temp_image, temp_image2, temp_image);
      temp_image.convertTo(dst, image.type());

      assert(dst.type() == image.type());
      assert(dst.size() == image.size());

      return dst;
    }

    
    cv::Mat addSaltAndPepperNoise(const cv::Mat &image,
				  float amount, float ratio)
    {
      assert(image.depth() == CV_8U);

      cv::Mat dst;

      cv::Mat noise(image.rows, image.cols, CV_8UC1); //image.type());
      cv::randu(noise, 0, 255);

      const int blackThreshold = static_cast<int>(255*amount*ratio + 0.5f);
      const int whiteThreshold = static_cast<int>(255 - 255*amount*(1.f-ratio) + 0.5f);

      cv::Mat black = noise < blackThreshold;
      cv::Mat white = noise > whiteThreshold;

      dst = image.clone();
      dst.setTo(255, white);
      dst.setTo(0, black);

      assert(dst.type() == image.type());
      assert(dst.size() == image.size());

      return dst;
    }


  }//namespace NoiseDegradation
}//namespace dc
  
