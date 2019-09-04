#include "docstring.h"

#include "document.h"

namespace Doc
{
  DocString::DocString(DocStyle* style, Document* document) : NodeOfLeafs< DocCharacter >(document), _style()
    {
        if (document != nullptr)
            _style = document->addStyle(style);
    }

    DocString::~DocString()
    {
      Document* doc = this->getDocument();
      if (doc != nullptr)
	doc->changeStyle(_style, nullptr);
    }

    DocString* DocString::clone()
    {
        auto e = new DocString(this->getStyle(), this->getDocument());
        for (DocCharacter* c : this->getElements())
            e->add(c->clone());
        e->_endOfSelection = _endOfSelection;
        e->_index = _index;
        e->_isSplitting = _isSplitting;
        e->setLength(this->length());
        e->setOffset(this->offset());

        return e;
    }

    DocString* DocString::getSelection()
    {
        const int offset = this->offset();
        if (this->isEmpty() || _endOfSelection == offset)
            return nullptr;

        auto result = new DocString(getStyle(), getDocument());

        const int min = (_endOfSelection < offset) ? _endOfSelection : offset;
        int max = (_endOfSelection > offset) ? _endOfSelection : offset;
        if (max > _elements.count())
            max = _elements.count();

        QList<DocCharacter*> list;
	list.reserve(max-min);
        for (int i = min ; i < max ; i++)
        {
            DocCharacter* c = _elements.at(i);
            if (c != nullptr)
                list.append(c->clone());
        }

        result->add(list);

        return result;
    }

    DocCharacter* DocString::currentCharacter()
    {
      return this->currentElement();
    }

    const QList<DocCharacter*> &DocString::getCharacters() const
    {
      return _elements;
    }

    void DocString::changeStyle(DocStyle* style)
    {
        Document* document = this->getDocument();
        if (document == nullptr)
            return;

        _style = document->changeStyle(_style, style);
    }

    DocStyle* DocString::getStyle()
    {
        Document* document = this->getDocument();
        if (document == nullptr)
            return nullptr;
        return document->getStyle(_style);
    }

    QList< DocString* > DocString::splitAtPosition(int position)
    {
        QList< DocString* > list;
        if (isEmpty())
            return list;

        DocString* firstPart = this->clone();
        DocString* secondPart = this->clone();

        firstPart->select(position, firstPart->length());
        firstPart->removeSelection();
        firstPart->setOffset(position);
        secondPart->select(0, position);
        secondPart->removeSelection();
        secondPart->setOffset(0);

        list.append(firstPart);
        list.append(secondPart);

        return list;
    }

} //namespace Doc
