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
  GridPageLayout(Doc::Document *doc,
		 int columns, int rows, int blockSpcing = 50,
		 int leftMargin = 0, int rightMargin = 0,
		 int topMargin = 0, int bottomMargin = 0,
		 QObject *parent = nullptr);

  GridPageLayout(Doc::Document *doc,
		 const QVector<QRect> &blocks,
		 QObject *parent = nullptr);

  ~GridPageLayout();

  /*
    Get @a i-th DocTextBlock.
    Take ownership of returned DocTextBlock
   */
  Doc::DocTextBlock *takeTextBlock(int blockNumber) override;

private:
  //using Grid = QVector<QVector<Doc::DocTextBlock *> *>;
  //Grid blocks;

  using Grid = QVector<Doc::DocTextBlock *>;
  Grid _blocks;
};

#endif // GRIDPAGELAYOUT_HPP
