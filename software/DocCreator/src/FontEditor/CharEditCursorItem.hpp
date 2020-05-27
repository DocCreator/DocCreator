#ifndef CHAREDITCURSORITEM_H
#define CHAREDITCURSORITEM_H

#include <QGraphicsLineItem>

enum CursorPosition
{
  LEFT,
  CENTER,
  RIGHT
};

class CharEditCursorItem
  : public QObject
  , public QGraphicsLineItem
{
  Q_OBJECT

public:
  explicit CharEditCursorItem(CursorPosition cursorPos = CENTER, int size = 20);

  QRectF boundingRect() const override;

  //Getters
  CursorPosition getCursorPosition() const { return _cursorPos; }

  //Setters
  void setCursorPosition(CursorPosition cursorPos);
  void setCursorSize(int size);

private:
  CursorPosition _cursorPos;
};

#endif // CHAREDITCURSORITEM_H
