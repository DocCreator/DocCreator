#include "documentfilemanager.h"

#include <QDebug>
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
  if (! file.open(QIODevice::ReadOnly)) {
    qWarning() << "DocumentFileManager: unable to read" << filepath << ':'
	       << file.errorString();
    return nullptr;
  }
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

  return docLoaderDirector.getDocument().release();
}

bool
DocumentFileManager::documentToXml(Doc::Document *document,
                                   const QString &filepath)
{
  XMLDocumentSaver docSaver(document);
  DocumentSaverDirector docSaverDirector(&docSaver);
  docSaverDirector.constructDocumentOutput();

  QString documentOutput = docSaverDirector.getDocumentOutput();

  QFile file(filepath);
  if (! file.open(QIODevice::WriteOnly)) {
    qWarning() << "DocumentFileManager: unable to write" << filepath << ':'
	       << file.errorString();
    return false;
  }

  QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  out.setCodec(QTextCodec::codecForName("utf8"));
#else
  out.setEncoding(QStringConverter::Utf8);
#endif
  out << documentOutput;
  out.flush();

  /*
    The write itself can fail after the open succeeded (a full disk, a
    quota); the stream status and the close both have to be looked at.
  */
  const bool writeOk = (out.status() == QTextStream::Ok);
  file.close();
  if (! writeOk || file.error() != QFile::NoError) {
    qWarning() << "DocumentFileManager: writing" << filepath << "failed:"
	       << file.errorString();
    return false;
  }
  return true;
}

bool
DocumentFileManager::documentToTxt(Doc::Document *document,
                                   const QString &filename)
{
  TxtDocumentSaver saver(document, filename);
  if (! saver.isOpen())
    return false;
  DocumentSaverDirector director(&saver);
  director.constructDocumentOutput();
  return true;
}

} //namespace IOManager
