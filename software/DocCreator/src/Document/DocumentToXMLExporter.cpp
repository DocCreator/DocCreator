#include "DocumentToXMLExporter.hpp"

#include "context/documentcontext.h"
#include "iomanager/documentfilemanager.h"

int DocumentToXMLExporter::_nb = 0;

void
DocumentToXMLExporter::toXML()
{
  QString filepath(_path);
  filepath.append("/");
  filepath.append("truth_");
  if (_numberWidth <= 0)
    filepath.append(QString::number(_nb));
  else
    filepath.append(QString("%1").arg(_nb, _numberWidth, 10, QChar('0')));
  filepath.append(".od");

  IOManager::DocumentFileManager::documentToXml(
    Context::DocumentContext::instance()->getCurrentDocument(), filepath);
  Context::DocumentContext::instance()->setCurrentDocumentPath(filepath);
  ++_nb;
}

void
DocumentToXMLExporter::setNumberWidth(int width)
{
  _numberWidth = width;
}
