#include "FontEditorController.hpp"

#include "FontEditorView.hpp"
#include "context/fontcontext.h"
#include "models/font.h"

FontEditorController::FontEditorController()
{
  _view = nullptr;
}

/* Setters */
void
FontEditorController::setView(FontEditorView *view)
{
  _view = view;
}

void
FontEditorController::addCharacter(const QString &s)
{
  if (_view == nullptr ||
      Context::FontContext::instance()->getCurrentFont() == nullptr)
    return;

  _view->addCharacter(
    Context::FontContext::instance()->getCurrentFont()->getCharacter(s));
}
