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
      Context::FontContext::instance()->getCurrentFont() == nullptr) {
    return;
  }

  //B: a character that is not in the font gave nullptr, which the view
  // dereferenced (CharEditScene::setCenterChar() for example)
  Models::Character *character =
    Context::FontContext::instance()->getCurrentFont()->getEditableCharacter(s);
  if (character == nullptr) {
    return;
  }
  _view->addCharacter(character);
}
