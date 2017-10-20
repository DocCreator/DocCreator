#include "GraphicsBlockItem.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QRectF>

#include "GraphicsMovableButton.hpp"

GraphicsBlockItem::GraphicsBlockItem(QGraphicsItem *parent)
  : QGraphicsRectItem(parent)
{
  _resizeButtonRectTop = createResizeButton(QCursor(Qt::SizeVerCursor));
  _resizeButtonRectBottom = createResizeButton(QCursor(Qt::SizeVerCursor));
  _resizeButtonRectLeft = createResizeButton(QCursor(Qt::SizeHorCursor));
  _resizeButtonRectRight = createResizeButton(QCursor(Qt::SizeHorCursor));
  _resizeButtonRectTopLeft = createResizeButton(QCursor(Qt::SizeFDiagCursor));
  _resizeButtonRectTopRight = createResizeButton(QCursor(Qt::SizeBDiagCursor));
  _resizeButtonRectBottomLeft =
    createResizeButton(QCursor(Qt::SizeBDiagCursor));
  _resizeButtonRectBottomRight =
    createResizeButton(QCursor(Qt::SizeFDiagCursor));

  _moveButtonTop = createMoveButton(QCursor(Qt::SizeAllCursor));
  _moveButtonBottom = createMoveButton(QCursor(Qt::SizeAllCursor));
  _moveButtonLeft = createMoveButton(QCursor(Qt::SizeAllCursor));
  _moveButtonRight = createMoveButton(QCursor(Qt::SizeAllCursor));

  createActions();
}

QRectF
GraphicsBlockItem::boundingRect() const
{
  return rect().normalized().adjusted(
    -2 * DELTA, -2 * DELTA, 2 * DELTA, 2 * DELTA);
}

void
GraphicsBlockItem::hide(bool transparent)
{
  if (transparent)
    setPen(QPen(Qt::transparent));
  else {
    QPen pen(Qt::red);
    pen.setStyle(Qt::DashLine);
    setPen(pen);
  }
  _resizeButtonRectTop->hide();
  _resizeButtonRectBottom->hide();
  _resizeButtonRectLeft->hide();
  _resizeButtonRectRight->hide();
  _resizeButtonRectTopLeft->hide();
  _resizeButtonRectTopRight->hide();
  _resizeButtonRectBottomLeft->hide();
  _resizeButtonRectBottomRight->hide();
}

void
GraphicsBlockItem::show()
{
  setPen(QPen(Qt::black));
  _resizeButtonRectTop->show();
  _resizeButtonRectBottom->show();
  _resizeButtonRectLeft->show();
  _resizeButtonRectRight->show();
  _resizeButtonRectTopLeft->show();
  _resizeButtonRectTopRight->show();
  _resizeButtonRectBottomLeft->show();
  _resizeButtonRectBottomRight->show();
}

void
GraphicsBlockItem::mousePressEvent(QGraphicsSceneMouseEvent * /*event*/)
{
  //event = nullptr; //B
}

void
GraphicsBlockItem::setRect(const QRectF &rect)
{
  QGraphicsRectItem::setRect(rect);
  updateButtons();
}

//Slots
void
GraphicsBlockItem::resizeTop(QPointF pos)
{
  QRectF newRect(this->rect());
  newRect.setTop(pos.y() + DELTA);
  setRect(newRect);
}

void
GraphicsBlockItem::resizeBottom(QPointF pos)
{
  QRectF newRect(this->rect());
  newRect.setBottom(pos.y() + DELTA);
  setRect(newRect);
}

void
GraphicsBlockItem::resizeLeft(QPointF pos)
{
  QRectF newRect(this->rect());
  newRect.setLeft(pos.x() + DELTA);
  setRect(newRect);
}

void
GraphicsBlockItem::resizeRight(QPointF pos)
{
  QRectF newRect(this->rect());
  newRect.setRight(pos.x() + DELTA);
  setRect(newRect);
}

