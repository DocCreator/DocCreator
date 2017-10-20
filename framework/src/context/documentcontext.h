#ifndef DOCUMENTCONTEXT_H
#define DOCUMENTCONTEXT_H

#include <QString>
#include <framework_global.h>
#include <patterns/singleton.h>

namespace Doc {
class Document;
}

namespace Context {

class FRAMEWORK_EXPORT DocumentContext
  : public Patterns::Singleton<DocumentContext>
{
public:
  static DocumentContext *instance();

  DocumentContext();

  Doc::Document *getCurrentDocument();
  void setCurrentDocument(Doc::Document *document);
  QString getCurrentDocumentPath();
  void setCurrentDocumentPath(const QString &path);
  bool modified();
  void setModified(bool modified);
  bool isNewDocument() const;
  void setNewDocument();

private:
  static DocumentContext *_instance;

  Doc::Document *_currentDocument;
  QString _currentDocumentPath;
  bool _modified;
};
}

#endif // DOCUMENTCONTEXT_H
