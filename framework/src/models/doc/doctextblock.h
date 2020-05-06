#ifndef DOCTEXTBLOCK_H
#define DOCTEXTBLOCK_H

#include <QString>
#include <framework_global.h>

#include "block.h"
#include "docparagraph.h"
#include "nodeofnodes.h"

namespace Doc
{
  class DocCharacter;
  class DocString;
  class DocStyle;
  
    class FRAMEWORK_EXPORT DocTextBlock : public NodeOfNodes< DocParagraph >, public Block
    {
    public:
        explicit DocTextBlock(Document* document);
        DocTextBlock(Document* document, int w, int h, int x, int y);
        ~DocTextBlock() = default;

        DocTextBlock* clone() override;

        DocTextBlock* getSelection() override;

        void add(DocCharacter* e);
        void add(const QList<DocCharacter*> &l);
        void add(DocString* e);
        void add(const QList<DocString*> &l);
        void add(DocParagraph* e) override;
        void add(const QList<DocParagraph*> &l) override;

        //void removeBeforeCursor();
        //void removeAfterCursor();

        DocParagraph* currentParagraph();
        QList<DocParagraph*> getParagraphs();

        void changeStyle(DocStyle* style);
        DocStyle* getStyle();

        QString content() const override;

    protected:
        void actionWhenElementIsEmpty(DocParagraph* empty) override;
        void removeBeforeAndCurrentAtBeginning() override;
        void removeAfterAndCurrentAtEnd() override;
    };
}

#endif // DOCTEXTBLOCK_H
