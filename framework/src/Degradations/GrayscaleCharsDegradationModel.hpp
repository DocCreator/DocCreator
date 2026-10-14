#ifndef GRAYSCALECHARSDEGRADATIONMODEL_HPP
#define GRAYSCALECHARSDEGRADATIONMODEL_HPP


#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <opencv2/core/core.hpp>


#include <framework_global.h>

namespace dc {

  namespace GrayscaleCharsDegradation {

  /**
     Implements ink degradation model detailled in [1]

     [1]
     An Efficient Parametrization of Character Degradation Model for Semi-synthetic Image Generation
     KIEU, Van Cuaong, JOURNET, Nicholas, VISANI, Muriel, MULLOT, Rémy, DOMENGER, Jean-Philippe
     In : 2nd International Workshop on Historical Document Imaging and Processing. 2013.


     We propose a local noise model for GRAYSCALE images. Its main principle is to locally degrade
     the image in the neighbourhoods of “seed-points” selected close to the character boundary.
     These points define the center of “noise regions”. The pixel values inside the noise region
     are modified by a Gaussian random distribution to make the final result more realistic.
     While Kanungo noise models scanning artifacts, our model simulates degradations due to the age
     of the document itself and printing/writing process such as ink splotches, white specks or
     streaks.

     There are 3 kinds of degradation spots: independent spots, overlapping spots and disconnection
     spots.
     Ratio of disconnection spots will be determined as (1.0 - @a indepentSpotsRatio - @a overlappingSpotsRatio).

     @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. An std::invalid_argument exception is thrown otherwise.
     Output image will be of the same type.

     @param level scale factor, in [0; 10]. The number of degradation spots will be determined according to the number of connected components in image and multiplied by this scale factor. The higher the level, the higher the number of degradation spots.
     @param indepentSpotsRatio ratio of independent spots, in [0.0; 1.0]
     @param overlappingSpotsRatio ratio of overlapping spots, in [0.0; 1.0]
      @param seed  seed of the random generator. If not provided, a non reproducible seed is used. Two calls with the same seed and the same parameters give the same result.

  */
    extern FRAMEWORK_EXPORT cv::Mat degradation(const cv::Mat &img, int level = 1,
						float indepentSpotsRatio = 0.33f,
						float overlappingSpotsRatio = 0.33,
						std::optional<uint32_t> seed = std::nullopt);

    /**
       One image, prepared once, degraded as many times as wanted.

       Most of the work of degradation() does not depend on the level, the
       ratios or the seed: the conversion to gray, the binarisation, the
       connected components and the distance of every candidate pixel to
       the nearest contour are a function of the image alone. On a
       4721x6564 page that is 503 ms of the 676 one degradation takes, and
       a caller that wants N degradations of the same image -- which is
       what generating a corpus is -- pays it N times.

       Preparing the image once and drawing from it N times costs that
       once:

       @code
       dc::GrayscaleCharsDegradation::PreparedImage prepared(img);
       for (int i = 0; i < n; ++i) {
         const cv::Mat out = prepared.degradation(level, .33f, .33f, seed + i);
       }
       @endcode

       Each call gives exactly what the free degradation() above would
       have given for the same arguments, seed included: the draws do not
       depend on each other, and the i-th does not depend on how many came
       before it.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. An
       std::invalid_argument exception is thrown otherwise.

       Not thread-safe: one instance is used by one thread at a time, and
       holds the whole state of the preparation. Several instances on
       several threads are fine, as everything else in this library.
    */
    class FRAMEWORK_EXPORT PreparedImage
    {
    public:
      explicit PreparedImage(const cv::Mat &img);
      ~PreparedImage();

      PreparedImage(const PreparedImage &) = delete;
      PreparedImage &operator=(const PreparedImage &) = delete;

      /**
	 @see degradation(), whose arguments and guarantees these are.
      */
      cv::Mat degradation(int level = 1,
			  float indepentSpotsRatio = 0.33f,
			  float overlappingSpotsRatio = 0.33f,
			  std::optional<uint32_t> seed = std::nullopt);

    private:
      class Impl;
      std::unique_ptr<Impl> m_impl;
    };

  } //namespace GrayscaleCharsDegradation

} //namespace dc


#endif /* ! GRAYSCALECHARSDEGRADATIONMODEL_HPP */
