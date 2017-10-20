#include "GraphicsTestBlockItem.hpp"
#include "DocumentController.hpp"

GraphicsTestBlockItem::GraphicsTestBlockItem(Mvc::IController *controller,
                                             QGraphicsItem *parent)
  : GraphicsBlockItem(parent)
  , ADocumentView<Doc::DocTestBlock>(controller)
{}

void
GraphicsTestBlockItem::clear()
{}

void
GraphicsTestBlockItem::setOffset(int /*value*/)
{}

void
GraphicsTestBlockItem::setRect(const QRectF &rect)
{
  GraphicsBlockItem::setRect(rect);
}

//protected:
void
GraphicsTestBlockItem::load()
{
  Doc::DocTestBlock *docTestBlock =
    dynamic_cast<Doc::DocTestBlock *>(getElement());
  if (docTestBlock == nullptr)
    return;

  const int width = docTestBlock->width();
  if (width == 0)
    docTestBlock->setWidth(100);
  const int height = docTestBlock->height();
  if (height == 0)
    docTestBlock->setHeight(100);

  setRect(QRectF(0, 0, docTestBlock->width(), docTestBlock->height()));

  setPos(docTestBlock->x(), docTestBlock->y());
  updatePosition();
}

void
GraphicsTestBlockItem::draw(bool /*complete*/)
{}

void
GraphicsTestBlockItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
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
GraphicsTestBlockItem::updatePosition()
{
  DocumentController *docController =
    static_cast<DocumentController *>(getController()); //B:dynamic_cast?
  if (docController != nullptr) {
    docController->setCurrentBlock(getElement());
  }

  GraphicsBlockItem::updatePosition();
  Doc::DocTestBlock *docTestBlock = getElement();
  if (docTestBlock == nullptr)
    return;
  if (docController != nullptr)
    docController->setBlockGeometry(x(), y(), rect().width(), rect().height());
}
