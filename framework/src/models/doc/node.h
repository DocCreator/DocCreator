#ifndef NODE_H
#define NODE_H

#include <QObject>
#include <framework_global.h>

#include "element.h"
#include "docstyle.h"

#include <iostream>//DEBUG

namespace Doc
{
    template< class T >
    class Node : public Element
    {
    public:
        explicit Node(Document* document);
        ~Node();

        virtual Node<T>* clone() = 0;

        virtual QList<T*> getElements() const { return _elements; }
        virtual Node<T>* getSelection() = 0;

        virtual void setOffset(int value);
        virtual int offset() const { return _index; }

        virtual void add(T* element) = 0;
        virtual void add(const QList<T*> &list) = 0;

        virtual int removeBeforeCursor() = 0;
        virtual int removeAfterCursor() = 0;
        virtual int removeSelection();

        virtual bool isEmpty() const { return !_elements.count() && !length(); }
        virtual bool isAtBeginning() const { return !_index; }
        virtual bool isAtEnd() const { return _index >= length(); }

        virtual void select(int from, int to);

    protected:
        virtual T* currentElement() const;
        virtual T* previousElement() const;
        virtual T* nextElement() const;
        virtual void insertElement(int pos, T* element);
        virtual int getIndexFromOffset(int value);
        virtual void selectHandler(int from, int to) = 0;

    protected:
        QList<T*> _elements;
        int _index; // index to insert
        bool _isSplitting;
        bool _isSelecting;
        bool _isRemovingSelection;
        int _endOfSelection;

    };

    template< class T >
    Node<T>::Node(Document* document) : Element(document)
    {
        _index = 0;
        _isSplitting = false;
        _isSelecting = false;
        _isRemovingSelection = false;
        _endOfSelection = _index;
    }

    template< class T >
    Node<T>::~Node()
    {
      for (T* e : _elements) {
	delete e;
      }
      _elements.clear();

      _index = 0;
      _isSplitting = false;
      _isSelecting = false;
      _isRemovingSelection = false;
      _endOfSelection = _index;
    }

    template< class T >
    Node<T>* clone()
    {
        return new Node<T>();
    }

    template< class T >
    int Node<T>::removeSelection()
    {
        int offset = this->offset();
        if (this->isEmpty() || _endOfSelection == offset)
            return 0;

        _isRemovingSelection = true;
        int counter = 0;
        if (_endOfSelection < offset)
        {
            while (_endOfSelection != offset)
            {
                counter = counter + this->removeBeforeCursor();
            }
        }

        else
        {
            while (_endOfSelection != offset)
            {
                counter = counter + this->removeAfterCursor();
                _endOfSelection = _endOfSelection - 1;
            }
        }
        _isRemovingSelection = false;

        return counter;
    }

    template< class T >
    void Node<T>::select(int from, int to)
    {
        _isSelecting = true;
        selectHandler(from, to);
        _isSelecting = false;
    }

    template< class T >
    T* Node<T>::currentElement() const
    {
        if (isEmpty())
            return nullptr;

        T* next = nextElement();
        T* previous = previousElement();
        T* current = next;
        if (next == nullptr)
            current = previous;

        return current;
    }

    template< class T >
    void Node<T>::setOffset(int value)
    {
        int l = length();
        if (value < 0)
            value = 0;
        else if (value > l)
            value = l;

        _index = getIndexFromOffset(value);

        if (!_isSelecting && !_isRemovingSelection)
            _endOfSelection = this->offset();
    }

    template< class T >
    T* Node<T>::previousElement() const
    {
        if ((1 <= _index) && (_index <= _elements.count()))
            return _elements.at(_index - 1);

        return nullptr;
    }

    template< class T >
    T* Node<T>::nextElement() const
    {
        if ((0 <= _index) && (_index < _elements.count()))
            return _elements.at(_index);

        return nullptr;
    }

    template< class T >
    void Node<T>::insertElement(int pos, T* element)
    {
        if (pos < 0 || pos > _elements.count())
            return;
	
#ifndef NDEBUG
	if (_elements.contains(element)) {
	  std::cerr<<"ERROR: Node<T>::insertElement("<<pos<<", "<<element<<") : element already present\n";
	  exit(12);
	}
#endif //NDEBUG

        _index = pos;
        _elements.insert(pos, element);
        _index = pos+1;
        setLength(length() + element->length());
    }

    template< class T >
    int Node<T>::getIndexFromOffset(int value)
    {
        int counter = 0;

        while (value > 0)
        {
            if (0 <= counter && counter < _elements.count())
            {
                T* e = _elements.at(counter);
                if (value < e->length())
                {
                    value = -1;
                    break;
                }
                value -= e->length();
            }
            else
            {
                value = -1;
                break;
            }
            ++counter;
        }
        return counter;
    }
}

#endif // NODE_H
