#include "GraphicsMovableButton.hpp"

#include <QBrush>
#include <QColor>
#include <QPen>

GraphicsMovableButton::GraphicsMovableButton(QGraphicsItem *parent)
  : QGraphicsRectItem(parent)
{
  setFlags(QGraphicsItem::ItemIsMovable);
  _pressed = false;
  show();
}

void
GraphicsMovableButton::hide()
{
  setPen(QPen(Qt::transparent));
  setBrush(QBrush(Qt::transparent));
}

void
GraphicsMovableButton::show()
{
  setPen(QPen(Qt::black));
  setBrush(QBrush(QColor(0, 255, 33)));
}

void
GraphicsMovableButton::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsRectItem::mouseMoveEvent(event);
  if (_pressed)
    emit moving(pos());
}

void
GraphicsMovableButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsRectItem::mousePressEvent(event);
  _pressed = true;
}

void
GraphicsMovableButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsRectItem::mouseReleaseEvent(event);
  _pressed = false;
  emit stopMoving();
}
