#include "DocumentController.hpp"

#include <cassert>
#include <iostream>

#include <QApplication>
#include <QClipboard>
#include <QXmlSchema>
#include <QXmlSchemaValidator>

#include <core/configurationmanager.h>
#include <iomanager/documentloaderdirector.h>
#include <iomanager/documentsaverdirector.h>
#include <iomanager/xmldocumentloader.h>
#include <iomanager/xmldocumentsaver.h>

#include "ADocumentView.hpp"
#include "DocumentView.hpp"
#include "appconstants.h"

#include "GridPageLayout.hpp"
#include <Lipsum4Qt.hpp>
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
  if (_document == nullptr)
    return -1;
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr)
    return -1;

  return currentTextBlock->offset();
}

int
DocumentController::getPageWidth() const
{
  if (_document == nullptr)
    return -1;
  return _document->pageWidth();
}

int
DocumentController::getPageHeight() const
{
  if (_document == nullptr)
    return -1;
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
  if (_document == nullptr)
    return -1;
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr)
    return -1;

  return block->marginTop();
}

int
DocumentController::getBlockMarginBottom() const
{
  if (_document == nullptr)
    return -1;
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr)
    return -1;

  return block->marginBottom();
}

int
DocumentController::getBlockMarginLeft() const
{
  if (_document == nullptr)
    return -1;
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr)
    return -1;

  return block->marginLeft();
}

int
DocumentController::getBlockMarginRight() const
{
  if (_document == nullptr)
    return -1;
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr)
    return -1;

  return block->marginRight();
}

/* Setters */
void
DocumentController::setPageWidth(int pageWidth)
{
  if (_document == nullptr)
    return;

  _document->setPageWidth(pageWidth);

  assert(_view);
  _view->drawElement(_document, true);

  setModified();
}

void
DocumentController::setPageHeight(int pageHeight)
{
  if (_document == nullptr)
    return;

  _document->setPageHeight(pageHeight);

  assert(_view);
  _view->drawElement(_document, true);

  setModified();
}

void
DocumentController::setParagraphLineSpacing(int lineSpacing)
{
  if (_document == nullptr)
    return;
  Doc::DocParagraph *currentParagraph = _document->currentParagraph();
  if (currentParagraph == nullptr)
    return;

  currentParagraph->setLineSpacing(lineSpacing);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::setBlockMarginTop(int marginTop)
{
  if (_document == nullptr)
    return;
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr)
    return;

  block->setMarginTop(marginTop);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::setBlockMarginBottom(int marginBottom)
{
  if (_document == nullptr)
    return;
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr)
    return;

  block->setMarginBottom(marginBottom);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::setBlockMarginLeft(int marginLeft)
{
  if (_document == nullptr)
    return;
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr)
    return;

  block->setMarginLeft(marginLeft);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::setBlockMarginRight(int marginRight)
{
  if (_document == nullptr)
    return;
  Doc::Block *block = _document->currentBlock();
  if (block == nullptr)
    return;

  block->setMarginRight(marginRight);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}
void
DocumentController::setBlockGeometry(int x, int y, int width, int height)
{
  if (_document == nullptr)
    return;
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
  if (_document == nullptr)
    return;
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr)
    return;

  currentTextBlock->setOffset(value);
  assert(_view);
  _view->setOffset(currentTextBlock->offset());

  Doc::DocStyle *currentStyle = currentTextBlock->getStyle();
  if (currentStyle != nullptr &&
      (Context::FontContext::instance()->getCurrentFont()->getName() !=
       currentStyle->getFontName()))
    Context::FontContext::instance()->setCurrentFont(
      currentStyle->getFontName());

  _view->drawElement(_document, false);
}

void
DocumentController::setCurrentBlock(Doc::Block *block)
{
  Doc::Page *page = _document->currentPage();
  if (page == nullptr)
    return;
  page->setCurrentBlock(block);

  assert(_view);
  _view->drawElement(_document, false);

  notifyAll();
}

