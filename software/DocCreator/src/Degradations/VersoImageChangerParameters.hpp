#ifndef VERSOIMAGECHANGERPARAMETERS_HPP
#define VERSOIMAGECHANGERPARAMETERS_HPP

#include <QObject>

class VersoImageChangerParameters : public QObject
{
  Q_OBJECT

public:
  explicit VersoImageChangerParameters(QObject *parent = 0);

  /*
  VersoImageChangerParameters(const VersoImageChangerParameters &o) :
    _versosDirPath(o._versosDirPath)
  {

  }
  */

  VersoImageChangerParameters &operator=(const VersoImageChangerParameters &o)
  {
    _versosDirPath = o._versosDirPath;
    return *this;
  }

public slots:
  void setVersoDirPath(const QString &versosDirPath)
  {
    _versosDirPath = versosDirPath;
  }

public:
  QString _versosDirPath;
};

#endif // VERSOIMAGECHANGERPARAMETERS_HPP
