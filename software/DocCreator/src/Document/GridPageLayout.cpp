#include "GridPageLayout.hpp"

#include <cassert>

GridPageLayout::GridPageLayout(Doc::Document *doc,
                               int columns, int rows, int blockSpacing,
			       int leftMargin, int rightMargin,
			       int topMargin, int bottomMargin,
                               QObject *parent)
  : PageLayout(doc, parent, leftMargin, rightMargin, topMargin, bottomMargin),
    _blocks(columns*rows, nullptr)
{
  assert(columns>0);
  assert(rows>0);
  assert(blockSpacing>=0);
  assert(_blocks.size() == rows*columns);

  const int blockWidth = std::max((_page.width() - (columns-1)*blockSpacing)/columns, 0);
  const int blockHeight = std::max((_page.height() - (rows-1)*blockSpacing)/rows, 0);
  
  int n = 0;
  for (int i=0; i<columns; ++i) {
    for (int j=0; j<rows; ++j) {

      assert(n<_blocks.size());
      assert(_blocks[n] == nullptr);

      //the coordinates of the block are in document (and not page) coordinate system
      const int x_block = _page.x()
	+ i * (blockWidth + blockSpacing);
      const int y_block = _page.y()
	+ j * (blockHeight + blockSpacing);

      Doc::DocTextBlock *textBlock = creatTextBlock(x_block, y_block, blockWidth, blockHeight);
	
      _blocks[n] = textBlock;

      ++n;
    }
  }

}


GridPageLayout::GridPageLayout(Doc::Document *doc,
			       const QVector<QRect> &blocks,
			       QObject *parent)
  : PageLayout(doc, parent),
    _blocks(blocks.size(), nullptr)
{
  const size_t numBlocks = blocks.size();
  for (size_t i=0; i<numBlocks; ++i) {

    const QRect &r = blocks[i];
    int block_x0 = r.x();
    int block_y0 = r.y();
    int block_x1 = r.x() + r.width();
    int block_y1 = r.y() + r.height();
    block_x0 = std::min(std::max(block_x0, 0), _width-1);
    block_y0 = std::min(std::max(block_y0, 0), _height-1);
    block_x1 = std::min(std::max(block_x1, 0), _width-1);
    block_y1 = std::min(std::max(block_y1, 0), _height-1);
    int block_width = std::max(0, block_x1-block_x0);
    int block_height = std::max(0, block_y1-block_y0);

    Doc::DocTextBlock *textBlock = creatTextBlock(block_x0, block_y0, block_width, block_height);

    _blocks[i] = textBlock;
  }

}

GridPageLayout::~GridPageLayout()
{
  const size_t numBlocks = _blocks.size();
  for (size_t i=0; i<numBlocks; ++i) {
    delete _blocks[i];
  }

}


Doc::DocTextBlock *
GridPageLayout::takeTextBlock(int blockNumber)
{
  if (blockNumber < _blocks.size()) {
    Doc::DocTextBlock *textBlock = _blocks[blockNumber];
    _blocks[blockNumber] = nullptr;
    return textBlock;
  }
  return nullptr;
}
