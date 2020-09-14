#ifndef ADOCUMENTVIEW_H
#define ADOCUMENTVIEW_H

#include <framework.h>

template<class T>
class ADocumentView : public Mvc::IView
{
public:
  explicit ADocumentView(Mvc::IController *controller);

  void drawElement(T *element, bool complete = false);
  T *getElement() { return _element; }
  void setController(Mvc::IController *controller) { _controller = controller; }
  Mvc::IController *getController() override { return _controller; }

  virtual void setOffset(int value) = 0;
  virtual void clear() = 0;

protected:
  virtual void load() = 0;
  virtual void draw(bool complete = false) = 0;

private:
  Mvc::IController *_controller;
  T *_element;
  bool _isDrawing;
};

template<class T>
ADocumentView<T>::ADocumentView(Mvc::IController *controller)
{
  _controller = controller;
  _element = nullptr;
  _isDrawing = false;
}

template<class T>
void
ADocumentView<T>::drawElement(T *element, bool complete)
{
  if (_isDrawing)
    return;

  _isDrawing = true;
  //if (element != nullptr && _element != element) //B: I don't understand this test yet !
  if (element != _element) {
    clear();
    _element = element;
    load();
  }
  draw(complete);
  _isDrawing = false;
}

#endif // ADOCUMENTVIEW_H
