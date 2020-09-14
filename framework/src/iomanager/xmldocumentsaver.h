#ifndef XMLDOCUMENTSAVER_H
#define XMLDOCUMENTSAVER_H

#include "documentsaver.h"
class QXmlStreamWriter;

namespace IOManager {
class FRAMEWORK_EXPORT XMLDocumentSaver : public DocumentSaver
{
public:
  explicit XMLDocumentSaver(Doc::Document *doc);
  ~XMLDocumentSaver();
  XMLDocumentSaver(const XMLDocumentSaver &) = delete;
  XMLDocumentSaver &operator=(const XMLDocumentSaver &) = delete;


  void createNewDocumentOutput() override;

  void buildStartDocument() override;

  void buildEndDocument() override;
  void buildStyles() override;
  void buildContent() override;


protected:
  void buildStyle(Doc::DocStyle *s) override;
  void buildPage(Doc::Page *p, int id = -1) override;
  void buildImageBlock(Doc::DocImageBlock *ib, int id = -1) override;
  void buildTextBlock(Doc::DocTextBlock *tb, int id = -1) override;
  void buildParagraph(Doc::DocParagraph *p, int id = -1) override;
  void buildString(Doc::DocString *s, int id = -1) override;
  void buildCharacter(Doc::DocCharacter *c, int id = -1) override;

  void buildHeaderDocument();
  void buildDocumentInfos();

  void buildMediaAttributeOfPage();
  void buildAcquisitionAttributeOfPage();
  void buildBackgroundInfosOfPage();
  void buildLayoutInfosOfPage();

  void buildMediaAttributeOfEOCs();
  void buildAcquisitionAttributeOfEOCs();
  void buildPositionOfEOCs();
  void buildShapeInfosOfEOCs(int x, int y, int w, int h);

  void groundtruthLuminosityModel();
  void groundtruthGlobalModel();

protected:
  QXmlStreamWriter *_writer;
  bool _isDIGIDOC;
};
}

#endif // XMLDOCUMENTSAVER_H
