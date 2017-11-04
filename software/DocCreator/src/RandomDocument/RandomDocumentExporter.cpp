#include "RandomDocumentExporter.hpp"

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QImage>
#include <QImageWriter>
#include <QTextStream>

#include "Document/DocumentController.hpp"
#include "context/documentcontext.h"
#include "iomanager/documentfilemanager.h"

int RandomDocumentExporter::_nbDocWritten = 0;

RandomDocumentExporter::RandomDocumentExporter(DocumentController *ctrl,
                                               const QString &path,
                                               QObject *parent)
  : QObject(parent)
  , _path(path)
  , _ctrl(ctrl)
  , _numberWidth(0)
{}

void
RandomDocumentExporter::setNumberWidth(int width)
{
  _numberWidth = width;
}

QString
RandomDocumentExporter::getFilePath(const QString &prefix,
                                    const QString &extension) const
{
  QString filename(_path);
  filename.append("/");
  filename.append(prefix);
  filename.append("_");
  if (_numberWidth <= 0)
    filename.append(QString::number(_nbDocWritten));
  else
    filename.append(
      QString("%1").arg(_nbDocWritten, _numberWidth, 10, QChar('0')));
  filename.append(extension);

  return filename;
}

QString
RandomDocumentExporter::getDocImageFilePath() const
{
  return getFilePath(QStringLiteral("image"), QStringLiteral(".png"));
}

#include <context/CurrentRandomDocumentContext.h>

void
RandomDocumentExporter::saveRandomDocument()
{
  qDebug() << metaObject()->className() << " :::::::::::::: saveDoc";

  //B: Why do we need the CurrentRandomDocumentContext ????
  //It seems that it is only to be able to call saveProperties(randDoc->getProperties());

  Context::CurrentRandomDocumentContext *randDoc =
    Context::CurrentRandomDocumentContext::instance();

  randDoc->setImage(_ctrl->toQImage(
    Color | WithTextBlocks |
    WithImageBlocks)); //B: here we consider one document==one page !?...

  writeImage(randDoc->getImage());
  //B: this is what takes time here (because we save in PNG)

  saveProperties(randDoc->getProperties());

  saveTextFile();

  ++_nbDocWritten;
}

void
RandomDocumentExporter::saveTextFile()
{
  const QString filename =
    getFilePath(QStringLiteral("text"), QStringLiteral(".txt"));

  IOManager::DocumentFileManager m;
  m.documentToTxt(Context::DocumentContext::instance()->getCurrentDocument(),
                  filename);
}

void
RandomDocumentExporter::writeImage(const QImage &image)
{
  qDebug() << metaObject()->className() << "path=" << getDocImageFilePath()
           << " image.isNull()=" << image.isNull() << "\n";

  if (!image.isNull()) {
    const QString filename = getDocImageFilePath();
    QImageWriter writer(filename, "png");
    writer.write(image);
    emit imageSaved(filename);
  }
}

void
RandomDocumentExporter::saveProperties(const QMap<QString, QString> &properties)
{
  QString filename =
    getFilePath(QStringLiteral("properties"), QStringLiteral(".xml"));

  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;

  QTextStream out(&file);

  QDomImplementation impl = QDomDocument().implementation();
  const QString name = QStringLiteral("stone");
  const QString publicId =
    QStringLiteral("-//XADECK//DTD Random Old Document 1.0 //EN");
  const QString systemId =
    QStringLiteral("http://www-imagis.imag.fr/DTD/rand.dtd"); //B: why ???
  QDomDocument doc(impl.createDocumentType(name, publicId, systemId));
  doc.appendChild(doc.createComment(
    QStringLiteral("This file describe an ancient document image")));
  doc.appendChild(doc.createTextNode(QStringLiteral("\n")));

  QDomElement root = doc.createElement(QStringLiteral("<properties>"));
  root.setAttribute(QStringLiteral("doc_image"), getDocImageFilePath());
  doc.appendChild(root);

  const QMap<QString, QString> &lol = properties;

  QMapIterator<QString, QString> i(lol);
  while (i.hasNext()) {
    i.next();
    QDomElement appearanceNode = doc.createElement(i.key());
    QDomText value = doc.createTextNode(i.value());
    appearanceNode.appendChild(value);
    root.appendChild(appearanceNode);
  }
  out << doc.toString();

  emit propertiesSaved(filename);
}
