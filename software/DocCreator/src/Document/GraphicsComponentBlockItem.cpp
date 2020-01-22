#include "GraphicsComponentBlockItem.hpp"

#include "DocumentController.hpp"

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "models/doc/doccomponent.h"
#include "models/doc/doccomponentblock.h"
#include "models/doc/doczone.h"

GraphicsComponentBlockItem::GraphicsComponentBlockItem(
  Mvc::IController *controller,
  QGraphicsItem *parent)
  : GraphicsBlockItem(parent)
  , ADocumentView<Doc::DocComponentBlock>(controller)
{
  _pixmapItem = nullptr;
}

void
GraphicsComponentBlockItem::clear()
{
  if (_pixmapItem != nullptr) {
    _pixmapItem->setPixmap(QPixmap());
  }
}

void
GraphicsComponentBlockItem::setOffset(int /*value*/)
{
  //?
}

void
GraphicsComponentBlockItem::setRect(const QRectF &rect)
{
  GraphicsBlockItem::setRect(rect);

  if (!_originalPixmap.isNull())
    _scaledPixmap = _originalPixmap.scaled(QSize(rect.width(), rect.height()));
  else
    _scaledPixmap = _originalPixmap;

  if (_pixmapItem != nullptr)
    _pixmapItem->setPixmap(_scaledPixmap);
}

void
GraphicsComponentBlockItem::load()
{
  Doc::DocComponentBlock *docComponentBlock =
    dynamic_cast<Doc::DocComponentBlock *>(getElement());
  if (docComponentBlock == nullptr) {
    return;
  }

  _pixmapItem = new QGraphicsPixmapItem();
  _pixmapItem->setParentItem(this);

  //B: I don't understand this ???
  const int width = docComponentBlock->width();
  if (width == 0)
    docComponentBlock->setWidth(docComponentBlock->width());
  const int height = docComponentBlock->height();
  if (height == 0)
    docComponentBlock->setHeight(docComponentBlock->height());

  setRect(
    QRectF(0, 0, docComponentBlock->width(), docComponentBlock->height()));

  setPos(docComponentBlock->x(), docComponentBlock->y());

  // drawing scanline
  const int X = docComponentBlock->x();
  const int Y = docComponentBlock->y();

  _originalPixmap =
    QPixmap(QSize(docComponentBlock->width(), docComponentBlock->height()));
  _originalPixmap.fill(Qt::transparent);
  QPainter painter(&_originalPixmap);
  painter.setPen(QPen(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

  for (Doc::DocZone *dz : docComponentBlock->getZones()) {
    for (Doc::DocComponent *dc : dz->getComponents()) {
      const Doc::Scanlines &sls = dc->getScanlines();
      QMap<int, std::vector<int>>::const_iterator it = sls.constBegin();
      while (it != sls.constEnd()) {
        const int y = it.key() - Y;
        const std::vector<int> &Xs = it.value();
        if (Xs.size() > 1) {
          for (size_t m = 1; m < Xs.size();
               m += 2) { //start from 1 ; incremented of 2 because scanline
            painter.drawLine(Xs.at(m - 1) - X, y, Xs.at(m) - X, y);
          }
        }
        ++it;
      }
    }
  }
  //qDebug() << "drawing the component FINISHED";
  //
  updatePosition();
}

void
GraphicsComponentBlockItem::draw(bool /*complete*/)
{
  //?
}

void
GraphicsComponentBlockItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  //qDebug()<< "x, y=" << event->scenePos().x() << " ," << event->scenePos().y();

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
GraphicsComponentBlockItem::updatePosition()
{
  DocumentController *docController =
    static_cast<DocumentController *>(getController()); //B:dynamic_cast?
  if (docController != nullptr) {
    docController->setCurrentBlock(getElement());
  }

  GraphicsBlockItem::updatePosition();
  Doc::DocComponentBlock *docComponentBlock = getElement();
  if (docComponentBlock == nullptr) {
    return;
  }
  if (docController != nullptr) {
    docController->setBlockGeometry(x(), y(), rect().width(), rect().height());
  }
}
