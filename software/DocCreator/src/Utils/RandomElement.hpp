#ifndef RANDOMELEMENT_HPP
#define RANDOMELEMENT_HPP

#include <QObject>
#include <cstdlib>
#include <time.h>

class RandomElement
  : public QObject //B: is it necessary to inherit from QObject ? Is it necessary to have a virtual method ???
{
  Q_OBJECT
public:
  explicit RandomElement(QObject *parent = nullptr);

  //B: return value in [min; max]
  virtual int randomInt(int min, int max)
  {
    static int Init = 0;
    int rc;

    if (Init == 0) {
      srand(time(nullptr));
      Init = 1;
    }
    rc = (rand() % (max - min + 1) + min);

    return (rc);
  }
};

#endif // RANDOMELEMENT_HPP
