#include "CharEditCursorItem.hpp"

#include <QCursor>
#include <QPen>

CharEditCursorItem::CharEditCursorItem(CursorPosition cursorPos, int size)
  : QGraphicsLineItem()
{
  _cursorPos = cursorPos;
  setLine(QLine(0, 0, 0, size));
  setCursor(QCursor(Qt::IBeamCursor));
}

QRectF
CharEditCursorItem::boundingRect() const
{
  qreal extra = (pen().width() + 2) / 2.0;

  return QRectF(line().p1(),
                QSizeF(line().p2().x() - line().p1().x(),
                       line().p2().y() - line().p1().y()))
    .normalized()
    .adjusted(-extra, -extra, extra, extra);
}

void
CharEditCursorItem::setCursorSize(int size)
{
  setLine(QLine(0, 0, 0, size));
}

void
CharEditCursorItem::setCursorPosition(CursorPosition cursorPos)
{
  _cursorPos = cursorPos;
}
