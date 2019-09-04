#include "KeyboardView.hpp"

#include <cassert>

#include "Document/DocumentController.hpp"
#include "FontEditor/FontEditorController.hpp"
#include "VirtualKeyboard/CharacterButtonView.hpp"
#include "VirtualKeyboard/ControlButtonView.hpp"
#include "VirtualKeyboard/KeyboardController.hpp"
#include "context/appcontext.h"
#include "context/fontcontext.h"
#include "models/character.h"
#include "models/characterdata.h"
#include "models/font.h"
#include <QIcon>

KeyboardView::KeyboardView(QWidget *parent)
  : QWidget(parent)
  , _controller(nullptr)
  , _documentController(nullptr)
  , _fontEditorController(nullptr)
  , _currentMode(LowerCase)
  , _ctrlPressed(false)
  , _altPressed(false)
  , _shiftPressed(false)
  , _altGrPressed(false)
  , _currentFontName()
{}

/* Setters */
void
KeyboardView::setKeyboardController(KeyboardController *keyboardController)
{
  _controller = keyboardController;
  _controller->setView(this);
}

void
KeyboardView::setDocumentController(DocumentController *documentController)
{
  _documentController = documentController;
}

void
KeyboardView::setFontEditorController(
  FontEditorController *fontEditorController)
{
  _fontEditorController = fontEditorController;
}

/* Events */
void
KeyboardView::keyPressEvent(QKeyEvent *e)
{
  if (e->matches(QKeySequence::Copy)) {
    _documentController->copy();
  } else if (e->matches(QKeySequence::Cut)) {
    _documentController->cut();
  } else if (e->matches(QKeySequence::Paste)) {
    _documentController->paste();
  } else {
    const int charKeyboardCode =
      _shiftPressed ? static_cast<QChar>(e->key()).unicode()
                    : static_cast<QChar>(e->key()).toLower().unicode();
    const int controlKey = e->key();

    auto it1 = _controlButtons.find(controlKey);
    if (it1 != _controlButtons.end()) {
      it1.value()->animateClick();
    } else {
      auto it2 = _characterButtonViews.find(charKeyboardCode);
      if (it2 != _characterButtonViews.end()) {
        it2.value()->animateClick();
      }
    }
  }
}

void
KeyboardView::keyReleaseEvent(QKeyEvent *e)
{
  processControlButtonRelease(e->key());
}

Mvc::IController *
KeyboardView::getController()
{
  return _controller;
}

void
KeyboardView::addCharButtonView(CharacterButtonView *view)
{
  _characterButtonViewsH.insert(view); //B: even if no keys ???
  for (int id : view->keys()) {
    if (id != -1)
      _characterButtonViews.insert(id, view);
  }

  view->setParent(this);
}

/*
void KeyboardView::removeCharButtonView(const int & id)
{
  auto it = _characterButtonViews.find(id);
  if (it != _characterButtonViews.end())
    _characterButtonViews.erase(it); //TODO: remove from _characterButtonViewsH if ??? !
}
*/

//B: all CharacterButtonView have this KeyboardView as parent, so they should be freed on destruction
// This method is not really needed.
/*
void KeyboardView::clearCharButtonViewList()
{
  for (auto it = _characterButtonViewsH.begin(); it != _characterButtonViewsH.end(); ++it) {
    CharacterButtonView* view = *it;
    view->setParent(nullptr);
    delete view;
  }
  _characterButtonViews.clear();
}
*/

void
KeyboardView::addControlButton(int id, ControlButtonView *button)
{
  _controlButtons.insert(id, button);
  button->setParent(this);
}

void
KeyboardView::removeControlButton(int id)
{
  auto it = _controlButtons.find(id);
  if (it != _controlButtons.end())
    _controlButtons.erase(it);
}

void
KeyboardView::clearControlButtonsList()
{
  auto it = _controlButtons.begin();
  const auto itEnd = _controlButtons.end();
  for (; it != itEnd; ++it) {
    ControlButtonView *button = it.value();
    button->setParent(nullptr);
    delete button;
  }
  _controlButtons.clear();
}

