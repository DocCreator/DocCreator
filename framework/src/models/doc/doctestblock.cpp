#include "doctestblock.h"

namespace Doc
{
DocTestBlock::DocTestBlock(const QString &filePath) :
  _filePath(filePath)
{

}

DocTestBlock::DocTestBlock(const QString &filePath, int w, int h, int x, int y) : 
  Block(w, h, x, y),
  _filePath(filePath)
{

}

QString DocTestBlock::content() const
{
    return "FILE:\""+_filePath+"\"";
}

} //namespace Doc
