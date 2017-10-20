#include "documentfilemanager.h"

#include <QFile>
#include <QTextCodec>
#include <QTextStream>

#include "TxtDocumentSaver.hpp"
//#include <models/doc.h>
#include "documentloaderdirector.h"
#include "documentsaverdirector.h"
#include "xmldocumentloader.h"
#include "xmldocumentsaver.h"

namespace IOManager {
Doc::Document *
DocumentFileManager::documentFromXml(const QString &filepath)
{
  QFile file(filepath);
  file.open(QIODevice::ReadOnly);
  QTextStream in(&file);
  in.setCodec(QTextCodec::codecForName("utf8"));
  QString xmlCode = in.readAll();
  file.close();

  XMLDocumentLoader docLoader(xmlCode);
  DocumentLoaderDirector docLoaderDirector(&docLoader);
  docLoaderDirector.constructDocument();

  return docLoaderDirector.getDocument();
}

void
DocumentFileManager::documentToXml(Doc::Document *document,
                                   const QString &filepath)
{
  XMLDocumentSaver docSaver(document);
  DocumentSaverDirector docSaverDirector(&docSaver);
  docSaverDirector.constructDocumentOutput();

  QString documentOutput = docSaverDirector.getDocumentOutput();

  QFile file(filepath);
  file.open(QIODevice::WriteOnly);

  QTextStream out(&file);
  out.setCodec(QTextCodec::codecForName("utf8"));
  out << documentOutput;

  file.close();
}

void
DocumentFileManager::documentToTxt(Doc::Document *document,
                                   const QString &filename)
{
  TxtDocumentSaver saver(document, filename);
  DocumentSaverDirector director(&saver);
  director.constructDocumentOutput();
}

} //namespace IOManager
