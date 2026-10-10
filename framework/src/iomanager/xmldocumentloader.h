#ifndef XMLDOCUMENTLOADER_H
#define XMLDOCUMENTLOADER_H

#include <memory>

#include "documentloader.h"

class QXmlStreamReader;

namespace IOManager {
class FRAMEWORK_EXPORT XMLDocumentLoader : public DocumentLoader
{
public:
  explicit XMLDocumentLoader(const QString &input);

  //defined in the .cpp: QXmlStreamReader is only forward declared here
  ~XMLDocumentLoader() override;

  void createNewDocument() override;

  void buildStyles() override;
  void buildContent() override;

protected:
  //log the parse error, if any
  void reportError() const;

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
