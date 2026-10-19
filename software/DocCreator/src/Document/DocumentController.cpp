#include "DocumentController.hpp"

#include <cassert>
#include <memory>
#include <iostream>

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QXmlStreamReader>

#include <core/configurationmanager.h>
#include <iomanager/documentloaderdirector.h>
#include <iomanager/documentsaverdirector.h>
#include <iomanager/xmldocumentloader.h>
#include <iomanager/xmldocumentsaver.h>

#include "ADocumentView.hpp"
#include "DocumentView.hpp"
#include "appconstants.h"

#include <Utils/connectedcomponentextraction.h>

DocumentController::DocumentController()
  : _view(nullptr)
  , _document(nullptr)
  , _baseLineVisibility(false)
  , _cursorBaseLine(0)
  , _cursorRightLine(0)
{
  initialize(nullptr, nullptr);
}

DocumentController::DocumentController(Doc::Document *document)
  : _view(nullptr)
  , _document(nullptr)
  , _baseLineVisibility(false)
  , _cursorBaseLine(0)
  , _cursorRightLine(0)
{
  initialize(nullptr, document);
}

void
DocumentController::setDocument(Doc::Document *document)
{
  //B:warning: client responsibility to delete previous document

  _document = document;

  if (_document != nullptr) {
    if (_document->pageWidth() <= 0 || _document->pageHeight() <= 0) {
      const int w =
        Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigPageSizeX)
          .toInt();
      const int h =
        Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigPageSizeY)
          .toInt();

      _document->setPageWidth(w);
      _document->setPageHeight(h);
    }
  }

  assert(_view);
  _view->drawElement(_document, true);

  Context::DocumentContext::instance()->setModified(false);
}

/* Getters */
int
DocumentController::getOffset() const
{
  if (_document == nullptr) {
    return -1;
  }
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr) {
    return -1;
  }

  return currentTextBlock->offset();
}

int
DocumentController::getPageWidth() const
{
  if (_document == nullptr) {
    return -1;
  }
  return _document->pageWidth();
}

int
DocumentController::getPageHeight() const
{
  if (_document == nullptr) {
    return -1;
  }
  return _document->pageHeight();
}

int
DocumentController::getParagraphLineSpacing() const
{
  if (_document == nullptr) {
    //std::cerr<<"DocumentController::getParagraphLineSpacing() no current document\n";
    return -1;
  }
  Doc::DocParagraph *currentParagraph = _document->currentParagraph();
  if (currentParagraph == nullptr) {
    //std::cerr<<"DocumentController::getParagraphLineSpacing() no current paragraph\n";
    return -1;
  }

  return currentParagraph->lineSpacing();
}

int
DocumentController::getBlockMarginTop() const
{
  if (_document == nullptr) {
    return -1;
  }
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr) {
    return -1;
  }

  return block->marginTop();
}

int
DocumentController::getBlockMarginBottom() const
{
  if (_document == nullptr) {
    return -1;
  }
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr) {
    return -1;
  }

  return block->marginBottom();
}

int
DocumentController::getBlockMarginLeft() const
{
  if (_document == nullptr) {
    return -1;
  }
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr) {
    return -1;
  }

  return block->marginLeft();
}

int
DocumentController::getBlockMarginRight() const
{
  if (_document == nullptr) {
    return -1;
  }
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr) {
    return -1;
  }

  return block->marginRight();
}

/* Setters */
void
DocumentController::setPageWidth(int pageWidth)
{
  if (_document == nullptr) {
    return;
  }

  _document->setPageWidth(pageWidth);

  assert(_view);
  _view->drawElement(_document, true);

  setModified();
}

void
DocumentController::setPageHeight(int pageHeight)
{
  if (_document == nullptr) {
    return;
  }

  _document->setPageHeight(pageHeight);

  assert(_view);
  _view->drawElement(_document, true);

  setModified();
}

