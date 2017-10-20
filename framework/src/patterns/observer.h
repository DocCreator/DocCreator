#ifndef OBSERVER_H
#define OBSERVER_H

#include <framework_global.h>

namespace Patterns {
class FRAMEWORK_EXPORT Observer
{
public:
  virtual ~Observer() {}

  virtual void update() = 0;
};
}

#endif // OBSERVER_H
