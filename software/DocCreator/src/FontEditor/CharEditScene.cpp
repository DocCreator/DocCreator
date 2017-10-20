#include "CharEditScene.hpp"

#include "CharEditCursorItem.hpp"
#include "CharEditLineItem.hpp"
#include "models/character.h"
#include "models/characterdata.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

CharEditScene::CharEditScene(Models::Character *ch, QObject *parent)
  : QGraphicsScene(parent)
  , _center(0, 0)
  , _centerChar(nullptr)
  , _leftChar(nullptr)
  , _rightChar(nullptr)
  , _centerCharItem(nullptr)
  , _leftCharItem(nullptr)
  , _rightCharItem(nullptr)
  , _upLineItem(nullptr)
  , _baseLineItem(nullptr)
  , _leftLineItem(nullptr)
  , _rightLineItem(nullptr)
  , _cursorItem(nullptr)
{
  QBrush brush(Qt::gray, Qt::CrossPattern);

  setBackgroundBrush(brush);

  _centerCharItem = nullptr;
  _leftCharItem = nullptr;
  _rightCharItem = nullptr;

  _upLineItem = new CharEditLineItem(HORIZONTAL, 1000, _center.y() - 20);
  addItem(_upLineItem);

  _baseLineItem = new CharEditLineItem(HORIZONTAL, 1000, _center.y() + 20);
  addItem(_baseLineItem);

  _leftLineItem = new CharEditLineItem(VERTICAL, 1000, _center.x() - 20);
  addItem(_leftLineItem);

  _rightLineItem = new CharEditLineItem(VERTICAL, 1000, _center.x() + 20);
  addItem(_rightLineItem);

  if (ch != nullptr)
    setCenterChar(ch);

  _cursorItem =
    new CharEditCursorItem(CENTER, _baseLineItem->y() - _upLineItem->y() - 4);
  _cursorItem->setPos(_leftLineItem->x() + 2, _upLineItem->y() + 2);
  _cursorItem->setZValue(10);
  addItem(_cursorItem);

  createActions();
}

void
CharEditScene::setCenterChar(Models::Character *ch)
{
  _centerChar = ch;
  //TODO: choose a median character
  const Models::CharacterData *charData = ch->getRandomCharacterData();

  if (_centerCharItem != nullptr)
    removeItem(_centerCharItem);

  const QImage charImg = charData->getImage();
  _centerCharItem = new QGraphicsPixmapItem(QPixmap::fromImage(charImg));
  _centerCharItem->setZValue(0);

  _center.setX(-charImg.width() / 2);
  _center.setY(-charImg.height() / 2);

  addItem(_centerCharItem);

  _upLineItem->setLinePos(_center.y() + (ch->getUpLine() / 100) *
                                          _centerCharItem->pixmap().height());
  _baseLineItem->setLinePos(_center.y() + (ch->getBaseLine() / 100) *
                                            _centerCharItem->pixmap().height());
  _leftLineItem->setLinePos(_center.x() + (ch->getLeftLine() / 100) *
                                            _centerCharItem->pixmap().width());
  _rightLineItem->setLinePos(_center.x() + (ch->getRightLine() / 100) *
                                             _centerCharItem->pixmap().width());

  redraw();
}

void
CharEditScene::setLeftChar(Models::Character *ch)
{
  if (_centerCharItem == nullptr)
    return;

  _leftChar = ch;
  //TODO: choose a median character
  const Models::CharacterData *charData = ch->getRandomCharacterData();

  if (_leftCharItem != nullptr)
    removeItem(_leftCharItem);

  const QImage charImg = charData->getImage();
  _leftCharItem = new QGraphicsPixmapItem(QPixmap::fromImage(charImg));
  _leftCharItem->setZValue(0);

  addItem(_leftCharItem);

  redraw();
}

void
CharEditScene::setRightChar(Models::Character *ch)
{
  if (_centerCharItem == nullptr)
    return;

  _rightChar = ch;
  //TODO: choose a median character
  const Models::CharacterData *charData = ch->getRandomCharacterData();

  if (_rightCharItem != nullptr)
    removeItem(_rightCharItem);

  const QImage charImg = charData->getImage();
  _rightCharItem = new QGraphicsPixmapItem(QPixmap::fromImage(charImg));
  _rightCharItem->setZValue(0);

  addItem(_rightCharItem);

  redraw();
}

