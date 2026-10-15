#ifndef DOCUMENTLOADER_H
#define DOCUMENTLOADER_H

#include <memory>
#include <utility>

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

  /*
    @return the document built, which the caller now owns: once
    createNewDocument() has run, it is never null. It is null only before,
    and after it has already been taken.
  */
  [[nodiscard]] std::unique_ptr<Doc::Document> getDocument() {
    return std::unique_ptr<Doc::Document>(std::exchange(_output, nullptr));
  }

  virtual void createNewDocument()
  {
    _output = new Doc::Document();
  }

  virtual void buildStyles() = 0;
  virtual void buildContent() = 0;

protected:
  /*
    Each build*() function reads one element and @return a new object of
    it, which the caller owns: never null.
  */
  virtual std::unique_ptr<Doc::DocStyle> buildStyle() = 0;
  virtual std::unique_ptr<Doc::Page> buildPage() = 0;
  virtual std::unique_ptr<Doc::DocImageBlock> buildImageBlock() = 0;
  virtual std::unique_ptr<Doc::DocTextBlock> buildTextBlock() = 0;
  virtual std::unique_ptr<Doc::DocParagraph> buildParagraph() = 0;
  virtual std::unique_ptr<Doc::DocString> buildString() = 0;
  virtual std::unique_ptr<Doc::DocCharacter> buildCharacter() = 0;

protected:
  QString _input;
  Doc::Document *_output;
};
}

#endif // DOCUMENTLOADER_H
