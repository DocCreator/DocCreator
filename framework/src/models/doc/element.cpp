#include "element.h"


namespace Doc
{
  Element::Element(Document* document) :
    _length(0),
    _document(document)
  {
    
  }
  
  Element::~Element()
  {
    _length = 0;
    _document = nullptr;
  }
  
  void Element::setLength(int value)
  {
    if (value < 0) {
      value = 0;
    }
    _length = value;
  }

} //namespace Doc