void
CharEditScene::redraw()
{
  if (_centerCharItem != nullptr)
    _centerCharItem->setPos(_center);

  if (_leftCharItem != nullptr) {
    int x = _leftLineItem->x() -
            (_leftChar->getRightLine() * _leftCharItem->pixmap().width() / 100);
    int y = _baseLineItem->y() -
            (_leftChar->getBaseLine() * _leftCharItem->pixmap().height() / 100);
    _leftCharItem->setPos(x, y);
  }

  if (_rightCharItem != nullptr) {
    int x = _rightLineItem->x() + (_rightChar->getLeftLine() *
                                   _rightCharItem->pixmap().width() / 100);
    int y = _baseLineItem->y() - (_rightChar->getBaseLine() *
                                  _rightCharItem->pixmap().height() / 100);
    _rightCharItem->setPos(x, y);
  }

  _cursorItem->setCursorSize(_baseLineItem->y() - _upLineItem->y() - 4);
  switch (_cursorItem->getCursorPosition()) {
    case LEFT:
      _cursorItem->setPos(_leftLineItem->x() - 22, _upLineItem->y() + 2);
      break;
    case RIGHT:
      _cursorItem->setPos(_rightLineItem->x() + 2, _upLineItem->y() + 2);
      break;
    default:
      _cursorItem->setPos(_leftLineItem->x() + 2, _upLineItem->y() + 2);
  }
}

void
CharEditScene::createActions()
{
  QObject::connect(
    _upLineItem, SIGNAL(moving(int)), this, SLOT(upLineMoved(int)));
  QObject::connect(
    _baseLineItem, SIGNAL(moving(int)), this, SLOT(baseLineMoved(int)));
  QObject::connect(
    _leftLineItem, SIGNAL(moving(int)), this, SLOT(leftLineMoved(int)));
  QObject::connect(
    _rightLineItem, SIGNAL(moving(int)), this, SLOT(rightLineMoved(int)));
}

void
CharEditScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  if (event->scenePos().x() < _leftLineItem->x())
    _cursorItem->setCursorPosition(LEFT);
  else if (event->scenePos().x() < _rightLineItem->x())
    _cursorItem->setCursorPosition(CENTER);
  else
    _cursorItem->setCursorPosition(RIGHT);
  redraw();
  QGraphicsScene::mousePressEvent(event);
}

//SLOTS
void
CharEditScene::setUpLinePos(int pos, bool emitSignal)
{
  if (pos >= _baseLineItem->y())
    pos = _baseLineItem->y() - 1;
  _upLineItem->setLinePos(pos, emitSignal);
  redraw();
}

void
CharEditScene::setBaseLinePos(int pos, bool emitSignal)
{
  if (pos <= _upLineItem->y())
    pos = _upLineItem->y() + 1;
  _baseLineItem->setLinePos(pos, emitSignal);
  redraw();
}

void
CharEditScene::setLeftLinePos(int pos, bool emitSignal)
{
  if (pos >= _rightLineItem->x())
    pos = _rightLineItem->x() - 1;
  _leftLineItem->setLinePos(pos, emitSignal);
  redraw();
}

void
CharEditScene::setRightLinePos(int pos, bool emitSignal)
{
  if (pos <= _leftLineItem->x())
    pos = _leftLineItem->x() + 1;
  _rightLineItem->setLinePos(pos, emitSignal);
  redraw();
}

void
CharEditScene::upLineMoved(int /*pos*/)
{
  if (_upLineItem->y() >= _baseLineItem->y())
    setUpLinePos(_baseLineItem->y() - 1);
  redraw();
}

void
CharEditScene::baseLineMoved(int /*pos*/)
{
  if (_baseLineItem->y() <= _upLineItem->y())
    setBaseLinePos(_upLineItem->y() + 1);
  redraw();
}

void
CharEditScene::leftLineMoved(int /*pos*/)
{
  if (_leftLineItem->x() >= _rightLineItem->x())
    setLeftLinePos(_rightLineItem->x() - 1);
  redraw();
}

void
CharEditScene::rightLineMoved(int /*pos*/)
{
  if (_rightLineItem->x() <= _leftLineItem->x())
    setRightLinePos(_leftLineItem->x() + 1);
  redraw();
}
