#include "GraphicsTextBlockItem.hpp"

#include <cassert>

#include <QGraphicsSceneMouseEvent>

#include "context/fontcontext.h"

#include "DocumentController.hpp"
#include "GraphicsCharacterItem.hpp"
#include "GraphicsCursorItem.hpp"
#include "GraphicsPageItem.hpp"

GraphicsTextBlockItem::GraphicsTextBlockItem(Mvc::IController *controller,
                                             QGraphicsItem *parent) :
  GraphicsBlockItem(parent),
  ADocumentView<Doc::DocTextBlock>(controller),
  _cursorBaseLine(0.0),
  _cursorRightLine(0.0),
  _charItems(),
  _cursorItem(nullptr),
  _mousePressed(false),
  _startSelectionOffset(0)
{
  _cursorItem = new GraphicsCursorItem();
  _cursorItem->setParentItem(this);
  _cursorItem->setPos(_cursorRightLine, _cursorBaseLine);
}

void
GraphicsTextBlockItem::clear()
{
  removeAllCharacterItem();
  _startSelectionOffset = 0;
  _cursorBaseLine = 0.0;
  _cursorRightLine = 0.0;
  _cursorItem->setPos(_cursorRightLine, _cursorBaseLine);
}

void
GraphicsTextBlockItem::setOffset(int value)
{
  for (GraphicsCharacterItem *charItem : _charItems) {
    if (charItem->index() == value) {
      setCursorOnCharacter(charItem, true);
      return;
    }
  }
}

void
GraphicsTextBlockItem::addCharacterItem(GraphicsCharacterItem *charItem)
{
  //if (charItem == nullptr) //B: does it happen ???
  //return;
  assert(charItem != nullptr);

  charItem->setParentItem(this);
  for (int i = charItem->index(); i < _charItems.count(); ++i) {
    GraphicsCharacterItem *e = _charItems.at(i);
    //if (e != nullptr) //B: does it happen ???
    assert(e != nullptr);
    e->setIndex(e->index() + 1);
  }
  _charItems.insert(charItem->index(), charItem);

  //bool isReturn = charItem->getCharacter()->getCharacterValue() == "\n";
  //setCursorOnCharacter(charItem, isReturn);
}

void
GraphicsTextBlockItem::removeCharacterItem(GraphicsCharacterItem *charItem)
{
  int index = _charItems.indexOf(charItem);
  //if (index == -1) //not found
  //return;
  assert(index != -1);

  assert(0 <= index && index < _charItems.size());
  _charItems.removeAt(index);
  delete charItem;
  for (int i = index; i < _charItems.count(); ++i) {
    GraphicsCharacterItem *e = _charItems.at(i);
    //if (e != nullptr) //B: deos it happen ?
    assert(e != nullptr);
    e->setIndex(e->index() - 1);
  }
}

void
GraphicsTextBlockItem::removeAllCharacterItem()
{
  //for (GraphicsCharacterItem *charItem : _charItems)
  //removeCharacterItem(charItem);

  //B:
  //We do not call removeCharacterItem as it delete item but also update index of others.
  //But as we will also delete them, it is not needed to update their index.
  //We just delete them;

  for (GraphicsCharacterItem *charItem : _charItems)
    delete charItem;
  _charItems.clear();
}

void
GraphicsTextBlockItem::setCursorOnCharacter(GraphicsCharacterItem *charItem,
                                            bool cursorBefore)
{
  const bool isReturn =
    (charItem->getCharacter()->getCharacterValue() == QLatin1String("\n"));
  cursorBefore = isReturn ? true : cursorBefore;

  const Models::Character *c = charItem->getCharacter();
  const int charImgWidth = charItem->pixmap().width();
  const int charImgHeight = charItem->pixmap().height();
  const int upLineExtra = charImgHeight * (-c->getUpLine() / 100);
  const int baseLineExtra = charImgHeight * ((c->getBaseLine() - 100) / 100);
  const int x = charItem->pos().x() + 1 + ((cursorBefore) ? 0 : charImgWidth);
  const int y = charItem->pos().y() - upLineExtra;

  //_docTextBlock->setOffset(charItem->index() + ((cursorBefore) ? 0 : 1));

  _cursorItem->setPos(x, y);
  _cursorItem->setCursorSize(charImgHeight + upLineExtra + baseLineExtra);
}

void
GraphicsTextBlockItem::hide(bool transparent)
{
  GraphicsBlockItem::hide(transparent);
  _cursorItem->hide();
}

void
GraphicsTextBlockItem::show()
{
  GraphicsBlockItem::show();
  _cursorItem->show();
}