void
KeyboardView::processCharButton(int keyboardCode, int fontCode, int id)
{
  if (!_characterButtonViews.contains(keyboardCode))
    return;

  QString s = static_cast<QChar>(fontCode);
  Mvc::IController *activeController =
    Context::AppContext::instance()->getActiveController();

  if (_documentController != nullptr &&
      activeController == _documentController) {
    _documentController->addCharacter(s, id);
  } else if (_fontEditorController != nullptr &&
             activeController == _fontEditorController) {
    _fontEditorController->addCharacter(s);
  }
}

void
KeyboardView::processControlButton(int key)
{
  int controlKey = key;

  if (controlKey == Qt::Key_Control) {
    _ctrlPressed = true;
    _controlButtons.value(Qt::Key_Control)->pressKey();
    redrawKeyboard();
  } else if (controlKey == Qt::Key_Alt) {
    _altPressed = true;
    _controlButtons.value(Qt::Key_Alt)->pressKey();
    redrawKeyboard();
  } else if (controlKey == Qt::Key_Shift)
    _shiftPressed = true;
  else if (controlKey == Qt::Key_AltGr)
    _altGrPressed = true;

  if (controlKey != Qt::Key_Shift && _ctrlPressed && _altPressed) {
    controlKey = Qt::Key_AltGr;
    _altGrPressed = true;
  }

  if (!_controlButtons.contains(key))
    return;

  //const int SIZE_TAB=5; //B:WARNING: SIZE_TAB also defined in documentcontroller.cpp

  switch (controlKey) {
    case Qt::Key_Return: //Enter
      _documentController->addParagraph();
      break;
    case Qt::Key_Tab: //Tab
      //for(int i = 0; i < SIZE_TAB; ++i)
      //_documentController->addCharacter(" ");
      _documentController->addCharacter(QStringLiteral("\t"));
      break;
    case Qt::Key_Delete: //Del
      _documentController->removeAfterCursor();
      break;
    case Qt::Key_Backspace: //Backspace
      _documentController->removeBeforeCursor();
      break;
    case Qt::Key_AltGr:
      _currentMode = Alternate;
      _controlButtons.value(Qt::Key_AltGr)->pressKey();
      redrawKeyboard();
      break;
    case Qt::Key_Shift:
      _currentMode = UpperCase;
      _controlButtons.value(Qt::Key_Shift)->pressKey();
      redrawKeyboard();
      break;
    default:;
  }
}

void
KeyboardView::processControlButtonRelease(int key)
{
  switch (key) {
    case Qt::Key_Control:
      _ctrlPressed = false;
      if (_shiftPressed && _altGrPressed) {
        _currentMode = UpperCase;
      } else {
        _currentMode = LowerCase;
      }
      _altGrPressed = false;
      _controlButtons.value(Qt::Key_Control)->releaseKey();
      _controlButtons.value(Qt::Key_AltGr)->releaseKey();
      redrawKeyboard();
      break;
    case Qt::Key_Alt:
      _altPressed = false;
      if (_shiftPressed && _altGrPressed) {
        _currentMode = UpperCase;
      } else {
        _currentMode = LowerCase;
      }
      _altGrPressed = false;
      _controlButtons.value(Qt::Key_Alt)->releaseKey();
      _controlButtons.value(Qt::Key_AltGr)->releaseKey();
      redrawKeyboard();
      break;
    case Qt::Key_AltGr:
      _altGrPressed = false;
      if (_shiftPressed)
        _currentMode = UpperCase;
      else
        _currentMode = LowerCase;
      _controlButtons.value(Qt::Key_AltGr)->releaseKey();
      redrawKeyboard();
      break;
    case Qt::Key_Shift:
      _shiftPressed = false;
      if (_altGrPressed)
        _currentMode = Alternate;
      else
        _currentMode = LowerCase;
      _controlButtons.value(Qt::Key_Shift)->releaseKey();
      redrawKeyboard();
      break;
    default:;
  }
}

