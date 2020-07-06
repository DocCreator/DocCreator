#include <QApplication>

#include "DocCreator.hpp"
#include "appconstants.h"
#include "core/configurationmanager.h"

#if (QT_VERSION > QT_VERSION_CHECK(5, 4, 0))

/*
cf http://doc.qt.io/qt-5/qopenglwidget.html
"Note: Calling QSurfaceFormat::setDefaultFormat() before constructing the QApplication instance is mandatory on some platforms (for example, macOS) when an OpenGL core profile context is requested. This is to ensure that resource sharing between contexts stays functional as all internal contexts are created using the correct version and profile."
*/

#include <QSurfaceFormat>

static void
setDefaultOpenGLSurfaceFormat()
{
  QSurfaceFormat format;
#ifndef Q_OS_LINUX
  format.setVersion(3, 3);
#endif
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);
}

#endif //QT_VERSION

int
main(int argc, char *argv[])
{
#if (QT_VERSION > QT_VERSION_CHECK(5, 4, 0))
  setDefaultOpenGLSurfaceFormat();
#endif //QT_VERSION

  QApplication a(argc, argv);
  Q_INIT_RESOURCE(doccreator);
  Core::ConfigurationManager::initialize(AppConfigFile);

  auto w =
    new DocCreator; //Must be allocated on heap (as Qt::WA_DeleteOnClose attribute is set) !!!

  w->show();

  //    w->close();

  a.exec();

  return 0;
}
