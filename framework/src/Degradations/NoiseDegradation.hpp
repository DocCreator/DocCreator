#ifndef NOISEDEGRADATION_HPP
#define NOISEDEGRADATION_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {
  namespace NoiseDegradation {


    /**
       Tells how the output image will be after noise is added.
       In particular, gray images may become color image if noise is added independantly on the different channels.
    */
    enum class AddNoiseType {ADD_NOISE_AS_IS=0, /** Added noise is independant on all channels. In particular, it may change a grayscale image in a color image. */
			     ADD_NOISE_AS_GRAY, /** Added noise is the same on all channels */
			     ADD_NOISE_AS_GRAY_IF_GRAY /** Added noise is the same on all channels only if the destination image is gray */
    };

    
    /**
       Add Gaussian-distributed additive noise.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
       Output image will be of the same type and size than the input img.

       @param img input image.
       @param average average of the Gaussian distribution
       @param standardDeviation standard deviation of the Gaussian distribution
       @param addType  tells whether noise may or may not change image into a color image
       @return modified image.
     */
    extern FRAMEWORK_EXPORT cv::Mat addGaussianNoise(const cv::Mat &img,
						     float average=0.0f,
						     float standardDeviation=10.0f,
						     AddNoiseType addType=AddNoiseType::ADD_NOISE_AS_IS);

    /**
       Add multiplicative noise using 
       out = image + n*image,
       where n is uniform noise with specified mean & variance.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
       Output image will be of the same type and size than the input img.

       @param img input image.
       @param average average of the Gaussian distribution
       @param standardDeviation standard deviation of the Gaussian distribution
       @param addType  tells whether noise may or may not change image into a color image
       @return modified image.
     */
    extern FRAMEWORK_EXPORT cv::Mat addSpeckleNoise(const cv::Mat &img,
						    float average=0.0f,
						    float standardDeviation=1.0f,
						    AddNoiseType addType=AddNoiseType::ADD_NOISE_AS_IS);


    /**
       Add Salt and pepper noise.

       Replaces random pixels with 0 or 1.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
       Output image will be of the same type and size than the input img.

       @param img input image.
       @param amount amount of noise (in [0; 1]).
       @param ratio ratio of black spots (in [0; 1]). Ratio of white points will be 1-ratio.
       @return modified image.
     */
    extern FRAMEWORK_EXPORT cv::Mat addSaltAndPepperNoise(const cv::Mat &img,
							  float amount=0.15f,
							  float ratio=0.5f);


  } //namespace NoiseDegradation
} //namespace dc


#endif /* ! NOISEDEGRADATION_HPP */    
    
