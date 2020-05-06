#ifndef CONTROLBUTTONVIEW_H
#define CONTROLBUTTONVIEW_H

#include "mvc/icontroller.h"
#include "mvc/iview.h"
#include <QPushButton>
class KeyboardView;

class ControlButtonView
  : public QPushButton
  , public Mvc::IView
{
  Q_OBJECT
public:
  ControlButtonView(KeyboardView *parent, int id);

  void pressKey();
  void releaseKey();

  /* Events */
  void keyPressEvent(QKeyEvent *e) override;
  void keyReleaseEvent(QKeyEvent *e) override;
  Mvc::IController *getController() override;

public slots:
  void buttonClicked();

private:
  void processKey();

private:
  int _idK;
  KeyboardView *_parent;
  bool _pressed;
};

#endif // CONTROLBUTTONVIEW_H
