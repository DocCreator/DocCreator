#ifndef XMLDOCUMENTLOADER_H
#define XMLDOCUMENTLOADER_H

#include "documentloader.h"

class QXmlStreamReader;

namespace IOManager {
class FRAMEWORK_EXPORT XMLDocumentLoader : public DocumentLoader
{
public:
  explicit XMLDocumentLoader(const QString &input);

  void createNewDocument() override;

  void buildStyles() override;
  void buildContent() override;

protected:
  Doc::DocStyle *buildStyle() override;
  Doc::Page *buildPage() override;
  Doc::DocImageBlock *buildImageBlock() override;
  Doc::DocTextBlock *buildTextBlock() override;
  Doc::DocParagraph *buildParagraph() override;
  Doc::DocString *buildString() override;
  Doc::DocCharacter *buildCharacter() override;

protected:
  //Attributes
  QXmlStreamReader *_reader;
};
}

#endif // XMLDOCUMENTLOADER_H
