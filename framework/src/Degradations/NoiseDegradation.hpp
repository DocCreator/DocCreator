#ifndef NOISEDEGRADATION_HPP
#define NOISEDEGRADATION_HPP

#include <cstdint>
#include <optional>

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  /** Noise: Gaussian, speckle, and salt and pepper. */
  namespace NoiseDegradation {


    /**
       Whether the noise added to a gray image may make it a color image.
    */
    enum class AddNoiseType {
      ADD_NOISE_AS_IS=0,        /**< the noise is drawn independently for each channel: a gray image may become a color image */
      ADD_NOISE_AS_GRAY,        /**< the same noise is added to every channel */
      ADD_NOISE_AS_GRAY_IF_GRAY /**< the same noise is added to every channel if the image is gray, independent noise otherwise */
    };

    
    /**
       Add Gaussian-distributed additive noise to an image.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
       std::invalid_argument exception is thrown otherwise. The alpha
       channel of a CV_8UC4 image is left unchanged.

       @param img  image to degrade.
       @param average  mean of the Gaussian distribution, in gray levels.
       @param standardDeviation  standard deviation of the Gaussian
       distribution, in gray levels.
       @param addType  whether the noise may make a gray image a color
       image.
       @param seed  seed of the random generator. If not provided, a
       non-reproducible seed is used. Two calls with the same seed and the
       same arguments give the same result.
       @return the noisy image, saturated to [0; 255], of the type and size
       of @a img.
     */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat addGaussianNoise(const cv::Mat &img,
						     float average=0.0f,
						     float standardDeviation=10.0f,
						     AddNoiseType addType=AddNoiseType::ADD_NOISE_AS_IS,
						     std::optional<uint32_t> seed = std::nullopt);

    /**
       Add multiplicative (speckle) noise to an image: each pixel becomes
       p + n * p, where n is drawn from a Gaussian distribution.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
       std::invalid_argument exception is thrown otherwise. The alpha
       channel of a CV_8UC4 image is left unchanged.

       @param img  image to degrade.
       @param average  mean of the Gaussian distribution.
       @param standardDeviation  standard deviation of the Gaussian
       distribution.
       @param addType  whether the noise may make a gray image a color
       image.
       @param seed  seed of the random generator. If not provided, a
       non-reproducible seed is used. Two calls with the same seed and the
       same arguments give the same result.
       @return the noisy image, saturated to [0; 255], of the type and size
       of @a img.
     */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat addSpeckleNoise(const cv::Mat &img,
						    float average=0.0f,
						    float standardDeviation=1.0f,
						    AddNoiseType addType=AddNoiseType::ADD_NOISE_AS_IS,
						    std::optional<uint32_t> seed = std::nullopt);


    /**
       Add salt and pepper noise to an image: random pixels become black
       (0) or white (255).

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4, and @a amount
       and @a ratio in [0; 1]. A std::invalid_argument exception is thrown
       otherwise. The alpha channel of a CV_8UC4 image is left unchanged.

       @param img  image to degrade.
       @param amount  proportion of the pixels changed, in [0; 1].
       @param ratio  proportion of the changed pixels that become black, in
       [0; 1]; the others become white.
       @param seed  seed of the random generator. If not provided, a
       non-reproducible seed is used. Two calls with the same seed and the
       same arguments give the same result.
       @return the noisy image, of the type and size of @a img.
     */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat addSaltAndPepperNoise(const cv::Mat &img,
							  float amount=0.15f,
							  float ratio=0.5f,
							  std::optional<uint32_t> seed = std::nullopt);


  } //namespace NoiseDegradation
} //namespace dc


#endif /* ! NOISEDEGRADATION_HPP */    
    
