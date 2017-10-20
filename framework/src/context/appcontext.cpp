#include "appcontext.h"

namespace Context {
AppContext *AppContext::_instance;

void
AppContext::setActiveController(Mvc::IController *activeController)
{
  _activeController = activeController;
}

Mvc::IController *
AppContext::getActiveController()
{
  return _activeController;
}

AppContext *
AppContext::instance()
{
  if (_instance == nullptr)
    _instance = new AppContext();
  return _instance;
}
} //namespace Context
