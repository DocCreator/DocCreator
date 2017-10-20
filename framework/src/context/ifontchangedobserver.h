#ifndef FONTCHANGEDOBSERVER_H
#define FONTCHANGEDOBSERVER_H

#include <framework_global.h>

namespace Context {
class FRAMEWORK_EXPORT IFontChangedObserver
{
public:
  virtual ~IFontChangedObserver() {}

  virtual void onFontChanged() = 0;
};
}

#endif // FONTCHANGEDOBSERVER_H
