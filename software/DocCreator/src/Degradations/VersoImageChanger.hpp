#ifndef VERSOIMAGECHANGER_HPP
#define VERSOIMAGECHANGER_HPP

#include "VersoImageChangerParameters.hpp"
#include <QObject>

class VersoImageChanger : public QObject
{
  Q_OBJECT

public:
  explicit VersoImageChanger(const VersoImageChangerParameters &params,
                             QObject *parent = 0);

signals:
  void versoChanged(const QString &verso);

public slots:
  void changeVerso();

protected:
  VersoImageChangerParameters _params;
};

#endif // VERSOIMAGECHANGER_HPP
