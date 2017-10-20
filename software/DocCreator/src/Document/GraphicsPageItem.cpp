#include "GraphicsPageItem.hpp"

#include <cassert>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPixmap>

#include "context/backgroundcontext.h"

#include "DocumentController.hpp"
#include "GraphicsComponentBlockItem.hpp"
#include "GraphicsImageBlockItem.hpp"
#include "GraphicsTestBlockItem.hpp"
#include "GraphicsTextBlockItem.hpp"

GraphicsPageItem::GraphicsPageItem(Mvc::IController *controller,
                                   QGraphicsItem *parent)
  : QGraphicsPixmapItem(parent)
  , ADocumentView<Doc::Page>(controller)
{}

void
GraphicsPageItem::clear()
{
  _map.clear();
  for (QGraphicsItem *item : childItems()) {
    item->setParentItem(nullptr);
    delete item;
  }
}

void
GraphicsPageItem::setOffset(int value)
{
  GraphicsBlockItem *current = currentBlockItem();
  GraphicsTextBlockItem *graphicTextBlock =
    dynamic_cast<GraphicsTextBlockItem *>(current);
  if (graphicTextBlock != nullptr)
    graphicTextBlock->setOffset(value);

  GraphicsImageBlockItem *graphicImageBlock =
    dynamic_cast<GraphicsImageBlockItem *>(current);
  if (graphicImageBlock != nullptr)
    graphicImageBlock->setOffset(value);

  GraphicsTestBlockItem *graphicTestBlock =
    dynamic_cast<GraphicsTestBlockItem *>(current);
  if (graphicTestBlock != nullptr)
    graphicTestBlock->setOffset(value);

  //added by kvcuong 09/05/2012
  GraphicsComponentBlockItem *graphicComponentBlock =
    dynamic_cast<GraphicsComponentBlockItem *>(current);
  if (graphicComponentBlock != nullptr)
    graphicComponentBlock->setOffset(value);
}

void
GraphicsPageItem::setBackground(const QString &filepath, int w, int h)
{
  QPixmap fond(filepath);
  fond = fond.scaled(QSize(w, h));
  setPixmap(fond);
}

/*
void GraphicsPageItem::selectBlockFromPosition(QPointF pos)
{
    for (GraphicsBlockItem* item : _map.values())
    {
        if (item->boundingRect().contains(mapToItem(item, pos)))
            continue;

        item->hide();
    }
}*/

//Events
void
GraphicsPageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsPixmapItem::mouseMoveEvent(event);
}

void
GraphicsPageItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsPixmapItem::mousePressEvent(event);

  //QPointF mousePos = event->pos();
  //selectBlockFromPosition(mousePos);
}

void
GraphicsPageItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsPixmapItem::mouseReleaseEvent(event);
}

