#ifndef NODEOFNODES_H
#define NODEOFNODES_H

#include <QObject>
#include <framework_global.h>

#include "node.h"

namespace Doc
{
    template< class T >
    class NodeOfNodes : public Node< T >
    {
    public:
      explicit NodeOfNodes(Document* document);

      void setOffset(int value) override;
      int offset() const override;
      
      void add(T* element) override;
      void add(const QList<T*> &list) override;
      
      int removeBeforeCursor() override;
      virtual void removeBeforeAndCurrentAtBeginning() = 0;
      int removeAfterCursor() override;
      virtual void removeAfterAndCurrentAtEnd() = 0;
      
      bool isAtBeginning() const override;
      bool isAtEnd() const override;
      
      virtual void actionWhenElementIsEmpty(T* empty) = 0;
      
    protected:
      virtual void insertAtBeginning(T* element);
      virtual void insertAtEnd(T* element);
      void selectHandler(int from, int to) override;
    };
    
    template< class T >
    NodeOfNodes<T>::NodeOfNodes(Document* document) : 
      Node< T >(document)
    {
    }

    template< class T >
    void NodeOfNodes<T>::setOffset(int value)
    {
        Node<T>::setOffset(value);
        T* current;
        for (int i = 0 ; i < this->_index; i++)
        {
            current = this->_elements.at(i);
            int l = current->length();
            if (current->offset() != l)
                current->setOffset(l);
            value = value - l;
        }

        if (0 <= this->_index && this->_index < this->_elements.count())
        {
            current = this->_elements.at(this->_index);
            current->setOffset(value);
        }

        for (int i = this->_elements.count()-1 ; i > this->_index; i--)
        {
            current = this->_elements.at(i);
            current->setOffset(0);
        }

        if (!this->_isSelecting && !this->_isRemovingSelection)
            this->_endOfSelection = this->offset();
    }

    template< class T >
    int NodeOfNodes<T>::offset() const
    {
        int offset = 0;
        for (int i = 0 ; i <= this->_index; i++)
        {
            if (i >= this->_elements.count())
                return offset;
            T* e = this->_elements.at(i);
            offset += e->offset();
        }
        return offset;
    }

    template< class T >
    void NodeOfNodes<T>::add(T* element)
    {
        T* next = this->nextElement();
        bool nextAtBeginning = (next != nullptr && next->isAtBeginning());
        bool nextAtEnd = (next != nullptr && next->isAtEnd());

        if (this->isAtBeginning() || this->isAtEnd() || nextAtBeginning)
        {
            insertAtBeginning(element);
            return;
        }
        else if (nextAtEnd)
        {
            insertAtEnd(element);
            return;
        }

        //else, we have to split the current element;
        this->_isSplitting = true;

        const int currentOffset = this->offset();
	assert(next);
        const int offsetOfNext = next->offset();

        QList<T*> list = next->splitAtPosition(next->offset());
        this->_elements.removeAt(this->_index);
        this->setLength(this->length()-next->length());
        this->setOffset(currentOffset - offsetOfNext);

        add(list.at(0));
        add(element);
        add(list.at(1));
        setOffset(currentOffset+element->length());
        this->_isSplitting = false;
    }

    template< class T >
    void NodeOfNodes<T>::add(const QList<T*> &list)
    {
      for (T* e : list)
            add(e);
    }

    template< class T >
    int NodeOfNodes<T>::removeBeforeCursor()
    {
        if (this->isEmpty() || this->isAtBeginning())
            return 0;

        T* next = this->nextElement();
        T* previous = this->previousElement();
        T* current = next;
        if (current == nullptr)
        {
            current = previous;
            if (current == nullptr)
                return 0;
        }

        int counter = 0;
        if (current->isAtBeginning())
        {
            removeBeforeAndCurrentAtBeginning();
            current = previous;
        }
        else
            counter = current->removeBeforeCursor();

        if (current != nullptr && current->isEmpty())
            actionWhenElementIsEmpty(current);

        this->setLength(this->length()-counter);
        return counter;
    }

    template< class T >
    int NodeOfNodes<T>::removeAfterCursor()
    {
        if (this->isEmpty() || this->isAtEnd())
            return 0;

        T* current = this->nextElement();
        if (current == nullptr)
            return 0;

        int counter = 0;
        if (current->isAtEnd())
            removeAfterAndCurrentAtEnd();
        else
            counter = current->removeAfterCursor();

        if (current->isEmpty())
            actionWhenElementIsEmpty(current);

        this->setLength(this->length()-counter);
        return counter;
    }

    template< class T >
    bool NodeOfNodes<T>::isAtBeginning() const 
    {
        T* next = this->nextElement();
        return (this->isEmpty() || (this->_index == 0 && (next != nullptr && next->isAtBeginning())));
    }

    template< class T >
    bool NodeOfNodes<T>::isAtEnd() const
    {
        return (this->isEmpty() || (this->_index == this->_elements.count()));
    }

    template< class T >
    void NodeOfNodes<T>::insertAtBeginning(T* element)
    {
        this->insertElement(this->_index, element);
        return;
    }

    template< class T >
    void NodeOfNodes<T>::insertAtEnd(T* element)
    {
        T* next = this->nextElement();
        if (next == nullptr)
            return;
        next->setOffset(next->length());
        this->_index = this->_index + 1;
        next = this->nextElement();
        if (next != nullptr)
            next->setOffset(0);
        insertAtBeginning(element);
        return;
    }

    template< class T >
    void NodeOfNodes<T>::selectHandler(int from, int to)
    {
        this->setOffset(to);
        to = this->offset();
        this->_endOfSelection = to;
        int indexTo = this->_index;
        this->setOffset(from);
        from = this->offset();
        if (from == to)
            return;

        int indexFrom = this->_index;
        int min = 0;
        int max = 0;
        int indexMin = 0;
        int indexMax = 0;
        if (from <= to)
        {
            indexMin = indexFrom;
            min = from;
            indexMax = indexTo;
            max = to;
        }
        else
        {
            indexMax = indexFrom;
            max = from;
            indexMin = indexTo;
            min = to;
        }
        if (indexMin < 0)
            indexMin = 0;
        if (indexMax >= this->_elements.count())
            indexMax = this->_elements.count()-1;

        int l = 0;
        for (int i = 0 ; i < indexMin ; i++)
        {
            T* e = this->_elements.at(i);
            int offsetMax = e->length();
            e->select(offsetMax, offsetMax);
            l = l + offsetMax;
        }

        for (int i = indexMin ; i <= indexMax ; i++)
        {
            T* e = this->_elements.at(i);
            int offsetMin = 0;
            if(i == indexMin)
            {
                offsetMin = min-l;
                //if (i == indexMax-1 && ((max-l) == e->length()))
                //    indexMax = indexMax - 1;
            }
            int offsetMax = e->length();
            if(i == indexMax)
                offsetMax = max-l;
            e->select(offsetMin, offsetMax);
            l = l + e->length();
        }
    }
}

#endif // NODEOFNODES_H
