#ifndef KEYBOARDVIEWDIRECTOR_H
#define KEYBOARDVIEWDIRECTOR_H

#include "KeyboardViewBuilder.hpp"

class KeyboardViewDirector
{
public:
  /**
   *
   * Takes ownership of @a kbb.
   */
  explicit KeyboardViewDirector(KeyboardViewBuilder *kbb)
  {
    _keyboardViewBuilder = kbb;
  }

  ~KeyboardViewDirector() { delete _keyboardViewBuilder; }

  KeyboardViewDirector(const KeyboardViewDirector &) = delete;
  KeyboardViewDirector &operator=(const KeyboardViewDirector &) = delete;
  
  
  KeyboardView *getKeyboardView()
  {
    return _keyboardViewBuilder->getKeyboardView();
  }

  void constructKeyboardView()
  {
    _keyboardViewBuilder->createNewKeyboardView();
    _keyboardViewBuilder->buildGeometry();
    _keyboardViewBuilder->buildCharButtons();
    _keyboardViewBuilder->buildControlButtons();
  }

private:
  KeyboardViewBuilder *_keyboardViewBuilder;
};

#endif //KEYBOARDVIEWDIRECTOR_H
