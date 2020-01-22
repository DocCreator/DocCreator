#include "ControlButtonView.hpp"

#include "KeyboardView.hpp"

ControlButtonView::ControlButtonView(KeyboardView *parent, int id)
{
  _parent = parent;
  _idK = id;
  _pressed = false;
  connect(this, SIGNAL(clicked()), this, SLOT(buttonClicked()));
}

void
ControlButtonView::pressKey()
{
  if (!_pressed) {
    processKey();
  }
}

void
ControlButtonView::releaseKey()
{
  if (_pressed) {
    processKey();
  }
}

/* Events */
void
ControlButtonView::keyPressEvent(QKeyEvent *e)
{
  _parent->keyPressEvent(e);
}

void
ControlButtonView::keyReleaseEvent(QKeyEvent *e)
{
  _parent->keyReleaseEvent(e);
}

Mvc::IController *
ControlButtonView::getController()
{
  return _parent->getController();
}

/* Slots */
void
ControlButtonView::buttonClicked()
{
  //QMessageBox::information(this, "Control clicked", QString::number(_idK));
  if (_pressed) {
    //QMessageBox::information(this, "Release", "Release");
    _parent->processControlButtonRelease(_idK);
  } else {
    //QMessageBox::information(this, "Press", "Press");
    _parent->processControlButton(_idK);
  }
}

void
ControlButtonView::processKey()
{
  _pressed = !_pressed;
  //QMessageBox::information(this, "process", (_pressed?"press key":"release key"));
  setFlat(_pressed);
}
