#ifndef BLURFILTER_HPP
#define BLURFILTER_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  /**
     Blur: of a whole image, or of a part of it delimited by a curve or by a
     pattern image; and the measure of the blur of an image, to imitate it.
  */
  namespace BlurFilter {
  
    /**
       Tolerance on the Fourier radius (cf. getRadiusFourier()): two images
       whose radii are closer than this are considered equally blurred.
    */
    static const int INTERVAL_FOURIER = 15;

    /**
       The curve that delimits the blurred area, cf. blurArea().

       The curve is drawn with its origin at the bottom left of the image:
       at column x, the function curves are at row rows - f(x), where
       f(x) is:
    */
    enum class Function {
      LINEAR=0,  /**< @a coeff * x + @a vertical */
      LOG,       /**< @a coeff * log(x) + @a vertical */
      PARABOLA,  /**< @a coeff * (x + @a horizontal)^2 + 4 * (x + @a horizontal) + @a vertical */
      SINUS,     /**< @a coeff * sin((x + @a horizontal) / 200) + @a vertical */
      ELLIPSE,   /**< a fixed ellipse, enlarged with @a coeff and moved by @a horizontal and @a vertical */
      HYPERBOLA  /**< a fixed hyperbola, changed with @a coeff and moved by @a horizontal and @a vertical */
    };

    /** Whether the whole image is blurred, or a part of it. Used by the application, not by the functions below. */
    enum class Mode {COMPLETE=0, AREA};

    /** The blur filter. */
    enum class Method {
      GAUSSIAN=0, /**< Gaussian blur, cf. cv::GaussianBlur() */
      MEDIAN,     /**< median filter, cf. cv::medianBlur() */
      NORMAL      /**< box filter, the mean of the kernel, cf. cv::blur() */
    };

    /** Which part of the image, relative to the curve, is blurred. */
    enum class Area {
      UP=0,    /**< the pixels whose row is greater than the curve's: below the curve on the image. For an ellipse or a hyperbola, the pixels between its two branches. */
      DOWN,    /**< the other pixels */
      CENTERED /**< the pixels within @a radius rows of the curve */
    };

    /**
      Blur a whole image.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
      std::invalid_argument exception is thrown otherwise.

      @param img  image to blur.
      @param method  blur filter.
      @param kernelSize  size of the filter kernel. It must be positive, and
      odd for Method::GAUSSIAN and Method::MEDIAN.
      @return the blurred image, of the type and size of @a img.
    */
    extern FRAMEWORK_EXPORT cv::Mat blur(const cv::Mat &img, Method method, int kernelSize);

    /**
      Blur the part of an image delimited by a curve.

      The curve is defined by @a function, @a coeff, @a vertical and
      @a horizontal (cf. Function), and @a area tells which part of the image
      is blurred (cf. Area).

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
      std::invalid_argument exception is thrown otherwise.

      @param img  image to blur.
      @param method  blur filter.
      @param kernelSize  size of the filter kernel. It must be positive, and
      odd for Method::GAUSSIAN and Method::MEDIAN.
      @param function  shape of the curve.
      @param area  part of the image blurred.
      @param coeff  coefficient of the curve.
      @param vertical  vertical offset of the curve, in pixels.
      @param horizontal  horizontal offset of the curve, in pixels. Not used
      by Function::LINEAR and Function::LOG.
      @param radius  half height, in pixels, of the band blurred around the
      curve with Area::CENTERED. Not used otherwise.
      @return the image blurred in the area, of the type and size of @a img.
      @see makePattern(), which gives the area as an image.
    */
    extern FRAMEWORK_EXPORT cv::Mat blurArea(const cv::Mat &img, Method method, int kernelSize, Function function, Area area=Area::UP, float coeff=1.f, int vertical=0, int horizontal=0, int radius=0);

    /**
      Make the pattern of the area blurArea() blurs: an image whose black
      pixels are blurred by applyPattern(), and whose white pixels are not.

      @a originalMat must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
      std::invalid_argument exception is thrown otherwise. Only its size is
      used.

      @param originalMat  image whose size the pattern takes.
      @param function, area, coeff1, vertical, horizontal, radius  the
      curve and the area, as for blurArea().
      @return a CV_8UC1 image of the size of @a originalMat: black (0) in
      the area, white (255) elsewhere.
     */
    extern FRAMEWORK_EXPORT cv::Mat makePattern(const cv::Mat &originalMat, Function function, Area area, float coeff1, int vertical, int horizontal, int radius = 10);

    /**
      Blur the pixels of an image that are black in a pattern.

      @a originalMat must be of type CV_8UC1, CV_8UC3 or CV_8UC4, and
      @a patternMat of type CV_8UC1. A std::invalid_argument exception is
      thrown otherwise.

      @param originalMat  image to blur.
      @param patternMat  pattern, resized to the size of @a originalMat:
      the image is blurred where it is black (0), and kept elsewhere.
      @param method  blur filter.
      @param kernelSize  size of the filter kernel. It must be positive, and
      odd for Method::GAUSSIAN and Method::MEDIAN.
      @return the image blurred where the pattern is black, of the type and
      size of @a originalMat.
      @see makePattern()
     */
    extern FRAMEWORK_EXPORT cv::Mat applyPattern(const cv::Mat &originalMat, const cv::Mat &patternMat, Method method, int kernelSize);

    /**
      Measure the blur of an image, from its Fourier transform.

      Method from Emile Vinsonneau's thesis, section 3.4.2 (descriptor D5):
      http://www.theses.fr/s131479

      Two images whose radii differ by less than INTERVAL_FOURIER are
      considered equally blurred.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
      std::invalid_argument exception is thrown otherwise.

      @param img  image to measure.
      @return the radius of the Fourier transform of @a img: the more
      blurred the image, the smaller the radius.
    */
    extern FRAMEWORK_EXPORT float getRadiusFourier(const cv::Mat &img);


    /**
      Find the kernel size of the Gaussian blur that makes @a img look as
      blurred as an image of Fourier radius @a dstRadius.

      @a dstRadius is typically getRadiusFourier() of that image. The
      kernel size found can then be given to blur() with Method::GAUSSIAN.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
      std::invalid_argument exception is thrown otherwise.

      @param img  image to blur.
      @param dstRadius  Fourier radius to reach.
      @return an odd kernel size, between 3 and 19.
    */
    extern FRAMEWORK_EXPORT int searchFitFourier(const cv::Mat &img, float dstRadius);

  } //namespace BlurFilter
    
} //namespace dc
  
#endif //BLURFILTER_HPP
