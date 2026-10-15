#ifndef BLEEDTHROUGH_HPP
#define BLEEDTHROUGH_HPP


#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  /**
     Bleed-through: the ink of the back of a page showing through its front,
     simulated by an iterative diffusion of the back image into the front
     one.
  */
  namespace BleedThrough {
  
    /**
      Apply a bleed-through effect of @a backImg onto @a frontImg, with
      @a numIter iterations.

      @a backImg is placed at @a pos in the frame of @a frontImg, and the
      effect is applied only where the two images overlap.

      @warning @a backImg is not transformed. In particular, it is
      considered already mirrored.

      @a frontImg and @a backImg must have the same type, CV_8UC1, CV_8UC3
      or CV_8UC4. A std::invalid_argument exception is thrown otherwise.

      @param frontImg  front image (recto).
      @param backImg  back image (verso).
      @param numIter  number of iterations. The higher the number of
      iterations, the more the back image bleeds through the front image.
      @param pos  position of the origin of @a backImg in the frame of
      @a frontImg. Coordinates may be negative.
      @param numThreads  maximum number of threads used to compute the
      effect. If 0 or negative, as many threads as OpenCV uses (cf.
      cv::setNumThreads()). The result does not depend on it.
      @return the degraded image, of the type and size of @a frontImg.
    */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat bleedThrough(const cv::Mat &frontImg, const cv::Mat &backImg, int numIter, cv::Point pos = cv::Point(0, 0), int numThreads=-1);
    
    /**
      Apply a bleed-through effect of @a backImg onto @a frontImg, with
      @a numIter iterations, @a originalFrontImg being the front image
      before any bleed-through.

      This makes the effect incremental: bleedThroughInc(o, f, b, n) applied
      to the result f of bleedThrough(o, b, m) gives the result of m + n
      iterations.

      @a originalFrontImg, @a frontImg and @a backImg must have the same
      type, CV_8UC1, CV_8UC3 or CV_8UC4, and @a originalFrontImg and
      @a frontImg the same size. A std::invalid_argument exception is thrown
      otherwise.

      The effect is applied only where @a frontImg and @a backImg overlap.

      @param originalFrontImg  original front image (recto).
      @param frontImg  current front image.
      @param backImg  back image (verso).
      @param numIter  number of iterations. The higher the number of
      iterations, the more the back image bleeds through the front image.
      @param pos  position of the origin of @a backImg in the frame of
      @a frontImg. Coordinates may be negative.
      @param numThreads  maximum number of threads used to compute the
      effect. If 0 or negative, as many threads as OpenCV uses (cf.
      cv::setNumThreads()). The result does not depend on it.
      @return the degraded image, of the type and size of @a frontImg.
    */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat bleedThroughInc(const cv::Mat &originalFrontImg, const cv::Mat &frontImg, const cv::Mat &backImg, int numIter, cv::Point pos = cv::Point(0, 0), int numThreads=-1);

  } //namespace BleedThrough

} //namespace dc
  
#endif // BLEEDTHROUGH_HPP
