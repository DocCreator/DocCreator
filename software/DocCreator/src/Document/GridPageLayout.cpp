#include "GridPageLayout.hpp"

#include <QDebug>

GridPageLayout::GridPageLayout(Doc::Document *doc,
                               int columns,
                               int rows,
                               QObject *parent)
  : PageLayout(doc, parent)
{
  blocks.resize(columns);
  for (auto &b : blocks) {
    b = new QVector<Doc::DocTextBlock *>(rows);
  }
}

Doc::DocTextBlock *
GridPageLayout::newTextBlock(const int blockNumber)
{
  const int h = (page.height() - page.y()) / blocks.size();
  const int w = (page.width() - page.x()) / blocks.at(0)->size();
  int nb = 0;
  Doc::DocTextBlock *addedTextBlock = nullptr;
  bool added = false;
  for (int i = 0; i < blocks.size() && !added; ++i) {
    for (int j = 0; j < blocks[i]->size() && !added; ++j) {
      if (blocks[i]->at(j) == nullptr) {
        //qDebug() << "i,j" << i << "," << j;
        if (blockNumber == -1 || blockNumber == nb) {
          //qDebug() << "Added";
          QVector<Doc::DocTextBlock *> *jBlock = blocks.at(i);
          addedTextBlock =
            creatTextBlock(j * w + page.x(), i * h + page.y(), w, h);
          (*jBlock)[j] = addedTextBlock;
          added = true;
        }
        if (added && (j < blocks.at(i)->size() - 1 && i <= blocks.size() - 1))
          addedTextBlock->setMarginRight(50);
      }
      ++nb;
    }
  }
  return addedTextBlock;
}
