#ifndef DOCUMENTLOADER_H
#define DOCUMENTLOADER_H

#include <models/doc.h>

namespace IOManager {
class FRAMEWORK_EXPORT DocumentLoader
{
public:
  explicit DocumentLoader(const QString &input)
    : _input(input)
    , _output(nullptr)
  {}

  DocumentLoader(const DocumentLoader &) = delete;
  DocumentLoader &operator=(const DocumentLoader &) = delete;

  virtual ~DocumentLoader() {
    delete _output;
  }

  //gives ownership
  //B:TODO: use a shared_ptr ?
  Doc::Document *getDocument() {
    Doc::Document *output = _output;
    _output = nullptr;
    return output;
  }

  virtual void createNewDocument()
  {
    _output = new Doc::Document();
  }

  virtual void buildStyles() = 0;
  virtual void buildContent() = 0;

protected:
  virtual Doc::DocStyle *buildStyle() = 0;
  virtual Doc::Page *buildPage() = 0;
  virtual Doc::DocImageBlock *buildImageBlock() = 0;
  virtual Doc::DocTextBlock *buildTextBlock() = 0;
  virtual Doc::DocParagraph *buildParagraph() = 0;
  virtual Doc::DocString *buildString() = 0;
  virtual Doc::DocCharacter *buildCharacter() = 0;

protected:
  QString _input;
  Doc::Document *_output;
};
}

#endif // DOCUMENTLOADER_H
