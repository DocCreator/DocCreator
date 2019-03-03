#ifndef PHANTOMCHARACTER_HPP
#define PHANTOMCHARACTER_HPP

#include "DocumentDegradation.hpp"
#include <framework_global.h>
#include <QImage>
#include <opencv2/core/core.hpp>

enum class Frequency {RARE=0, FREQUENT, VERY_FREQUENT}; 

/*
  RARE is 15% probability of occurrence of phantom character.
  FREQUENT is 40%
  VERY_FREQUENT is 70%
*/

extern FRAMEWORK_EXPORT QImage phantomCharacter(const QImage &imgOriginal, Frequency frequency, const QString &phantomPatternsPath);
extern FRAMEWORK_EXPORT cv::Mat phantomCharacter(const cv::Mat &imgOriginal, Frequency frequency, const QString &phantomPatternsPath);


class FRAMEWORK_EXPORT PhantomCharacter : public DocumentDegradation
{
  Q_OBJECT

public : 

  /*

   */
  explicit PhantomCharacter(const QImage &original, Frequency frequency, const QString &phantomPatternsPath, QObject *parent =0) :
    DocumentDegradation(parent), _frequency(frequency), _phantomPatternsPath(phantomPatternsPath), _original(original)
  {}

public slots :

  virtual QImage apply() override;

signals:

  void imageReady(const QImage &);

protected :
  const Frequency _frequency;
  QImage _original;
  const QString _phantomPatternsPath;

};

#endif //PHANTOMCHARACTER_HPP
