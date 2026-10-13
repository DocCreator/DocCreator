#ifndef NODEOFLEAFS_H
#define NODEOFLEAFS_H

#include <QObject>
#include <framework_global.h>

#include "node.h"
#include "leaf.h"

namespace Doc
{
    template< class T >
    class NodeOfLeafs : public Node< T >
    {
    public:
      explicit NodeOfLeafs(Document* document);

      void add(T* element) override;
      void add(const QList<T*> &list) override;
      
      int removeBeforeCursor() override;
      int removeAfterCursor() override;
      
    protected:
      void selectHandler(int from, int to) override;
    };

    template< class T >
    NodeOfLeafs<T>::NodeOfLeafs(Document* document) : Node< T >(document)
    {
    }

    template< class T >
    void NodeOfLeafs<T>::add(T* element)
    {
      this->insertElement(this->_index, element);
    }

    template< class T >
    void NodeOfLeafs<T>::add(const QList<T*> &list)
    {
      for (T* e : list)
	add(e);
      //B:TODO:OPTIM: would be faster if class Node provided an insertElements(index, elements) method !
    }

    /*
      B: the removed leaf was taken out of _elements and dropped. The node
      owns its elements (cf ~Node, which deletes them all) and nothing else
      holds the removed one -- these two return a length, not the element --
      so every removed leaf leaked. Removing the selection of a string of n
      characters leaked n of them, which is what every split does twice.
    */
    template< class T >
    int NodeOfLeafs<T>::removeBeforeCursor()
    {
      if (this->isEmpty() || this->isAtBeginning())
	return 0;
      
      int oldOffset = this->offset();
      this->_index = this->_index - 1;
      T* removed = this->_elements.at(this->_index);
      int counter = removed->length();
      this->_elements.removeAt(this->_index);
      delete removed;
      this->setLength(this->length()-counter);
      this->setOffset(oldOffset-counter);
      
      return counter;
    }
    
    template< class T >
    int NodeOfLeafs<T>::removeAfterCursor()
    {
        if (this->isEmpty() || this->isAtEnd())
            return 0;

        T* removed = this->_elements.at(this->_index);
        int counter = removed->length();
        this->_elements.removeAt(this->_index);
        delete removed;
        this->setLength(this->length()-counter);

        return counter;
    }

    template< class T >
    void NodeOfLeafs<T>::selectHandler(int from, int to)
    {
      this->setOffset(to);
      to = this->offset();
      this->_endOfSelection = to;
      //int indexTo = this->_index;  //B?
      this->setOffset(from);
      //from = this->offset(); //B?
    }
}

#endif // NODEOFLEAFS_H
