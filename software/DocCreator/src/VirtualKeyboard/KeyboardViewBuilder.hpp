#ifndef KEYBOARDVIEWBUILDER_H
#define KEYBOARDVIEWBUILDER_H

#include "KeyboardView.hpp"

class KeyboardViewBuilder
{
public:
  KeyboardViewBuilder() : _keyboardView(nullptr) {}
  virtual ~KeyboardViewBuilder() {}

  KeyboardView *getKeyboardView() { return _keyboardView; }
  void createNewKeyboardView() { _keyboardView = new KeyboardView(); }

  virtual void buildGeometry() = 0;
  virtual void buildCharButtons() = 0;
  virtual void buildControlButtons() = 0;

protected:
  KeyboardView *_keyboardView;
};

#endif // KEYBOARDVIEWBUILDER_H
