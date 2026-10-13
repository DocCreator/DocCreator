/*
  libFuzzer target for the font readers.

  FontFileManager::readFont() turns a file into a Models::Font: it looks at
  the header, then reads either the binary format (.bof, a QDataStream of
  counts and QImages) or the XML one (.of). Both are pure bytes -> object
  functions driven entirely by the file, which is the shape a fuzzer wants.

  The two bugs the readers have had come from exactly there: a count read
  from the file and used as a reserve() (a 40 byte file asking for four
  billion versions), and a geometry read from the file and used as an
  allocation.

  Cf fuzz/README.txt for how to build and run this.
*/

#include <cstddef>
#include <cstdint>

#include <QByteArray>
#include <QCoreApplication>
#include <QTemporaryFile>

#include "iomanager/fontfilemanager.h"
#include "models/font.h"

namespace {

  /*
    QImage, which the readers build, needs the Qt plugin paths and the
    image format handlers, so an application object has to exist. One is
    built once for the whole run.
  */
  int argc = 1;
  char argv0[] = "fuzz_font";
  char *argv[] = { argv0, nullptr };

  QCoreApplication *application()
  {
    static QCoreApplication app(argc, argv);
    return &app;
  }

} //namespace

extern "C" int
LLVMFuzzerInitialize(int *, char ***)
{
  application();
  return 0;
}

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  //the readers take a path, so the input has to reach the disk
  QTemporaryFile file;
  if (! file.open()) {
    return 0;
  }
  file.write(reinterpret_cast<const char *>(data),
	     static_cast<qint64>(size));
  file.flush();

  delete IOManager::FontFileManager::readFont(file.fileName());

  //and once through each reader, so that a file whose header does not
  //match still exercises the binary one
  delete IOManager::FontFileManager::fontFromBinary(file.fileName());
  delete IOManager::FontFileManager::fontFromXml(file.fileName());

  return 0;
}
