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
  /*
    B: the styles were read with _reader up to </styles>: without a
    <styles> element, or with it after <content>, the whole document was
    read here, buildContent() then found nothing, and the document was
    loaded empty with no error. The styles are now read by a reader of
    their own, wherever they are in the document, and _reader stays where
    buildContent() starts.
  */
  std::unique_ptr<QXmlStreamReader> contentReader = std::move(_reader);
  _reader = std::make_unique<QXmlStreamReader>(_input);
  while (!_reader->atEnd()) {
    if (_reader->name() == QLatin1String("style") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      _output->addStyle(buildStyle().release());
    }
    _reader->readNext();
  }
  //a parse error is reported by buildContent(), with the content reader
  _reader = std::move(contentReader);
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
      _output->add(buildPage().release());
    }
    _reader->readNext();
  }

  reportError();
}

//protected:
std::unique_ptr<Doc::DocStyle>
XMLDocumentLoader::buildStyle()
{
  auto s = std::make_unique<Doc::DocStyle>(
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

std::unique_ptr<Doc::Page>
XMLDocumentLoader::buildPage()
{
  auto p = std::make_unique<Doc::Page>(_output);
  p->setBackgroundFileName(_reader->attributes()
                             .value(QStringLiteral("backgroundFileName"))
                             .toString());

  //Reading style from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == QLatin1String("page")) &&
         !_reader->atEnd()) {
    if (_reader->name() == QLatin1String("textBlock") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      p->add(buildTextBlock().release());
    }
    if (_reader->name() == QLatin1String("imageBlock") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      p->add(buildImageBlock().release());
    }
    _reader->readNext();
  }

  return p;
}

std::unique_ptr<Doc::DocImageBlock>
XMLDocumentLoader::buildImageBlock()
{
  auto ib = std::make_unique<Doc::DocImageBlock>(
    _reader->attributes().value(QStringLiteral("filePath")).toString());

  ib->setX(readIntAttribute(QStringLiteral("x")));
  ib->setY(readIntAttribute(QStringLiteral("y")));
  ib->setWidth(readIntAttribute(QStringLiteral("width"), 0));
  ib->setHeight(readIntAttribute(QStringLiteral("height"), 0));

  return ib;
}

std::unique_ptr<Doc::DocTextBlock>
XMLDocumentLoader::buildTextBlock()
{
  auto tb = std::make_unique<Doc::DocTextBlock>(_output);

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
      tb->add(buildParagraph().release());
    }
    _reader->readNext();
  }

  return tb;
}

std::unique_ptr<Doc::DocParagraph>
XMLDocumentLoader::buildParagraph()
{
  auto p = std::make_unique<Doc::DocParagraph>(_output);

  p->setLineSpacing(readIntAttribute(QStringLiteral("lineSpacing"), 0));
  p->setTabulationSize(readIntAttribute(QStringLiteral("tabulationSize"), 0));

  //Reading textblock content from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == QLatin1String("paragraph")) &&
         !_reader->atEnd()) {
    if (_reader->name() == QLatin1String("string") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      p->add(buildString().release());
    }
    _reader->readNext();
  }

  return p;
}

std::unique_ptr<Doc::DocString>
XMLDocumentLoader::buildString()
{
  /*
    B: a style was allocated for every string, although the <styles>
    section has already registered them all: the document kept the first
    style of each name and the others were dropped.
  */
  const QString styleName =
    _reader->attributes().value(QStringLiteral("style")).toString();
  Doc::DocStyle *style =
    (_output != nullptr) ? _output->getStyle(styleName) : nullptr;
  if (style == nullptr) {
    style = new Doc::DocStyle(styleName);
  }

  auto s = std::make_unique<Doc::DocString>(style, _output);

  //Reading textblock content from xml
  while (!(_reader->tokenType() == QXmlStreamReader::EndElement &&
           _reader->name() == QLatin1String("string")) &&
         !_reader->atEnd()) {
    if (_reader->name() == QLatin1String("char") &&
        _reader->tokenType() == QXmlStreamReader::StartElement) {
      s->add(buildCharacter().release());
    }
    _reader->readNext();
  }

  return s;
}

std::unique_ptr<Doc::DocCharacter>
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
  auto c = std::make_unique<Doc::DocCharacter>(display, id, _output);
  c->setX(x);
  c->setY(y);
  c->setHeight(h);
  c->setWidth(w);
  //    qDebug() << id << " " << display << " " << x << " " << y << " " << w <<"
  //    " << h;
  return c;
}

} //namespace IOManager