void
DocumentController::setParagraphLineSpacing(int lineSpacing)
{
  if (_document == nullptr) {
    return;
  }
  Doc::DocParagraph *currentParagraph = _document->currentParagraph();
  if (currentParagraph == nullptr) {
    return;
  }

  currentParagraph->setLineSpacing(lineSpacing);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::setBlockMarginTop(int marginTop)
{
  if (_document == nullptr) {
    return;
  }
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr) {
    return;
  }

  block->setMarginTop(marginTop);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::setBlockMarginBottom(int marginBottom)
{
  if (_document == nullptr) {
    return;
  }
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr) {
    return;
  }

  block->setMarginBottom(marginBottom);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::setBlockMarginLeft(int marginLeft)
{
  if (_document == nullptr) {
    return;
  }
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr) {
    return;
  }

  block->setMarginLeft(marginLeft);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::setBlockMarginRight(int marginRight)
{
  if (_document == nullptr) {
    return;
  }
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr) {
    return;
  }

  block->setMarginRight(marginRight);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}
void
DocumentController::setBlockGeometry(int x, int y, int width, int height)
{
  if (_document == nullptr) {
    return;
  }
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr) {
    qDebug() << "Block Null";
    return;
  }
  //qDebug() << " documentcontroller:: Block OK";

  block->setX(x);
  block->setY(y);

  block->setWidth(width);
  block->setHeight(height);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::setBaseLineVisibility(bool baseLineVisibility)
{
  _baseLineVisibility = baseLineVisibility;
  assert(_view);
  _view->drawElement(_document, false);
}

void
DocumentController::setView(DocumentView *view)
{
  _view = view;
}

void
DocumentController::setOffset(int value)
{
  if (_document == nullptr) {
    return;
  }
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr) {
    return;
  }

  currentTextBlock->setOffset(value);
  assert(_view);
  _view->setOffset(currentTextBlock->offset());

  Doc::DocStyle *currentStyle = currentTextBlock->getStyle();
  if (currentStyle != nullptr &&
      (Context::FontContext::instance()->getCurrentFont()->getName() !=
       currentStyle->getFontName())) {
    Context::FontContext::instance()->setCurrentFont(
      currentStyle->getFontName());
  }

  _view->drawElement(_document, false);
}

void
DocumentController::setCurrentBlock(Doc::Block *block)
{
  Doc::Page *page = _document->currentPage();
  if (page == nullptr) {
    return;
  }
  page->setCurrentBlock(block);

  assert(_view);
  _view->drawElement(_document, false);

  notifyAll();
}

