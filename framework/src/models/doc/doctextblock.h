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

        virtual DocTextBlock* clone() override;

        virtual DocTextBlock* getSelection() override;

        void add(DocCharacter* e);
        void add(const QList<DocCharacter*> &l);
        void add(DocString* e);
        void add(const QList<DocString*> &l);
        virtual void add(DocParagraph* e) override;
        virtual void add(const QList<DocParagraph*> &l) override;

        //void removeBeforeCursor();
        //void removeAfterCursor();

        DocParagraph* currentParagraph();
        QList<DocParagraph*> getParagraphs();

        void changeStyle(DocStyle* style);
        DocStyle* getStyle();

        virtual QString content() const override;

    protected:
        virtual void actionWhenElementIsEmpty(DocParagraph* empty) override;
        virtual void removeBeforeAndCurrentAtBeginning() override;
        virtual void removeAfterAndCurrentAtEnd() override;
    };
}

#endif // DOCTEXTBLOCK_H
