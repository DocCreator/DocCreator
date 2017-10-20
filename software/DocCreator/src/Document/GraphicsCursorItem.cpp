#include "GraphicsCursorItem.hpp"

#include <QCursor>
#include <QPen>

GraphicsCursorItem::GraphicsCursorItem(int size, QGraphicsItem *parent)
  : QGraphicsLineItem(parent)
  , _cursorPos(0)
{
  setLine(QLine(0, 0, 0, size));
  setCursor(QCursor(Qt::IBeamCursor));
  setPen(QPen(QColor(192, 0, 0)));
}

QRectF
GraphicsCursorItem::boundingRect() const
{
  qreal extra = (pen().width() + 2) / 2.0;

  return QRectF(line().p1(),
                QSizeF(line().p2().x() - line().p1().x(),
                       line().p2().y() - line().p1().y()))
    .normalized()
    .adjusted(-extra, -extra, extra, extra);
}

void
GraphicsCursorItem::hide()
{
  setPen(QPen(Qt::transparent));
}

void
GraphicsCursorItem::show()
{
  setPen(QPen(QColor(192, 0, 0)));
}

void
GraphicsCursorItem::setCursorPosition(int cursorPos)
{
  _cursorPos = cursorPos;
}

void
GraphicsCursorItem::setCursorSize(int size)
{
  setLine(QLine(0, 0, 0, size));
}
