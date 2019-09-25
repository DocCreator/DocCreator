#include "CharacterButtonView.hpp"

#include <cassert>
#include <iostream>

#include "KeyboardView.hpp"
#include "Utils/CharacterListView.hpp"
#include "Utils/CharacterListWidgetItem.hpp"
#include "context/fontcontext.h"
#include "models/characterdata.h"
#include "models/font.h"

CharacterButtonView::CharacterButtonView(KeyboardView *parent)
  : QPushButton(parent)
  , _currentMode(LowerCase)
{
  connect(this, SIGNAL(clicked()), this, SLOT(buttonClicked()));
}

void
CharacterButtonView::addModeCharacter(ModeTypeEnum mode, int charValue)
{
  assert(mode != NUMBER_MODES);
  _keys[mode].kbCode = charValue;
}

void
CharacterButtonView::removeModeCharacter(ModeTypeEnum mode)
{
  assert(mode != NUMBER_MODES);
  _keys[mode].kbCode = -1;
}

int
CharacterButtonView::getKeyboardCode(ModeTypeEnum mode) const
{
  assert(mode != NUMBER_MODES);
  return _keys[mode].kbCode;
}

int
CharacterButtonView::getKeyboardCode() const
{
  assert(_currentMode != NUMBER_MODES);
  return _keys[_currentMode].kbCode;
}

int
CharacterButtonView::getFontCode(ModeTypeEnum mode) const
{
  assert(mode != NUMBER_MODES);
  return _keys[mode].fontCode;
}

QList<int>
CharacterButtonView::keys() const
{
  QList<int> kbCodes;
  kbCodes.reserve(NUMBER_MODES);
  for (int i = 0; i < NUMBER_MODES; ++i) {
    kbCodes.push_back(_keys[i].kbCode);
  }
  return kbCodes;
}

void
CharacterButtonView::setMode(ModeTypeEnum mode)
{
  assert(mode != NUMBER_MODES);
  _currentMode = mode;
}

// keyboard code value mapping with font code value: unicode 16 bit
void
CharacterButtonView::addKeyboardCodesMapToFontCodesValues(int kbCodeValue,
                                                          int fontCodeValue)
{
  for (int i = 0; i < NUMBER_MODES; ++i) {
    if (_keys[i].kbCode == kbCodeValue) {
      _keys[i].fontCode = fontCodeValue;
      return;
    }
  }

  std::cerr << "Warning: keayboard code " << kbCodeValue
            << " not found in CharacterButtonView " << this << "\n";
}

void
CharacterButtonView::eraseFontCode(int kbCodeValue)
{
  addKeyboardCodesMapToFontCodesValues(kbCodeValue, -1);
}

int
CharacterButtonView::getFontCodeValue(int kbCodeValue) const
{
  for (int i = 0; i < NUMBER_MODES; ++i) {
    if (_keys[i].kbCode == kbCodeValue) {
      return _keys[i].fontCode;
    }
  }
  std::cerr << "Warning: keayboard code " << kbCodeValue
            << " not found in CharacterButtonView " << this << "\n";
  return -1;
}

int
CharacterButtonView::getFontCodeValue() const
{
  assert(_currentMode != NUMBER_MODES);
  return _keys[_currentMode].fontCode;
}

/*
QList<int> CharacterButtonView::getMappedFontCodeValues()
{
  return _keyboardCodesMapToFontCodesValues.keys();
}
*/
bool
CharacterButtonView::hasKeyboardCode(int kbCode) const
{
  for (int i = 0; i < NUMBER_MODES; ++i) {
    if (_keys[i].kbCode == kbCode)
      return true;
  }
  return false;
}
bool
CharacterButtonView::hasFontCodeForKbCode(int kbCode) const
{
  for (int i = 0; i < NUMBER_MODES; ++i) {
    if (_keys[i].kbCode == kbCode && _keys[i].fontCode != -1)
      return true;
  }
  return false;
}

/* Character choice */
void
CharacterButtonView::openCharacterChoice()
{
  const int fontCode = getFontCodeValue();
  assert(fontCode !=
         -1); //Button shoud be disabled if not associated to a font code
  const QString charValue = QString(QChar(fontCode));
  Models::Character *c =
    Context::FontContext::instance()->getCurrentFont()->getCharacter(charValue);
  assert(c);
  //Models::CharacterDataList charDatas = c->getAllCharacterData();

  auto characterList =
    new CharacterListView(c, pos(), static_cast<QWidget *>(parent()));

  connect(characterList,
          SIGNAL(characterItemClicked(CharacterListWidgetItem*)),
          this,
          SLOT(onCharacterItemClicked(CharacterListWidgetItem*)));

  characterList->show();
}

/* Events */
void
CharacterButtonView::keyPressEvent(QKeyEvent *e)
{
  assert(parent());
  static_cast<KeyboardView *>(parent())->keyPressEvent(e);
}

void
CharacterButtonView::keyReleaseEvent(QKeyEvent *e)
{
  assert(parent());
  static_cast<KeyboardView *>(parent())->keyReleaseEvent(e);
}

Mvc::IController *
CharacterButtonView::getController()
{
  assert(parent());
  return static_cast<KeyboardView *>(parent())->getController();
}

void
CharacterButtonView::mousePressEvent(QMouseEvent *e)
{
  if (e->button() == Qt::RightButton) {
    openCharacterChoice();
  }
  else {
    QPushButton::mousePressEvent(e);
  }
}

/* Slots */
void
CharacterButtonView::buttonClicked()
{
  const int keyboardCode = getKeyboardCode();
  const QChar c = static_cast<QChar>(keyboardCode);
  const int fontCode = getFontCodeValue();
  assert(fontCode !=
         -1); //Button shoud be disabled if not associated to a font code

  assert(parent());
  static_cast<KeyboardView *>(parent())->processCharButton(
    c.unicode(), fontCode); //B:why c.unicode() ???
}

void
CharacterButtonView::onCharacterItemClicked(CharacterListWidgetItem *charItem)
{
  const int charKey = getKeyboardCode();
  Models::CharacterData *charData = charItem->getCharacterData();
  assert(parent());
  static_cast<KeyboardView *>(parent())->processCharButton(charKey,
                                                           charData->getId());
}
