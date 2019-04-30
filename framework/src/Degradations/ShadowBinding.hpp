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
      @param[in] distance   size in pixels of degradation.
      @param[in] intensity intinsity in [0; 1]
      @param[in] angle angle in degrees in [0; 90]
    */
    extern FRAMEWORK_EXPORT void shadowBinding(cv::Mat &matOut, Border border,
					       int distance, float intensity, float angle);

    /*
      @param[in] distanceRatio  ratio of width or height used to compute size in pixels of degradation, in [0; 1].
      @param[in] intensity intinsity in [0; 1]
      @param[in] angle angle in degrees in [0; 90]
    */
    extern FRAMEWORK_EXPORT void shadowBinding(cv::Mat &matOut, float distanceRatio,
					       Border border, float intensity, float angle);


  } //namespace ShadowBinding 

} //namespace dc
  
#endif //SHADOWBINDING_HPP