void
GraphicsPageItem::load()
{
  clear();

  Doc::Page *page = getElement();
  if (page == nullptr)
    return;

  Doc::Block *previousBlock = page->currentBlock();

  for (Doc::DocTextBlock *block : page->getTextBlocks()) {
    page->setCurrentBlock(block);
    addTextBlock(block);
  }

  for (Doc::DocImageBlock *block : page->getImageBlocks()) {
    page->setCurrentBlock(block);
    addImageBlock(block);
  }

  for (Doc::DocTestBlock *block : page->getTestBlocks()) {
    page->setCurrentBlock(block);
    addTestBlock(block);
  }

  //added by kvcuong
  for (Doc::DocComponentBlock *block : page->getComponentBlocks()) {
    page->setCurrentBlock(block);
    addComponentBlock(block);
  }

  page->setCurrentBlock(previousBlock);
}
void
GraphicsPageItem::draw(bool complete)
{
  //qDebug() <<  " draw page";

  Doc::Page *page = getElement();
  if (page == nullptr)
    return;

  if (Context::BackgroundContext::instance()->changed() || complete) {
    DocumentController *controller =
      static_cast<DocumentController *>(getController()); //B:dynamic_cast?
    if (controller != nullptr) {
      QString bgFileName =
        Context::BackgroundContext::instance()->getCurrentBackground();
      QString bgpath =
        Context::BackgroundContext::instance()->getPath() + bgFileName;
      int w = controller->getPageWidth();
      int h = controller->getPageHeight();

      Context::BackgroundContext::instance()->setChanged(false);
      page->setBackgroundFileName(bgFileName);
      setBackground(bgpath, w, h);
    }
  }

  synchroniseWithElement();

  if (complete) {
    //QMessageBox::information(nullptr, "COMPLETE", "redraw"); <<<<< Why re-draw all blocks?
    //for (Doc::Block* b : _map.keys()) {
    for (auto it = _map.begin(); it != _map.end(); ++it) {
      Doc::Block *b = it.key();

      GraphicsBlockItem *v = _map.value(b);
      if (v != nullptr) {
        GraphicsTextBlockItem *graphicTextBlock =
          dynamic_cast<GraphicsTextBlockItem *>(v);
        Doc::DocTextBlock *docTextBlock = dynamic_cast<Doc::DocTextBlock *>(b);
        if (graphicTextBlock != nullptr && docTextBlock != nullptr)
          graphicTextBlock->drawElement(docTextBlock, complete);
        else {
          GraphicsImageBlockItem *graphicImageBlock =
            dynamic_cast<GraphicsImageBlockItem *>(v);
          Doc::DocImageBlock *docImageBlock =
            dynamic_cast<Doc::DocImageBlock *>(b);
          if (graphicImageBlock != nullptr && docImageBlock != nullptr) {
            graphicImageBlock->drawElement(docImageBlock, complete);
          } else {
            //added by kvcuong
            GraphicsComponentBlockItem *graphicComponentBlock =
              dynamic_cast<GraphicsComponentBlockItem *>(v);
            Doc::DocComponentBlock *docComponentBlock =
              dynamic_cast<Doc::DocComponentBlock *>(b);
            if (graphicComponentBlock != nullptr &&
                docComponentBlock != nullptr) {
              //qDebug() << "graphics page item: drawing component block";
              graphicComponentBlock->drawElement(docComponentBlock, complete);
            } else {
              //qDebug() << "graphics page item: drawing test block";
              GraphicsTestBlockItem *graphicTestBlock =
                dynamic_cast<GraphicsTestBlockItem *>(v);
              Doc::DocTestBlock *docTestBlock =
                dynamic_cast<Doc::DocTestBlock *>(b);
              if (graphicTestBlock != nullptr && docTestBlock != nullptr)
                graphicTestBlock->drawElement(docTestBlock, complete);
            }
          }
        }
      }
    }
  } else {

    GraphicsBlockItem *currentBlock = currentBlockItem();
    if (currentBlock != nullptr) {
      GraphicsTextBlockItem *graphicTextBlock =
        dynamic_cast<GraphicsTextBlockItem *>(currentBlock);
      Doc::DocTextBlock *docTextBlock =
        dynamic_cast<Doc::DocTextBlock *>(page->currentBlock());
      if (graphicTextBlock != nullptr && docTextBlock != nullptr) {

        graphicTextBlock->drawElement(docTextBlock, complete);
      } else {

        GraphicsImageBlockItem *graphicImageBlock =
          dynamic_cast<GraphicsImageBlockItem *>(currentBlock);
        Doc::DocImageBlock *docImageBlock =
          dynamic_cast<Doc::DocImageBlock *>(page->currentBlock());
        if (graphicImageBlock != nullptr && docImageBlock != nullptr) {
          graphicImageBlock->drawElement(docImageBlock, complete);
        }
        // added by kvcuong
        else {
          //qDebug() << "graphics page item: drawing current component block";

          GraphicsComponentBlockItem *graphicComponentBlock =
            dynamic_cast<GraphicsComponentBlockItem *>(currentBlock);
          Doc::DocComponentBlock *docComponentBlock =
            dynamic_cast<Doc::DocComponentBlock *>(page->currentBlock());
          if (graphicComponentBlock != nullptr &&
              docComponentBlock != nullptr) {
            graphicComponentBlock->drawElement(docComponentBlock, complete);
          } else {
            GraphicsTestBlockItem *graphicTestBlock =
              dynamic_cast<GraphicsTestBlockItem *>(currentBlock);
            Doc::DocTestBlock *docTestBlock =
              dynamic_cast<Doc::DocTestBlock *>(page->currentBlock());
            if (graphicTestBlock != nullptr && docTestBlock != nullptr)
              graphicTestBlock->drawElement(docTestBlock, complete);
          }
        }
      }
    }
    //QMessageBox::information(nullptr, "PARTIAL", "redraw");
  }

  GraphicsBlockItem *current = currentBlockItem();
  auto it1 = this->_map.begin();
  const auto it1End = this->_map.end();
  for (; it1 != it1End; ++it1) {
    GraphicsBlockItem *e = it1.value();
    assert(e);
    if (e == current)
      e->show();
    else
      e->hide();
  }
}

GraphicsBlockItem *
GraphicsPageItem::currentBlockItem()
{
  Doc::Page *p = getElement();
  if (p == nullptr)
    return nullptr;

  return _map.value(p->currentBlock());
}

