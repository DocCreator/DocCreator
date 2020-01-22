#include "CharEditLineItem.hpp"

#include <QCursor>
#include <QKeyEvent>
#include <QPen>

CharEditLineItem::CharEditLineItem(LineOrientation orientation,
                                   qreal size,
                                   qreal position,
                                   QGraphicsItem *parent)
  : QGraphicsLineItem(parent)
  , _fixedPos(0)
{
  _orientation = orientation;
  if (_orientation == HORIZONTAL) {
    setLine(QLine(-size, 0, size, 0));
    setPos(_fixedPos, position);
    setCursor(QCursor(Qt::SizeVerCursor));
  } else if (_orientation == VERTICAL) {
    setLine(QLine(0, -size, 0, size));
    setPos(position, _fixedPos);
    setCursor(QCursor(Qt::SizeHorCursor));
  }
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
  QPen pen(Qt::DashDotLine);
  pen.setColor(Qt::red);
  pen.setWidth(1);
  setPen(pen);
  setZValue(100);
}

QRectF
CharEditLineItem::boundingRect() const
{
  qreal extra = (pen().width() + 2) / 2.0;

  return QRectF(line().p1(),
                QSizeF(line().p2().x() - line().p1().x(),
                       line().p2().y() - line().p1().y()))
    .normalized()
    .adjusted(-extra, -extra, extra, extra);
}

void
CharEditLineItem::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem *opt,
                        QWidget *widget)
{
  QGraphicsLineItem::paint(painter, opt, widget);
}

void
CharEditLineItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsLineItem::mouseMoveEvent(event);
  setLinePos((_orientation == HORIZONTAL) ? this->y() : this->x());
}

void
CharEditLineItem::keyPressEvent(QKeyEvent *event)
{
  if (!isSelected())
    return;

  switch (event->key()) {
    case Qt::Key_Left:
      setLinePos(this->x() - 1);
      break;

    case Qt::Key_Right:
      setLinePos(this->x() + 1);
      break;

    case Qt::Key_Up:
      setLinePos(this->y() - 1);
      break;

    case Qt::Key_Down:
      setLinePos(this->y() + 1);
      break;

    default:
      break;
  }
}
/*********
 * SLOTS *
 *********/

void
CharEditLineItem::setLinePos(int pos, bool emitSignal)
{
  if (_orientation == HORIZONTAL)
    this->setPos(_fixedPos, pos);
  else if (_orientation == VERTICAL)
    this->setPos(pos, _fixedPos);

  if (emitSignal) {
    emit moving(pos);
  }
}
