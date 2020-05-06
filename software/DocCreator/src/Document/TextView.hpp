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

  void clear() override;
  void setOffset(int value) override;

  void removeCurrentBlock();

  void keyPressEvent(QKeyEvent *e) override;
  void keyReleaseEvent(QKeyEvent *e) override;

public slots:
  void onCursorPositionChanged();

protected:
  void load() override;
  void draw(bool complete) override;

private:
  DocumentView *_parent;
  bool _cursorIsMoving;
};

#endif /* TEXTVIEW_H */
