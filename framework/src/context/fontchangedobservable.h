#ifndef FONTCHANGEDOBSERVABLE_H
#define FONTCHANGEDOBSERVABLE_H

#include <QList>
#include <framework_global.h>

namespace Context {

class IFontChangedObserver;

class FRAMEWORK_EXPORT FontChangedObservable
{
public:
  static void add(IFontChangedObserver *observer);
  static void remove(IFontChangedObserver *observer);
  static void notifyFontChanged();

private:
  static QList<IFontChangedObserver *> _fontChangedObservers;
};
}

#endif // FONTCHANGEDOBSERVABLE_H
