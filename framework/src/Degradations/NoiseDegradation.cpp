#include "NoiseDegradation.hpp"

#include <cassert>
#include <vector>

#include <opencv2/imgproc/imgproc.hpp>

#include "ColorUtils.hpp" //dc::isGray()
#include "Random.hpp"
#include "Validate.hpp"

namespace dc {
  namespace NoiseDegradation {

    // We need to work with signed images (as noise can be
    // negative as well as positive). We use 16 bit signed
    // images as otherwise we would lose precision.

    static cv::Mat makeRandnNoise(RandomEngine &rng,
				  const cv::Mat &image,
				  double average, double standardDeviation,
				  AddNoiseType addType)
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
	rng.fill(noise_image, cv::RNG::NORMAL,
		 cv::Scalar::all(average), cv::Scalar::all(standardDeviation));
      }
      else {
	cv::Mat noise_image1(image.size(), CV_16SC1);
	rng.fill(noise_image1, cv::RNG::NORMAL,
		 cv::Scalar::all(average), cv::Scalar::all(standardDeviation));
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

    /*
      Copy the alpha channel of @a src onto @a dst, that must have the same
      size and type. Does nothing if the images have no alpha channel.

      The noise is meant to degrade the colors of an image, not to make
      parts of it transparent.
    */
    static void restoreAlpha(const cv::Mat &src, cv::Mat &dst)
    {
      assert(src.type() == dst.type());
      assert(src.size() == dst.size());

      if (src.channels() != 4) {
	return;
      }
      int from_to[] = {3, 3};
      cv::mixChannels(&src, 1, &dst, 1, from_to, 1);
    }

    cv::Mat addGaussianNoise(const cv::Mat &image,
			     float average, float standardDeviation,
			     AddNoiseType addType,
			     std::optional<uint32_t> seed)
    {
      require8UC134(image, "img", "dc::NoiseDegradation::addGaussianNoise");
  
      cv::Mat dst;

      const int type = CV_MAKETYPE(CV_16S, image.channels());
      RandomEngine rng = makeRandomEngine(seed);
      cv::Mat noise_image =
	makeRandnNoise(rng, image, (double)average, (double)standardDeviation, addType);
  
      cv::Mat temp_image;
      image.convertTo(temp_image, type);
      cv::add(temp_image, noise_image, temp_image);
  
      temp_image.convertTo(dst, image.type());
      restoreAlpha(image, dst);
  
      assert(dst.type() == image.type());
      assert(dst.size() == image.size());
  
      return dst;
    }


    cv::Mat addSpeckleNoise(const cv::Mat &image,
			    float average, float standardDeviation,
			    AddNoiseType addType,
			    std::optional<uint32_t> seed)
    {
      require8UC134(image, "img", "dc::NoiseDegradation::addSpeckleNoise");
  
      cv::Mat dst;

      const int type = CV_MAKETYPE(CV_16S, image.channels());
      RandomEngine rng = makeRandomEngine(seed);
      cv::Mat noise_image =
	makeRandnNoise(rng, image, average, standardDeviation, addType);
  
      cv::Mat temp_image, temp_image2;
      image.convertTo(temp_image, type);
      cv::multiply(temp_image, noise_image, temp_image2);
      cv::add(temp_image, temp_image2, temp_image);
      temp_image.convertTo(dst, image.type());
      restoreAlpha(image, dst);

      assert(dst.type() == image.type());
      assert(dst.size() == image.size());

      return dst;
    }

    
    cv::Mat addSaltAndPepperNoise(const cv::Mat &image,
				  float amount, float ratio,
				  std::optional<uint32_t> seed)
    {
      require8UC134(image, "img", "dc::NoiseDegradation::addSaltAndPepperNoise");

      cv::Mat dst;

      RandomEngine rng = makeRandomEngine(seed);
      cv::Mat noise(image.rows, image.cols, CV_8UC1); //image.type());
      rng.fill(noise, cv::RNG::UNIFORM, 0, 255);

      const int blackThreshold = static_cast<int>(255*amount*ratio + 0.5f);
      const int whiteThreshold = static_cast<int>(255 - 255*amount*(1.f-ratio) + 0.5f);

      cv::Mat black = noise < blackThreshold;
      cv::Mat white = noise > whiteThreshold;

      dst = image.clone();
      if (dst.channels() == 4) {
	//B: setTo() with a mask writes every channel: the alpha channel of
	// the selected pixels would be set to 0 or 255 too.
	std::vector<cv::Mat> channels;
	cv::split(dst, channels);
	for (int c = 0; c < 3; ++c) {
	  channels[c].setTo(255, white);
	  channels[c].setTo(0, black);
	}
	cv::merge(channels, dst);
      }
      else {
	dst.setTo(255, white);
	dst.setTo(0, black);
      }

      assert(dst.type() == image.type());
      assert(dst.size() == image.size());

      return dst;
    }


  }//namespace NoiseDegradation
}//namespace dc
  
