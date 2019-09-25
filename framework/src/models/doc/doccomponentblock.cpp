#include "doccomponentblock.h"

#include "models/doc/doccomponent.h"


namespace Doc
{
DocComponentBlock::DocComponentBlock(Document* document): 
  NodeOfNodes<DocZone>(document)
{     
}

DocComponentBlock::DocComponentBlock(Document* document, int w, int h, int x, int y) : 
  NodeOfNodes<DocZone>(document), Block(w, h, x, y)
{

}



DocComponentBlock* DocComponentBlock::clone(){
    auto e = new DocComponentBlock(this->getDocument(), this->width(), this->height(), this->x(), this->y());
    e->_elements = _elements;
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

DocComponentBlock* DocComponentBlock::getSelection()
{
    int offset = this->offset();
    if (this->isEmpty() || _endOfSelection == offset)
        return nullptr;

    auto result = new DocComponentBlock(getDocument());
    QList<DocZone*> list;
    list.reserve(_elements.size());
    const auto &elements = _elements; //getZones()
    for (DocZone* e : elements)
    {
        DocZone* selection = e->getSelection();
        if (selection != nullptr)
            list.append(selection);
    }
    result->add(list);

    return result;
}

void DocComponentBlock::add(DocComponent* e)
{
    DocZone* current = currentElement();
    if (current == nullptr)
    {
        current = new DocZone(this->getDocument());
        add(current);
        setOffset(offset()-1);
    }
    current->add(e);
    setLength(length()+e->length());
}

void DocComponentBlock::add(const QList<DocComponent*> &l)
{
  for (DocComponent* e : l)
      this->add(e);
}

void DocComponentBlock::add(DocZone* e)
{
    DocZone* next = nextElement();
    DocZone* previous = previousElement();
    const bool dontSplitNext = (next == nullptr || (next->isAtBeginning() || next->isAtEnd()));
    const bool dontSplit = (this->isEmpty() || this->isAtBeginning() || this->isAtEnd());
    if (dontSplit || dontSplitNext)
    {
        int oldOffset = this->offset();
        NodeOfNodes<DocZone>::add(e);
        this->setOffset(oldOffset+e->length());
    }
    else
    {
        DocZone* current = next;
        if (current == nullptr) {
            current = previous;
	}
        if (current == nullptr) {
            return;
	}

        int oldOffset = this->offset();
        QList<DocZone*> splitted = current->splitAtPosition(current->offset());
        DocZone* toRemove = _elements.at(_index);
        _elements.removeAt(_index);
        for (DocZone* p : splitted) {
            NodeOfNodes<DocZone>::add(p);
	}
        this->setLength(this->length()-toRemove->length());
        this->setOffset(oldOffset+e->length());
    }
}

void DocComponentBlock::add(const QList<DocZone*> &l)
{
  for (DocZone* e : l)
    this->add(e);
}

DocZone* DocComponentBlock::currentZone()
{
    return this->currentElement();
}

QList<DocZone*> DocComponentBlock::getZones()
{
  return _elements;
}

void DocComponentBlock::actionWhenElementIsEmpty(DocZone* /*empty*/)
{
}

void DocComponentBlock::removeBeforeAndCurrentAtBeginning()
{
}

void DocComponentBlock::removeAfterAndCurrentAtEnd()
{

}

QString DocComponentBlock::content() const
{
  return QStringLiteral("FILE:\\");
}
  
} //namespace Doc