void
DocumentController::addCharacter(const QString &s, int id)
{
  if (_document == nullptr) {
    return;
  }
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr) {
    return;
  }

  QString fontName;
  Doc::DocStyle *style = currentTextBlock->getStyle();
  if (style == nullptr) {
    Models::Font *f = Context::FontContext::instance()->getCurrentFont();
    if (f == nullptr) {
      return;
    }
    style = new Doc::DocStyle(f->getName(), f->getName());
    assert(_document);
    _document->addStyle(style);
  }
  assert(style);
  //B: the name of the style was taken as the name of its font, which the
  // rendering takes from getFontName(): with a style named otherwise (a
  // loaded document), nothing could be typed
  fontName = style->getFontName();

  Models::Font *font = Context::FontContext::instance()->getFont(fontName);
  if (_view == nullptr || font == nullptr) {
    return;
  }

  const Models::Character *character = font->getCharacter(s);
  if (character == nullptr) {
    return;
  }
  if (id == -1) {
    id = character->getRandomCharacterData().getId();
  }

  assert(_document);
  _document->add(new Doc::DocCharacter(s, id, _document));
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::addCharacters(const QList<QString> &charList)
{
  //std::cerr<<"DocumentController::addCharacters _document="<<_document<<" currentBlock="<<(_document ? _document->currentBlock() : nullptr)<<"\n";

  if (_document == nullptr) {
    return;
  }

  //B:TODO:DESIGN: wrong ! It can fail without notice !!!

  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr) {
    return;
  }

  //std::cerr<<"DC::addCharacters currentTextBlock x="<<currentTextBlock->x()<<" y="<<currentTextBlock->y()<<" w="<<currentTextBlock->width()<<" h="<<currentTextBlock->height()<<"\n";

  QString fontName;
  //B: the last style of the document was taken, not the one of the block
  // the characters go to, as addCharacter() does
  Doc::DocStyle *style = currentTextBlock->getStyle();
  if (style == nullptr) {
    style = _document->getStyle();
  }
  if (style == nullptr) {
    Models::Font *f = Context::FontContext::instance()->getCurrentFont();
    if (f == nullptr) {
      std::cerr << "DocumentController::addCharacters() nothing done because "
                   "no current font !!!\n";
      return;
    }
    style = new Doc::DocStyle(f->getName(), f->getName());

    assert(_document);
    _document->addStyle(style);
  }

  assert(style);
  fontName = style->getFontName(); //B: was getName(), cf addCharacter()
  Models::Font *font = Context::FontContext::instance()->getFont(fontName);
  if (_view == nullptr || font == nullptr) {
    return;
  }

  const QString returnStr = QStringLiteral("\n");

  assert(_document);
  const int lineSpacing = getParagraphLineSpacing();

  const int nbParagraphs = currentTextBlock->getParagraphs().size();
  if (nbParagraphs == 1) {
    //This is the first paragraph
    if (_cursorBaseLine == currentTextBlock->marginTop() &&
        _cursorRightLine == currentTextBlock->marginLeft()) {
      //This is the first time we add characters
      // We set _cursorBaseline correctly
      _cursorBaseLine = currentTextBlock->marginTop() + lineSpacing;

      //TODO: the first baseline should be at  currentTextBlock->marginTop()+computeBestAboveBaselineHeight() !
      //see comment in StructureDialog::loremIpsum()
      //Where should this bestAboveBaselineHeight be stored ? In DocParagraph ? In TextBlock ?
    }
  }

  //Warning: CODE DUPLICATION with GraphicsTextBlockItem::drawCharacter() !!!

  //
  for (const QString &s : charList) {

    const Models::Character *character = font->getCharacter(s);

    if (character != nullptr) {

      if (_cursorBaseLine >=
          (currentTextBlock->height() - currentTextBlock->marginBottom())) {
        //not enough place in block
        break;
      }

      //B: no (direct) dynamic allocation till we are sure that the character will fit in the text block
      const Models::CharacterData &charData =
        character->getRandomCharacterData();

      qreal right = _cursorRightLine +
                    (charData.width() *
                     (character->getRightLine() - character->getLeftLine())) /
                      100;
      //B: as GraphicsTextBlockItem::drawCharacter(): the character goes to
      // the next line only if the line already has something, and a
      // character wider than the block is put alone on its line. It was
      // only refused when it was both too wide and below the block: a
      // character below the block was added when it was not too wide.
      if (right >=
            (currentTextBlock->width() - currentTextBlock->marginRight()) &&
          _cursorRightLine > currentTextBlock->marginLeft()) {
        //Not enough space on line to add character
        _cursorBaseLine += lineSpacing;
        _cursorRightLine = currentTextBlock->marginLeft();
        right = _cursorRightLine +
                (charData.width() *
                 (character->getRightLine() - character->getLeftLine())) /
                  100;
        if (_cursorBaseLine >=
            (currentTextBlock->height() - currentTextBlock->marginBottom())) {
          //no space for a new line
          break;
        }
      }
      //const qreal base = _cursorBaseLine - (charData.height() * character->getBaseLine())/100;
      //const qreal left = _cursorRightLine - (charData.width() * character->getLeftLine())/100;
      Doc::DocCharacter *docChar =
        new Doc::DocCharacter(s, charData.getId(), _document);
      _document->add(docChar);
      _cursorRightLine = right;
    }
    else {
      //character not found in font
      //(it is the case for new line or tab in particular)

      if (s == returnStr) {
        //this is a "new line" character

        _cursorBaseLine += lineSpacing;
        _cursorRightLine = currentTextBlock->marginLeft();
        if (_cursorBaseLine >=
            (currentTextBlock->height() - currentTextBlock->marginBottom()))
          break;

        const int currentLineSpacing = getParagraphLineSpacing();
        this->addParagraph();
        setParagraphLineSpacing(currentLineSpacing);
      } else {
        qDebug() << "=>  character not found in Font: " << s;
      }
    }
  }

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::addString(const QString &s)
{
  //B: transform each character of string @a s into an individual QString...
  // It handles characters with accent, that can be on two (consecutive) QChars.
  // It also transform tab characters in several space characters.

  const QString tabStr = QStringLiteral("\t");

  QList<QString> characters;
  const int len = s.length();
  characters.reserve(len);
  for (int i = 0; i < len; ++i) {
    //B: a character out of the BMP (two QChars, a surrogate pair) was cut
    // in two characters, neither in any font; and the \r of the line ends
    // of Windows was taken for a character
    if (s[i] == QLatin1Char('\r')) {
      continue;
    }
    if (s[i].isHighSurrogate() && i < len - 1 && s[i + 1].isLowSurrogate()) {
      characters.append(s.mid(i, 2));
      ++i;
      continue;
    }
    if (s[i] != tabStr) {
      if (i < len - 1 && s[i + 1].isMark()) {
        //there is another character after
        //and this an accent on current character
        QChar data[2] = { s[i], s[i + 1] };
        characters.append(QString(data, 2));
        ++i;
      } else {
        characters.append(QString(s[i]));
      }
    } else {
      //replace tab character by SIZE_TAB space characters.
      const int SIZE_TAB = 5;
      for (int k = 0; k < SIZE_TAB; ++k)
        characters.append(QChar(' '));
    }
  }

  addCharacters(characters);
}

