#include "TxtDocumentSaver.hpp"

#include <QDebug>
#include <QFile>
#include <QList>
#include <QTextStream>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif

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
  //B: no encoding was set: with Qt 5, that of the locale, an ANSI code page
  // on Windows, and the characters it does not have were written as '?'
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  _writer.setCodec(QTextCodec::codecForName("utf8"));
#else
  _writer.setEncoding(QStringConverter::Utf8);
#endif
}

bool
TxtDocumentSaver::finish()
{
  //B: nothing checked that the text was written: documentToTxt() said yes
  // as soon as the file was opened (a full disk, a quota)
  _writer.flush();
  const bool writeOk = (_writer.status() == QTextStream::Ok);
  _file.close();
  if (! writeOk || _file.error() != QFile::NoError) {
    qWarning() << "TxtDocumentSaver: writing" << _file.fileName() << "failed:"
	       << _file.errorString();
    return false;
  }
  return true;
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
  //B: the end of each paragraph was not written, and nothing else was:
  // the paragraphs of the document were glued together. They are now
  // separated by a line break.
  if (_paragraphWritten) {
    _writer << '\n';
  }
  _paragraphWritten = true;

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
