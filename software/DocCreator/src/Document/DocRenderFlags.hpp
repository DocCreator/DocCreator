#ifndef DOCRENDERFLAGS_HPP
#define DOCRENDERFLAGS_HPP

#include <QFlags>

enum DocRenderFlag
{
  Color = 0x1,
  WithTextBlocks = 0x2,
  WithImageBlocks = 0x4
};

Q_DECLARE_FLAGS(DocRenderFlags, DocRenderFlag)

Q_DECLARE_OPERATORS_FOR_FLAGS(DocRenderFlags)

#endif /* ! DOCRENDERFLAGS_HPP */
