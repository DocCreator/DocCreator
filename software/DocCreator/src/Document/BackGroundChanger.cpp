#include "BackGroundChanger.hpp"

//#include <QDebug>
#include <QImageWriter>
#include <context/backgroundcontext.h>

BackGroundChanger::BackGroundChanger(QObject *parent)
  : QObject(parent)
{}

#if 0
void BackGroundChanger::changeBackground(const QImage * image)
{
    /*QString path = Context::BackgroundContext::instance()->getPath();
    QString newBackGroundPath(path+"newBackGround.png");
    QImageWriter writer(newBackGroundPath,"png");
    writer.write(*image);

    //addBackground("newBackGround.png");
    Context::BackgroundContext::instance()->setCurrentBackground("newBackGround.png");*/
}
#endif

void
BackGroundChanger::changeBackGroundImage(const QImage &image)
{
  //B:TODO:UGLY !!! We don't want to write the image on disk !!!

  static int index = 0;

  //B:TODO: check that filename does not already exist !
  QString path = Context::BackgroundContext::instance()->getPath();
  QString filename =
    QStringLiteral("newBackGround") + QString::number(index++) + ".png";
  QString absFilename = path + filename;
  const QString &newBackGroundPath = absFilename;
  QImageWriter writer(newBackGroundPath, "png");
  const bool writeOk = writer.write(image);

  if (writeOk) {
    Context::BackgroundContext::instance()->setCurrentBackground(filename);

    //qDebug() << metaObject()->className() << "emit backgroundChanged() filename="<<absFilename;

    emit backgroundChanged();
  } else {
    //qDebug() << metaObject()->className() << ":::: !!! BackGroundChanger::changeBackGroundImage() unable to write "<<absFilename;
  }
}
