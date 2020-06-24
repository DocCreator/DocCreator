#include "GridPageLayout.hpp"

#include <cassert>


GridPageLayout::GridPageLayout(Doc::Document *doc,
                               int columns,
                               int rows,
			       int blockSpacing,
                               QObject *parent)
  : PageLayout(doc, parent),
    _blocks(columns*rows, nullptr),
    _columns(columns),
    _rows(rows),
    _blockSpacing(blockSpacing),
    _blockWidth(0), _blockHeight(0)
{
  assert(columns>0);
  assert(rows>0);
  assert(blockSpacing>=0);
  assert(_blocks.size() == rows*columns);

}

Doc::DocTextBlock *
GridPageLayout::newTextBlock(int blockNumber)
{
  _blockWidth = std::max((page.width() - (_columns-1)*_blockSpacing)/_columns, 0);
  _blockHeight = std::max((page.height() - (_rows-1)*_blockSpacing)/_rows, 0);
  //B: _blockWidth & _blockHeight must be computed each time
  //  because page may have changed.
  
  Doc::DocTextBlock *addedTextBlock = nullptr;
  int n = 0;
  for (int i=0; i<_columns; ++i) {
    for (int j=0; j<_rows; ++j) {

      assert(n<_blocks.size());
      
      if (blockNumber == n && _blocks[n] != nullptr) {
	return nullptr;
	//B: original code was like this... Is it really what we want ?
      }
      
      if ((_blocks[n] == nullptr && blockNumber == -1) ||
	  (blockNumber == n && _blocks[n] == nullptr)) {
	const int x_block = page.x() //? are the coords in document or page coordinate system ???
	  + i * (_blockWidth + _blockSpacing);
	const int y_block = page.y() //? are the coords in document or page coordinate system ???
	  + j * (_blockHeight + _blockSpacing);

	addedTextBlock = creatTextBlock(x_block, y_block, _blockWidth, _blockHeight);
	
	_blocks[n] = addedTextBlock;
	return addedTextBlock;
      }

      ++n;
    }
  }
  return addedTextBlock;
}
