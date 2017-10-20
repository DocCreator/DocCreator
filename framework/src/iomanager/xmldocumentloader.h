#ifndef XMLDOCUMENTLOADER_H
#define XMLDOCUMENTLOADER_H

#include "documentloader.h"

class QXmlStreamReader;

namespace IOManager {
class FRAMEWORK_EXPORT XMLDocumentLoader : public DocumentLoader
{
public:
  explicit XMLDocumentLoader(const QString &input);

  virtual void createNewDocument() override;

  virtual void buildStyles() override;
  virtual void buildContent() override;

protected:
  virtual Doc::DocStyle *buildStyle() override;
  virtual Doc::Page *buildPage() override;
  virtual Doc::DocImageBlock *buildImageBlock() override;
  virtual Doc::DocTextBlock *buildTextBlock() override;
  virtual Doc::DocParagraph *buildParagraph() override;
  virtual Doc::DocString *buildString() override;
  virtual Doc::DocCharacter *buildCharacter() override;

  //Attributes
  QXmlStreamReader *_reader;
};
}

#endif // XMLDOCUMENTLOADER_H
