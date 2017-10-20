#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <QList>
#include <framework_global.h>

namespace Patterns {

class Observer;

class FRAMEWORK_EXPORT Observable
{
public:
  void addObserver(Observer *observer);
  void removeObserver(Observer *observer);
  void notifyAll();

private:
  QList<Observer *> observers;
};
}

#endif // OBSERVABLE_H
