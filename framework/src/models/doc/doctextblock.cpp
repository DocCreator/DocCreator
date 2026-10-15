#include "doctextblock.h"

#include <cassert>
//#include <iostream> //DEBUG

#include "doccharacter.h"
#include "docstring.h"
#include "docstyle.h"
#include "document.h"


namespace Doc
{
    DocTextBlock::DocTextBlock(Document* document) : 
      NodeOfNodes< DocParagraph >(document)
    {
    }

    DocTextBlock::DocTextBlock(Document* document, int w, int h, int x, int y) : 
      NodeOfNodes< DocParagraph >(document), 
      Block(w, h, x, y)
    {
    }

    DocTextBlock* DocTextBlock::clone()
    {
        auto e = new DocTextBlock(this->getDocument(), this->width(), this->height(), this->x(), this->y());
        //e->_elements = _elements; //B:BUG: should be cloned !!!
	//B ????
	for (DocParagraph* p : this->getElements())
	  NodeOfNodes<DocParagraph>::add(p->clone()); //e->add(p->clone());
	//B ??? correct ???

        e->_endOfSelection = _endOfSelection;
        e->_index = _index;
        e->_isSplitting = _isSplitting;
        e->setLength(this->length());
        e->setOffset(this->offset());
        e->setMarginTop(this->marginTop());
        e->setMarginBottom(this->marginBottom());
        e->setMarginLeft(this->marginLeft());
        e->setMarginRight(this->marginRight());

        return e;
    }

    DocTextBlock* DocTextBlock::getSelection()
    {
        int offset = this->offset();
        if (this->isEmpty() || _endOfSelection == offset) {
            return nullptr;
	}

        auto result = new DocTextBlock(getDocument());
        QList<DocParagraph*> list;
	list.reserve(_elements.size());
	const auto &elements = _elements;//getParagraphs())
        for (DocParagraph* e : elements) 
        {
            DocParagraph* selection = e->getSelection();
            if (selection != nullptr) {
                list.append(selection);
	    }
        }
/*
        int min = (_endOfSelection < offset) ? _endOfSelection : offset;
        int max = (_endOfSelection > offset) ? _endOfSelection : offset;
        if (max > _elements.count())
            max = _elements.count();

        for (int i = min ; i < max ; i++)
        {
            DocParagraph* p = _elements.at(i);
            DocParagraph* selection = p->getSelection();
            if (selection != nullptr)
                list.append(selection);
        }*/

        result->add(list);

        return result;
    }

    void DocTextBlock::add(DocCharacter* e)
    {
      DocParagraph* current = currentElement();
      if (current == nullptr) {
	current = new DocParagraph(getDocument());	
	//std::cerr<<"DocTextBlock::add(DocCharacter* "<<e<<") document="<< getDocument()<<" new paragrpah="<<current<<"\n";
	add(current);
	assert(getDocument() && getDocument()->currentParagraph() == current);

	setOffset(offset()-1);
      }
      current->add(e);
      setLength(length() + e->length()); //B: assert(e->length() == 1) ???
    }

    void DocTextBlock::add(const QList<DocCharacter*> &l)
    {
      for (DocCharacter* e : l)
            this->add(e);
    }

    void DocTextBlock::add(DocString* e)
    {
      DocParagraph* current = currentElement();
      if (current == nullptr) {
	current = new DocParagraph(getDocument());
	add(current);
      }
      current->add(e);
      setLength(length()+e->length());
    }

    void DocTextBlock::add(const QList<DocString*> &l)
    {
      for (DocString* e : l)
	this->add(e);
    }