void
GraphicsBlockItem::resizeTopLeft(QPointF pos)
{
  QRectF newRect(this->rect());
  newRect.setTopLeft(QPointF(pos.x() + DELTA, pos.y() + DELTA));
  setRect(newRect);
}

void
GraphicsBlockItem::resizeTopRight(QPointF pos)
{
  QRectF newRect(this->rect());
  newRect.setTopRight(QPointF(pos.x() + DELTA, pos.y() + DELTA));
  setRect(newRect);
}

void
GraphicsBlockItem::resizeBottomLeft(QPointF pos)
{
  QRectF newRect(this->rect());
  newRect.setBottomLeft(QPointF(pos.x() + DELTA, pos.y() + DELTA));
  setRect(newRect);
}

void
GraphicsBlockItem::resizeBottomRight(QPointF pos)
{
  QRectF newRect(this->rect());
  newRect.setBottomRight(QPointF(pos.x() + DELTA, pos.y() + DELTA));
  setRect(newRect);
}

void
GraphicsBlockItem::moveBlockTop(QPointF pos)
{
  QRectF newRect(this->rect());
  newRect.moveTopLeft(QPointF(pos.x() - DELTA, pos.y() + DELTA));
  setRect(newRect);
}

void
GraphicsBlockItem::moveBlockBottom(QPointF pos)
{
  QRectF newRect(this->rect());
  newRect.moveBottomLeft(QPointF(pos.x() - DELTA, pos.y() + DELTA));
  setRect(newRect);
}

void
GraphicsBlockItem::moveBlockRight(QPointF pos)
{
  QRectF newRect(this->rect());
  newRect.moveTopRight(QPointF(pos.x() + DELTA, pos.y() - DELTA));
  setRect(newRect);
}

void
GraphicsBlockItem::moveBlockLeft(QPointF pos)
{
  QRectF newRect(this->rect());
  newRect.moveTopLeft(QPointF(pos.x() + DELTA, pos.y() - DELTA));
  setRect(newRect);
}

void
GraphicsBlockItem::updatePosition()
{
  QRectF newRect(0, 0, rect().width(), rect().height());
  setPos(x() + rect().x(), y() + rect().y());
  setRect(newRect);
}

//Private methods
GraphicsMovableButton *
GraphicsBlockItem::createResizeButton(const QCursor &curs)
{
  auto resizeButton = new GraphicsMovableButton(this);
  resizeButton->setCursor(curs);
  resizeButton->setZValue(10);
  resizeButton->setRect(0, 0, 2 * DELTA, 2 * DELTA);
  return resizeButton;
}

GraphicsMovableButton *
GraphicsBlockItem::createMoveButton(const QCursor &curs)
{
  auto moveButton = new GraphicsMovableButton(this);
  moveButton->setCursor(curs);
  moveButton->setZValue(5);
  moveButton->hide();
  return moveButton;
}

void
GraphicsBlockItem::updateButtons()
{
  int w = rect().width();
  int h = rect().height();
  int x = rect().x();
  int y = rect().y();

  _resizeButtonRectTop->setPos(x + w / 2 - DELTA, y - DELTA);
  _resizeButtonRectBottom->setPos(x + w / 2 - DELTA, y + h - DELTA);
  _resizeButtonRectLeft->setPos(x - DELTA, y + h / 2 - DELTA);
  _resizeButtonRectRight->setPos(x + w - DELTA, y + h / 2 - DELTA);
  _resizeButtonRectTopLeft->setPos(x - DELTA, y - DELTA);
  _resizeButtonRectTopRight->setPos(x + w - DELTA, y - DELTA);
  _resizeButtonRectBottomLeft->setPos(x - DELTA, y + h - DELTA);
  _resizeButtonRectBottomRight->setPos(x + w - DELTA, y + h - DELTA);

  _moveButtonTop->setRect(0, 0, w - 2 * DELTA, 2 * DELTA);
  _moveButtonTop->setPos(x + DELTA, y - DELTA);
  _moveButtonBottom->setRect(0, 0, w - 2 * DELTA, 2 * DELTA);
  _moveButtonBottom->setPos(x + DELTA, y + h - DELTA);
  _moveButtonLeft->setRect(0, 0, 2 * DELTA, h - 2 * DELTA);
  _moveButtonLeft->setPos(x - DELTA, y + DELTA);
  _moveButtonRight->setRect(0, 0, 2 * DELTA, h - 2 * DELTA);
  _moveButtonRight->setPos(x + w - DELTA, y + DELTA);
}

