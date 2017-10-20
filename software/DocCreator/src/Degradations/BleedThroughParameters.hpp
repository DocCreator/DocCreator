#ifndef BLEEDTHROUGHPARAMETERS_HPP
#define BLEEDTHROUGHPARAMETERS_HPP

#include <QObject>

class BleedThroughParameters : public QObject
{
  Q_OBJECT

public:
  explicit BleedThroughParameters(QObject *parent = 0);

  /*
  BleedThroughParameters(const BleedThroughParameters &o) :
    _pathToVersoImage(o._pathToVersoImage),
    _nbIterations(o._nbIterations)
  {}
  */

  BleedThroughParameters &operator=(const BleedThroughParameters &o)
  {
    _pathToVersoImage = o._pathToVersoImage;
    _nbIterations = o._nbIterations;
    return *this;
  }

  const QString &getPathToVersoImage() const { return _pathToVersoImage; }

  int getNbIterations() const { return _nbIterations; }

public slots:

  void setPathToVersoImage(const QString &s) { _pathToVersoImage = s; }

  void setNbIterations(int nb) { _nbIterations = nb; }

protected:
  QString _pathToVersoImage;
  int _nbIterations;
};

#endif // BLEEDTHROUGHPARAMETERS_HPP
