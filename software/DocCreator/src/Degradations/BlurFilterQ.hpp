#ifndef BLURFILTERQ_HPP
#define BLURFILTERQ_HPP

#include <QImage>
#include "Degradations/DocumentDegradation.hpp"
#include "Degradations/BlurFilter.hpp"

namespace dc {

  namespace BlurFilter {
  
    /*
      Apply blur to whole image
    */
    extern FRAMEWORK_EXPORT QImage blur(const QImage &originalImg,
					dc::BlurFilter::Method method,
					int intensity);

    /*
      Apply blur to specific image area
    */
    extern FRAMEWORK_EXPORT QImage blur(const QImage &originalImg, Method method, int intensity, Function function, Area area=Area::UP, float coeff=1, int vertical=0, int horizontal=0, int radius=0); //Vertical : Position vertical of the area, Horizontal : Position Horizontal of the area

    extern FRAMEWORK_EXPORT QImage makePattern(const QImage &originalImg, Function function, Area area, float coeff, int vertical, int horizontal, int radius = 10);
    extern FRAMEWORK_EXPORT QImage applyPattern(const QImage &originalImg, const QImage &pattern, Method method, int intensity);

    extern FRAMEWORK_EXPORT float getRadiusFourier(const QImage &img);

    extern FRAMEWORK_EXPORT int searchFitFourier(const QImage &img, float dstRadius);

  } //namespace BlurFilter
    

  class FRAMEWORK_EXPORT BlurFilterQ : public DocumentDegradation
  {
    Q_OBJECT

  public : 

    explicit BlurFilterQ(const QImage &original,
			 dc::BlurFilter::Method method,
			 int intensity,
			 dc::BlurFilter::Mode mode = dc::BlurFilter::Mode::COMPLETE,
			 dc::BlurFilter::Function function = dc::BlurFilter::Function::LINEAR,
			 dc::BlurFilter::Area area = dc::BlurFilter::Area::UP,
			 float coeff=1, int vertical=0, int horizontal=0, int radius=0,
			 const QImage &pattern = QImage(), QObject *parent =0) :
      DocumentDegradation(parent), _intensity(intensity), _method(method), _mode(mode), _function(function), _area(area), _coeff(coeff), _vertical(vertical), _horizontal(horizontal), _radius(radius), _original(original), _pattern(pattern)
    {}


  public slots :

    virtual QImage apply() override;

  signals:

    void imageReady(const QImage &);

  protected :
    const int _intensity;
    const dc::BlurFilter::Method _method;
    const dc::BlurFilter::Mode _mode;
    const dc::BlurFilter::Function _function;
    const dc::BlurFilter::Area _area;
    const float _coeff;
    const int _vertical;
    const int _horizontal;
    const int _radius;
    QImage _original;
    QImage _pattern;

  };

} //namespace dc

#endif /* ! BLURFILTERQ_HPP */
