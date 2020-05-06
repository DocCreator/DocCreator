#ifndef GRAPHICSCOMPONENTBLOCKITEM_H
#define GRAPHICSCOMPONENTBLOCKITEM_H

#include "ADocumentView.hpp"
#include "GraphicsBlockItem.hpp"
#include "mvc/icontroller.h"

class QGraphicsPixmapItem;

class GraphicsComponentBlockItem
  : public GraphicsBlockItem
  , public ADocumentView<Doc::DocComponentBlock>
{
  Q_OBJECT

public:
  explicit GraphicsComponentBlockItem(Mvc::IController *controller,
                                      QGraphicsItem *parent = nullptr);

  void clear() override;
  void setOffset(int value) override;

  void setRect(const QRectF &rect) override;

  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void keyPressEvent(QKeyEvent * /*event*/) override {}
  void keyReleaseEvent(QKeyEvent * /*event*/) override {}

public slots:
  void updatePosition() override;

protected:
  void load() override;
  void draw(bool complete) override;

private:
  QPixmap _originalPixmap;
  QPixmap _scaledPixmap;
  QGraphicsPixmapItem *_pixmapItem;
};

#endif // GRAPHICSCOMPONENTBLOCKITEM_H
