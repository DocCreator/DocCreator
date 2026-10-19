#include <QApplication>
#include <QCommandLineParser>
#include <iostream>

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
  QApplication::setApplicationName(QStringLiteral("DocCreator"));
  QApplication::setApplicationVersion(QStringLiteral(DOCCREATOR_VERSION));
  Q_INIT_RESOURCE(doccreator);

  /*
    --version, and --self-test, which checks the installation without
    showing the window and gives the result as exit code: the continuous
    integration only checked that the program was still running after 15
    seconds, which a dialog about a missing DLL or missing data also is.
    The output goes to stdout (redirect it on Windows, where the program
    has no console).
  */
  //The help and the version are written to stdout: Qt shows them in a
  // message box for a program without console on Windows, which waits.
  QCommandLineParser parser;
  parser.setApplicationDescription(QStringLiteral("DocCreator, a synthetic document image generator"));
  const QCommandLineOption helpOption(
    { QStringLiteral("h"), QStringLiteral("help") },
    QStringLiteral("Display this help and exit."));
  const QCommandLineOption versionOption(
    { QStringLiteral("v"), QStringLiteral("version") },
    QStringLiteral("Display the version and exit."));
  const QCommandLineOption selfTestOption(
    QStringLiteral("self-test"),
    QStringLiteral("Check the installation (data, fonts, image formats, tesseract models) and exit with 0 if it is complete."));
  parser.addOption(helpOption);
  parser.addOption(versionOption);
  parser.addOption(selfTestOption);
  if (!parser.parse(QApplication::arguments())) {
    std::cerr << parser.errorText().toStdString() << "\n";
    return 2;
  }
  if (parser.isSet(helpOption)) {
    std::cout << parser.helpText().toStdString();
    return 0;
  }
  if (parser.isSet(versionOption)) {
    std::cout << "DocCreator " << DOCCREATOR_VERSION << "\n";
    return 0;
  }

  Core::ConfigurationManager::initialize(AppConfigFile);

  auto w =
    new DocCreator; //Must be allocated on heap (as Qt::WA_DeleteOnClose attribute is set) !!!

  if (parser.isSet(selfTestOption)) {
    const bool ok = w->selfTest(std::cout);
    delete w;
    return ok ? 0 : 1;
  }

  w->show();

  //    w->close();

  a.exec();

  return 0;
}
