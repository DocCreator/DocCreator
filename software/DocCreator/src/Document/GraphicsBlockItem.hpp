#ifndef GRAPHICSBLOCKITEM_H
#define GRAPHICSBLOCKITEM_H

#include <QCursor>
#include <QGraphicsRectItem>

class QGraphicsSceneMouseEvent;
class GraphicsMovableButton;

static const int DELTA = 3;

class GraphicsBlockItem
  : public QObject
  , public QGraphicsRectItem
{
  Q_OBJECT

public:
  explicit GraphicsBlockItem(QGraphicsItem *parent = nullptr);

  QRectF boundingRect() const override;
  virtual void hide(bool transparent = false);
  virtual void show();

  //Events
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  virtual void setRect(const QRectF &rect);

public slots:
  virtual void resizeTop(QPointF pos);
  virtual void resizeBottom(QPointF pos);
  virtual void resizeLeft(QPointF pos);
  virtual void resizeRight(QPointF pos);
  virtual void resizeTopLeft(QPointF pos);
  virtual void resizeTopRight(QPointF pos);
  virtual void resizeBottomLeft(QPointF pos);
  virtual void resizeBottomRight(QPointF pos);
  virtual void moveBlockTop(QPointF pos);
  virtual void moveBlockBottom(QPointF pos);
  virtual void moveBlockRight(QPointF pos);
  virtual void moveBlockLeft(QPointF pos);

  virtual void updatePosition();

protected:
  GraphicsMovableButton *createResizeButton(const QCursor &curs);
  GraphicsMovableButton *createMoveButton(const QCursor &curs);
  virtual void updateButtons();
  virtual void createActions();

private:
  //Attributes
  GraphicsMovableButton *_resizeButtonRectTop;
  GraphicsMovableButton *_resizeButtonRectBottom;
  GraphicsMovableButton *_resizeButtonRectLeft;
  GraphicsMovableButton *_resizeButtonRectRight;
  GraphicsMovableButton *_resizeButtonRectTopLeft;
  GraphicsMovableButton *_resizeButtonRectTopRight;
  GraphicsMovableButton *_resizeButtonRectBottomLeft;
  GraphicsMovableButton *_resizeButtonRectBottomRight;

  GraphicsMovableButton *_moveButtonTop;
  GraphicsMovableButton *_moveButtonBottom;
  GraphicsMovableButton *_moveButtonLeft;
  GraphicsMovableButton *_moveButtonRight;
};

#endif // GRAPHICSBLOCKITEM_H
