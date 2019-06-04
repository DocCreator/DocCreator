#ifndef PHANTOMCHARACTER_HPP
#define PHANTOMCHARACTER_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {
  namespace PhantomCharacter {

    enum class Frequency {RARE=0, FREQUENT, VERY_FREQUENT}; 

    /*
      RARE is 15% probability of occurrence of phantom character.
      FREQUENT is 40%
      VERY_FREQUENT is 70%

      @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat phantomCharacter(const cv::Mat &imgOriginal,
						     Frequency frequency,
						     const std::string &phantomPatternsPath);


  } //namespace PhantomCharacter
} //namespace dc
  
#endif //PHANTOMCHARACTER_HPP
