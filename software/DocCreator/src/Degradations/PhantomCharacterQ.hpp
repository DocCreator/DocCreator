#ifndef PHANTOMCHARACTERQ_HPP
#define PHANTOMCHARACTERQ_HPP

#include <QImage>
#include "Degradations/DocumentDegradation.hpp"
#include "Degradations/PhantomCharacter.hpp"

namespace dc {

  namespace PhantomCharacter {

    extern FRAMEWORK_EXPORT QImage phantomCharacter(const QImage &imgOriginal, Frequency frequency, const QString &phantomPatternsPath);

    
  } //namespace PhantomCharacter 
  

  class FRAMEWORK_EXPORT PhantomCharacterQ : public DocumentDegradation
  {
    Q_OBJECT

  public : 

    /*

     */
    explicit PhantomCharacterQ(const QImage &original,
			       dc::PhantomCharacter::Frequency frequency,
			       const QString &phantomPatternsPath,
			       QObject *parent =0) :
      DocumentDegradation(parent),
      _frequency(frequency),
      _original(original),
      _phantomPatternsPath(phantomPatternsPath)
      {}

  public slots :

    virtual QImage apply() override;

  signals:

    void imageReady(const QImage &);

  protected :
    const dc::PhantomCharacter::Frequency _frequency;
    QImage _original;
    const QString _phantomPatternsPath;

  };


} //namespace dc

#endif /* ! PHANTOMCHARACTERQ_HPP */
