#ifndef DOCELEMENT_H
#define DOCELEMENT_H

#include <framework_global.h>

namespace Doc
{
    class Document;
}

namespace Doc
{
    class FRAMEWORK_EXPORT Element
    {
    public:
        explicit Element(Document* document);
        
	virtual ~Element();

        virtual void setLength(int value);
        virtual int length() const { return _length; }

        Document* getDocument() { return _document; }

    private:
        int _length;
        Document* _document;
    };
}

#endif // DOCELEMENT_H
