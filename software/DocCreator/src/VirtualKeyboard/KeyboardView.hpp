#ifndef KEYBOARDVIEW_H
#define KEYBOARDVIEW_H

#include <set>

#include "VirtualKeyboard/ModeTypeEnum.hpp"
#include "mvc/icontroller.h"
#include "mvc/iview.h"
#include <QWidget>

class KeyboardController;
class DocumentController;
class FontEditorController;
class CharacterButtonView;
class ControlButtonView;
namespace Models {
class Font;
}

class KeyboardView
  : public QWidget
  , public Mvc::IView
{
  Q_OBJECT

public:
  explicit KeyboardView(QWidget *parent = nullptr);

  /* Setters */
  void setKeyboardController(KeyboardController *keyboardController);
  void setDocumentController(DocumentController *documentController);
  void setFontEditorController(FontEditorController *fontEditorController);

  /* Buttons design */
  void addCharButtonView(CharacterButtonView *view);
  //void removeCharButtonView(const int & id);
  //void clearCharButtonViewList();

  void addControlButton(int id, ControlButtonView *button);
  void removeControlButton(int id);
  void clearControlButtonsList();

  /* Layout */
  void drawKeyboard(Models::Font *font);

  /* Events */
  void keyPressEvent(QKeyEvent *e) override;
  void keyReleaseEvent(QKeyEvent *e) override;

  Mvc::IController *getController() override;

public:
  void processCharButton(int keyboardCode, int fontCode, int id = -1);
  void processControlButton(int key);
  void processControlButtonRelease(int key);

private:
  void mapKeyboardCodeValuesToFontCodes(const Models::Font *font);
  void redrawKeyboard();

private:
  KeyboardController *_controller;
  DocumentController *_documentController;
  FontEditorController *_fontEditorController;
  QMap<int, CharacterButtonView *>
    _characterButtonViews; //B:TODO:OPTIM: QHash woule be better !!!
  QMap<int, ControlButtonView *> _controlButtons;

  std::set<CharacterButtonView *> _characterButtonViewsH;
  //Here we use a std::set instead of an std::unordered_set
  // to be sure that buttons are always displayed in the same order when font is
  // changed

  ModeTypeEnum _currentMode;
  bool _ctrlPressed, _altPressed, _shiftPressed, _altGrPressed;

  QString _currentFontName;
};

#endif /* KEYBOARDVIEW_H */
