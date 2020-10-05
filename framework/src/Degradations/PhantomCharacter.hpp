#ifndef PHANTOMCHARACTER_HPP
#define PHANTOMCHARACTER_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {
  namespace PhantomCharacter {

    /*
      Insert "phantom characters" on border of characters.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. Output image will be of the same type.

      With an @a occurenceProbability of 0.15, phantom characters will appear rarely.
      With an @a occurenceProbability of 0.40, phantom characters will appear rather frequently.
      With an @a occurenceProbability of 0.70, phantom characters will appear very frequently.


      @param img input image to degrade.
      @param occurenceProbability probability of occurence of phantom characters. Must be in [0; 1].
      @param phantomPatternsPath directory to load phantom patterns from. Phantom patterns are loaded as grayscale images.

      @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat phantomCharacter(const cv::Mat &img,
						     float occurenceProbability,
						     const std::string &phantomPatternsPath);


  } //namespace PhantomCharacter
} //namespace dc
  
#endif //PHANTOMCHARACTER_HPP