void
GraphicsTextBlockItem::load()
{
  Doc::DocTextBlock *docTextBlock = getElement();
  if (docTextBlock != nullptr) {

    setPos(docTextBlock->x(), docTextBlock->y());
    setRect(QRectF(0, 0, docTextBlock->width(), docTextBlock->height()));

    updatePosition();
  }
}

void
GraphicsTextBlockItem::draw(bool /*complete*/)
{
  //qDebug() << "GraphicsTextBlockItem::draw("<<complete<<") *************\n";

  removeAllCharacterItem();

  //B: delete and recreate each item at each re-draw ????!

  Doc::DocTextBlock *docTextBlock = getElement();
  if (docTextBlock == nullptr) {
    return;
  }

  QList<Doc::DocParagraph *> paragraphs = docTextBlock->getParagraphs();

  //qDebug() << "GraphicsTextBlockItem::draw("<<complete<<")  paragraphs.size()="<<paragraphs.size()<<"\n";

  if (paragraphs.empty())
    return;

  Doc::DocParagraph *firstParagraph = paragraphs.first();
  if (firstParagraph == nullptr) { //B: does it happen ?
    return;
  }

  _cursorBaseLine = firstParagraph->lineSpacing() + docTextBlock->marginTop();
  _cursorRightLine = docTextBlock->marginLeft();

  int offset = 0;
  for (Doc::DocParagraph *p : paragraphs) {
    drawParagraph(p, offset);
    offset = offset + p->length();
    goNextLine(p);
  }
  setOffset(docTextBlock->offset());
  //setCursorFromOffset(docTextBlock->offset());
}

/* Private methods */
qreal
GraphicsTextBlockItem::findUpPosition(const Models::Character *character,
                                      int /*characterImageWidth*/,
                                      int characterImageHeight) const
{
  assert(character != nullptr);
  const qreal absolutePosition =
    (characterImageHeight *
     (character->getUpLine() - character->getBaseLine())) /
    100;
  return _cursorBaseLine - absolutePosition;
}

qreal
GraphicsTextBlockItem::findBasePosition(const Models::Character *character,
                                        int /*characterImageWidth*/,
                                        int characterImageHeight) const
{
  assert(character != nullptr);
  const qreal absolutePosition =
    (characterImageHeight * character->getBaseLine()) / 100;
  return _cursorBaseLine - absolutePosition;
}

qreal
GraphicsTextBlockItem::findLeftPosition(const Models::Character *character,
                                        int characterImageWidth,
                                        int /*characterImageHeight*/) const
{
  assert(character != nullptr);
  const qreal absolutePosition =
    (characterImageWidth * character->getLeftLine()) / 100;
  return _cursorRightLine - absolutePosition;
}

qreal
GraphicsTextBlockItem::findRightPosition(const Models::Character *character,
                                         int characterImageWidth,
                                         int /*characterImageHeight*/) const
{
  assert(character != nullptr);
  const qreal absolutePosition =
    (characterImageWidth *
     (character->getRightLine() - character->getLeftLine())) /
    100;
  return _cursorRightLine + absolutePosition;
}

void
GraphicsTextBlockItem::goNextLine(Doc::DocParagraph *p)
{
  /*if(((DocumentController*)getController())->baseLineVisibility())
    {
        GraphicsTextBlockItem * textBlock = _currentPageItem->getCurrentTextBlockItem();
        QGraphicsLineItem * baseline = new QGraphicsLineItem(QLineF(_cursorRightLine,
                                                                    _cursorBaseLine,
                                                                    textBlock->boundingRect().width(),
                                                                    _cursorBaseLine));
        baseline->setPen(QPen(QColor(192, 0, 0)));
        baseline->setZValue(200);
        baseline->setParentItem(textBlock);
    }*/

  Doc::DocTextBlock *docTextBlock = getElement();
  if (docTextBlock == nullptr) {
    return;
  }
  this->_cursorRightLine = docTextBlock->marginLeft();
  Doc::DocParagraph *docParagraph =
    (p != nullptr) ? p : docTextBlock->currentParagraph();
  if (docParagraph == nullptr) {
    return;
  }
  this->_cursorBaseLine += docParagraph->lineSpacing();
}

void
GraphicsTextBlockItem::drawParagraph(Doc::DocParagraph *p, int offset)
{
  for (Doc::DocString *s : p->getStrings()) {
    drawString(s, offset, p);
    offset = offset + s->length();
  }
}

void
GraphicsTextBlockItem::drawString(Doc::DocString *s,
                                  int offset,
                                  Doc::DocParagraph *p)
{
  // TO DO: re-order characters from left to right and from up to down
  // Can do it in DocumentFromXML function

  for (Doc::DocCharacter *c :
       s->getCharacters()) { // this for-loop draw characters by orde not by
                             // position
    drawCharacter(c, offset, s->getStyle(), p);
    offset = offset + c->length();
  }
}

