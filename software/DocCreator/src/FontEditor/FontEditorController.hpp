#ifndef FONTEDITORCONTROLLER_H
#define FONTEDITORCONTROLLER_H

#include "mvc/icontroller.h"
#include <QString>

class FontEditorView;

class FontEditorController : public Mvc::IController
{
public:
  FontEditorController();

  /* Setters */
  void setView(FontEditorView *view);
  void addCharacter(const QString &s);

private:
  FontEditorView *_view;
};

#endif /* FONTEDITORCONTROLLER_H */
