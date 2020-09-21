#ifndef COLORUTILS_HPP
#define COLORUTILS_HPP

#include <framework_global.h>

#include <opencv2/core/core.hpp>

namespace dc {

  /*
    Tells whether the provided image @a in is gray.

    It may traverse the whole image to find out.
  */
  extern FRAMEWORK_EXPORT
  bool
  isGray(const cv::Mat &in);

} //namespace dc

#endif /* ! COLORUTILS_HPP */
