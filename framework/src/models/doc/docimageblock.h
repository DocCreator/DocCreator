#ifndef DOCIMAGEBLOCK_H
#define DOCIMAGEBLOCK_H

#include <QString>
#include <framework_global.h>
#include "block.h"

namespace Doc
{
  class FRAMEWORK_EXPORT DocImageBlock : public Block
    {
    public:
        explicit DocImageBlock(const QString &filePath);
        DocImageBlock(const QString &filePath, int w, int h, int x, int y);

        void setFilePath(const QString &filePath);
        QString filePath() const { return _filePath; }

        QString content() const override;

    private:
        QString _filePath;

    };
}

#endif // DOCIMAGEBLOCK_H