    void DocTextBlock::add(DocParagraph* e)
    {
        DocParagraph* next = nextElement();
        const bool dontSplitNext = (next == nullptr || (next->isAtBeginning() || next->isAtEnd()));
        const bool dontSplit = (this->isEmpty() || this->isAtBeginning() || this->isAtEnd());
        if (dontSplit || dontSplitNext) {
            const int oldOffset = this->offset();
            //B: read before add(), which deletes an element it refuses
            const int eLength = e->length();
            NodeOfNodes<DocParagraph>::add(e);
            this->setOffset(oldOffset+eLength);
        }
        else {
            /*
              The cursor is inside a paragraph: it is split there, and the
              split is the paragraph break. An empty paragraph -- what the
              Return key adds -- is nothing more than that break, and is
              deleted. A paragraph with content goes between the two halves.

              B: the paragraph that was split was taken out of the list and
              never deleted, with its strings, which kept their style
              referenced; and @a e was neither inserted nor deleted. Both
              leaked on every Return inside a paragraph.
            */
            DocParagraph* current = next; //not null: dontSplitNext is false
            const int oldOffset = this->offset();
            const QList<DocParagraph*> halves =
              current->splitAtPosition(current->offset());
            if (halves.size() != 2) { //current is not empty: cannot happen
                delete e;
                return;
            }

            _elements.removeAt(_index);
            this->setLength(this->length()-current->length());

            NodeOfNodes<DocParagraph>::add(halves.at(0));
            //the end of paragraph of the first half, cf getStrings()
            int moved = halves.at(0)->getStrings().last()->length();
            if (e->isEmpty()) {
                delete e;
            }
            else {
                moved += e->length();
                NodeOfNodes<DocParagraph>::add(e);
            }
            NodeOfNodes<DocParagraph>::add(halves.at(1));
            delete current;

            this->setOffset(oldOffset+moved);
        }
    }

    void DocTextBlock::add(const QList<DocParagraph*> &l)
    {
      for (DocParagraph* e : l) {
            this->add(e);
      }
    }

    DocParagraph* DocTextBlock::currentParagraph()
    {
        return this->currentElement();
    }

    const QList<DocParagraph*> &DocTextBlock::getParagraphs() const
    {
      return _elements;
    }

    void DocTextBlock::changeStyle(DocStyle* style)
    {
      if (isEmpty()) {
            return;
      }

        DocParagraph* current = currentElement();
        if (current == nullptr)
        {
            DocParagraph* previous = previousElement();
            if (previous == nullptr) {
                return;
	    }
            current = previous;
        }
        current->changeStyle(style);
    }

    DocStyle* DocTextBlock::getStyle()
    {
        DocParagraph* current = currentElement();
        if (current == nullptr)
        {
            DocParagraph* previous = previousElement();
            if (previous == nullptr) {
                return nullptr;
	    }
            current = previous;
        }
        return current->getStyle();
    }

    void DocTextBlock::actionWhenElementIsEmpty(DocParagraph* /*empty*/)
    {
    }

    void DocTextBlock::removeBeforeAndCurrentAtBeginning()
    {
        DocParagraph* next = nextElement();
        DocParagraph* previous = previousElement();

        if (next != nullptr && next->isEmpty())
        {
            _elements.removeAt(_index);
            this->setLength(this->length()-next->length());
            this->setOffset(offset()-1);
        }
        else if (next != nullptr && next->isAtBeginning())
        {
            QList<DocString*> listPrevious = previous->getElements();
            int oldOffset = this->offset();
            listPrevious.pop_back();
            next->add(listPrevious);
            _index = _index - 1;
            _elements.removeAt(_index);
            this->setLength(this->length()-1);
            this->setOffset(oldOffset-1);
        }
    }

    void DocTextBlock::removeAfterAndCurrentAtEnd()
    {
        DocParagraph* next = nextElement();
        this->setOffset(this->offset()+1);
        DocParagraph* nextOfNext = nextElement();
        this->setOffset(this->offset()-1);

        if (next != nullptr && _index == _elements.count()-1) {
            return;
	}

        if (next != nullptr && next->isEmpty())
        {
            _elements.removeAt(_index);
            this->setLength(this->length()-next->length());
        }
        else if (next != nullptr && next->isAtEnd())
        {
            int oldOffset = this->offset();
            QList<DocString*> listNext = nextOfNext->getElements();
            listNext.pop_back();
            next->add(listNext);
            _index = _index + 1;
            _elements.removeAt(_index);
            this->setLength(this->length()-1);
            this->setOffset(oldOffset);
        }
    }

    QString DocTextBlock::content() const
    {
      QString content;
      for (DocParagraph* p : getElements()) {
	for (DocString* s : p->getElements()) {
	  for (DocCharacter* c :  s->getElements()) {
	    content = content+c->getDisplay();
	  }
	}
      }
      return content;
    }
}//namespace Doc
