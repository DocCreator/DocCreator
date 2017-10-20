#ifndef GRAPHICSIMAGEBLOCKITEM_H
#define GRAPHICSIMAGEBLOCKITEM_H

#include "ADocumentView.hpp"
#include "GraphicsBlockItem.hpp"
#include "mvc/icontroller.h"

class QGraphicsPixmapItem;

class GraphicsImageBlockItem
  : public GraphicsBlockItem
  , public ADocumentView<Doc::DocImageBlock>
{
  Q_OBJECT

public:
  explicit GraphicsImageBlockItem(Mvc::IController *controller,
                                  QGraphicsItem *parent = nullptr);

  virtual void clear() override;
  virtual void setOffset(int value) override;

  virtual void setRect(const QRectF &rect) override;

  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  virtual void keyPressEvent(QKeyEvent * /*event*/) override {}
  virtual void keyReleaseEvent(QKeyEvent * /*event*/) override {}

public slots:
  virtual void updatePosition() override;

protected:
  virtual void load() override;
  virtual void draw(bool complete) override;

private:
  QPixmap _originalPixmap;
  QPixmap _scaledPixmap;
  QGraphicsPixmapItem *_pixmapItem;
};

#endif // GRAPHICSIMAGEBLOCKITEM_H
