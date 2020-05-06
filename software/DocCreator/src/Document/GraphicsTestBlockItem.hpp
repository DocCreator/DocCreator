#ifndef GRAPHICSTESTBLOCKITEM_H
#define GRAPHICSTESTBLOCKITEM_H

#include "ADocumentView.hpp"
#include "GraphicsBlockItem.hpp"
#include "mvc/icontroller.h"

class GraphicsTestBlockItem
  : public GraphicsBlockItem
  , public ADocumentView<Doc::DocTestBlock>
{
  Q_OBJECT

public:
  explicit GraphicsTestBlockItem(Mvc::IController *controller,
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
};

#endif // GRAPHICSTESTBLOCKITEM_H
