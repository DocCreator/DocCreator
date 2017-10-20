#ifndef KEYBOARDCONTROLLER_H
#define KEYBOARDCONTROLLER_H

#include "mvc/icontroller.h"
#include "patterns/observer.h"

class QKeyEvent;
class KeyboardView;

class KeyboardController
  : public Mvc::IController
  , public Patterns::Observer
{
public:
  KeyboardController();

  /* Setters */
  void setView(KeyboardView *view);

  /* Events */
  void keyPressEvent(QKeyEvent *e);
  void keyReleaseEvent(QKeyEvent *e);

  /* Observer method */
  virtual void update() override;

private:
  KeyboardView *_view;
};

#endif /* KEYBOARDCONTROLLER_H */
