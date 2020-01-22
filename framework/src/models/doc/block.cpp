#include "block.h"

namespace Doc
{
  Block::Block(int w, int h, int x, int y) :
    _width(w),
    _height(h),
    _x(x),
    _y(y),
    _marginLeft(0),
    _marginRight(0),
    _marginTop(0),
    _marginBottom(0)
  {
    if (_width < 0) {
      _width = 0;
    }
    if (_height < 0) {
      _height = 0;
    }
  }

  void Block::setWidth(int value)
  {
    if (value < 0) {
      value = 0;
    }
    _width = value;
  }
  
  void Block::setHeight(int value)
  {
    if (value < 0) {
      value = 0;
    }
    _height = value;
  }
  
  void Block::setX(int value)
  {
    _x = value;
  }
  
  void Block::setY(int value)
  {
    _y = value;
  }
  
  /*
  void Block::Initialize(int w, int h, int x, int y)
  {
    setWidth(w);
    setHeight(h);
    setX(x);
    setY(y);
    
    setMarginLeft(0);
    setMarginRight(0);
    setMarginTop(0);
    setMarginBottom(0);
  }
  */

} //namespace Doc
