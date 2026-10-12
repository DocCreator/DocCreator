#include "xmldocumentloader.h"

#include <algorithm>
#include <iostream>
#include <limits>

#include <QDebug>
#include <QXmlStreamReader>

namespace IOManager {
XMLDocumentLoader::XMLDocumentLoader(const QString &input)
  : DocumentLoader(input)
  , _reader(std::make_unique<QXmlStreamReader>(_input))
{}

XMLDocumentLoader::~XMLDocumentLoader() = default;

/*
  The reading loops below stop on the end of the document as well as on a
  parse error: tell them apart, otherwise a truncated file silently gives a
  truncated document.
*/
void
XMLDocumentLoader::reportError() const
{
  if (_reader->hasError()) {
    std::cerr << "ERROR: unable to read the document: "
	      << _reader->errorString().toStdString() << " (line "
	      << _reader->lineNumber() << ", column " << _reader->columnNumber()
	      << ")\n";
  }
}

int
XMLDocumentLoader::readIntAttribute(const QString &name,
				    int minValue,
				    int maxValue) const
{
  const QString value = _reader->attributes().value(name).toString();
  if (value.isEmpty()) {
    return 0;
  }

  bool ok = false;
  const int v = value.toInt(&ok);
  if (! ok) {
    std::cerr << "Warning: attribute " << name.toStdString() << "=\""
	      << value.toStdString() << "\" is not an integer (line "
	      << _reader->lineNumber() << ")\n";
    return 0;
  }

  if (v < minValue || v > maxValue) {
    std::cerr << "Warning: attribute " << name.toStdString() << "=" << v
	      << " is out of [" << minValue << "; " << maxValue
	      << "] (line " << _reader->lineNumber() << "): it is clamped\n";
    return std::min(std::max(v, minValue), maxValue);
  }

  return v;
}

void
XMLDocumentLoader::createNewDocument()
{
  DocumentLoader::createNewDocument();
  //B: the reader built by the constructor used to be leaked here
  _reader = std::make_unique<QXmlStreamReader>(_input);

  int w = 0;
  int h = 0;

  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == QLatin1String("document")) &&
         !_reader->atEnd()) {
    if (_reader->name() == QLatin1String("document") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      w = readIntAttribute(QStringLiteral("width"), 0);
      h = readIntAttribute(QStringLiteral("height"), 0);
      break;
    }
    _reader->readNext();
  }
  reportError();

  qDebug() << " loader w,h = " << w << ", " << h;
  _output->setPageWidth(w);
  _output->setPageHeight(h);
}

void
XMLDocumentLoader::buildStyles()
{
  //Reading styles from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == QLatin1String("styles")) &&
         !_reader->atEnd()) {
    if (_reader->name() == QLatin1String("style") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      _output->addStyle(buildStyle());
    }
    _reader->readNext();
  }

  reportError();
}

void
XMLDocumentLoader::buildContent()
{
  //Reading document from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == QLatin1String("content")) &&
         !_reader->atEnd()) {
    //QString name = _reader->name().toString();
    //QXmlStreamReader::TokenType token = _reader->tokenType();

    if (_reader->name() == QLatin1String("page") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      _output->add(buildPage());
    }
    _reader->readNext();
  }

  reportError();
}

//protected:
Doc::DocStyle *
XMLDocumentLoader::buildStyle()
{
  Doc::DocStyle *s = new Doc::DocStyle(
    _reader->attributes().value(QStringLiteral("name")).toString());

  //Reading style from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == QLatin1String("style")) &&
         !_reader->atEnd()) {
    if (_reader->name() == QLatin1String("font") &&
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
           _reader->name() == QLatin1String("page")) &&
         !_reader->atEnd()) {
    if (_reader->name() == QLatin1String("textBlock") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      p->add(buildTextBlock());
    }
    if (_reader->name() == QLatin1String("imageBlock") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      p->add(buildImageBlock());
    }
    _reader->readNext();
  }

  return p;
}

Doc::DocImageBlock *
XMLDocumentLoader::buildImageBlock()
{
  auto ib = new Doc::DocImageBlock(
    _reader->attributes().value(QStringLiteral("filePath")).toString());

  ib->setX(readIntAttribute(QStringLiteral("x")));
  ib->setY(readIntAttribute(QStringLiteral("y")));
  ib->setWidth(readIntAttribute(QStringLiteral("width"), 0));
  ib->setHeight(readIntAttribute(QStringLiteral("height"), 0));

  return ib;
}

Doc::DocTextBlock *
XMLDocumentLoader::buildTextBlock()
{
  auto tb = new Doc::DocTextBlock(_output);

  tb->setX(readIntAttribute(QStringLiteral("x")));
  tb->setY(readIntAttribute(QStringLiteral("y")));
  tb->setWidth(readIntAttribute(QStringLiteral("width"), 0));
  tb->setHeight(readIntAttribute(QStringLiteral("height"), 0));
  tb->setMarginTop(readIntAttribute(QStringLiteral("marginTop"), 0));
  tb->setMarginBottom(readIntAttribute(QStringLiteral("marginBottom"), 0));
  tb->setMarginLeft(readIntAttribute(QStringLiteral("marginLeft"), 0));
  tb->setMarginRight(readIntAttribute(QStringLiteral("marginRight"), 0));

  //Reading textblock content from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == QLatin1String("textBlock")) &&
         !_reader->atEnd()) {
    if (_reader->name() == QLatin1String("paragraph") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      tb->add(buildParagraph());
    }
    _reader->readNext();
  }

  return tb;
}

Doc::DocParagraph *
XMLDocumentLoader::buildParagraph()
{
  auto p = new Doc::DocParagraph(_output);

  p->setLineSpacing(readIntAttribute(QStringLiteral("lineSpacing"), 0));
  p->setTabulationSize(readIntAttribute(QStringLiteral("tabulationSize"), 0));

  //Reading textblock content from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == QLatin1String("paragraph")) &&
         !_reader->atEnd()) {
    if (_reader->name() == QLatin1String("string") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      p->add(buildString());
    }
    _reader->readNext();
  }

  return p;
}

Doc::DocString *
XMLDocumentLoader::buildString()
{
  auto *s = new Doc::DocString(
    new Doc::DocStyle(
      _reader->attributes().value(QStringLiteral("style")).toString()),
    _output);

  //Reading textblock content from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == QLatin1String("string")) &&
         !_reader->atEnd()) {
    if (_reader->name() == QLatin1String("char") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      s->add(buildCharacter());
    }
    _reader->readNext();
  }

  return s;
}

Doc::DocCharacter *
XMLDocumentLoader::buildCharacter()
{
  const int id = readIntAttribute(QStringLiteral("id"),
				  0,
				  std::numeric_limits<int>::max());
  const QString display =
    _reader->attributes().value(QStringLiteral("display")).toString();
  const int x = readIntAttribute(QStringLiteral("x"));
  const int y = readIntAttribute(QStringLiteral("y"));
  const int w = readIntAttribute(QStringLiteral("width"), 0);
  const int h = readIntAttribute(QStringLiteral("height"), 0);
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
