#include "doczone.h"


namespace Doc
{
DocZone::DocZone(Document* document) : NodeOfLeafs<DocComponent>(document)
{

}
  

DocZone* DocZone::clone()
{
    auto *dz = new DocZone(this->getDocument());
    for (DocComponent* c : this->getElements())
        dz->add(c->clone());

    dz->_endOfSelection = _endOfSelection;
    dz->_index = _index;
    dz->_isSplitting = _isSplitting;

    dz->setLength(this->length());
    dz->setOffset(this->offset());

    return dz;
}

DocZone* DocZone::getSelection()
{
    const int offset = this->offset();
    if (this->isEmpty() || _endOfSelection == offset)
        return nullptr;

    auto result = new DocZone(getDocument());
    QList<DocComponent*> list;

    const int min = (_endOfSelection < offset) ? _endOfSelection : offset;
    int max = (_endOfSelection > offset) ? _endOfSelection : offset;
    if (max > _elements.count())
        max = _elements.count();

    list.reserve(max-min);
    for (int i = min ; i < max ; i++)
    {
        DocComponent* c = _elements.at(i);
        if (c != nullptr)
            list.append(c->clone());
    }

    result->add(list);

    return result;
}

DocComponent* DocZone::currentComponent(){
    return this->currentElement();
}

QList<DocComponent*> DocZone::getComponents(){
    return _elements;
}

QList<DocZone*> DocZone::splitAtPosition(int position){
    QList< DocZone* > list;
    if (isEmpty())
        return list;

    DocZone* firstPart = this->clone();
    DocZone* secondPart = this->clone();

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
