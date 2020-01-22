#include "docimageblock.h"

namespace Doc
{
  DocImageBlock::DocImageBlock(const QString &filePath) :
    Block(),
    _filePath(filePath)
    {
    }

    DocImageBlock::DocImageBlock(const QString &filePath, int w, int h, int x, int y) : 
      Block(w, h, x, y),
    _filePath(filePath)
    {
    }

    QString DocImageBlock::content() const
    {
        return "FILE:\""+_filePath+"\"";
    }
  
} //namespace Doc
