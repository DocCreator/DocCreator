#ifndef BLEEDTHROUGH_HPP
#define BLEEDTHROUGH_HPP


#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  namespace BleedThrough {
  
    /*
      Apply bleedthrough effect between @a imgRecto and @a imgVerso, with @a nbIter iterations. 
      @a imgVerso is positionned at (@a x, @a y) in @a imgRecto frame.
      
      @warning @a imgVerso is not transformed. In particular, it is considered already mirrored.
      
      @a imgRecto and @a imgVerso must have the same type. Accepted type are CV_8UC3, CV_8UC3, CV_8UC4. Output image is of the same type than @a imgRecto.
      
      Bleedthrough effect is applied only on the overlapping part betwen @a imgRecto and @a imgVerso.
      
      @param imgRecto  recto image.
      @param imgVerso  verso image.
      @param nbIter   number of iterations. The higher the number of iterations, the more the verso image will bleed through the recto image.
      @param x  x coordinate of @a imgVerso image origine in @a imgRecto frame. It may be negative.
      @param y  y coordinate of @a imgVerso image origine in @a imgRecto frame. It may be negative.
      @param nbThreads  number of threads. If @a nbThreads is negative a value is determined according to image size and number of available CPU cores.
      @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat bleedThrough(const cv::Mat &imgRecto, const cv::Mat &imgVerso, int nbIter, int x=0, int y=0, int nbThreads=-1);
    
    /*
      Function provided for convenience.
      
      Apply bleedthrough effect between @a imgRecto and @a imgVerso, with @a nbIter iterations, but considering @a originalRecto as the original recto image. It allows to apply the bleedthrough incrementally.
      
      @a originalRecto, @a imgRecto and @a imgVerso must have the same type. Accepted type are CV_8UC3, CV_8UC3, CV_8UC4. Output image is of the same type than @a imgRecto.
      @a originalRecto and @a imgRecto must have the same size.
      
      Bleedthrough effect is applied only on the overlapping part betwen @a imgRecto and @a imgVerso.
      
      
      @param originalRecto  original recto image.
      @param imgRecto  current recto image.
      @param imgVerso  verso image.
      @param nbIter   number of iterations. The higher the number of iterations, the more the verso image will bleed through the recto image.
      @param x  x coordinate of @a imgVerso image origine in @a imgRecto frame. It may be negative.
      @param y  y coordinate of @a imgVerso image origine in @a imgRecto frame. It may be negative.
      @param nbThreads  number of threads. If @a nbThreads is negative a value is determined according to image size and number of available CPU cores.
      @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat bleedThrough(const cv::Mat &originalRecto, const cv::Mat &imgRecto, const cv::Mat &imgVerso, int nbIter, int x=0, int y=0, int nbThreads=-1);

  } //namespace BleedThrough

} //namespace dc
  
#endif // BLEEDTHROUGH_HPP
