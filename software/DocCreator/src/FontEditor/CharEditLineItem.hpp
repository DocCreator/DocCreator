#ifndef CHAREDITLINEITEM_H
#define CHAREDITLINEITEM_H

#include <QGraphicsLineItem>

class QKeyEvent;

enum LineOrientation
{
  HORIZONTAL,
  VERTICAL
};

class CharEditLineItem
  : public QObject
  , public QGraphicsLineItem
{
  Q_OBJECT

public:
  CharEditLineItem(LineOrientation orientation,
                   qreal size,
                   qreal position,
                   QGraphicsItem *parent = 0);

  QRectF boundingRect() const override;

  void keyPressEvent(QKeyEvent *event) override;

protected:
  void paint(QPainter *painter,
             const QStyleOptionGraphicsItem *opt,
             QWidget *widget = 0) override;

  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

public slots:
  void setLinePos(int pos, bool emitSignal = true);

signals:
  void moving(int pos);

private:
  LineOrientation _orientation;
  int _fixedPos;
};

#endif //CHAREDITLINEITEM_H
