#include "KeyboardController.hpp"

#include "KeyboardView.hpp"
#include "context/fontcontext.h"

KeyboardController::KeyboardController()
{
  _view = nullptr;
}

/* Setters */
void
KeyboardController::setView(KeyboardView *view)
{
  _view = view;
}

/* Events */
void
KeyboardController::keyPressEvent(QKeyEvent *e)
{
  if (_view == nullptr) {
    return;
  }
  _view->keyPressEvent(e);
}

void
KeyboardController::keyReleaseEvent(QKeyEvent *e)
{
  if (_view == nullptr) {
    return;
  }
  _view->keyReleaseEvent(e);
}

void
KeyboardController::update()
{
  //QString keyboardPath = Core::ConfigurationManager::get(AppConfigKeyBoardGroup, AppConfigKbFolderKey).toString() + Core::ConfigurationManager::get(AppConfigKeyBoardGroup, AppConfigKbDefautlFormatKey).toString();

  _view->drawKeyboard(Context::FontContext::instance()->getCurrentFont());
}