void
DocumentController::addParagraph()
{
  if (_document == nullptr) {
    return;
  }
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr) {
    return;
  }

  auto p = new Doc::DocParagraph(_document);
  currentTextBlock->add(p);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::addTextBlock(int x, int y, int w, int h)
{
  if (_document == nullptr) {
    return;
  }

  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr) {
    return;
  }

  auto textBlock = new Doc::DocTextBlock(_document);
  textBlock->setMarginTop(0); //B: why are these values hard-coded ???
  textBlock->setMarginBottom(0);
  textBlock->setMarginLeft(10);
  textBlock->setMarginRight(0);
  textBlock->setHeight(h);
  textBlock->setWidth(w);
  textBlock->setX(x);
  textBlock->setY(y);

  currentPage->add(textBlock);
  currentPage->setCurrentBlock(textBlock);

  _cursorBaseLine = textBlock->marginTop();
  _cursorRightLine = textBlock->marginLeft();

  std::cerr << "************************ addTextBlock _cursorBaseLine="
            << _cursorBaseLine << " _cursorRightLine=" << _cursorRightLine
            << "\n";

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::addTextBlock()
{
  const int margin =
    std::min(50, (std::min(getPageWidth(), getPageHeight()) - 1) / 2);
  const int x = margin;
  const int y = margin;
  const int w = std::max(getPageWidth() - 2 * margin, 1);
  const int h = std::max(getPageHeight() - 2 * margin, 1);

  addTextBlock(x, y, w, h);
}

void
DocumentController::addImageBlock(const QString &imagePath)
{
  if (_document == nullptr) {
    return;
  }
  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr) {
    return;
  }

  auto imageBlock = new Doc::DocImageBlock(imagePath);
  currentPage->add(imageBlock);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::addTestBlock(const QString &imagePath,
                                 int x,
                                 int y,
                                 int w,
                                 int h)
{
  if (_document == nullptr) {
    return;
  }
  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr) {
    return;
  }

  auto testBlock = new Doc::DocTestBlock(imagePath, w, h, x, y);
  currentPage->add(testBlock);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::addComponentBlock(const QString &imagePath)
{
  QImage src(imagePath);
  addComponentBlock(src);
}

