#include "docparagraph.h"

#include <cassert>

#include "document.h"
#include "doccharacter.h"

namespace Doc
{
    DocParagraph::DocParagraph(Document* document) : 
      NodeOfNodes< DocString >(document),
      _styleTMP(nullptr),
      _endOfParagraph(nullptr),
      _lineSpacing(0),
      _tabulationSize(0)
    {
      Document* doc = this->getDocument();
      if (doc != nullptr)
	_styleTMP = doc->getStyle();

      //std::cerr<<"# DocParagraph::DocParagraph this="<<this<<" document="<<doc<<" _styleTMP="<<_styleTMP<<"\n";

      _endOfParagraph = new DocString(_styleTMP, doc);
      _endOfParagraph->add(new DocCharacter(QStringLiteral("\n"), 1, doc));
      this->setOffset(0);

      _lineSpacing = 55;
      _tabulationSize = 0;
    }

    DocParagraph::~DocParagraph()
    {
        delete _endOfParagraph;
        _endOfParagraph = nullptr;
        _styleTMP = nullptr;
        _lineSpacing = 0;
        _tabulationSize = 0;
    }

    DocParagraph* DocParagraph::clone()
    {
        auto e = new DocParagraph(this->getDocument());
        QList<DocString*> elements = this->getElements();
        elements.pop_back();
        for (DocString* s : elements)
            e->add(s->clone());
        e->_endOfSelection = _endOfSelection;
        e->_index = _index;
        e->_isSplitting = _isSplitting;
        e->setLength(this->length());
        e->setOffset(this->offset());
        e->setTabulationSize(this->tabulationSize());
        e->setLineSpacing(this->lineSpacing());

        return e;
    }

    DocParagraph* DocParagraph::getSelection()
    {
        int offset = this->offset();
        if (this->isEmpty() || _endOfSelection == offset)
            return nullptr;

        auto result = new DocParagraph(getDocument());
        QList<DocString*> list;
	list.reserve(_elements.size()+1);
        for (DocString* e : getStrings())
        {
            DocString* selection = e->getSelection();
            if (selection != nullptr)
                list.append(selection);
        }

/*
        int min = (_endOfSelection < offset) ? _endOfSelection : offset;
        int max = (_endOfSelection > offset) ? _endOfSelection : offset;
        if (max > _elements.count())
            max = _elements.count();

        for (int i = min ; i < max ; i++)
        {
            DocString* s = _elements.at(i);
            DocString* selection = s->getSelection();
            if (selection != nullptr)
                list.append(selection);
        }
*/
        result->add(list);

        return result;
    }

    QList<DocString*> DocParagraph::getElements() const
    {
        QList<DocString*> list = NodeOfNodes<DocString>::getElements();
        list.append(_endOfParagraph);
        return list;
    }

    void DocParagraph::add(DocCharacter* e)
    {
      assert(e);
      //std::cerr<<"***** DocParagraph::add(DocCharacter *c= "<<e<<")\n";

        DocString* current = currentElement();
        DocString* previous = previousElement();
        DocStyle* styleOfCurrent = (current != nullptr) ? current->getStyle() : nullptr;
        bool noStyleChanges = (_styleTMP == styleOfCurrent ||
                               (_styleTMP != nullptr && styleOfCurrent != nullptr &&
                                _styleTMP->getName() == styleOfCurrent->getName()));

	//std::cerr<<" ***** DocParagraph::add this->isEmpty()="<<this->isEmpty()<<" current="<<current<<" noStyleChanges="<<noStyleChanges<<" _styleTMP="<<_styleTMP<<" this->getDocument()="<<this->getDocument()<<"\n";

        if (this->isEmpty()) {
	  current = new DocString(_styleTMP, getDocument());
	  add(current);
        }
        else if (current == nullptr || !noStyleChanges) {
	  const bool verifyPrevious = (current == nullptr || current->isAtBeginning());
	  DocStyle* styleOfPrevious = (previous != nullptr) ? previous->getStyle() : nullptr;
	  const bool previousHasSameStyle = (_styleTMP == styleOfPrevious ||
					     (_styleTMP != nullptr && styleOfPrevious != nullptr &&
					      _styleTMP->getName() == styleOfPrevious->getName()));
	  if (verifyPrevious && previousHasSameStyle) {
	    current = previous;
	  }
	  else {
	    current = new DocString(_styleTMP, getDocument());
	    add(current);
	  }
        }

        current->add(e);
        setLength(length()+e->length());
    }

    void DocParagraph::add(const QList<DocCharacter*> &l)
    {
      for (DocCharacter* e : l)
	this->add(e);
    }

