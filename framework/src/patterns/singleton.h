#ifndef SINGLETON_H
#define SINGLETON_H

#include <framework_global.h>

namespace Patterns {
template<class T>
class Singleton
{
public:
  static T *instance()
  {
    if (_singleton == nullptr)
      _singleton = new T();

    return _singleton;
  }

private:
  static T *_singleton;
};
template<class T>
T *Singleton<T>::_singleton = nullptr;
}

#endif
