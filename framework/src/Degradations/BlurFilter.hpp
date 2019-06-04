#ifndef BLURFILTER_HPP
#define BLURFILTER_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  namespace BlurFilter {
  
    static const int INTERVAL_FOURIER = 15;

    enum class Function {LINEAR=0, LOG, PARABOLA, SINUS, ELLIPSE, HYPERBOLA};
    enum class Mode {COMPLETE=0, AREA};
    enum class Method {GAUSSIAN=0, MEDIAN, NORMAL};
    enum class Area {UP=0, DOWN, CENTERED};

    /*
      Apply blur to whole image

      @param intensity is kernel size.

      @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat blur(const cv::Mat &originalImg, Method method, int intensity);

    /*
      Apply blur to specific image area

      @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat blur(const cv::Mat &originalImg, Method method, int intensity, Function function, Area area=Area::UP, float coeff=1.f, int vertical=0, int horizontal=0, int radius=0); 

    extern FRAMEWORK_EXPORT cv::Mat makePattern(const cv::Mat &originalMat, Function function, Area area, float coeff1, int vertical, int horizontal, int radius = 10);
    extern FRAMEWORK_EXPORT cv::Mat applyPattern(const cv::Mat &originalMat, const cv::Mat &patternMat, Method method, int intensity);

    /*
      Compute the Fourier transform of image to measure the blur
      Method taken from Emile Vinsonneau's thesis, section 3.4.2 (descriptor D5).
      Thesis is here : http://www.theses.fr/s131479

      If distance between two radiusFourier is inferior to FOURIER_INTERVAL,
      we consider that images look alike.
    */
    extern FRAMEWORK_EXPORT float getRadiusFourier(const cv::Mat &img);


    /*
      Search the size of filter that we have to apply to original image & img 
      to look like the dst image.
      @a dstRadius is computed with getRadiusFourier() on dst image.

      The obtained filter size can then be used as intensity parameter of blurFilter() function, with method=Method::GAUSSIAN.
    */
    extern FRAMEWORK_EXPORT int searchFitFourier(const cv::Mat &img, float dstRadius);

  } //namespace BlurFilter
    
} //namespace dc
  
#endif //BLURFILTER_HPP
