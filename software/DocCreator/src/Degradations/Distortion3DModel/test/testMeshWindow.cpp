#include <cstdlib>
#include <iostream>
#include <QDebug>
#include <QApplication>
#include <QImage>

#include "MeshWindow.hpp"


#if QT_VERSION > 0x050400

/*
  cf http://doc.qt.io/qt-5/qopenglwidget.html
  "Note: Calling QSurfaceFormat::setDefaultFormat() before constructing the QApplication instance is mandatory on some platforms (for example, macOS) when an OpenGL core profile context is requested. This is to ensure that resource sharing between contexts stays functional as all internal contexts are created using the correct version and profile."
 */

#include <QSurfaceFormat>

static void setDefaultOpenGLSurfaceFormat()
{
  QSurfaceFormat format;
#ifndef Q_OS_LINUX
  format.setVersion(3, 3);
#endif
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);
}

#endif //QT_VERSION


int main(int argc, char** argv) {

#if QT_VERSION > 0x050400
  setDefaultOpenGLSurfaceFormat();
#endif //QT_VERSION

  QApplication app(argc, argv);




  MeshWindow *window = new MeshWindow();

  window->show();


  if (argc > 1) {
    const char *meshFilename = argv[1];
    window->loadMeshFile(meshFilename);
  }

  if (argc > 2) {
    const char *imageFilename = argv[2];
    window->loadImageFile(imageFilename);
  }
  
  if (argc > 3) {
   const char *outMeshFilename = argv[3];
   window->computeTextureCoords(outMeshFilename);
  }
  
  return app.exec();
}

