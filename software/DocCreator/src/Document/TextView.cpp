#include "TextView.hpp"

#include <QTextCursor>
#include <cassert>

#include "DocumentView.hpp"

TextView::TextView(Mvc::IController *controller, DocumentView *parent)
  : QTextEdit()
  , ADocumentView<Doc::DocTextBlock>(controller)
{
  _parent = parent;
  _cursorIsMoving = false;

  QObject::connect(this,
                   SIGNAL(cursorPositionChanged()),
                   this,
                   SLOT(onCursorPositionChanged()));
}

void
TextView::clear()
{
  setText(QString());
}

void
TextView::setOffset(int value)
{
  _cursorIsMoving = true;
  QTextCursor curs = this->textCursor();

  curs.setPosition(value);
  this->setTextCursor(curs);
  _cursorIsMoving = false;
}

void
TextView::removeCurrentBlock()
{
  clear();
}

void
TextView::keyPressEvent(QKeyEvent *e)
{
  int key = e->key();
  switch (key) {
    case Qt::Key_Backspace:
      setOffset(this->textCursor().position() - 1);
      break;
    default:;
  }

  if (_parent != nullptr) {
    _parent->keyPressEvent(e);
  }
}

void
TextView::keyReleaseEvent(QKeyEvent *e)
{
  if (_parent != nullptr) {
    _parent->keyReleaseEvent(e);
  }
}

/* public slots */
void
TextView::onCursorPositionChanged()
{
  //    QTextCursor curs = this->textCursor();
  //    curs.setPosition(curs.position()+1);
  //    this->setTextCursor(curs);
  //    qDebug() << "cursor is moving";
  /**if (_cursorIsMoving)
        return;

    DocumentController* c = (DocumentController*)this->getController();
    if (c == nullptr)
        return;

    c->setOffset(this->textCursor().position());*/
}

/* protected methods */
void
TextView::load()
{
  clear();
}

void
TextView::draw(bool)
{
  Doc::DocTextBlock *currentTextBlock = this->getElement();
  if (currentTextBlock == nullptr) {
    setText(QString());
    return;
  }

  assert(currentTextBlock);
  QString text;
  QList<Doc::DocParagraph *> paragraphs = currentTextBlock->getParagraphs();
  for (const Doc::DocParagraph *p : paragraphs) {
    assert(p);
    const QList<Doc::DocString *> strings = p->getStrings();
    for (const Doc::DocString *s : strings) {
      assert(s);
      const QList<Doc::DocCharacter *> characters = s->getCharacters();
      for (const Doc::DocCharacter *c : characters) {
        assert(c);
        text.append(c->getDisplay()); //here the rect is already set...
      }
    }
  }
  setText(text);

  setOffset(text.length());
  //qDebug() << " cursor position" << this->textCursor().position();
}
