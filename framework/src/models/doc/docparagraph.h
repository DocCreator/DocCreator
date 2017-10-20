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

        virtual DocParagraph* clone() override;

        virtual DocParagraph* getSelection() override;

        virtual QList<DocString*> getElements() const override;

        void add(DocCharacter* e);
        void add(const QList<DocCharacter*> &l);
        virtual void add(DocString* e) override;
        virtual void add(const QList<DocString*> &l) override;

        void setLineSpacing(int value);
        int lineSpacing() { return _lineSpacing; }
        void setTabulationSize(int value);
        int tabulationSize() { return _tabulationSize; }

        virtual int length() const override;
        virtual void setOffset(int value) override;
        virtual int offset() const override;

	const DocString* currentString() const;
        DocString* currentString();

        QList<DocString*> getStrings() const;

        void changeStyle(DocStyle* style);
        DocStyle* getStyle();

        QList<DocParagraph*> splitAtPosition(int position);

        virtual bool isEmpty() const override;
        //virtual bool isAtBeginning() const override { return !_index; }
        virtual bool isAtEnd() const override;

    protected:
        virtual void setLength(int value) override;
        virtual void actionWhenElementIsEmpty(DocString* empty) override;
        virtual void removeBeforeAndCurrentAtBeginning() override;
        virtual void removeAfterAndCurrentAtEnd() override;

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
