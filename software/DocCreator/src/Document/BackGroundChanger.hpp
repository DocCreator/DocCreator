#ifndef BACKGROUNDCHANGER_HPP
#define BACKGROUNDCHANGER_HPP

#include <QObject>

class QImage;

class BackGroundChanger : public QObject
{
  Q_OBJECT
public:
  explicit BackGroundChanger(QObject *parent = 0);

  //void changeBackground(const QImage *image);

signals:
  void backgroundChanged();

public slots:
  void changeBackGroundImage(const QImage &image);
};

#endif // BACKGROUNDCHANGER_HPP
