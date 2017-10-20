#ifndef GRAPHICSCURSORITEM_H
#define GRAPHICSCURSORITEM_H

#include <QGraphicsLineItem>

class GraphicsCursorItem : public QGraphicsLineItem
{
public:
  explicit GraphicsCursorItem(int size = 20, QGraphicsItem *parent = nullptr);

  virtual QRectF boundingRect() const override;

  virtual void hide();
  virtual void show();

  //Getters
  int getCursorPosition() const { return _cursorPos; }

  //Setters
  void setCursorPosition(int cursorPos);
  void setCursorSize(int size);

private:
  int _cursorPos;
};

#endif // GRAPHICSCURSORITEM_H
