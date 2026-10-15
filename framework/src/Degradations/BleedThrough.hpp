#ifndef BLEEDTHROUGH_HPP
#define BLEEDTHROUGH_HPP


#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  namespace BleedThrough {
  
    /*
      Apply a bleedthrough effect between @a frontImg and @a backImg, with @a numIter iterations.
      @a backImg is positionned at (@a x, @a y) in @a frontImg frame.
      
      @warning @a backImg is not transformed.
      In particular, it is considered already mirrored.
      
      @a frontImg and @a backImg must have the same type.
      Accepted type are CV_8UC1, CV_8UC3, CV_8UC4.
      An std::invalid_argument exception is thrown otherwise.
      Output image is of the same type than @a frontImg.
      
      Bleedthrough effect is applied only on the overlapping part 
      between @a frontImg and @a backImg.
      
      @param frontImg  front image.
      @param backImg  back image.
      @param numIter   number of iterations. The higher the number of iterations, the more the back image will bleed through the front image.
      @param pos  position of @a backImg image origine in @a frontImg frame. Coordinates may be negative.
      @param numThreads  maximum number of threads used to compute the effect. If 0 or negative, as many threads as OpenCV uses (cf cv::setNumThreads()). The result does not depend on it.
      @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat bleedThrough(const cv::Mat &frontImg, const cv::Mat &backImg, int numIter, cv::Point pos = cv::Point(0, 0), int numThreads=-1);
    
    /*
      Function provided for convenience.
      
      Apply bleedthrough effect between @a frontImg and @a backImg, with @a numIter iterations, but considering @a originalFrontImg as the original recto image. It allows to apply the bleedthrough incrementally.
      
      @a originalFrontImg, @a frontImg and @a backImg must have the same type.
      Accepted type are CV_8UC1, CV_8UC3, CV_8UC4.
      An std::invalid_argument exception is thrown otherwise.
      Output image is of the same type than @a frontImg.
      @a originalFrontImg and @a frontImg must have the same size.
      
      Bleedthrough effect is applied only on the overlapping part between 
      @a frontImg and @a backImg.
      
      
      @param originalFrontImg  original recto image.
      @param frontImg  current recto image.
      @param backImg  verso image.
      @param numIter   number of iterations. The higher the number of iterations, the more the verso image will bleed through the recto image.
      @param pos  position of @a backImg image origine in @a frontImg frame. Coordinates may be negative.
      @param numThreads  maximum number of threads used to compute the effect. If 0 or negative, as many threads as OpenCV uses (cf cv::setNumThreads()). The result does not depend on it.
      @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat bleedThroughInc(const cv::Mat &originalFrontImg, const cv::Mat &frontImg, const cv::Mat &backImg, int numIter, cv::Point pos = cv::Point(0, 0), int numThreads=-1);

  } //namespace BleedThrough

} //namespace dc
  
#endif // BLEEDTHROUGH_HPP
