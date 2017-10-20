#ifndef DOCUMENTFILEMANAGER_H
#define DOCUMENTFILEMANAGER_H

#include <framework_global.h>
namespace Doc {
class Document;
}
class QString;

namespace IOManager {
class FRAMEWORK_EXPORT DocumentFileManager
{
public:
  virtual ~DocumentFileManager() {}

  //Font Input/Output
  static Doc::Document *documentFromXml(const QString &filepath);
  static void documentToXml(Doc::Document *document, const QString &filepath);

  virtual void documentToTxt(Doc::Document *document, const QString &filename);
};
}

#endif // DOCUMENTFILEMANAGER_H
