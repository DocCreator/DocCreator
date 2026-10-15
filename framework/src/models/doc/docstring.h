#ifndef DOCSTRING_H
#define DOCSTRING_H

#include <QString>
#include <framework_global.h>

#include "doccharacter.h"
#include "nodeofleafs.h"

namespace Doc
{
  class DocStyle;
  
    class FRAMEWORK_EXPORT DocString : public NodeOfLeafs<DocCharacter>
    {
    public:
        DocString(DocStyle* style, Document* document);
        ~DocString();

        DocString* clone() override;

        /**
           @return a copy of this string that belongs to @a document, and
           that the caller owns: its characters are copied, and its style
           is the style of @a document of the same name, or a copy of this
           string's style registered in @a document if it has none.

           clone() keeps the document of this string. This one is for a
           string taken from another document, which may be destroyed
           while the copy is still in use.
        */
        [[nodiscard]] DocString* cloneInto(Document* document);

        DocString* getSelection() override;

        DocCharacter* currentCharacter();
        const QList<DocCharacter*> &getCharacters() const;

        void changeStyle(DocStyle* style);
        DocStyle* getStyle();

        QList<DocString*> splitAtPosition(int position);

    private:
        QString _style;
    };
}

#endif // DOCSTRING_H
