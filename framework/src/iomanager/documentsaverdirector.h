#ifndef DOCUMENTSAVERDIRECTOR_H
#define DOCUMENTSAVERDIRECTOR_H

#include "documentsaver.h"

namespace IOManager {
class FRAMEWORK_EXPORT DocumentSaverDirector
{
public:
  explicit DocumentSaverDirector(DocumentSaver *docSaver)
  {
    _documentSaver = docSaver;
  }
  ~DocumentSaverDirector() {}

  QString getDocumentOutput() { return _documentSaver->getDocumentOutput(); }

  void constructDocumentOutput()
  {
    _documentSaver->createNewDocumentOutput();
    _documentSaver->buildStartDocument();
    _documentSaver->buildStyles();
    _documentSaver->buildContent();
    _documentSaver->buildEndDocument();
  }

private:
  DocumentSaver *_documentSaver;
};
}

#endif // DOCUMENTSAVERDIRECTOR_H
