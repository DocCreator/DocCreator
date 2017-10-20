#ifndef LEAF_H
#define LEAF_H

#include <framework_global.h>

#include "element.h"

namespace Doc
{
    class FRAMEWORK_EXPORT Leaf : public Element
    {
    public:
        explicit Leaf(Document* document);
    };
}

#endif // LEAF_H
