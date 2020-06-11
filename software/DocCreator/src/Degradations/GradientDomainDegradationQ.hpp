#ifndef GRADIENTDOMAINDEGRADATIONQ_HPP
#define GRADIENTDOMAINDEGRADATIONQ_HPP

#include <QImage>
#include "Degradations/DocumentDegradation.hpp"
#include "Degradations/GradientDomainDegradation.hpp"

namespace dc {

  namespace GradientDomainDegradation {

    extern FRAMEWORK_EXPORT QImage degradation(const QImage &img,
					       const QString &stainImageDir,
					       size_t numStainsToInsert = 20,
					       InsertType insertType = InsertType::INSERT_AS_GRAY_IF_GRAY,
					       bool doRotations = true);

  } //namespace GradientDomainDegradation

  class FRAMEWORK_EXPORT GradientDomainDegradationQ : public DocumentDegradation
  {
    Q_OBJECT

  public:

    explicit GradientDomainDegradationQ(const QImage &original,
					const QString &stainImageDir,
					size_t numStainsToInsert = 20,
					GradientDomainDegradation::InsertType insertType = GradientDomainDegradation::InsertType::INSERT_AS_GRAY_IF_GRAY,
					bool doRotations = true,
					QObject *parent = 0) :
      DocumentDegradation(parent),
      _original(original),
      _stainImageDir(stainImageDir),
      _numStainsToInsert(numStainsToInsert),
      _insertType(insertType),
      _doRotations(doRotations)
      {}

  public slots:

    virtual QImage apply() override;

  signals:

    void imageReady(const QImage &);

  protected:
    
    QImage _original;
    QString _stainImageDir;
    size_t _numStainsToInsert;
    GradientDomainDegradation::InsertType _insertType;
    bool _doRotations;
  };
					
} //namespace dc


#endif /* ! GRADIENTDOMAINDEGRADATIONQ_HPP */
