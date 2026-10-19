#include "documentfilemanager.h"

#include <QDebug>
#include <QFile>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif
#include <QTextStream>
#include <QXmlStreamReader>

#include "TxtDocumentSaver.hpp"
//#include <models/doc.h>
#include "documentloaderdirector.h"
#include "documentsaverdirector.h"
#include "xmldocumentloader.h"
#include "xmldocumentsaver.h"

namespace IOManager {

namespace {

  /*
    @return @a bytes decoded with the encoding its XML declaration gives,
    UTF-8 if it gives none or one that Qt does not know.

    B: the file was decoded as UTF-8 whatever it declared: the accented
    characters of an ISO-8859-1 document were mangled. (The loader takes a
    QString, which QXmlStreamReader does not decode again.)
  */
  QString
  decodeXml(const QByteArray &bytes)
  {
    QXmlStreamReader reader(bytes);
    while (!reader.atEnd() && reader.tokenType() != QXmlStreamReader::StartDocument &&
           reader.tokenType() != QXmlStreamReader::StartElement) {
      reader.readNext();
    }
    const QString encoding = reader.documentEncoding().toString().trimmed();
    if (encoding.isEmpty() ||
        encoding.compare(QLatin1String("UTF-8"), Qt::CaseInsensitive) == 0) {
      return QString::fromUtf8(bytes);
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec *codec = QTextCodec::codecForName(encoding.toLatin1());
    if (codec != nullptr) {
      return codec->toUnicode(bytes);
    }
#else
    QStringDecoder decoder(encoding);
    if (decoder.isValid()) {
      return decoder.decode(bytes);
    }
    //QStringDecoder only knows the Unicode encodings and Latin-1 by name
    if (encoding.compare(QLatin1String("ISO-8859-1"), Qt::CaseInsensitive) == 0 ||
        encoding.compare(QLatin1String("latin1"), Qt::CaseInsensitive) == 0) {
      return QString::fromLatin1(bytes);
    }
#endif
    qWarning() << "DocumentFileManager: unknown encoding" << encoding
               << ", read as UTF-8";
    return QString::fromUtf8(bytes);
  }

} //namespace
Doc::Document *
DocumentFileManager::documentFromXml(const QString &filepath)
{
  QFile file(filepath);
  if (! file.open(QIODevice::ReadOnly)) {
    qWarning() << "DocumentFileManager: unable to read" << filepath << ':'
	       << file.errorString();
    return nullptr;
  }
  const QByteArray bytes = file.readAll();
  file.close();
  const QString xmlCode = decodeXml(bytes);

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
