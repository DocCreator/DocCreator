#ifndef COLORUTILS_HPP
#define COLORUTILS_HPP

#include <framework_global.h>

#include <opencv2/core/core.hpp>

namespace dc {

  /**
    Tell whether an image is gray: one channel, or blue, green and red equal
    in every pixel. The alpha channel of a 4 channel image is not looked at.

    It may traverse the whole image to find out.

    @param in  image to test, of type CV_8UC1, CV_8UC3 or CV_8UC4. Unlike
    the degradations, this is not checked.
    @return true if @a in is gray.
  */
  extern FRAMEWORK_EXPORT
  bool
  isGray(const cv::Mat &in);

} //namespace dc

#endif /* ! COLORUTILS_HPP */
