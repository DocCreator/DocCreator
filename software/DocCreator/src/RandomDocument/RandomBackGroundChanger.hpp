#ifndef RANDOMBACKGROUNDCHANGER_HPP
#define RANDOMBACKGROUNDCHANGER_HPP

#include <QObject>

class RandomBackGroundChanger : public QObject
{
  Q_OBJECT
public:
  explicit RandomBackGroundChanger(int nb, QObject *parent = 0);

signals:
  void backGroundChanged();

public slots:
  void changeBackGround();

private:
  int _nb;
};

#endif // RANDOMBACKGROUNDCHANGER_HPP