void
GraphicsTextBlockItem::drawCharacter(Doc::DocCharacter *c,
                                     int offset,
                                     Doc::DocStyle *style,
                                     Doc::DocParagraph *p)
{
  Doc::DocTextBlock *docTextBlock = getElement();
  if (docTextBlock == nullptr) { //B: does it happen ?
    return;
  }

  assert(c);
  const QString cDisplay = c->getDisplay();

  //Special character for return, draw a transparent rect  //B: why is it necessary ????
  if (cDisplay == QLatin1String("\n")) {

    Doc::DocParagraph *docParagraph =
      (p != nullptr) ? p : docTextBlock->currentParagraph();
    if (docParagraph == nullptr) {
      return;
    }

    const int width = rect().width() - _cursorRightLine;
    const int height = docParagraph->lineSpacing();
    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::transparent); //release
    //B: DEBUG *******
    //image.fill(Qt::red);

    auto data = new Models::CharacterData(image, 0);
    auto character = new Models::Character();
    character->setCharacterValue(QStringLiteral("\n"));
    character->add(data);

    const bool mustDeleteCharacter =
      true; //This character is not in the font and thus shoud be deleted when GraphicsCharacterItem is.
    auto item =
      new GraphicsCharacterItem(character, 0, offset, mustDeleteCharacter);
    item->setPos(_cursorRightLine, _cursorBaseLine - height);

    //B: Do we really need to construct this transparent item ? Could we not just go to the next line ?
    //   Is it necessary in case of backspace ???
    item->setZValue(-200);
    addCharacterItem(item);

    return;
  }

  Models::Font *font = nullptr;
  if (style != nullptr) {
    font = Context::FontContext::instance()->getFont(style->getFontName());
  }
  if (font == nullptr) {
    font = Context::FontContext::instance()->getCurrentFont();
  }
  assert(font != nullptr);

  const Models::Character *character = font->getCharacter(cDisplay);
  if (character == nullptr) { //B: does it happen ?
    return;
  }
  const Models::CharacterData *data = character->getCharacterData(c->getId());
  if (data == nullptr) {
    return;
  }
  const QImage characterImage = data->getImage();

  //assert(character != nullptr);
  assert(!characterImage.isNull());

  //Warning: CODE DUPLICATION with DocumentController::addCharacters() !!!

  const qreal right = findRightPosition(
    character, characterImage.width(), characterImage.height());

  if (right >= (rect().width() - docTextBlock->marginRight())) {
    goNextLine(p);
    drawCharacter(c, offset, style);

    return;
  }

  //qDebug() << metaObject()->className() << " " << c->getDisplay() << " x, y" << c->x() << " " << c->y();

  //const qreal up = findUpPosition(character, characterImage.width(), characterImage.height());
  const qreal base = findBasePosition(
    character, characterImage.width(), characterImage.height());
  const qreal left = findLeftPosition(
    character, characterImage.width(), characterImage.height());

  //B:REM:
  //We do not check if the character fits according to block height.

  auto item = new GraphicsCharacterItem(character, c->getId(), offset);
  item->setPos(left, base);
  addCharacterItem(item);

  //Update DocCharacter position and size (to save in xml)
  c->setX(item->x());
  c->setY(item->y());
  c->setWidth(
    item->pixmap()
      .width()); //B:TODO:OPTIM? is it necessary to use pixmap() ? Could we not just use characterImage ?
  c->setHeight(item->pixmap().height());

  //Find the position where we have to put the next item/QPixmap
  this->_cursorRightLine = right;
}

void
GraphicsTextBlockItem::hightlightSelection(int startOffset, int endOffset)
{
  for (int i = startOffset; i < endOffset; i++) {
    if (i >= _charItems.count() || i < 0) {
      continue;
    }
    GraphicsCharacterItem *charItem =
      static_cast<GraphicsCharacterItem *>(_charItems.at(i)); //B:dynamic_cast?
    if (charItem != nullptr) {
      charItem->setHighlighted(true);
    }
  }
}

//Events
void
GraphicsTextBlockItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  GraphicsBlockItem::mousePressEvent(event);
  DocumentController *docController =
    static_cast<DocumentController *>(getController()); //B:dynamic_cast?
  if (docController != nullptr) {
    docController->setCurrentBlock(getElement());
  }

  QPointF mousePos = event->pos();

  for (GraphicsCharacterItem *charItem : _charItems)
    charItem->setHighlighted(false);

  Doc::DocTextBlock *docTextBlock = getElement();
  if (docTextBlock == nullptr) {
    return;
  }
  setCursorFromPosition(mousePos);
  _startSelectionOffset = docTextBlock->offset();

  _mousePressed = true;
}