void
KeyboardView::mapKeyboardCodeValuesToFontCodes(const Models::Font *font)
{
  const int SPACE = 32;

  QList<int> checkedFontCodes; //B:TODO:OPTIM replace with unordered_set !!!!
  QList<int> checkedKBCodes;

  //- 0) get uniques CharacterButtonView* in characterButtonViewsH

  const int size = _characterButtonViews.size();
  checkedKBCodes.reserve(size);
  checkedFontCodes.reserve(size);

  //- 1) traverse keyboard, associate keyboard code to equal font character if present

  // (on considère que _characterButtonViews contient tous les keyboardCodes de
  // tous les CharacterButtonViews)
  auto it = _characterButtonViews.begin();
  const auto itEnd = _characterButtonViews.end();

  for (; it != itEnd; ++it) {
    CharacterButtonView *charButtonView = it.value();
    const int k = it.key();
    assert(charButtonView->hasKeyboardCode(k));

    charButtonView->eraseFontCode(
      k); //remove all font codes associated to all keyboard codes.

    if (k == SPACE) { // space
      charButtonView->addKeyboardCodesMapToFontCodesValues(k, k);
      if (!checkedFontCodes.contains(k))
        checkedFontCodes.push_back(k);
    } else {
      const QString charValue = QString(static_cast<QChar>(k));
      const Models::Character *c = font->getCharacter(
        charValue); //B:TODO:OPTIM: we could construct beforehand an HashMap that at each QString or (better) at each k associates a Models::Character

      if (
        c != nullptr &&
        !checkedFontCodes.contains(
          k)) { //B: checkedFontCodes.contains(k) should not happen if we checked in KeyboardView::addCharButtonView() that we have no duplicates ????
        charButtonView->addKeyboardCodesMapToFontCodesValues(k, k);
        checkedFontCodes.push_back(k);
        checkedKBCodes.push_back(k);
      }
    }
  }

  //- 2) add font characters not yet associated with a keyboard code

  //B:TODO:OPTIM
  // checkedFontCodes & checkedKBCodes devraient être des unordered_set
  // On devrait avoir une unordered_set des caracteres non encore affectés et
  // qu'on mettrait à jour dès qu'un caractère est affecté (on l'enleverait du
  // unordered_set)
  //Est-ce que checkedFontCodes sert vraiment à quelquechose ????

  const Models::CharacterMap &fontCharsMap = font->getCharacters();
  auto iter = fontCharsMap.begin();

  // fill keyboard mode = lowercase
  if (checkedFontCodes.size() < fontCharsMap.size()) {
    ModeTypeEnum mode = LowerCase;
    auto it1 = _characterButtonViewsH.begin();
    const auto it1End = _characterButtonViewsH.end();
    for (; it1 != it1End; ++it1) {
      CharacterButtonView *charButtonView = *it1;
      if (charButtonView->getKeyboardCode(LowerCase) != SPACE) {
        // lowercase
        const int keyboardCode = charButtonView->getKeyboardCode(mode);
        const int fontCode = charButtonView->getFontCode(mode);
        if (fontCode == -1) {
          int indexChar = 0;
          for (
            iter = fontCharsMap.begin(); iter != fontCharsMap.end(); ++iter,
           ++indexChar) { //TODO:OPTIM: on ne veut pas retraverser tous les caracteres pour savoir ceux qui sont allloués !!!
            const int u = iter.key().unicode()->unicode();
            if (!checkedFontCodes.contains(u) &&
                !checkedKBCodes.contains(keyboardCode)) {
              const int fontCodeValue = u;
              charButtonView->addKeyboardCodesMapToFontCodesValues(
                keyboardCode, fontCodeValue);
              //std::cerr << "LOWER index=" << indexChar << " char=" << iter.key().toStdString() << " kbCode=" << keyboardCode << " fontCode=" << fontCodeValue<<"\n";
              checkedFontCodes.push_back(fontCodeValue);
              checkedKBCodes.push_back(keyboardCode);
              break;
            }
          }
        }
      }
    }
  }

  // fill keyboard mode = uppercase
  if (checkedFontCodes.size() < fontCharsMap.size()) {
    auto it1 = _characterButtonViewsH.begin();
    const auto it1End = _characterButtonViewsH.end();
    for (; it1 != it1End; ++it1) {
      CharacterButtonView *charButtonView = *it1;
      if (charButtonView->getKeyboardCode(LowerCase) != SPACE) {
        // uppercase
        const int keyboardCode_Uppercase =
          charButtonView->getKeyboardCode(UpperCase);

        if (!charButtonView->hasFontCodeForKbCode(keyboardCode_Uppercase)) {
          int indexChar = 0;
          for (iter = fontCharsMap.begin(); iter != fontCharsMap.end();
               ++iter, ++indexChar) {
            const int u = iter.key().unicode()->unicode();
            if (!checkedFontCodes.contains(u) &&
                !checkedKBCodes.contains(keyboardCode_Uppercase)) {
              const int fontCodeValue = u;
              charButtonView->addKeyboardCodesMapToFontCodesValues(
                keyboardCode_Uppercase, fontCodeValue);
              //std::cerr << "UPPER index=" << indexChar << " char=" << iter.key().toStdString() << " kbCode=" << keyboardCode_Uppercase << " fontCode=" << fontCodeValue<<"\n";
              checkedFontCodes.push_back(fontCodeValue);
              checkedKBCodes.push_back(keyboardCode_Uppercase);
              break;
            }
          }
        }
      }
    }
  }

  // fill keyboard mode = alternate
  if (checkedFontCodes.size() < fontCharsMap.size()) {
    auto it1 = _characterButtonViewsH.begin();
    const auto it1End = _characterButtonViewsH.end();
    for (; it1 != it1End; ++it1) {
      CharacterButtonView *charButtonView = *it1;
      if (charButtonView->getKeyboardCode(LowerCase) != SPACE) {
        // alternate
        const int keyboardCode_Alternate =
          charButtonView->getKeyboardCode(Alternate);

        if (!charButtonView->hasFontCodeForKbCode(keyboardCode_Alternate)) {
          int indexChar = 0;
          for (iter = fontCharsMap.begin(); iter != fontCharsMap.end();
               ++iter, ++indexChar) {
            const int u = iter.key().unicode()->unicode();
            if (!checkedFontCodes.contains(u) &&
                !checkedKBCodes.contains(keyboardCode_Alternate)) {
              const int fontCodeValue = u;
              charButtonView->addKeyboardCodesMapToFontCodesValues(
                keyboardCode_Alternate, fontCodeValue);
              //std::cerr << "ALTER index=" << indexChar << " char=" << iter.key().toStdString() << " kbCode=" << keyboardCode_Alternate << " fontCode=" << fontCodeValue<<"\n";
              checkedFontCodes.push_back(fontCodeValue);
              checkedKBCodes.push_back(keyboardCode_Alternate);
              break;
            }
          }
        }
      }
    }
  }
}

