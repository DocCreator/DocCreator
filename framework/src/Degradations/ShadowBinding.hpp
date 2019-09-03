#ifndef SHADOWBINDING_HPP
#define SHADOWBINDING_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  namespace ShadowBinding {
  
    /**
       Implements "non-linear illumination model" from :
       "Global and Local Document Degradation Models"
       Tapas Kanungo, Robert M. Haralick and Thsin Phillips
       ICDAR 1993
    */

    enum class Border {TOP=0, RIGHT, BOTTOM, LEFT};

    /*
      Add a shadow on a given border of image.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. Output image will be of the same type.

      @prama[in] img  input original image.
      @param[in] border  border on which shadow is added.
      @param[in] distance   size in pixels of degradation.
      @param[in] intensity intinsity in [0; 1]
      @param[in] angle angle in degrees in [0; 90]

      @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat shadowBinding(const cv::Mat &img, Border border,
						  int distance, float intensity, float angle);

    /*
      Add a shadow on a given border of image.

      Shadow width is computed as @a distanceRatio * image width if border is LEFT or RIGHT, @a distanceRatio * image height if border is TOP or BOTTOM. 

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. Output image will be of the same type.

      @prama[in] img  input original image.
      @param[in] border  border on which shadow is added.
      @param[in] distanceRatio  ratio of width or height used to compute size in pixels of degradation, in [0; 1].
      @param[in] intensity intinsity in [0; 1]
      @param[in] angle angle in degrees in [0; 90]

      @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat shadowBinding(const cv::Mat &img, float distanceRatio,
						  Border border, float intensity, float angle);


  } //namespace ShadowBinding 

} //namespace dc
  
#endif //SHADOWBINDING_HPP