void
GraphicsTextBlockItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

  GraphicsBlockItem::mouseMoveEvent(event);
  if (!_mousePressed) {
    return;
  }

  QPointF mousePos = event->pos();

  for (GraphicsCharacterItem *charItem : _charItems)
    charItem->setHighlighted(false);

  int startOffset = _startSelectionOffset;
  setCursorFromPosition(mousePos);
  Doc::DocTextBlock *docTextBlock = getElement();
  if (docTextBlock == nullptr) {
    return;
  }
  int endOffset = docTextBlock->offset();

  if (startOffset > endOffset) {
    int tmp = startOffset;
    startOffset = endOffset;
    endOffset = tmp;
  }

  hightlightSelection(startOffset, endOffset);
}

void
GraphicsTextBlockItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  GraphicsBlockItem::mouseReleaseEvent(event);
  if (!_mousePressed) {
    return;
  }

  QPointF mousePos = event->pos();
  setCursorFromPosition(mousePos);

  Doc::DocTextBlock *docTextBlock = getElement();
  if (docTextBlock == nullptr) {
    return;
  }

  Doc::DocStyle *style = docTextBlock->getStyle();
  if (style == nullptr) {
    return;
  }
  if (style->getFontName() !=
      Context::FontContext::instance()->getCurrentFont()->getName()) {
    Models::Font *font =
      Context::FontContext::instance()->getFont(style->getFontName());
    if (font != nullptr) {
      Context::FontContext::instance()->setCurrentFont(font->getName());
    }
  }

  int startOffset = _startSelectionOffset;
  int endOffset = docTextBlock->offset();

  if (startOffset > endOffset) {
    std::swap(startOffset, endOffset);
  }

  hightlightSelection(startOffset, endOffset);

  docTextBlock->select(startOffset, endOffset);
}

//Slots
//Overloaded
void
GraphicsTextBlockItem::updatePosition()
{
  DocumentController *docController =
    static_cast<DocumentController *>(getController()); //B:dynamic_cast?
  if (docController != nullptr) {
    docController->setCurrentBlock(getElement());
  }

  GraphicsBlockItem::updatePosition();
  Doc::DocTextBlock *docTextBlock = getElement();
  if (docTextBlock == nullptr)
    return;

  //qDebug() << " text block x, y " << docTextBlock->x() << " " << docTextBlock->y();

  //    for (Doc::DocParagraph* p : docTextBlock->getParagraphs())
  //        for (Doc::DocString* ds : p->getStrings())
  //            for (Doc::DocCharacter* c : ds->getCharacters())
  //                qDebug()<< c->getDisplay() << " x, y = " << c->x() << " " <<
  //                c->y();

  if (docController != nullptr) {
    docController->setBlockGeometry(x(), y(), rect().width(), rect().height());
    //qDebug() << "Changing text block position";
  }
  draw(false);

  //     DocTextBlock * docTextBlock1 = getElement();
  //     qDebug() << " text block 1 x, y " << docTextBlock1->x() << " " <<
  //     docTextBlock1->y();
  //    for (Doc::DocParagraph* p : docTextBlock1->getParagraphs())
  //        for (Doc::DocString* ds : p->getStrings())
  //            for (Doc::DocCharacter* c : ds->getCharacters())
  //                qDebug()<< c->getDisplay() << " x, y = " << c->x() << " " <<
  //                c->y();
}

/* private methods : */
bool
GraphicsTextBlockItem::clickOnFirstPart(GraphicsCharacterItem *charItem,
                                        QPointF pos)
{
  QRectF firstPart = charItem->boundingRect();
  firstPart.setRight(firstPart.right() / 2.);
  return firstPart.contains(mapToItem(charItem, pos));
}

void
GraphicsTextBlockItem::setCursorFromPosition(QPointF pos)
{
  for (GraphicsCharacterItem *charItem : _charItems) {
    if (charItem->boundingRect().contains(mapToItem(charItem, pos))) {
      const bool isReturn =
        (charItem->getCharacter()->getCharacterValue() == QLatin1String("\n"));
      const bool cursorBefore =
        isReturn ? true : clickOnFirstPart(charItem, pos);
      setCursorOnCharacter(charItem, cursorBefore);
      DocumentController *controller =
        static_cast<DocumentController *>(getController()); //B:dynamic_cast?
      if (controller != nullptr)
        controller->setOffset(charItem->index() + ((cursorBefore) ? 0 : 1));
      return;
    }
  }
}
