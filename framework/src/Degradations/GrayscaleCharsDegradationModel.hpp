#ifndef GRAYSCALECHARSDEGRADATIONMODEL_HPP
#define GRAYSCALECHARSDEGRADATIONMODEL_HPP


#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <opencv2/core/core.hpp>


#include <framework_global.h>

namespace dc {

  /**
     Character degradation: the ink of the characters degraded around
     seed points chosen near their contours, as described in:

     Kieu, Van Cuong; Journet, Nicholas; Visani, Muriel; Mullot, Rémy;
     Domenger, Jean-Philippe. An Efficient Parametrization of Character
     Degradation Model for Semi-synthetic Image Generation. In: 2nd
     International Workshop on Historical Document Imaging and Processing,
     2013.
  */
  namespace GrayscaleCharsDegradation {

  /**
     Degrade the characters of an image.

     The model is a local noise model for gray images. It degrades the image
     in the neighborhoods of seed points chosen close to the contours of
     the characters: each seed point is the center of a noise region, whose
     pixel values are changed with a Gaussian random distribution. Where
     the Kanungo model simulates the artifacts of scanning, this one
     simulates the degradations due to the age of the document and to the
     printing or writing process: ink splotches, white specks, streaks.

     There are three kinds of degradation spots: independent spots,
     overlapping spots and disconnection spots. The ratio of disconnection
     spots is 1 - @a indepentSpotsRatio - @a overlappingSpotsRatio.

     The model works on gray levels: a color image is converted to gray,
     and comes back gray, in its type. The alpha channel of a 4 channel
     image is kept.

     @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4, the two ratios in
     [0; 1], and their sum at most 1. A std::invalid_argument exception is
     thrown otherwise.

     @param img  image to degrade.
     @param level  scale factor, typically in [0; 10]. The number of
     degradation spots is proportional to the number of connected
     components of the image and to @a level: the higher the level, the
     more spots. 0 gives none.
     @param indepentSpotsRatio  ratio of independent spots, in [0; 1].
     @param overlappingSpotsRatio  ratio of overlapping spots, in [0; 1].
     @param seed  seed of the random generator. If not provided, a
     non-reproducible seed is used. Two calls with the same seed and the
     same arguments give the same result.
     @return the degraded image, of the type and size of @a img.
  */
    extern FRAMEWORK_EXPORT cv::Mat degradation(const cv::Mat &img, int level = 1,
						float indepentSpotsRatio = 0.33f,
						float overlappingSpotsRatio = 0.33,
						std::optional<uint32_t> seed = std::nullopt);

    /**
       One image, prepared once, degraded as many times as wanted.

       Most of the work of degradation() does not depend on the level, the
       ratios or the seed: the conversion to gray, the binarization, the
       connected components and the distance of every candidate pixel to
       the nearest contour are a function of the image alone, and they are
       most of the time one degradation takes. A caller that wants N
       degradations of the same image -- which is what generating a corpus
       is -- pays it N times.

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

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
       std::invalid_argument exception is thrown otherwise.

       Not thread-safe: an instance holds the whole state of the
       preparation, and is to be used by one thread at a time. Several
       instances on several threads are fine, as is everything else in this
       library.
    */
    class FRAMEWORK_EXPORT PreparedImage
    {
    public:
      /**
	 Prepare @a img.

	 @param img  image to degrade, of type CV_8UC1, CV_8UC3 or CV_8UC4.
      */
      explicit PreparedImage(const cv::Mat &img);
      ~PreparedImage();

      PreparedImage(const PreparedImage &) = delete;
      PreparedImage &operator=(const PreparedImage &) = delete;

      /**
	 Degrade the prepared image.

	 The result and the exceptions are those of the free function
	 GrayscaleCharsDegradation::degradation() for the same arguments.

	 @param level  scale factor of the number of degradation spots.
	 @param indepentSpotsRatio  ratio of independent spots, in [0; 1].
	 @param overlappingSpotsRatio  ratio of overlapping spots, in [0; 1].
	 @param seed  seed of the random generator. If not provided, a
	 non-reproducible seed is used.
	 @return the degraded image, of the type and size of the prepared
	 image.
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
