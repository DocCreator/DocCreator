#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include "framework_global.h"

#include <patterns/singleton.h>

namespace Mvc {
class IController;
}

namespace Context {

class FRAMEWORK_EXPORT AppContext : public Patterns::Singleton<AppContext>
{
private:
  Mvc::IController *_activeController;
  static AppContext *_instance;

public:
  void setActiveController(Mvc::IController *activeController);
  Mvc::IController *getActiveController();
  static AppContext *instance();
};
}

#endif //APPCONTEXT_H
