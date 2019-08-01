#include "GraphicsImageBlockItem.hpp"
#include "DocumentController.hpp"

#include <QGraphicsPixmapItem>

GraphicsImageBlockItem::GraphicsImageBlockItem(Mvc::IController *controller,
                                               QGraphicsItem *parent)
  : GraphicsBlockItem(parent)
  , ADocumentView<Doc::DocImageBlock>(controller)
{
  _pixmapItem = nullptr;
}

void
GraphicsImageBlockItem::clear()
{
  if (_pixmapItem != nullptr)
    _pixmapItem->setPixmap(QPixmap());
}

void
GraphicsImageBlockItem::setOffset(int /*value*/)
{
  //?
}

void
GraphicsImageBlockItem::setRect(const QRectF &rect)
{
  GraphicsBlockItem::setRect(rect);
  _scaledPixmap = _originalPixmap.scaled(QSize(rect.width(), rect.height()));

  if (_pixmapItem != nullptr) {
    _pixmapItem->setPixmap(_scaledPixmap);
    _pixmapItem->setPos(rect.topLeft());
  }
}

//protected:
void
GraphicsImageBlockItem::load()
{
  Doc::DocImageBlock *docImageBlock =
    dynamic_cast<Doc::DocImageBlock *>(getElement());
  if (docImageBlock == nullptr)
    return;
  _originalPixmap = QPixmap(docImageBlock->filePath());
  _pixmapItem = new QGraphicsPixmapItem();
  _pixmapItem->setParentItem(this);

  int width = docImageBlock->width();
  if (width == 0)
    docImageBlock->setWidth(_originalPixmap.width());
  int height = docImageBlock->height();
  if (height == 0)
    docImageBlock->setHeight(_originalPixmap.height());

  setRect(QRectF(0, 0, docImageBlock->width(), docImageBlock->height()));

  setPos(docImageBlock->x(), docImageBlock->y());
  updatePosition();
}

void
GraphicsImageBlockItem::draw(bool /*complete*/)
{
  //B: do something ???
}

void
GraphicsImageBlockItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  GraphicsBlockItem::mousePressEvent(event);
  DocumentController *docController =
    static_cast<DocumentController *>(getController()); //B:dynamic_cast?
  if (docController != nullptr) {
    docController->setCurrentBlock(getElement());
  }
}

//Slots
//Overloaded
void
GraphicsImageBlockItem::updatePosition()
{
  DocumentController *docController =
    static_cast<DocumentController *>(getController()); //B:dynamic_cast?
  if (docController != nullptr) {
    docController->setCurrentBlock(getElement());
  }

  GraphicsBlockItem::updatePosition();
  Doc::DocImageBlock *docImageBlock = getElement();
  if (docImageBlock == nullptr)
    return;
  if (docController != nullptr)
    docController->setBlockGeometry(x(), y(), rect().width(), rect().height());
}
