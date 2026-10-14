#include "NoiseDegradation.hpp"

#include <algorithm> //std::min, std::max
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

    /*
      Build a noise image of the size of @a image, with @a depth as depth.
    */
    static cv::Mat makeRandnNoise(RandomEngine &rng,
				  const cv::Mat &image,
				  int depth,
				  double average, double standardDeviation,
				  AddNoiseType addType)
    {
      const int type = CV_MAKETYPE(depth, image.channels());
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
	cv::Mat noise_image1(image.size(), CV_MAKETYPE(depth, 1));
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
	makeRandnNoise(rng, image, CV_16S, (double)average, (double)standardDeviation, addType);
  
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

      //B: the noise is a multiplicative factor, thus a real number: in
      // CV_16S, a standard deviation below 0.5 is rounded to a noise of 0
      // everywhere, and image*noise saturates at 32767 as soon as the
      // standard deviation is large.
      const int type = CV_MAKETYPE(CV_32F, image.channels());
      RandomEngine rng = makeRandomEngine(seed);
      cv::Mat noise_image =
	makeRandnNoise(rng, image, CV_32F, average, standardDeviation, addType);
  
      /*
	B: this was

	  image.convertTo(temp, type);
	  cv::multiply(temp, noise_image, temp2);
	  cv::add(temp, temp2, temp);
	  temp.convertTo(dst, image.type());

	which holds three full size CV_32F images at once: 1.1 GB for a
	6564x4721 page, where the page itself is 93 MB. Two things follow
	from what it computes.

	temp + temp*noise is temp*(1+noise), which needs one image fewer;
	the one is added to the noise in place. [The two do not give exactly
	the same float, so about two pixels in a million come out one grey
	level apart.]

	And no pixel of the result depends on any pixel but its own, so the
	conversion and the product are done a band at a time, into a buffer
	the size of a band instead of the size of the image.
      */
      //B: cv::Scalar::all(), and not 1.f: a float becomes Scalar(1, 0, 0,
      // 0), which would add the one to the first channel only.
      noise_image += cv::Scalar::all(1.0);

      dst.create(image.size(), image.type());

      //enough rows for about a megapixel, so the buffer stays small
      //whatever the image is, and wide images still get whole rows
      constexpr int BAND_PIXELS = 1 << 20;
      const int bandRows =
	std::max(1, std::min(image.rows, BAND_PIXELS / std::max(1, image.cols)));

      cv::Mat band;
      for (int y = 0; y < image.rows; y += bandRows) {
	const cv::Rect r(0, y, image.cols, std::min(bandRows, image.rows - y));
	image(r).convertTo(band, type);
	cv::multiply(band, noise_image(r), band);
	cv::Mat dstBand = dst(r);
	band.convertTo(dstBand, image.type()); //saturates to [0; 255]
      }

      restoreAlpha(image, dst);

      assert(dst.type() == image.type());
      assert(dst.size() == image.size());

      return dst;
    }

    
    cv::Mat addSaltAndPepperNoise(const cv::Mat &image,
				  float amount, float ratio,
				  std::optional<uint32_t> seed)
    {
      static const char *func = "dc::NoiseDegradation::addSaltAndPepperNoise";
      require8UC134(image, "img", func);
      //B: both are documented in [0; 1] and were used as they came: an
      // amount of 5 made every pixel black or white, a negative one made
      // thresholds that select nothing or everything.
      requireInRange(amount, 0.0, 1.0, "amount", func);
      requireInRange(ratio, 0.0, 1.0, "ratio", func);

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
  
