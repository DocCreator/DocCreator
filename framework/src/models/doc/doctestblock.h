#ifndef DOCTESTBLOCK_H
#define DOCTESTBLOCK_H

#include <QString>
#include <framework_global.h>

#include "block.h"


namespace Doc
{
  class FRAMEWORK_EXPORT DocTestBlock : public Block
  {
  public:
    explicit DocTestBlock(const QString &filePath);
    DocTestBlock(const QString &filePath, int w, int h, int x, int y);

    void setFilePath(const QString &filePath);
    QString filePath() const { return _filePath; }

    QString content() const override;

  private:
    QString _filePath;

  };
}
#endif // DOCTESTBLOCK_H
