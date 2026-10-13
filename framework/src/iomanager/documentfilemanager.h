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

  /*
    @return the document read from @a filepath, or nullptr when the file
    cannot be read. The caller owns the returned document.
  */
  [[nodiscard]] static Doc::Document *documentFromXml(const QString &filepath);

  /*
    @return whether @a document could be written to @a filepath.
  */
  [[nodiscard]] static bool documentToXml(Doc::Document *document,
					  const QString &filepath);

  /*
    @return whether @a document could be written to @a filename.
  */
  [[nodiscard]] virtual bool documentToTxt(Doc::Document *document,
					   const QString &filename);
};
}

#endif // DOCUMENTFILEMANAGER_H
