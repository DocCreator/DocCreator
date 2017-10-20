#include "CharEditView.hpp"

#include <QKeyEvent>
#include <QWheelEvent>

#include "CharEditLineItem.hpp"
#include "CharEditScene.hpp"

CharEditView::CharEditView(CharEditScene *scene, Mvc::IView *parent)
  : QGraphicsView()
{
  this->_scene = scene;
  this->_parent = parent;

  setScene(_scene);

  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  centerOn(0, 0);
  setSceneRect(-50, -50, 100, 100);

  setCursor(Qt::CrossCursor);

  zoomScaleIndex = 0;
}

void
CharEditView::wheelEvent(QWheelEvent *event)
{
  if (event->delta() > 0) {
    if (zoomScaleIndex < MAX_ZOOM)
      zoomIn();
  } else {
    if (zoomScaleIndex > -MAX_ZOOM)
      zoomOut();
  }
}

void
CharEditView::keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
    case Qt::Key_Left:
      _scene->getLeftLineItem()->keyPressEvent(event);
      _scene->getRightLineItem()->keyPressEvent(event);
      break;

    case Qt::Key_Right:
      _scene->getLeftLineItem()->keyPressEvent(event);
      _scene->getRightLineItem()->keyPressEvent(event);
      break;

    case Qt::Key_Up:
      _scene->getUpLineItem()->keyPressEvent(event);
      _scene->getBaseLineItem()->keyPressEvent(event);
      break;

    case Qt::Key_Down:
      _scene->getUpLineItem()->keyPressEvent(event);
      _scene->getBaseLineItem()->keyPressEvent(event);
      break;

    default:;
  }
  _parent->keyPressEvent(event);
}

void
CharEditView::keyReleaseEvent(QKeyEvent *event)
{
  _parent->keyReleaseEvent(event);
}

Mvc::IController *
CharEditView::getController()
{
  return _parent->getController();
}

/*********
 * SLOTS *
 *********/

void
CharEditView::zoomIn()
{
  scale(1.2, 1.2);
  zoomScaleIndex++;
}

void
CharEditView::zoomOut()
{
  scale(1 / 1.2, 1 / 1.2);
  zoomScaleIndex--;
}
