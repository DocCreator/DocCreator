#ifndef PHANTOMCHARACTER_HPP
#define PHANTOMCHARACTER_HPP

#include <cstdint>
#include <optional>

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  /**
     Phantom characters: faint marks next to the characters, as left by the
     ink of another page or of a previous impression.
  */
  namespace PhantomCharacter {

    /**
      Add phantom characters to an image: marks, taken from the pattern
      images of @a phantomPatternsPath, pasted next to characters, in the
      gaps between the characters of a word.

      With an @a occurenceProbability of 0.15, phantom characters appear
      rarely; with 0.40, rather frequently; with 0.70, very frequently.

      If @a phantomPatternsPath holds no image file (cf.
      dc::listImageFiles()), a copy of @a img is returned.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4, and
      @a occurenceProbability in [0; 1]. A std::invalid_argument exception
      is thrown otherwise.

      @param img  image to degrade.
      @param occurenceProbability  probability that a phantom character is
      added at a place where it can be, in [0; 1].
      @param phantomPatternsPath  directory of the phantom patterns, which
      are loaded as gray images.
      @param seed  seed of the random generator. If not provided, a
      non-reproducible seed is used. Two calls with the same seed and the
      same arguments give the same result.
      @return the degraded image, of the type and size of @a img.
    */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat phantomCharacter(const cv::Mat &img,
						     float occurenceProbability,
						     const std::string &phantomPatternsPath,
						     std::optional<uint32_t> seed = std::nullopt);


  } //namespace PhantomCharacter
} //namespace dc
  
#endif //PHANTOMCHARACTER_HPP
