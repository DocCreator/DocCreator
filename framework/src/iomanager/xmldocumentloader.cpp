#include "xmldocumentloader.h"
#include <QDebug>
#include <QXmlStreamReader>

namespace IOManager {
XMLDocumentLoader::XMLDocumentLoader(const QString &input)
  : DocumentLoader(input)
{
  _reader = new QXmlStreamReader(_input);
}

void
XMLDocumentLoader::createNewDocument()
{
  DocumentLoader::createNewDocument();
  _reader = new QXmlStreamReader(_input);

  int w = 0;
  int h = 0;

  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == "document") &&
         !_reader->atEnd()) {
    if (_reader->name() == "document" &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      w =
        _reader->attributes().value(QStringLiteral("width")).toString().toInt();
      h = _reader->attributes()
            .value(QStringLiteral("height"))
            .toString()
            .toInt();
      break;
    }
    _reader->readNext();
  }
  qDebug() << " loader w,h = " << w << ", " << h;
  _output->setPageWidth(w);
  _output->setPageHeight(h);
}

void
XMLDocumentLoader::buildStyles()
{
  //Reading styles from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == "styles") &&
         !_reader->atEnd()) {
    if (_reader->name() == "style" &&
        _reader->tokenType() == QXmlStreamReader::StartElement)
      _output->addStyle(buildStyle());
    _reader->readNext();
  }
}

void
XMLDocumentLoader::buildContent()
{
  //Reading document from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == "content") &&
         !_reader->atEnd()) {
    //QString name = _reader->name().toString();
    //QXmlStreamReader::TokenType token = _reader->tokenType();

    if (_reader->name() == "page" &&
        _reader->tokenType() == QXmlStreamReader::StartElement)
      _output->add(buildPage());
    _reader->readNext();
  }
}

//protected:
Doc::DocStyle *
XMLDocumentLoader::buildStyle()
{
  Doc::DocStyle *s = new Doc::DocStyle(
    _reader->attributes().value(QStringLiteral("name")).toString());

  //Reading style from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == "style") &&
         !_reader->atEnd()) {
    if (_reader->name() == "font" &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      s->setFontName(_reader->readElementText());
    }
    _reader->readNext();
  }

  return s;
}

Doc::Page *
XMLDocumentLoader::buildPage()
{
  auto p = new Doc::Page(_output);
  p->setBackgroundFileName(_reader->attributes()
                             .value(QStringLiteral("backgroundFileName"))
                             .toString());

  //Reading style from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == "page") &&
         !_reader->atEnd()) {
    if (_reader->name() == "textBlock" &&
        _reader->tokenType() == QXmlStreamReader::StartElement)
      p->add(buildTextBlock());
    if (_reader->name() == "imageBlock" &&
        _reader->tokenType() == QXmlStreamReader::StartElement)
      p->add(buildImageBlock());
    _reader->readNext();
  }

  return p;
}

Doc::DocImageBlock *
XMLDocumentLoader::buildImageBlock()
{
  Doc::DocImageBlock *ib = new Doc::DocImageBlock(
    _reader->attributes().value(QStringLiteral("filePath")).toString());

  ib->setX(_reader->attributes().value(QStringLiteral("x")).toString().toInt());
  ib->setY(_reader->attributes().value(QStringLiteral("y")).toString().toInt());
  ib->setWidth(
    _reader->attributes().value(QStringLiteral("width")).toString().toInt());
  ib->setHeight(
    _reader->attributes().value(QStringLiteral("height")).toString().toInt());

  return ib;
}

Doc::DocTextBlock *
XMLDocumentLoader::buildTextBlock()
{
  auto tb = new Doc::DocTextBlock(_output);

  tb->setX(_reader->attributes().value(QStringLiteral("x")).toString().toInt());
  tb->setY(_reader->attributes().value(QStringLiteral("y")).toString().toInt());
  tb->setWidth(
    _reader->attributes().value(QStringLiteral("width")).toString().toInt());
  tb->setHeight(
    _reader->attributes().value(QStringLiteral("height")).toString().toInt());
  tb->setMarginTop(_reader->attributes()
                     .value(QStringLiteral("marginTop"))
                     .toString()
                     .toInt());
  tb->setMarginBottom(_reader->attributes()
                        .value(QStringLiteral("marginBottom"))
                        .toString()
                        .toInt());
  tb->setMarginLeft(_reader->attributes()
                      .value(QStringLiteral("marginLeft"))
                      .toString()
                      .toInt());
  tb->setMarginRight(_reader->attributes()
                       .value(QStringLiteral("marginRight"))
                       .toString()
                       .toInt());

  //Reading textblock content from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == "textBlock") &&
         !_reader->atEnd()) {
    if (_reader->name() == "paragraph" &&
        _reader->tokenType() == QXmlStreamReader::StartElement)
      tb->add(buildParagraph());
    _reader->readNext();
  }

  return tb;
}

Doc::DocParagraph *
XMLDocumentLoader::buildParagraph()
{
  auto p = new Doc::DocParagraph(_output);

  p->setLineSpacing(_reader->attributes()
                      .value(QStringLiteral("lineSpacing"))
                      .toString()
                      .toInt());
  p->setTabulationSize(_reader->attributes()
                         .value(QStringLiteral("tabulationSize"))
                         .toString()
                         .toInt());

  //Reading textblock content from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == "paragraph") &&
         !_reader->atEnd()) {
    if (_reader->name() == "string" &&
        _reader->tokenType() == QXmlStreamReader::StartElement)
      p->add(buildString());
    _reader->readNext();
  }

  return p;
}

Doc::DocString *
XMLDocumentLoader::buildString()
{
  Doc::DocString *s = new Doc::DocString(
    new Doc::DocStyle(
      _reader->attributes().value(QStringLiteral("style")).toString()),
    _output);

  //Reading textblock content from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == "string") &&
         !_reader->atEnd()) {
    if (_reader->name() == "char" &&
        _reader->tokenType() == QXmlStreamReader::StartElement)
      s->add(buildCharacter());
    _reader->readNext();
  }

  return s;
}

Doc::DocCharacter *
XMLDocumentLoader::buildCharacter()
{
  const int id =
    _reader->attributes().value(QStringLiteral("id")).toString().toInt();
  const QString display =
    _reader->attributes().value(QStringLiteral("display")).toString();
  const int x =
    _reader->attributes().value(QStringLiteral("x")).toString().toInt();
  const int y =
    _reader->attributes().value(QStringLiteral("y")).toString().toInt();
  const int w =
    _reader->attributes().value(QStringLiteral("width")).toString().toInt();
  const int h =
    _reader->attributes().value(QStringLiteral("height")).toString().toInt();
  Doc::DocCharacter *c = new Doc::DocCharacter(display, id, _output);
  c->setX(x);
  c->setY(y);
  c->setHeight(h);
  c->setWidth(w);
  //    qDebug() << id << " " << display << " " << x << " " << y << " " << w <<"
  //    " << h;
  return c;
}

} //namespace IOManager
