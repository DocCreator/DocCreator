#ifndef BLEEDTHROUGHQ_HPP
#define BLEEDTHROUGHQ_HPP


#include <QImage>
#include "Degradations/DocumentDegradation.hpp"
#include "Degradations/BleedThrough.hpp"

namespace dc {

  namespace BleedThrough {

    /*
      Apply bleedthrough effect between @a imgRecto and @a imgVerso, with @a nbIters iterations. 
      
      @a imgVerso is positionned at (@a x, @a y) in @a imgRecto frame.
      
      @warning @a imgVerso is not transformed. In particular, it is considered already mirrored.
      
      If @a numThreads is negative a value is determined according to image size.
      
      @return modified image.
    */
    extern FRAMEWORK_EXPORT QImage bleedThrough(const QImage &imgRecto, const QImage &imgVerso, int nbIters, QPoint pos = QPoint(0, 0), int numThreads=-1);

    /*
      Function provided for convenience.
      
      Apply bleedthrough effect between @a imgRecto and @a imgVerso, with @a nbIters iterations, but considering @a originalRecto as the original recto image. It allows to apply the bleedthrough incrementally.
      
      @return modified image.
    */
    extern FRAMEWORK_EXPORT QImage bleedThroughInc(const QImage &originalRecto, const QImage &imgRecto, const QImage &imgVerso, int nbIters, QPoint pos = QPoint(0, 0), int numThreads=-1);

  } //namespace BleedThrough
    

  class FRAMEWORK_EXPORT BleedThroughQ : public DocumentDegradation
  {
    Q_OBJECT

  public:

    explicit BleedThroughQ(int nbIterations, QObject *parent = 0) :
      DocumentDegradation(parent),
      _nbIter(nbIterations)
      {}

    void setVerso(const QString &path);

  public slots :

    QImage apply() override;
    virtual QImage apply(const QString &rectoPath);
    void setVersoAndApply(const QString &versoPath);

  signals:

    void imageReady(const QImage &);

  protected:
    QImage apply(QImage rectoImg);
    
  protected :
    QImage _verso;
    int _nbIter;
  };

} //namespace dc
  
#endif /* ! BLEEDTHROUGHQ_HPP */
