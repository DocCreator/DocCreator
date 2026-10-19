#include "BackGroundChanger.hpp"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QImageWriter>
#include <QMessageBox>
#include <QStandardPaths>
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

bool
BackGroundChanger::changeBackGroundImage(const QImage &image)
{
  //B:TODO:UGLY !!! We don't want to write the image on disk !!!

  /*
    B: the image was written in the backgrounds folder of the data, which is
    read-only for an installation in Program Files: every degradation
    applied did nothing, silently. The index of the name started again at
    each session, so the images of a previous session were overwritten.
    The image is written in the data folder of the user, under a name not
    taken yet, and a failure is reported.
  */
  const QString dirPath =
    QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
    QStringLiteral("/backgrounds");
  if (!QDir().mkpath(dirPath)) {
    qWarning() << metaObject()->className() << ": unable to create" << dirPath;
    QMessageBox::warning(nullptr, tr("Background"),
                         tr("Unable to create the folder %1.").arg(dirPath));
    return false;
  }
  const QDir dir(dirPath);
  const QString stamp =
    QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-hhmmss-zzz"));
  QString absFilename;
  for (int index = 0;; ++index) {
    absFilename = dir.absoluteFilePath(QStringLiteral("background_") + stamp +
                                       QLatin1Char('_') +
                                       QString::number(index) +
                                       QStringLiteral(".png"));
    if (!QFileInfo::exists(absFilename)) {
      break;
    }
  }

  QImageWriter writer(absFilename, "png");
  if (!writer.write(image)) {
    qWarning() << metaObject()->className() << ": unable to write" << absFilename
               << ":" << writer.errorString();
    QMessageBox::warning(nullptr, tr("Background"),
                         tr("Unable to write the new background %1: %2")
                           .arg(absFilename, writer.errorString()));
    return false;
  }

  //an absolute path, cf BackgroundContext::getFilePath()
  Context::BackgroundContext::instance()->setCurrentBackground(absFilename);
  emit backgroundChanged();
  return true;
}
