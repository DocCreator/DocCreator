#include "leaf.h"

namespace Doc
{
    Leaf::Leaf(Document* document) : 
      Element(document)
    {
        setLength(1);
    }

} //namespace Doc