    void DocParagraph::add(DocString* e)
    {
      //std::cerr<<"***** DocParagraph::add(DocString *s= "<<e<<") style="<<(e != nullptr ? e->getStyle() : nullptr)<<"\n";

        NodeOfNodes<DocString>::add(e);
    }

    void DocParagraph::add(const QList<DocString*> &l)
    {
      for (DocString* e : l)
            this->add(e);
    }

    void DocParagraph::setLineSpacing(int value)
    {
        if (value < 0)
            value = 0;

        _lineSpacing = value;
    }

    void DocParagraph::setTabulationSize(int value)
    {
        if (value < 0)
            value = 0;

        _tabulationSize = value;
    }

    void DocParagraph::setLength(int value)
    {
        NodeOfNodes<DocString>::setLength(value - _endOfParagraph->length());
    }

    int DocParagraph::length() const
    {
      assert(_endOfParagraph != nullptr);
      //B: can we have _endOfParagraph->length() != 1 ???
      const int eopLength = _endOfParagraph->length();
      return NodeOfNodes<DocString>::length() + eopLength;
    }

    void DocParagraph::actionWhenElementIsEmpty(DocString* empty)
    {
        int index = _elements.indexOf(empty);
        if (index == -1)
            return;

        if (empty != nullptr && empty->isEmpty())
        {
            _index = index;
            _elements.removeAt(_index);
            this->setLength(this->length()-empty->length());
            delete empty;
        }
    }

    void DocParagraph::removeBeforeAndCurrentAtBeginning()
    {
        this->setOffset(this->offset()-1);
        DocString* current = this->nextElement();
        if (current == nullptr)
            return;
        current->setOffset(current->length());
        current->removeBeforeCursor();
        this->setLength(this->length()-1);
    }

    void DocParagraph::removeAfterAndCurrentAtEnd()
    {
        this->setOffset(this->offset()+1);
        this->removeBeforeCursor();
    }

    void DocParagraph::setOffset(int value)
    {
        int l = length();
        if (value >= l)
            _endOfParagraph->setOffset(_endOfParagraph->length());
        else
            _endOfParagraph->setOffset(0);

        if (value < 0)
            value = 0;
        else if (value > (l - _endOfParagraph->length()))
            value = l - _endOfParagraph->length();

        const int previousOffset = offset();
        NodeOfNodes<DocString>::setOffset(value);

        if (!_isSelecting && !_isRemovingSelection)
            _endOfSelection = this->offset();

        if (offset() == previousOffset)
            return;

        DocString* current = currentElement();
        DocString* previous = previousElement();
        if (current->isAtBeginning() && previous != nullptr)
            current = previous;
        if (current != nullptr)
            _styleTMP = current->getStyle();
    }

    int DocParagraph::offset() const
    {
        return NodeOfNodes<DocString>::offset() + _endOfParagraph->offset();
    }

    const DocString* DocParagraph::currentString() const
    {
        return this->currentElement();
    }

    DocString* DocParagraph::currentString()
    {
        return this->currentElement();
    }

    QList<DocString*> DocParagraph::getStrings() const
    {
        QList<DocString*> list = _elements;
        list.append(_endOfParagraph);
        return list;
    }

    void DocParagraph::changeStyle(DocStyle* style)
    {
        _styleTMP = style;
    }

    DocStyle* DocParagraph::getStyle()
    {
        DocString* current = currentElement();
        if (current == nullptr || current->getStyle() != _styleTMP)
            return _styleTMP;

        return current->getStyle();
    }

    QList<DocParagraph*> DocParagraph::splitAtPosition(int position)
    {
        QList< DocParagraph* > list;
        if (isEmpty())
            return list;

        DocParagraph* firstPart = this->clone();
        DocParagraph* secondPart = this->clone();

        firstPart->select(position, secondPart->length());
        firstPart->removeSelection();
        firstPart->setOffset(0);
        secondPart->select(0, position);
        secondPart->removeSelection();
        secondPart->setOffset(position);

        list.append(firstPart);
        list.append(secondPart);

        return list;
    }

    bool DocParagraph::isEmpty() const
    {
      assert(_endOfParagraph != nullptr);
      const int eopLength = _endOfParagraph->length();
      return _elements.isEmpty() && length() == eopLength;
    }

    bool DocParagraph::isAtEnd() const
    {
        const DocString* current = this->currentString();
        const bool last = (_index == _elements.count()-1);
        return NodeOfNodes<DocString>::isAtEnd() || (current != nullptr && current->isAtEnd() && last);
    }
  
} //namespace Doc