void
GraphicsPageItem::synchroniseWithElement()
{
  Doc::Page *page = getElement();
  if (page == nullptr)
    return;

  Doc::Block *previousBlock = page->currentBlock();

  /* removing items that doesn't exist on the document */
  QList<Doc::DocTextBlock *> textBlocks = page->getTextBlocks();
  QList<Doc::DocImageBlock *> imageBlocks = page->getImageBlocks();
  QList<Doc::DocTestBlock *> testBlocks = page->getTestBlocks();
  QList<Doc::DocComponentBlock *> componentBlocks = page->getComponentBlocks();

  QList<Doc::Block *> otherBlocks = page->getOtherBlocks();
  QList<Doc::Block *> blocksToRemove;
  blocksToRemove.reserve(_map.size());
  //for (Doc::Block *block : _map.keys()) {
  for (auto it = _map.begin(); it != _map.end(); ++it) {
    Doc::Block *block = it.key();

    //B: static_cast were dynamic_cast here ! Not needed ???
    Doc::DocTextBlock *textBlock = static_cast<Doc::DocTextBlock *>(block);
    Doc::DocImageBlock *imageBlock = static_cast<Doc::DocImageBlock *>(block);
    Doc::DocTestBlock *testBlock = static_cast<Doc::DocTestBlock *>(block);
    Doc::DocComponentBlock *componentBlock =
      static_cast<Doc::DocComponentBlock *>(block);

    if (!otherBlocks.contains(block) && !textBlocks.contains(textBlock) &&
        !imageBlocks.contains(imageBlock) &&
        !componentBlocks.contains(componentBlock) &&
        !testBlocks.contains(testBlock))
      blocksToRemove << block;
  }
  for (Doc::Block *block : blocksToRemove)
    removeBlock(block);
  //B: why is it done in two steps ??? (first fill "blocksToRemove", then actually remove blocks)
  //   Because chnage _map and would invalidate iterators !!

  /* adding items that exists on the document and not on graphic */
  QList<Doc::Block *> blocks =
    _map
      .keys(); //B:TODO:OPTIM: is the "copy" necessary here ? could we do _map.contains(block) in the loop ??? [similar code in graphicview.cpp]
  for (Doc::DocTextBlock *block : page->getTextBlocks()) {
    page->setCurrentBlock(block);
    if (!blocks.contains(block))
      addTextBlock(block);
  }

  for (Doc::DocImageBlock *block : page->getImageBlocks()) {
    page->setCurrentBlock(block);
    if (!blocks.contains(block))
      addImageBlock(block);
  }

  for (Doc::DocTestBlock *block : page->getTestBlocks()) {
    page->setCurrentBlock(block);
    if (!blocks.contains(block))
      addTestBlock(block);
  }
  // added by kvcuong
  for (Doc::DocComponentBlock *block : page->getComponentBlocks()) {
    page->setCurrentBlock(block);
    if (!blocks.contains(block))
      addComponentBlock(block);
  }
  page->setCurrentBlock(previousBlock);
}

void
GraphicsPageItem::addTextBlock(Doc::DocTextBlock *docTextBlock)
{
  //qDebug() << "New text Block";
  DocumentController *controller =
    static_cast<DocumentController *>(getController()); //B:dynamic_cast?
  if (controller != nullptr) {
    const int x = docTextBlock->x();
    const int y = docTextBlock->y();
    const int width = docTextBlock->width();
    const int height = docTextBlock->height();
    /* if (width == 0)
         width = 400;
     if (height == 0)
         height = 600;*/

    controller->setBlockGeometry(x, y, width, height);
  }

  auto textBlockItem = new GraphicsTextBlockItem(getController());
  textBlockItem->setParentItem(this);
  _map.insert(docTextBlock, textBlockItem);
}

void
GraphicsPageItem::addImageBlock(Doc::DocImageBlock *docImageBlock)
{
  auto imageBlockItem = new GraphicsImageBlockItem(getController());
  imageBlockItem->setParentItem(this);
  _map.insert(docImageBlock, imageBlockItem);
}

void
GraphicsPageItem::addTestBlock(Doc::DocTestBlock *docTestBlock)
{
  auto testBlockItem = new GraphicsTestBlockItem(getController());
  testBlockItem->setParentItem(this);
  _map.insert(docTestBlock, testBlockItem);
}

void
GraphicsPageItem::addComponentBlock(Doc::DocComponentBlock *docComponentBlock)
{
  auto componentBlockItem = new GraphicsComponentBlockItem(getController());
  componentBlockItem->setParentItem(this);
  _map.insert(docComponentBlock, componentBlockItem);
}

void
GraphicsPageItem::removeBlock(Doc::Block *b)
{
  GraphicsBlockItem *block = _map.value(b);
  scene()->removeItem(block);
  block->setParentItem(nullptr);
  delete block;
  _map.remove(b);
}
