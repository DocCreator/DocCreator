#ifndef XMLDOCUMENTLOADER_H
#define XMLDOCUMENTLOADER_H

#include <memory>

#include "documentloader.h"

class QXmlStreamReader;

namespace IOManager {
class FRAMEWORK_EXPORT XMLDocumentLoader : public DocumentLoader
{
public:
  /*
    Maximum value accepted for a size or a coordinate read from a document,
    in pixels. The sizes read from the file end up in allocations: a
    document declaring width="2000000000" used to be loaded as is.
  */
  static constexpr int MAX_PAGE_SIZE = 20000;

  explicit XMLDocumentLoader(const QString &input);

  //defined in the .cpp: QXmlStreamReader is only forward declared here
  ~XMLDocumentLoader() override;

  void createNewDocument() override;

  void buildStyles() override;
  void buildContent() override;

protected:
  //log the parse error, if any
  void reportError() const;

  /*
    @return the integer value of attribute @a name of the current element,
    clamped to [@a minValue; @a maxValue], or 0 if the attribute is absent
    or is not an integer.
  */
  int readIntAttribute(const QString &name,
		       int minValue = -MAX_PAGE_SIZE,
		       int maxValue = MAX_PAGE_SIZE) const;

  std::unique_ptr<Doc::DocStyle> buildStyle() override;
  std::unique_ptr<Doc::Page> buildPage() override;
  std::unique_ptr<Doc::DocImageBlock> buildImageBlock() override;
  std::unique_ptr<Doc::DocTextBlock> buildTextBlock() override;
  std::unique_ptr<Doc::DocParagraph> buildParagraph() override;
  std::unique_ptr<Doc::DocString> buildString() override;
  std::unique_ptr<Doc::DocCharacter> buildCharacter() override;

protected:
  //Attributes
  std::unique_ptr<QXmlStreamReader> _reader;
};
}

#endif // XMLDOCUMENTLOADER_H
