#ifndef DOCPARAGRAPH_H
#define DOCPARAGRAPH_H

#include <QList>
#include <framework_global.h>

#include "nodeofnodes.h"
#include "docstring.h"

namespace Doc
{
  class DocCharacter;
  class DocStyle;
  class Document;
  
    class FRAMEWORK_EXPORT DocParagraph : public NodeOfNodes<DocString>
    {
    public:
        explicit DocParagraph(Document* document);
        ~DocParagraph();

        DocParagraph* clone() override;

        DocParagraph* getSelection() override;

        QList<DocString*> getElements() const override;

        void add(DocCharacter* e);
        void add(const QList<DocCharacter*> &l);
        void add(DocString* e) override;
        void add(const QList<DocString*> &l) override;

        void setLineSpacing(int value);
        int lineSpacing() { return _lineSpacing; }
        void setTabulationSize(int value);
        int tabulationSize() { return _tabulationSize; }

        int length() const override;
        void setOffset(int value) override;
        int offset() const override;

	const DocString* currentString() const;
        DocString* currentString();

        QList<DocString*> getStrings() const;

        void changeStyle(DocStyle* style);
        DocStyle* getStyle();

        QList<DocParagraph*> splitAtPosition(int position);

        bool isEmpty() const override;
        //bool isAtBeginning() const override { return !_index; }
        bool isAtEnd() const override;

    protected:
        void setLength(int value) override;
        void actionWhenElementIsEmpty(DocString* empty) override;
        void removeBeforeAndCurrentAtBeginning() override;
        void removeAfterAndCurrentAtEnd() override;

    private:
	DocParagraph(const DocParagraph &);
	DocParagraph &operator=(const DocParagraph &);
	
    private:
        DocStyle* _styleTMP; //B: ??? should be a shared_ptr ???
        DocString* _endOfParagraph; //B: why is it a DocString ? and not a DocCharacter ?
        int _lineSpacing;
        int _tabulationSize;
    };
}

#endif // DOCPARAGRAPH_H
