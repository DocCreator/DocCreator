#ifndef DOCUMENTVIEW_H
#define DOCUMENTVIEW_H

#include "mvc/icontroller.h"

#include "ADocumentView.hpp"
#include "DocRenderFlags.hpp"

class TextView;
class GraphicView;
class KeyboardController;
class QPrinter;

class DocumentView : public ADocumentView<Doc::Document>
{
public:
  explicit DocumentView(Mvc::IController *controller);

  void setTextView(TextView *view);
  void setGraphicView(GraphicView *view);

  void setKeyboardController(KeyboardController *keyboardController);

  void clear() override;
  void print(QPrinter *printer);
  void saveToImage(const QString &path);
  QImage toQImage(DocRenderFlags flags);
  QSize getImageSize();

  void setOffset(int value) override;

  void keyPressEvent(QKeyEvent *e) override;
  void keyReleaseEvent(QKeyEvent *e) override;

protected:
  void load() override;
  void draw(bool complete) override;

private:
  TextView *_textView;
  GraphicView *_graphicView;
  KeyboardController *_keyboardController;
};

#endif /* DOCUMENTVIEW_H */
