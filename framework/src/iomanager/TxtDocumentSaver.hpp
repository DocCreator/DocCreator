#ifndef TXTDOCUMENTSAVER_HPP
#define TXTDOCUMENTSAVER_HPP

#include "documentsaver.h"

#include <QFile>
#include <QTextStream>

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
    TxtDocumentSaver(const TxtDocumentSaver &o) = delete;
    TxtDocumentSaver &operator=(const TxtDocumentSaver &o) = delete;

    /*
      @return whether the file passed to the constructor could be opened for
      writing. The build*() methods write nothing when it could not.
    */
    bool isOpen() const { return _file.isOpen(); }

    void createNewDocumentOutput() override;

    void buildStartDocument() override;
    void buildEndDocument() override;
    void buildStyles() override;
    void buildContent() override;
	  
  protected:
    void buildStyle(Doc::DocStyle *s) override;

    void buildPage(Doc::Page *p, int id) override;
    void buildImageBlock(Doc::DocImageBlock *ib, int id) override;
    void buildTextBlock(Doc::DocTextBlock *tb, int id) override;
    void buildParagraph(Doc::DocParagraph *p, int id) override;
    void buildString(Doc::DocString *s, int id) override;
    void buildCharacter(Doc::DocCharacter *c, int id) override;
	  
  protected:
    QFile _file;
    QTextStream _writer;

  };
}

#endif // TXTDOCUMENTSAVER_HPP
