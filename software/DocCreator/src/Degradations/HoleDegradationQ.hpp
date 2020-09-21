#ifndef HOLEDEGRADATIONQ_HPP
#define HOLEDEGRADATIONQ_HPP

#include <QColor>
#include <QImage>
#include "Degradations/DocumentDegradation.hpp"
#include "Degradations/HoleDegradation.hpp"

namespace dc {
  namespace HoleDegradation {

    extern FRAMEWORK_EXPORT QImage holeDegradation(const QImage &imgOriginal, const QImage &holePattern, int xOrigin, int yOrigin, int size, HoleType type, int side, const QColor &color, const QImage &pageBelow=QImage(), int width=0, float intensity=1000);

    extern FRAMEWORK_EXPORT QImage holeDegradation(const QImage &imgOriginal, const QImage &holePattern, float ratioOutside, int size, HoleType type, int side, const QColor &color, const QImage &pageBelow=QImage(), int width=0, float intensity=1000);
    

  } //namespace HoleDegradation

  class FRAMEWORK_EXPORT HoleDegradationQ : public DocumentDegradation
  {
    Q_OBJECT

  public : 

    explicit HoleDegradationQ(const QImage &original, const QImage &holePattern,
			      int xOrigin, int yOrigin, int size,
			      dc::HoleDegradation::HoleType type, int side,
			      const QColor &color, const QImage &pageBelow=QImage(),
			      int width=0, float intensity=1000, QObject *parent =0) :
      DocumentDegradation(parent), _pattern(holePattern), _xOrigin(xOrigin), _yOrigin(yOrigin), _size(size), _type(type), _side(side), _width(width), _intensity(intensity), _color(color), _pageBelow(pageBelow), _original(original)
    {}

  public slots :

    QImage apply() override;

  signals:

    void imageReady(const QImage &);

  protected :
    QImage _pattern;
    const int _xOrigin;
    const int _yOrigin;
    const int _size;
    const dc::HoleDegradation::HoleType _type;
    const int _side;
    const int _width;
    const float _intensity;
    QColor _color;
    QImage _pageBelow;

    QImage _original;

  };
  
} //namespace dc
  

#endif /* !  HOLEDEGRADATIONQ_HPP */
