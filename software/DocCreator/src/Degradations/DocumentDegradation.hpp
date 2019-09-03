#ifndef DOCUMENTDEGRADATION_HPP
#define DOCUMENTDEGRADATION_HPP

#include <QObject>
#include <QImage>
#include <context/backgroundcontext.h>
#include <framework_global.h>

class FRAMEWORK_EXPORT DocumentDegradation : public QObject
{
  Q_OBJECT
 public:

  explicit DocumentDegradation(QObject *parent = 0):
    QObject(parent)
  {}
  
  virtual QImage getBackGround()
  {
    //B:TODO:DESIGN: have a "QImage Context::BackgroundContext::instance()->getCurrentBackgroundImage()" ???

    const QString path = Context::BackgroundContext::instance()->getPath();
    const QString filename = path+Context::BackgroundContext::instance()->getCurrentBackground();
    QImage background(filename);
    //background = background.convertToFormat(QImage::Format_RGB32);
    return background;
  }

 public slots :
  virtual QImage apply() = 0;

};

#endif // DOCUMENTDEGRADATION_HPP
