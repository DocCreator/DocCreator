#include "observable.h"

#include "observer.h"

namespace Patterns {

void
Observable::addObserver(Observer *observer)
{
  if (!observers.contains(observer) && observer != nullptr) {
    observers.append(observer);
  }
}

void
Observable::removeObserver(Observer *observer)
{
  if (observer != nullptr && observers.contains(observer)) {
    observers.removeOne(observer);
  }
}

void
Observable::notifyAll()
{
  for (Observer *observer : observers) {
    observer->update();
  }
}
} //namespace Patterns