void
KeyboardView::drawKeyboard(Models::Font *font)
{
  if (font == nullptr)
    return;

  //assert(font != nullptr);
  if (_currentFontName != font->getName()) {
    //change font
    mapKeyboardCodeValuesToFontCodes(font);
    _currentFontName = font->getName();
  }

  auto it1 = _characterButtonViewsH.begin();
  const auto it1End = _characterButtonViewsH.end();
  for (; it1 != it1End; ++it1) {
    CharacterButtonView *charButtonView = *it1;
    charButtonView->setText(QString());
    charButtonView->setIcon(QIcon());
    charButtonView->setMode(_currentMode);

    const int fontCodeValue = charButtonView->getFontCodeValue();

    if (fontCodeValue > -1) {

      const QString charValue(static_cast<QChar>(fontCodeValue));
      Models::Character *c = font->getCharacter(charValue);

      if (c != nullptr) {
        charButtonView->setEnabled(true);
        charButtonView->setText(charValue);
        charButtonView->setIcon(
          QIcon(QPixmap::fromImage(c->getRandomCharacterData()->getImage())));
      } else {
        charButtonView->setEnabled(false);
      }
    } else {
      charButtonView->setEnabled(false);
    }
  }
}

void
KeyboardView::redrawKeyboard()
{
  drawKeyboard(Context::FontContext::instance()->getCurrentFont());
}
