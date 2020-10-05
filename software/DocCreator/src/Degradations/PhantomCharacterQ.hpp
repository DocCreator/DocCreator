#ifndef PHANTOMCHARACTERQ_HPP
#define PHANTOMCHARACTERQ_HPP

#include <QImage>
#include "Degradations/DocumentDegradation.hpp"
#include "Degradations/PhantomCharacter.hpp"

namespace dc {

  namespace PhantomCharacter {

    extern FRAMEWORK_EXPORT QImage phantomCharacter(const QImage &imgOriginal, float occurenceProbability, const QString &phantomPatternsPath);

    
  } //namespace PhantomCharacter 
  

  class FRAMEWORK_EXPORT PhantomCharacterQ : public DocumentDegradation
  {
    Q_OBJECT

  public : 

    /*

     */
    explicit PhantomCharacterQ(const QImage &original,
			       float occurenceProbability,
			       const QString &phantomPatternsPath,
			       QObject *parent =0) :
      DocumentDegradation(parent),
      _occurenceProbability(occurenceProbability),
      _original(original),
      _phantomPatternsPath(phantomPatternsPath)
      {}

  public slots :

    QImage apply() override;

  signals:

    void imageReady(const QImage &);

  protected :
    const float _occurenceProbability;
    QImage _original;
    const QString _phantomPatternsPath;

  };


} //namespace dc

#endif /* ! PHANTOMCHARACTERQ_HPP */
