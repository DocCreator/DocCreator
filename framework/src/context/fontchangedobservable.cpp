#include "fontchangedobservable.h"

#include "ifontchangedobserver.h"

namespace Context {
QList<IFontChangedObserver *> FontChangedObservable::_fontChangedObservers;

void
FontChangedObservable::add(IFontChangedObserver *observer)
{
  if (observer != nullptr && !_fontChangedObservers.contains(observer)) {
    _fontChangedObservers.append(observer);
  }
}

void
FontChangedObservable::remove(IFontChangedObserver *observer)
{
  if (observer != nullptr) // && fontChangedObservers.contains(observer))
    _fontChangedObservers.removeOne(
      observer); //B: will do nothing if observer is not in list
}

void
FontChangedObservable::notifyFontChanged()
{
  const QList<IFontChangedObserver *> &fontChangedObservers =
    _fontChangedObservers;
  for (IFontChangedObserver *observer : fontChangedObservers) {
    observer->onFontChanged();
  }
}
} //namespace Context
