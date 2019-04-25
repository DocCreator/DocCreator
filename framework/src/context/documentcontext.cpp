#include "documentcontext.h"

namespace Context {

DocumentContext *DocumentContext::_instance;

DocumentContext::DocumentContext()
  : _currentDocument(nullptr)
  , _currentDocumentPath(QLatin1String(""))
  , _modified(false)
{}

DocumentContext *
DocumentContext::instance()
{
  if (_instance == nullptr) {
    _instance = new DocumentContext();
  }
  return _instance;
}

Doc::Document *
DocumentContext::getCurrentDocument()
{
  return _currentDocument;
}

void
DocumentContext::setCurrentDocument(Doc::Document *document)
{
  _currentDocument = document;
  setNewDocument();
}

QString
DocumentContext::getCurrentDocumentPath()
{
  return _currentDocumentPath;
}

void
DocumentContext::setCurrentDocumentPath(const QString &path)
{
  _currentDocumentPath = path;
}

bool
DocumentContext::modified()
{
  return _modified;
}

void
DocumentContext::setModified(bool modified)
{
  _modified = modified;
}

bool
DocumentContext::isNewDocument() const
{
  return _currentDocumentPath.isEmpty();
}

void
DocumentContext::setNewDocument()
{
  _currentDocumentPath = QString();
}
} //Context