void
DocumentController::addComponentBlock(const QImage &image)
{
  if (image.isNull()) {
    return; //B
  }
  if (_document == nullptr) {
    return;
  }

  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr) {
    return;
  }

  QList<Doc::DocComponent *> listComponents =
    ConnectedComponentExtraction::getListComponents(image, _document);

  for (Doc::DocComponent *dc : listComponents) {
    auto componentBlock = new Doc::DocComponentBlock(_document);
    componentBlock->setY(dc->y());
    componentBlock->setX(dc->x());
    componentBlock->setHeight(dc->height());
    componentBlock->setWidth(dc->width());
    componentBlock->add(dc);

    currentPage->add(componentBlock);

    assert(_view);
    _view->drawElement(_document, false);

    setModified();
  }
}

void
DocumentController::removeAfterCursor()
{
  if (_document == nullptr) {
    return;
  }
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr) {
    return;
  }

  //if(currentTextBlock->getSelection() == nullptr)
  currentTextBlock->removeAfterCursor();
  /*else
 {
 removeSelection();
 return;
 }*/

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::removeBeforeCursor()
{
  if (_document == nullptr) {
    return;
  }
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr) {
    return;
  }

  //if(currentTextBlock->getSelection() == nullptr)
  currentTextBlock->removeBeforeCursor();
  /*else
 {
 removeSelection();
 return;
 }*/

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::removeSelection()
{
  if (_document == nullptr) {
    return;
  }
  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr) {
    return;
  }
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(currentPage->currentBlock());
  if (currentTextBlock == nullptr) {
    return;
  }

  currentTextBlock->removeSelection();

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::removeCurrentBlock()
{
  if (_document == nullptr) {
    return;
  }
  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr) {
    return;
  }
  Doc::Block *currentBlock = _document->currentBlock();
  if (currentBlock == nullptr) {
    return;
  }

  currentPage->remove(currentBlock);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::copy()
{
  if (_document == nullptr) {
    return;
  }
  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr) {
    return;
  }
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(currentPage->currentBlock());
  if (currentTextBlock == nullptr) {
    return;
  }

  Doc::DocTextBlock *selectionBlock = currentTextBlock->getSelection();
  if (selectionBlock == nullptr) {
    return;
  }

  //B: the document of the selection was never deleted. It held the styles
  // of _document, which it would have deleted with it: it holds copies.
  // (The strings of the selection refer to their style by its name.)
  const std::unique_ptr<Doc::Document> selectionDoc(new Doc::Document());
  for (Doc::DocStyle *s : _document->getStyles())
    selectionDoc->addStyle(new Doc::DocStyle(s->getName(), s->getFontName()));
  auto page = new Doc::Page(selectionDoc.get());
  selectionDoc->add(page);
  page->add(selectionBlock);

  IOManager::XMLDocumentSaver docSaver(selectionDoc.get());
  IOManager::DocumentSaverDirector docSaverDirector(&docSaver);
  docSaverDirector.constructDocumentOutput();

  QString documentOutput = docSaverDirector.getDocumentOutput();

  QApplication::clipboard()->setText(documentOutput);
}

void
DocumentController::cut()
{
  copy();
  removeSelection();
}

/*
  Return true if @a text is a document in XML, as copy() puts in the
  clipboard: its first element is <document>.
*/
static bool
isDocumentXml(const QString &text)
{
  QXmlStreamReader reader(text);
  while (!reader.atEnd()) {
    reader.readNext();
    if (reader.isStartElement()) {
      return reader.name() == QLatin1String("document");
    }
  }
  return false;
}

