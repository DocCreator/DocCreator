#ifndef PHANTOMCHARACTER_HPP
#define PHANTOMCHARACTER_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {
  namespace PhantomCharacter {

    /**
       Tells the probability of occurence of phantom characters.
     */
    enum class Frequency {RARE=0, /** Probability of occurence is 15% */
			  FREQUENT, /** Probability of occurence is 40% */
			  VERY_FREQUENT /** Probability of occurence is 70% */
    }; 

    /*
      Insert "phantom characters" on border of characters.

      @param imgOriginal input image to degrade.
      @param frequency frequency of occurence of phantom characters.
      @param phantomPatternsPath directory to load phantom patterns from.

      @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat phantomCharacter(const cv::Mat &imgOriginal,
						     Frequency frequency,
						     const std::string &phantomPatternsPath);


  } //namespace PhantomCharacter
} //namespace dc
  
#endif //PHANTOMCHARACTER_HPP
