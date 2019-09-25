#include "DocumentView.hpp"

#include "DocumentController.hpp"
#include "GraphicView.hpp"
#include "TextView.hpp"
#include "VirtualKeyboard/KeyboardController.hpp"

DocumentView::DocumentView(Mvc::IController *controller)
  : ADocumentView<Doc::Document>(controller)
  , _textView(nullptr)
  , _graphicView(nullptr)
  , _keyboardController(nullptr)
{
  DocumentController *c = static_cast<DocumentController *>(getController());
  if (c != nullptr) {
    c->setView(this);
  }
}

void
DocumentView::setTextView(TextView *view)
{
  _textView = view;
}
void
DocumentView::setGraphicView(GraphicView *view)
{
  _graphicView = view;
}

void
DocumentView::setKeyboardController(KeyboardController *keyboardController)
{
  _keyboardController = keyboardController;
}

void
DocumentView::clear()
{
  if (_graphicView != nullptr) {
    _graphicView->clear();
  }

  if (_textView != nullptr) {
    _textView->clear();
  }
}

void
DocumentView::print(QPrinter *printer)
{
  if (_graphicView != nullptr) {
    _graphicView->print(printer);
  }
}

void
DocumentView::saveToImage(const QString &path)
{
  if (_graphicView != nullptr) {
    _graphicView->saveToImage(path);
  }
}

QImage
DocumentView::toQImage(DocRenderFlags flags)
{
  if (_graphicView != nullptr)
    return _graphicView->toQImage(flags);
  return QImage();
}

QSize
DocumentView::getImageSize()
{
  if (_graphicView != nullptr) {
    return _graphicView->getImageSize();
  }
  return QSize();
}

void
DocumentView::setOffset(int value)
{
  if (_graphicView != nullptr) {
    _graphicView->setOffset(value);
  }
  if (_textView != nullptr) {
    _textView->setOffset(value);
  }
}
void
DocumentView::keyPressEvent(QKeyEvent *e)
{
  if (_keyboardController != nullptr) {
    _keyboardController->keyPressEvent(e);
  }
}
void
DocumentView::keyReleaseEvent(QKeyEvent *e)
{
  if (_keyboardController != nullptr) {
    _keyboardController->keyReleaseEvent(e);
  }
}

/* protected methods */
void
DocumentView::load()
{
  Doc::Document *document = getElement();
  if (document == nullptr) {
    return;
  }
  /*
    if(_textView!=nullptr)
        _textView->drawElement(dynamic_cast<DocTextBlock*>(document->currentBlock()));
    if(_graphicView!=nullptr)
        _graphicView->drawElement(document);*/
}

void
DocumentView::draw(bool complete)
{
  Doc::Document *document = getElement();
  if (document == nullptr) {
    return;
  }

  if (_textView != nullptr) {
    Doc::Block *b = document->currentBlock();
    Doc::DocTextBlock *tb = dynamic_cast<Doc::DocTextBlock *>(b);
    if (tb != nullptr)
      _textView->drawElement(tb, complete);

    //B:impl:
    //not sure if test should be here,
    // or if ADocumentView<T>::drawElement() should be changed  ???
    //Without this test, we pass nullptr to ADocumentView<T>::drawElement()
    // and if a previous document (that do not exist anymore) was set for the
    // view,
    //we have _element that is not reset ADocumentView<T>::drawElement() and
    // thus it is pointing to an invalid location when we call
    // TextView::draw()...
  }

  if (_graphicView != nullptr) {
    _graphicView->drawElement(document, complete);
  }
}
