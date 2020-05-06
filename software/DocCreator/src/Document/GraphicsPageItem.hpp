#ifndef GRAPHICSPAGEITEM_H
#define GRAPHICSPAGEITEM_H

#include "ADocumentView.hpp"
#include <QGraphicsPixmapItem>
#include <mvc/icontroller.h>

class GraphicsBlockItem;

class GraphicsPageItem
  : public QGraphicsPixmapItem
  , public ADocumentView<Doc::Page>
{
public:
  explicit GraphicsPageItem(Mvc::IController *controller,
                            QGraphicsItem *parent = nullptr);

  void clear() override;
  void setOffset(int value) override;
  void setBackground(const QString &filepath, int w, int h);

  void selectBlockFromPosition(QPointF pos);

  QList<GraphicsBlockItem *> getGraphicsBlockItems() const
  {
    return _map.values();
  }

  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  void keyPressEvent(QKeyEvent * /*event*/) override {}
  void keyReleaseEvent(QKeyEvent * /*event*/) override {}

protected:
  void load() override;
  void draw(bool complete) override;

private:
  GraphicsBlockItem *currentBlockItem();
  void synchroniseWithElement();
  void addTextBlock(Doc::DocTextBlock *docTextBlock);
  void addImageBlock(Doc::DocImageBlock *docImageBlock);
  void addTestBlock(Doc::DocTestBlock *docTestBlock);
  void addComponentBlock(Doc::DocComponentBlock *docComponentBlock);

  void removeBlock(Doc::Block *b);

private:
  QMap<Doc::Block *, GraphicsBlockItem *> _map;
};

#endif // GRAPHICSPAGEITEM_H
