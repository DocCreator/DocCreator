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
                          QObject *parent = nullptr);

  virtual Doc::DocTextBlock *newTextBlock(const int blockNumber = -1) override;

private:
  using Grid = QVector<QVector<Doc::DocTextBlock *> *>;
  Grid blocks;
};

#endif // GRIDPAGELAYOUT_HPP
