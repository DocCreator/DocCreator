#include "documentfilemanager.h"

#include <QFile>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  in.setCodec(QTextCodec::codecForName("utf8"));
#else
  in.setEncoding(QStringConverter::Utf8);
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  out.setCodec(QTextCodec::codecForName("utf8"));
#else
  out.setEncoding(QStringConverter::Utf8);
#endif
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
