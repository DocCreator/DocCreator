#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include "ADocumentView.hpp"
#include "mvc/icontroller.h"
#include <QTextEdit>

class DocumentView;

class TextView
  : public QTextEdit
  , public ADocumentView<Doc::DocTextBlock>
{
  Q_OBJECT

public:
  TextView(Mvc::IController *controller, DocumentView *parent);

  virtual void clear() override;
  virtual void setOffset(int value) override;

  void removeCurrentBlock();

  virtual void keyPressEvent(QKeyEvent *e) override;
  virtual void keyReleaseEvent(QKeyEvent *e) override;

public slots:
  void onCursorPositionChanged();

protected:
  virtual void load() override;
  virtual void draw(bool complete) override;

private:
  DocumentView *_parent;
  bool _cursorIsMoving;
};

#endif /* TEXTVIEW_H */
