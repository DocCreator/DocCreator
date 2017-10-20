#ifndef DOCUMENTLOADERDIRECTOR_H
#define DOCUMENTLOADERDIRECTOR_H

#include "documentloader.h"

namespace IOManager {
class FRAMEWORK_EXPORT DocumentLoaderDirector
{
public:
  explicit DocumentLoaderDirector(DocumentLoader *docLoader)
  {
    _documentLoader = docLoader;
  }
  ~DocumentLoaderDirector() {}

  Doc::Document *getDocument() { return _documentLoader->getDocument(); }

  void constructDocument()
  {
    _documentLoader->createNewDocument();
    _documentLoader->buildStyles();
    _documentLoader->buildContent();
  }

private:
  DocumentLoader *_documentLoader;
};
}

#endif // DOCUMENTLOADERDIRECTOR_H
