#include "TxtDocumentSaver.hpp"

#include <QDebug>
#include <QFile>
#include <QList>
#include <QTextStream>

namespace IOManager {

TxtDocumentSaver::TxtDocumentSaver(Doc::Document *doc,
				   const QString &filename)
  : DocumentSaver(doc),
    _file(filename),
    _writer(&_file)
{
  /*
    B: the stream was only built when the open succeeded, and
    buildCharacter() dereferenced it without looking: writing to a path
    that cannot be opened crashed. The stream is now always there, and
    isOpen() tells the caller whether anything will reach the disk.
  */
  if (! _file.open(QFile::WriteOnly | QFile::Truncate)) {
    qWarning() << "TxtDocumentSaver: unable to write" << filename << ':'
	       << _file.errorString();
  }
}

void
TxtDocumentSaver::createNewDocumentOutput()
{}

void
TxtDocumentSaver::buildStartDocument()
{}

void
TxtDocumentSaver::buildEndDocument()
{}

void
TxtDocumentSaver::buildStyles()
{}

void
TxtDocumentSaver::buildContent()
{
  int id = 0;
  for (Doc::Page *p : _input->getPages()) {
    buildPage(p, id);
    //            buildPage(p);
  }
}

void
TxtDocumentSaver::buildStyle(Doc::DocStyle *) //B:TODO: is the parameter useful?
{}

void
TxtDocumentSaver::buildPage(Doc::Page *p, int id)
//    void TxtDocumentSaver::buildPage (Page *p)
{
  for (Doc::DocTextBlock *tb : p->getTextBlocks()) {
    buildTextBlock(tb, id);
    //              buildTextBlock(tb);
  }
  for (Doc::DocImageBlock *ib : p->getImageBlocks()) {
    buildImageBlock(ib, id);
    //            buildImageBlock(ib);
  }
}

void
TxtDocumentSaver::buildImageBlock(Doc::DocImageBlock * /*ib*/, int /*id*/)
//    void TxtDocumentSaver::buildImageBlock(Doc::DocImageBlock *ib)
{}

void
TxtDocumentSaver::buildTextBlock(Doc::DocTextBlock *tb, int id)
//    void TxtDocumentSaver::buildTextBlock(Doc::DocTextBlock *tb)
{
  for (Doc::DocParagraph *p : tb->getParagraphs()) {
    buildParagraph(p, id);
    //              buildParagraph(p);
  }
}

void
TxtDocumentSaver::buildParagraph(Doc::DocParagraph *p, int id)
//    void TxtDocumentSaver::buildParagraph(DocParagraph *p)
{
  QList<Doc::DocString *> strings = p->getStrings();
  strings
    .pop_back(); //On ne sauvegarde pas le caractre de retour de ligne ("\n")
  for (Doc::DocString *s : strings) {
    buildString(s, id);
    //            buildString(s);
  }
}

void
TxtDocumentSaver::buildString(Doc::DocString *s, int id)
//    void TxtDocumentSaver::buildString(Doc::DocString *s)
{
  for (Doc::DocCharacter *c : s->getCharacters()) {
    buildCharacter(c, id);
    //            buildCharacter(c);
  }
}

void
TxtDocumentSaver::buildCharacter(Doc::DocCharacter *c, int /*id*/)
//    void TxtDocumentSaver::buildCharacter(Doc::DocCharacter *c)
{
  QString s = c->getDisplay();
  _writer << s;
}

} //namespace IOManager
