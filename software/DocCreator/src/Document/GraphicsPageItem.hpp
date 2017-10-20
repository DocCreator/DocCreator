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

  virtual void clear() override;
  virtual void setOffset(int value) override;
  void setBackground(const QString &filepath, int w, int h);

  void selectBlockFromPosition(QPointF pos);

  QList<GraphicsBlockItem *> getGraphicsBlockItems() const
  {
    return _map.values();
  }

  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  virtual void keyPressEvent(QKeyEvent * /*event*/) override {}
  virtual void keyReleaseEvent(QKeyEvent * /*event*/) override {}

protected:
  virtual void load() override;
  virtual void draw(bool complete) override;

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
