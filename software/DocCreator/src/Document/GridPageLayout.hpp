#ifndef GRIDPAGELAYOUT_HPP
#define GRIDPAGELAYOUT_HPP

#include "PageLayout.hpp"
#include <QVector>

namespace Doc {
class DocTextBlock;
class Document;
}

class GridPageLayout : public PageLayout
{

public:
  explicit GridPageLayout(Doc::Document *doc,
                          int columns,
                          int rows,
			  int blockSpcing = 50,
                          QObject *parent = nullptr);

  Doc::DocTextBlock *newTextBlock(const int blockNumber = -1) override;

private:
  //using Grid = QVector<QVector<Doc::DocTextBlock *> *>;
  //Grid blocks;

  using Grid = QVector<Doc::DocTextBlock *>;
  Grid _blocks;
  int _columns;
  int _rows;
  int _blockSpacing;
  int _blockWidth;
  int _blockHeight;
};

#endif // GRIDPAGELAYOUT_HPP