void
DocumentController::addCharacter(const QString &s, int id)
{
  if (_document == nullptr)
    return;
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr)
    return;

  QString fontName;
  Doc::DocStyle *style = currentTextBlock->getStyle();
  if (style == nullptr) {
    Models::Font *f = Context::FontContext::instance()->getCurrentFont();
    if (f == nullptr)
      return;
    style = new Doc::DocStyle(f->getName(), f->getName());
    assert(_document);
    _document->addStyle(style);
  }
  assert(style);
  fontName = style->getName();

  Models::Font *font = Context::FontContext::instance()->getFont(fontName);
  if (_view == nullptr || font == nullptr)
    return;

  Models::Character *character = font->getCharacter(s);
  if (character == nullptr)
    return;
  if (id == -1)
    id = character->getRandomCharacterData()->getId();

  assert(_document);
  _document->add(new Doc::DocCharacter(s, id, _document));
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::addCharacters(const QList<QString> &charList)
{
  //std::cerr<<"DocumentController::addCharacters _document="<<_document<<" currentBlock="<<(_document ? _document->currentBlock() : nullptr)<<"\n";

  if (_document == nullptr)
    return;

  //B:TODO:DESIGN: wrong ! It can fail without notice !!!

  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr)
    return;

  //std::cerr<<"DC::addCharacters currentTextBlock x="<<currentTextBlock->x()<<" y="<<currentTextBlock->y()<<" w="<<currentTextBlock->width()<<" h="<<currentTextBlock->height()<<"\n";

  QString fontName;
  Doc::DocStyle *style = _document->getStyle();
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
  fontName = style->getName();
  Models::Font *font = Context::FontContext::instance()->getFont(fontName);
  if (_view == nullptr || font == nullptr)
    return;

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

    Models::Character *character = font->getCharacter(s);

    if (character != nullptr) {

      if (_cursorBaseLine >=
          (currentTextBlock->height() - currentTextBlock->marginBottom())) {
        //not enough place in block
        break;
      }

      //B: no (direct) dynamic allocation till we are sure that the character will fit in the text block
      const Models::CharacterData *charData =
        character->getRandomCharacterData();
      assert(charData);

      qreal right = _cursorRightLine +
                    (charData->width() *
                     (character->getRightLine() - character->getLeftLine())) /
                      100;
      if (right >=
          (currentTextBlock->width() - currentTextBlock->marginRight())) {
        //Not enough space on line to add character
        _cursorBaseLine += lineSpacing;
        _cursorRightLine = currentTextBlock->marginLeft();
        right = _cursorRightLine +
                (charData->width() *
                 (character->getRightLine() - character->getLeftLine())) /
                  100;
        if (right >=
              (currentTextBlock->width() - currentTextBlock->marginRight()) &&
            _cursorBaseLine >=
              (currentTextBlock->height() - currentTextBlock->marginBottom())) {
          //not enough space on new line
          break;
        }
      }
      //const qreal base = _cursorBaseLine - (charData->height() * character->getBaseLine())/100;
      //const qreal left = _cursorRightLine - (charData->width() * character->getLeftLine())/100;
      Doc::DocCharacter *docChar =
        new Doc::DocCharacter(s, charData->getId(), _document);
      _document->add(docChar);
      _cursorRightLine = right;
    } else {
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
  if (_document == nullptr)
    return;
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr)
    return;

  auto p = new Doc::DocParagraph(_document);
  currentTextBlock->add(p);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::addTextBlock(int x, int y, int w, int h)
{
  if (_document == nullptr)
    return;

  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr)
    return;

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
  if (_document == nullptr)
    return;
  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr)
    return;

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
  if (_document == nullptr)
    return;
  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr)
    return;

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
  if (image.isNull())
    return; //B

  if (_document == nullptr)
    return;

  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr)
    return;

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
  if (_document == nullptr)
    return;
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr)
    return;

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
  if (_document == nullptr)
    return;
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
  if (currentTextBlock == nullptr)
    return;

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
  if (_document == nullptr)
    return;
  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr)
    return;
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(currentPage->currentBlock());
  if (currentTextBlock == nullptr)
    return;

  currentTextBlock->removeSelection();

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::removeCurrentBlock()
{
  if (_document == nullptr)
    return;
  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr)
    return;
  Doc::Block *currentBlock = _document->currentBlock();
  if (currentBlock == nullptr)
    return;

  currentPage->remove(currentBlock);

  assert(_view);
  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::copy()
{
  if (_document == nullptr)
    return;
  Doc::Page *currentPage = _document->currentPage();
  if (currentPage == nullptr)
    return;
  Doc::DocTextBlock *currentTextBlock =
    dynamic_cast<Doc::DocTextBlock *>(currentPage->currentBlock());
  if (currentTextBlock == nullptr)
    return;

  Doc::DocTextBlock *selectionBlock = currentTextBlock->getSelection();
  if (selectionBlock == nullptr)
    return;

  auto selectionDoc = new Doc::Document();
  for (Doc::DocStyle *s : _document->getStyles())
    selectionDoc->addStyle(s);
  auto page = new Doc::Page(selectionDoc);
  selectionDoc->add(page);
  page->add(selectionBlock);

  IOManager::XMLDocumentSaver docSaver(selectionDoc);
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

void
DocumentController::paste()
{
  //TODO: tester le clipboard pour coller un texte ou coller un xml
  const QString toPaste = QApplication::clipboard()->text();
  bool isValidXml = false;

  QString XsdPath = Core::ConfigurationManager::get(
                      AppConfigMainGroup, AppConfigXmlCheckerFolderKey)
                      .toString() +
                    Core::ConfigurationManager::get(
                      AppConfigMainGroup, AppConfigDocumentXSDCheckerKey)
                      .toString();
  QUrl schemaUrl(XsdPath);
  QXmlSchema schema;
  schema.load(schemaUrl);
  if (schema.isValid()) {
    QXmlSchemaValidator validator(schema);
    isValidXml = validator.validate(toPaste.toUtf8());
  }

  if (!isValidXml) {
    //Plain text
    QList<QString> charList;

    for (QString::const_iterator its = toPaste.begin(); its != toPaste.end();
         ++its) {
      QString s = QString((*its).unicode());

      QChar qc = QChar(s.unicode()->unicode());
      qDebug() << qc << " " << s.unicode() << " " << s.unicode()->unicode();

      if (s.unicode()->unicode() == 10) { // Enter 10
        //                if(charList.size()==1 && (charList.at(0)==" " ||
        //                charList.at(0)=="")) charList.clear();
        //                else{
        addParagraph();
        addCharacters(charList);
        charList.clear();
        //                }
      } else {
        charList << QString((*its).unicode());
      }
    }
    addCharacters(charList);
  } else {
    //Xml
    if (_document == nullptr)
      return;
    Doc::Page *currentPage = _document->currentPage();
    if (currentPage == nullptr)
      return;
    Doc::DocTextBlock *currentTextBlock =
      dynamic_cast<Doc::DocTextBlock *>(currentPage->currentBlock());
    if (currentTextBlock == nullptr)
      return;
    //Doc::DocParagraph* currentParagraph = currentTextBlock->currentParagraph();

    IOManager::XMLDocumentLoader docLoader(toPaste);
    IOManager::DocumentLoaderDirector docLoaderDirector(&docLoader);
    docLoaderDirector.constructDocument();

    Doc::Document *docToPaste = docLoaderDirector.getDocument();

    QList<Doc::DocString *> stringsToPaste;
    stringsToPaste.reserve(20); //arbitrary
    for (Doc::Page *page : docToPaste->getPages()) {
      for (Doc::DocTextBlock *textBlock : page->getTextBlocks()) {
        for (Doc::DocParagraph *paragraph : textBlock->getParagraphs())
          stringsToPaste.append(paragraph->getStrings());
      }
    }
    stringsToPaste.pop_back();
    for (Doc::DocString *s : stringsToPaste) {
      QList<Doc::DocCharacter *> c = s->getCharacters();
      if (c.count() == 1 && c.at(0)->getDisplay() == QLatin1String("\n"))
        currentTextBlock->add(new Doc::DocParagraph(s->getDocument()));
      else
        currentTextBlock->add(s);
    }
  }

  _view->drawElement(_document, false);

  setModified();
}

void
DocumentController::exportToImage(const QString &imagePath)
{
  assert(_view);
  _view->saveToImage(imagePath);
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
    if (font == nullptr)
      return;
    assert(font);
    QString fontname = font->getName();
    if (_document == nullptr)
      return;
    Doc::DocTextBlock *current =
      dynamic_cast<Doc::DocTextBlock *>(_document->currentBlock());
    if (current != nullptr) {
      Doc::DocStyle *style = current->getStyle();
      if (style == nullptr)
        return;
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
  if (currentTextBlock == nullptr)
    return;
  _cursorBaseLine = currentTextBlock->marginTop();
  _cursorRightLine = currentTextBlock->marginLeft();
}
