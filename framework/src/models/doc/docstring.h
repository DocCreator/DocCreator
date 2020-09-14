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
