#ifndef IVIEW_H
#define IVIEW_H

#include "icontroller.h"
#include <QKeyEvent>
#include <framework_global.h>

namespace Mvc {
class IView
{
public:
  virtual ~IView() {}

  virtual void keyPressEvent(QKeyEvent *event) = 0;
  virtual void keyReleaseEvent(QKeyEvent *event) = 0;

  virtual IController *getController() = 0;
};
}

#endif
