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

  Doc::DocStyle *buildStyle() override;
  Doc::Page *buildPage() override;
  Doc::DocImageBlock *buildImageBlock() override;
  Doc::DocTextBlock *buildTextBlock() override;
  Doc::DocParagraph *buildParagraph() override;
  Doc::DocString *buildString() override;
  Doc::DocCharacter *buildCharacter() override;

protected:
  //Attributes
  std::unique_ptr<QXmlStreamReader> _reader;
};
}

#endif // XMLDOCUMENTLOADER_H