void
GraphicsBlockItem::createActions()
{
  connect(_resizeButtonRectTop,
          SIGNAL(moving(QPointF)),
          this,
          SLOT(resizeTop(QPointF)));
  connect(_resizeButtonRectBottom,
          SIGNAL(moving(QPointF)),
          this,
          SLOT(resizeBottom(QPointF)));
  connect(_resizeButtonRectLeft,
          SIGNAL(moving(QPointF)),
          this,
          SLOT(resizeLeft(QPointF)));
  connect(_resizeButtonRectRight,
          SIGNAL(moving(QPointF)),
          this,
          SLOT(resizeRight(QPointF)));
  connect(_resizeButtonRectTopLeft,
          SIGNAL(moving(QPointF)),
          this,
          SLOT(resizeTopLeft(QPointF)));
  connect(_resizeButtonRectTopRight,
          SIGNAL(moving(QPointF)),
          this,
          SLOT(resizeTopRight(QPointF)));
  connect(_resizeButtonRectBottomLeft,
          SIGNAL(moving(QPointF)),
          this,
          SLOT(resizeBottomLeft(QPointF)));
  connect(_resizeButtonRectBottomRight,
          SIGNAL(moving(QPointF)),
          this,
          SLOT(resizeBottomRight(QPointF)));
  connect(
    _moveButtonTop, SIGNAL(moving(QPointF)), this, SLOT(moveBlockTop(QPointF)));
  connect(_moveButtonBottom,
          SIGNAL(moving(QPointF)),
          this,
          SLOT(moveBlockBottom(QPointF)));
  connect(_moveButtonLeft,
          SIGNAL(moving(QPointF)),
          this,
          SLOT(moveBlockLeft(QPointF)));
  connect(_moveButtonRight,
          SIGNAL(moving(QPointF)),
          this,
          SLOT(moveBlockRight(QPointF)));

  connect(
    _resizeButtonRectTop, SIGNAL(stopMoving()), this, SLOT(updatePosition()));
  connect(_resizeButtonRectBottom,
          SIGNAL(stopMoving()),
          this,
          SLOT(updatePosition()));
  connect(
    _resizeButtonRectLeft, SIGNAL(stopMoving()), this, SLOT(updatePosition()));
  connect(
    _resizeButtonRectRight, SIGNAL(stopMoving()), this, SLOT(updatePosition()));
  connect(_resizeButtonRectTopLeft,
          SIGNAL(stopMoving()),
          this,
          SLOT(updatePosition()));
  connect(_resizeButtonRectTopRight,
          SIGNAL(stopMoving()),
          this,
          SLOT(updatePosition()));
  connect(_resizeButtonRectBottomLeft,
          SIGNAL(stopMoving()),
          this,
          SLOT(updatePosition()));
  connect(_resizeButtonRectBottomRight,
          SIGNAL(stopMoving()),
          this,
          SLOT(updatePosition()));
  connect(_moveButtonTop, SIGNAL(stopMoving()), this, SLOT(updatePosition()));
  connect(
    _moveButtonBottom, SIGNAL(stopMoving()), this, SLOT(updatePosition()));
  connect(_moveButtonLeft, SIGNAL(stopMoving()), this, SLOT(updatePosition()));
  connect(_moveButtonRight, SIGNAL(stopMoving()), this, SLOT(updatePosition()));
}
