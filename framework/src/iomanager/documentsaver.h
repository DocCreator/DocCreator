#ifndef DOCUMENTSAVER_H
#define DOCUMENTSAVER_H

#include <QString>
#include <models/doc.h>

namespace IOManager {
class FRAMEWORK_EXPORT DocumentSaver
{
public:
  explicit DocumentSaver(Doc::Document *input) { _input = input; }
  virtual ~DocumentSaver() {}

  QString getDocumentOutput() { return _output; }
  virtual void createNewDocumentOutput() { _output = QString(); }

  virtual void buildStartDocument() = 0;
  virtual void buildEndDocument() = 0;
  virtual void buildStyles() = 0;
  virtual void buildContent() = 0;

protected:
  virtual void buildStyle(Doc::DocStyle *s) = 0;
  virtual void buildPage(Doc::Page *p, int id = -1) = 0;
  virtual void buildImageBlock(Doc::DocImageBlock *ib, int id = -1) = 0;
  virtual void buildTextBlock(Doc::DocTextBlock *tb, int id = -1) = 0;
  virtual void buildParagraph(Doc::DocParagraph *p, int id = -1) = 0;
  virtual void buildString(Doc::DocString *s, int id = -1) = 0;
  virtual void buildCharacter(Doc::DocCharacter *c, int id = -1) = 0;

protected:
  Doc::Document *_input;
  QString _output;
};
}

#endif // DOCUMENTSAVER_H
