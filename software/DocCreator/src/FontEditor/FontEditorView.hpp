#ifndef FONTEDITORVIEW_H
#define FONTEDITORVIEW_H

#include "mvc/icontroller.h"
#include "mvc/iview.h"

class FontEditorController;
class KeyboardController;

class CharEditView;
class CharEditScene;
#include "ui_fonteditorview.h"

namespace Models {
class Character;
}

class FontEditorView
  : public QWidget
  , public Mvc::IView
{
  Q_OBJECT

public:
  explicit FontEditorView(FontEditorController *controller,
                          QWidget *parent = nullptr);

  /* Setters */
  void setCenterChar(Models::Character *ch);
  void setLeftChar(const Models::Character *ch);
  void setRightChar(const Models::Character *ch);
  void addCharacter(Models::Character *ch);
  void setKeyboardController(KeyboardController *keyboardController);

  /* Events */
  void keyPressEvent(QKeyEvent *e) override;
  void keyReleaseEvent(QKeyEvent *e) override;

  Mvc::IController *getController() override
  {
    return (Mvc::IController *)_controller;
  }

public slots:
  void saveCharacter();

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  void updateUpLineSpinBox(int pos);
  void updateBaseLineSpinBox(int pos);
  void updateLeftLineSpinBox(int pos);
  void updateRightLineSpinBox(int pos);
  void updateUpLineItem(int value);
  void updateBaseLineItem(int value);
  void updateLeftLineItem(int value);
  void updateRightLineItem(int value);

private:
  void createActions();

private:
  Ui::FontEditorViewClass _ui;

  Models::Character *_ch;
  QString _chCenterFontName;
  CharEditScene *_scene;
  CharEditView *_view;

  FontEditorController *_controller;
  KeyboardController *_keyboardController;
};

#endif /* FONTEDITORVIEW_H */
