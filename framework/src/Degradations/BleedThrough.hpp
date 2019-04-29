#ifndef BLEEDTHROUGH_HPP
#define BLEEDTHROUGH_HPP


#include "DocumentDegradation.hpp"
#include <framework_global.h>

namespace dc {

/*
  Apply bleedthrough effect between @a imgRecto and @a imgVerso, with @a nbIter iterations. 

  @a imgVerso is positionned at (@a x, @a y) in @a imgRecto frame.

  @warning @a imgVerso is not transformed. In particular, it is considered already mirrored.

  If @a nbThreads is negative a value is determined according to image size.
 */
extern FRAMEWORK_EXPORT QImage bleedThrough(const QImage &imgRecto, const QImage &imgVerso, int nbIter, int x=0, int y=0, int nbThreads=-1);

/*
  Function provided for convenience.
  
  Apply bleedthrough effect between @a imgRecto and @a imgVerso, with @a nbIter iterations, but considering @a originalRecto as the original recto image. It allows to apply the bleedthrough incrementally.

 */
extern FRAMEWORK_EXPORT QImage bleedThrough(const QImage &originalRecto, const QImage &imgRecto, const QImage &imgVerso, int nbIter, int x=0, int y=0, int nbThreads=-1);



class FRAMEWORK_EXPORT BleedThrough : public DocumentDegradation
{
  Q_OBJECT

public:

  explicit BleedThrough(int nbIterations, QObject *parent = 0) :
    DocumentDegradation(parent),
    _nbIter(nbIterations)
  {}

  void setVerso(const QString &path);

public slots :

  virtual QImage apply() override;
  virtual QImage apply(const QString &rectoPath);
  void setVersoAndApply(const QString &versoPath);

signals:

    void imageReady(const QImage &);

protected :
  QImage _verso;
  int _nbIter;

};

} //namespace dc
  
#endif // BLEEDTHROUGH_HPP