void
DocumentController::paste()
{
  const QString toPaste = QApplication::clipboard()->text();
  /*
    B: the XML was recognized by QXmlSchema, which Qt 6 does not have: the
    XML was never recognized, and Copy then Paste pasted its markup. The
    first element is checked instead (with Qt 5 too).

    The plain text added a paragraph at each line end, then the line before
    it: each line went into the paragraph after it. It also cut surrogate
    pairs, and did not join accents. addString() does all that, and makes
    the paragraphs at the line ends.
  */
  const bool isValidXml = isDocumentXml(toPaste);

  if (!isValidXml) {
    //Plain text
    addString(toPaste);
    return;
  }
  else {
    //Xml
    if (_document == nullptr) {
      return;
    }
    Doc::Page *currentPage = _document->currentPage();
    if (currentPage == nullptr) {
      return;
    }
    Doc::DocTextBlock *currentTextBlock =
      dynamic_cast<Doc::DocTextBlock *>(currentPage->currentBlock());
    if (currentTextBlock == nullptr) {
      return;
    }
    //Doc::DocParagraph* currentParagraph = currentTextBlock->currentParagraph();

    IOManager::XMLDocumentLoader docLoader(toPaste);
    IOManager::DocumentLoaderDirector docLoaderDirector(&docLoader);
    docLoaderDirector.constructDocument();

    const std::unique_ptr<Doc::Document> docToPaste = docLoaderDirector.getDocument();
    if (docToPaste == nullptr) { //B: XML that could not be loaded
      return;
    }

    QList<Doc::DocString *> stringsToPaste;
    stringsToPaste.reserve(20); //arbitrary
    for (Doc::Page *page : docToPaste->getPages()) {
      for (Doc::DocTextBlock *textBlock : page->getTextBlocks()) {
        for (Doc::DocParagraph *paragraph : textBlock->getParagraphs())
          stringsToPaste.append(paragraph->getStrings());
      }
    }
    //B: pop_back() of an empty list is undefined: pasted XML without text
    if (stringsToPaste.isEmpty()) {
      return;
    }
    stringsToPaste.pop_back();
    /*
      B: the strings themselves were added, with the paragraphs made for
      the line breaks, and docToPaste was then deleted: it deleted them,
      and the text block went on with freed strings, whose document and
      style were freed too. They are copied into _document instead.
    */
    for (Doc::DocString *s : stringsToPaste) {
      const QList<Doc::DocCharacter *> &c = s->getCharacters();
      if (c.count() == 1 && c.at(0)->getDisplay() == QLatin1String("\n")) {
        currentTextBlock->add(new Doc::DocParagraph(_document));
      }
      else {
        currentTextBlock->add(s->cloneInto(_document));
      }
    }
  }

  _view->drawElement(_document, false);

  setModified();
}

bool
DocumentController::exportToImage(const QString &imagePath)
{
  assert(_view);
  return _view->saveToImage(imagePath);
}

QImage
DocumentController::toQImage(DocRenderFlags flags)
{
  assert(_view);
  return _view->toQImage(flags);
}

QSize
DocumentController::getImageSize()
{
  assert(_view);
  return _view->getImageSize();
}

void
DocumentController::printDocument(QPrinter *printer)
{
  assert(_view);
  _view->print(printer);
}

/* Initializer */
void
DocumentController::initialize(DocumentView *view, Doc::Document *document)
{
  _view = view;
  _document = document;
  _baseLineVisibility = false;
}

void
DocumentController::update()
{
  if (Context::FontContext::instance()->fontHasChanged()) {
    Models::Font *font = Context::FontContext::instance()->getCurrentFont();
    if (font == nullptr) {
      return;
    }
    assert(font);
    const QString fontname = font->getName();
    if (_document == nullptr) {
      return;
    }
    Doc::DocTextBlock *current =
      dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
    if (current != nullptr) {
      Doc::DocStyle *style = current->getStyle();
      if (style == nullptr) {
        return;
      }
      if (fontname != style->getFontName()) {
        auto st = new Doc::DocStyle(fontname, fontname);
        current->changeStyle(st);
      }
    }
    if (Context::FontContext::instance()->fontCurrentModified()) {
      _view->drawElement(_document, true);
    }
  } else {
    _view->drawElement(_document, false);
  }

  setModified();
}

void
DocumentController::setModified()
{
  Context::DocumentContext::instance()->setModified(true);
  notifyAll();
}

/*
  Must be used for a new TextBlock, before addCharacters()
 */
void
DocumentController::resetCurrentTextBlockCursor()
{
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr) {
    return;
  }
  _cursorBaseLine = currentTextBlock->marginTop();
  _cursorRightLine = currentTextBlock->marginLeft();
}
