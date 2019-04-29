#ifndef BLURFILTER_HPP
#define BLURFILTER_HPP

#include "DocumentDegradation.hpp"
#include <framework_global.h>
#include <QImage>
#include <opencv2/core/core.hpp>

namespace dc {

static const int INTERVAL_FOURIER = 15;

enum class Function {LINEAR=0, LOG, PARABOLA, SINUS, ELLIPSE, HYPERBOLA};
enum class Mode {COMPLETE=0, AREA};
enum class Method {GAUSSIAN=0, MEDIAN, NORMAL};
enum class Area {UP=0, DOWN, CENTERED};

/*
  Apply blur to whole image
 */
extern FRAMEWORK_EXPORT QImage blurFilter(const QImage &originalImg, Method method, int intensity);

/*
  Apply blur to specific image area
*/
extern FRAMEWORK_EXPORT QImage blurFilter(const QImage &originalImg, Method method, int intensity, Function function, Area area=Area::UP, float coeff=1, int vertical=0, int horizontal=0, int radius=0); //Vertical : Position vertical of the area, Horizontal : Position Horizontal of the area

extern FRAMEWORK_EXPORT QImage makePattern(const QImage &originalImg, Function function, Area area, float coeff, int vertical, int horizontal, int radius = 10);
extern FRAMEWORK_EXPORT QImage applyPattern(const QImage &originalImg, const QImage &pattern, Method method, int intensity);

/*
  Apply blur to whole image
*/
extern FRAMEWORK_EXPORT cv::Mat blurFilter(const cv::Mat &originalImg, Method method, int intensity);

/*
  Apply blur to specific image area
*/
extern FRAMEWORK_EXPORT cv::Mat blurFilter(const cv::Mat &originalImg, Method method, int intensity, Function function, Area area=Area::UP, float coeff=1, int vertical=0, int horizontal=0, int radius=0); 

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

extern FRAMEWORK_EXPORT float getRadiusFourier(const QImage &img);

/*
  Search the size of filter that we have to apply to original image & img 
  to look like the dst image.
  @a dstRadius is computed with getRadiusFourier() on dst image.

  The obtained filter size can then be used as intensity parameter of blurFilter() function, with method=Method::GAUSSIAN.
*/
extern FRAMEWORK_EXPORT int searchFitFourier(const cv::Mat &img, float dstRadius);

extern FRAMEWORK_EXPORT int searchFitFourier(const QImage &img, float dstRadius);



class FRAMEWORK_EXPORT BlurFilter : public DocumentDegradation
{
  Q_OBJECT

public : 

  explicit BlurFilter(const QImage &original, Method method, int intensity, Mode mode=Mode::COMPLETE, Function function=Function::LINEAR, Area area=Area::UP, float coeff=1, int vertical=0, int horizontal=0, int radius=0, const QImage &pattern = QImage(), QObject *parent =0) :
    DocumentDegradation(parent), _intensity(intensity), _method(method), _mode(mode), _function(function), _area(area), _coeff(coeff), _vertical(vertical), _horizontal(horizontal), _radius(radius), _original(original), _pattern(pattern)
{}

  static void calcSolutions(float a, float b, float discr, float &y1, float &y2);
  static void calculFunction(Function fct, int rows, int x, int y, int vertical, int horizontal, float coeff, int &yFunction, int &y2Function);
  static bool upperThan(Function fct, int rows, int x, int y, int vertical, int horizontal, float coeff);
  static bool isNearFunction(int x, int y, int rows, Function fct, int horizontal, int vertical, float coeff, int radius);

public slots :

  virtual QImage apply() override;

signals:

  void imageReady(const QImage &);

protected :
  const int _intensity;
  const Method _method;
  const Mode _mode;
  const Function _function;
  const Area _area;
  const float _coeff;
  const int _vertical;
  const int _horizontal;
  const int _radius;
  QImage _original;
  QImage _pattern;

};

} //namespace dc
  
#endif //BLURFILTER_HPP
