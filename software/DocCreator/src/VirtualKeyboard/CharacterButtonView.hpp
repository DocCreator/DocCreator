#ifndef CHARACTERBUTTONVIEW_H
#define CHARACTERBUTTONVIEW_H

#include <array>

#include "ModeTypeEnum.hpp"
#include "mvc/icontroller.h"
#include "mvc/iview.h"
#include <QPushButton>
class KeyboardView;
class CharacterListWidgetItem;
class QKeyEvent;
class QMouseEvent;

class CharacterButtonView
  : public QPushButton
  , public Mvc::IView
{
  Q_OBJECT

public:
  explicit CharacterButtonView(KeyboardView *parent);

  void addModeCharacter(ModeTypeEnum mode, int charValue);
  void removeModeCharacter(ModeTypeEnum mode);

  /*
    Get keyboard code value for current mode.
    Return -1 of no keyboard code associated to current mode.

    @sa{setMode(ModeTypeEnum mode),  getKeyboardCode(ModeTypeEnum mode)}
  */
  int getKeyboardCode() const;

  /*
    Get keyboard code value for given mode @a mode.
    Return -1 of no keyboard code associated to this mode.
  */
  int getKeyboardCode(ModeTypeEnum mode) const;

  /*
    Get font code value for given mode @a mode.
    Return -1 of no font code associated to this mode.
    Warning does not check that a keyboard code is associated to this mode.
  */
  int getFontCode(ModeTypeEnum mode) const;

  QList<int> keys() const;

  void setMode(ModeTypeEnum mode);

  // keyboard code value mapping with font code value: unicode 16 bit
  void addKeyboardCodesMapToFontCodesValues(int kbCodeValue, int fontCodeValue);

  /*
    Get font code value for a given keyboard code value @a kbCodeValue.
    Return -1 if no font code value associated to this keyboard code value.
  */
  int getFontCodeValue(int kbCodeValue) const;

  /*
    Get font code value for keyboard code value associated to current mode if any.
    Return -1 if no keyboard code is associated to current mode or if no font code value is associated to this keyboard code value.
  */
  int getFontCodeValue() const;

  /*
    Remove keyborad code @a kbCodeValue if present.
   */
  void eraseFontCode(int kbCodeValue);

  //QList<int> getMappedFontCodeValues();
  bool hasFontCodeForKbCode(int kbCode) const;

  bool hasKeyboardCode(int kbCode) const;

  /* Character choice dialog */
  void openCharacterChoice();

  /* Events */
  void keyPressEvent(QKeyEvent *e) override;
  void keyReleaseEvent(QKeyEvent *e) override;
  void mousePressEvent(QMouseEvent *e) override;

  Mvc::IController *getController() override;

public slots:
  void buttonClicked();
  void onCharacterItemClicked(CharacterListWidgetItem *charItem);

private:
  struct Key
  {
    int kbCode;
    int fontCode;

    Key()
      : kbCode(-1)
      , fontCode(-1)
    {}
  };

  std::array<Key, NUMBER_MODES> _keys;

  ModeTypeEnum
    _currentMode; //B:DESIGN: is it necessary to store the current mode here ? It is already stored in the keyboardview ...
};

#endif // CHARACTERBUTTONVIEW_H
