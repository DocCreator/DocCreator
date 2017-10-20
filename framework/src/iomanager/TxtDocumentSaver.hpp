#ifndef TXTDOCUMENTSAVER_HPP
#define TXTDOCUMENTSAVER_HPP

#include "documentsaver.h"

class QFile;
class QTextStream;
namespace Doc { class DocCharacter; }
namespace Doc { class DocImageBlock; }
namespace Doc { class DocParagraph; }
namespace Doc { class DocString; }
namespace Doc { class DocStyle; }
namespace Doc { class DocTextBlock; }
namespace Doc { class Document; }
namespace Doc { class Page; }


namespace IOManager
{
  class FRAMEWORK_EXPORT TxtDocumentSaver : public DocumentSaver
  {
  public:
    TxtDocumentSaver(Doc::Document *doc, const QString& filename);
    ~TxtDocumentSaver();

    virtual void createNewDocumentOutput() override;
	  
    virtual void buildStartDocument() override;
    virtual void buildEndDocument() override;
    virtual void buildStyles() override;
    virtual void buildContent() override;
	  
  protected:
    virtual void buildStyle(Doc::DocStyle *s) override;
	  
    virtual void buildPage(Doc::Page *p, int id) override;
    virtual void buildImageBlock(Doc::DocImageBlock *ib, int id) override;
    virtual void buildTextBlock(Doc::DocTextBlock *tb, int id) override;
    virtual void buildParagraph(Doc::DocParagraph *p, int id) override;
    virtual void buildString(Doc::DocString *s, int id) override;
    virtual void buildCharacter(Doc::DocCharacter *c, int id) override;
	  
  protected:
    QTextStream *_writer;
    QFile *_file;

  };
}

#endif // TXTDOCUMENTSAVER_HPP
