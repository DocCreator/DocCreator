#include "docstyle.h"

namespace Doc
{
  DocStyle::DocStyle(const QString &name) :
    _name(name),
    _fontName(name) //B
  {
  }

  DocStyle::DocStyle(const QString &name, const QString &fontName) :
    _name(name), 
    _fontName(fontName)
  {

  }

    DocStyle* DocStyle::clone() const
    {
      auto s = new DocStyle(_name, _fontName);
      return s;
    }
}//namespace Doc
