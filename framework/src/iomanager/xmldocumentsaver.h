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

  virtual void createNewDocumentOutput() override;

  virtual void buildStartDocument() override;

  virtual void buildEndDocument() override;
  virtual void buildStyles() override;
  virtual void buildContent() override;

  bool _isDIGIDOC; //B:UGLY: why public ???

protected:
  virtual void buildStyle(Doc::DocStyle *s) override;
  virtual void buildPage(Doc::Page *p, int id = -1) override;
  virtual void buildImageBlock(Doc::DocImageBlock *ib, int id = -1) override;
  virtual void buildTextBlock(Doc::DocTextBlock *tb, int id = -1) override;
  virtual void buildParagraph(Doc::DocParagraph *p, int id = -1) override;
  virtual void buildString(Doc::DocString *s, int id = -1) override;
  virtual void buildCharacter(Doc::DocCharacter *c, int id = -1) override;

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
};
}

#endif // XMLDOCUMENTSAVER_H
