#include "FontDocumentGenerator.hpp"

#include <iostream>

#include <QDebug>

#include "Document/DocumentController.hpp"
#include "Utils/FontUtils.hpp" //computeBestLineSpacing
#include "context/fontcontext.h"
#include "models/character.h"
#include "models/characterdata.h"
#include "models/doc/docparagraph.h"
#include "models/doc/document.h"
#include "models/doc/page.h"
#include "models/font.h"

/*
  Remove all pages (and thus paragraphs) from document @a doc,
  and add a new empty page.
*/
static void
clearDocument(Doc::Document *doc)
{
  QList<Doc::Page *> pages = doc->getPages();
  for (Doc::Page *p : pages)
    doc->remove(p);

  doc->add(new Doc::Page(doc));
}

/*
  Create a text block taking much of the page and a new paragraph.
*/
static void
addTextBlockAndParagraph(DocumentController *ctrl,
                         Doc::Document *doc,
                         int lineSpacing)
{
  const int margin = std::min(
    10, (std::min(ctrl->getPageWidth(), ctrl->getPageHeight()) - 1) / 2);
  const int x = margin;
  const int y = margin;
  const int width = std::max(ctrl->getPageWidth() - 2 * margin, 1);
  const int height = std::max(ctrl->getPageHeight() - 2 * margin, 1);

  ctrl->addTextBlock(x, y, width, height);

  auto para = new Doc::DocParagraph(doc);
  doc->add(para);
  Doc::DocParagraph *currentParagraph =
    doc
      ->currentParagraph(); //B:TODO: why call currentParagraph() when we alreay have para ?
  currentParagraph->setLineSpacing(lineSpacing);

  ctrl->resetCurrentTextBlockCursor();
}

void
buildDocumentWithWholeFont(DocumentController *ctrl)
{
  Models::Font *font = Context::FontContext::instance()->getCurrentFont();
  if (font == nullptr) {
    std::cerr << "No current font\n";
    return;
  }

  Doc::Document *doc = ctrl->getDocument();
  if (doc == nullptr) {
    std::cerr << "No current document\n";
    return;
  }

  clearDocument(doc);

  auto style = new Doc::DocStyle(font->getName(), font->getName());
  doc->addStyle(style);
  //B: REM: it is to set the style before adding the paragraphs
  // Otherwise paragraphs do not have a style and it crashes when we try to
  // export to PNG.

  const int lineSpacing = computeBestLineSpacing(*font);

  addTextBlockAndParagraph(ctrl, doc, lineSpacing);

  //Add all instances of all characters to the document
  const Models::CharacterMap &characters = font->getCharacters();
  for (auto it = characters.begin(); it != characters.end(); ++it) {

    const Models::Character &ch = *(it.value());
    const QString &s = ch.getCharacterValue();

    qDebug() << s << "\n";

    const Models::CharacterDataList &cdl = ch.getAllCharacterData();
    const int nbInstances = cdl.size();

    for (int i = 0; i < nbInstances; ++i) {
      const int id = cdl.at(i)->getId();

      ctrl->addCharacter(s, id);
    }
  }
}
