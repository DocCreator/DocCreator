#ifndef SHADOWBINDING_HPP
#define SHADOWBINDING_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  /**
     Shadow of the binding: the shadow along a border of a page scanned
     from a bound book, after the "non-linear illumination model" of:

     Kanungo, Tapas; Haralick, Robert M.; Phillips, Ihsin T. Global and
     Local Document Degradation Models. In: Proc. of the International
     Conference on Document Analysis and Recognition (ICDAR), 1993.
  */
  namespace ShadowBinding {

    /** The border of the image the shadow is on. */
    enum class Border {TOP=0, RIGHT, BOTTOM, LEFT};

    /**
      Add a shadow along a border of an image.

      @a angle is the angle between the page and the spine of the book when
      the page is scanned.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
      std::invalid_argument exception is thrown otherwise.

      @param img  image to degrade.
      @param border  border on which the shadow is added.
      @param distance  width of the shadow, in pixels.
      @param intensity  intensity, in [0; 1]. The higher, the darker the
      shadow.
      @param angle  angle in degrees, in [0; 90]. The higher, the darker the
      shadow.
      @return the image with the shadow, of the type and size of @a img.
    */
    extern FRAMEWORK_EXPORT cv::Mat shadowBinding(const cv::Mat &img,
						  Border border,
						  int distance,
						  float intensity,
						  float angle);

    /**
      Add a shadow along a border of an image, of a width relative to the
      image: @a distanceRatio times its width for the left and right
      borders, times its height for the top and bottom borders.

      @a angle is the angle between the page and the spine of the book when
      the page is scanned.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
      std::invalid_argument exception is thrown otherwise.

      @param img  image to degrade.
      @param distanceRatio  width of the shadow, relative to the width or
      the height of the image, in [0; 1].
      @param border  border on which the shadow is added.
      @param intensity  intensity, in [0; 1]. The higher, the darker the
      shadow.
      @param angle  angle in degrees, in [0; 90]. The higher, the darker the
      shadow.
      @return the image with the shadow, of the type and size of @a img.
    */
    extern FRAMEWORK_EXPORT cv::Mat shadowBinding2(const cv::Mat &img,
						   float distanceRatio,
						   Border border,
						   float intensity,
						   float angle);


  } //namespace ShadowBinding 

} //namespace dc
  
#endif //SHADOWBINDING_HPP
