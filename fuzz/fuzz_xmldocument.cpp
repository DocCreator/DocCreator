/*
  libFuzzer target for the document XML loader.

  XMLDocumentLoader takes the whole document as a QString and builds a
  Doc::Document from it: every size, coordinate and count in the model
  comes from those bytes, and the sizes end up in the allocations of the
  generation path.

  Cf fuzz/README.txt for how to build and run this.
*/

#include <cstddef>
#include <cstdint>

#include <QCoreApplication>
#include <QString>

#include "iomanager/documentloaderdirector.h"
#include "iomanager/xmldocumentloader.h"
#include "models/doc/document.h"

namespace {

  int argc = 1;
  char argv0[] = "fuzz_xmldocument";
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
  const QString xml = QString::fromUtf8(reinterpret_cast<const char *>(data),
					static_cast<int>(size));

  IOManager::XMLDocumentLoader loader(xml);
  IOManager::DocumentLoaderDirector director(&loader);
  director.constructDocument();

  delete director.getDocument();

  return 0;
}
