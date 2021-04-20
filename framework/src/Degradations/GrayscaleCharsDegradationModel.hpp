#ifndef GRAYSCALECHARSDEGRADATIONMODEL_HPP
#define GRAYSCALECHARSDEGRADATIONMODEL_HPP


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

     @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
     Output image will be of the same type.

     @param level scale factor, in [0; 10]. The number of degradation spots will be determined according to the number of connected components in image and multiplied by this scale factor. The higher the level, the higher the number of degradation spots.
     @param indepentSpotsRatio ratio of independent spots, in [0.0; 1.0]
     @param overlappingSpotsRatio ratio of overlapping spots, in [0.0; 1.0]

  */
    extern FRAMEWORK_EXPORT cv::Mat degradation(const cv::Mat &img, int level = 1,
						float indepentSpotsRatio = 0.33f,
						float overlappingSpotsRatio = 0.33);

  } //namespace GrayscaleCharsDegradation

} //namespace dc


#endif /* ! GRAYSCALECHARSDEGRADATIONMODEL_HPP */
