#ifndef CHAREDITSCENE_H
#define CHAREDITSCENE_H

#include <QGraphicsScene>

class QGraphicsPixmapItem;
class CharEditLineItem;
class CharEditCursorItem;
namespace Models {
class Character;
}

class CharEditScene : public QGraphicsScene
{
  Q_OBJECT

public:
  explicit CharEditScene(Models::Character *ch = nullptr,
                         QObject *parent = nullptr);

  //Setters
  void setCenterChar(const Models::Character *ch);
  void setLeftChar(const Models::Character *ch);
  void setRightChar(const Models::Character *ch);

  //Getters
  QGraphicsPixmapItem *getCenterCharItem() { return _centerCharItem; }
  QGraphicsPixmapItem *getLeftCharItem() { return _leftCharItem; }
  QGraphicsPixmapItem *getRightCharItem() { return _rightCharItem; }
  CharEditLineItem *getUpLineItem() { return _upLineItem; }
  CharEditLineItem *getBaseLineItem() { return _baseLineItem; }
  CharEditLineItem *getLeftLineItem() { return _leftLineItem; }
  CharEditLineItem *getRightLineItem() { return _rightLineItem; }
  QPointF getCenter() const { return _center; };
  CharEditCursorItem *getCursorItem() { return _cursorItem; }

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

public slots:

  void setUpLinePos(int pos, bool emitSignal = true);
  void setBaseLinePos(int pos, bool emitSignal = true);
  void setLeftLinePos(int pos, bool emitSignal = true);
  void setRightLinePos(int pos, bool emitSignal = true);

private slots:

  void upLineMoved(int pos);
  void baseLineMoved(int pos);
  void leftLineMoved(int pos);
  void rightLineMoved(int pos);
  void redraw();

private:
  void createActions();

private:
  QPointF _center;

  const Models::Character *_centerChar, *_leftChar, *_rightChar;
  QGraphicsPixmapItem *_centerCharItem, *_leftCharItem, *_rightCharItem;
  CharEditLineItem *_upLineItem, *_baseLineItem, *_leftLineItem,
    *_rightLineItem;

  CharEditCursorItem *_cursorItem;
};

#endif // CHAREDITSCENE_H
