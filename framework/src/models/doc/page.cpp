#include "page.h"

#include <cassert>

#include "block.h"
#include "doccomponentblock.h"
#include "docimageblock.h"
#include "doctestblock.h"
#include "doctextblock.h"


namespace Doc
{

  Page::Page(Document* document) :
    _document(document), 
    _currentElement(nullptr),
    _backgroundFileName()
  {
  }

  Page::~Page()
  {
    for (DocTextBlock* b : _textBlocks) {
      delete b;
    }
    for (DocImageBlock* b : _imageBlocks) {
      delete b;
    }
    for (DocTestBlock* b : _testBlocks) {
      delete b;
    }
    for (DocComponentBlock* b : _componentBlocks) {
      delete b;
    }
    for (Block* b : _otherBlocks) {
      delete b;
    }

  }


  Page* Page::clone()
  {
    auto e = new Page(_document);
    e->_textBlocks = _textBlocks;
    e->_imageBlocks = _imageBlocks;
    e->_testBlocks = _testBlocks;
    e->_componentBlocks = _componentBlocks;


    e->_otherBlocks = _otherBlocks;
    e->_backgroundFileName = _backgroundFileName;

    return e;
  }

Page* Page::getSelection()
{
    auto b = dynamic_cast<DocTextBlock*>(currentBlock());
    if (b == nullptr) {
        return nullptr;
    }

    DocTextBlock* selection = b->getSelection();
    if (selection == nullptr) {
        return nullptr;
    }

    auto result = new Page(_document);
    result->add(selection);

    return result;
}

void Page::changeStyle(DocStyle* style)
{
    DocTextBlock* current = dynamic_cast<DocTextBlock*>(currentBlock());
    if (current == nullptr) {
        return;
    }

    current->changeStyle(style);
}

DocStyle* Page::getStyle()
{
    DocTextBlock* current = dynamic_cast<DocTextBlock*>(currentBlock());
    if (current == nullptr) {
        return nullptr;
    }

    return current->getStyle();
}

void Page::add(Block* e)
{
  if (_otherBlocks.contains(e)) {
        return;
  }

    //B:TODO: better to have an enum for type in Block 
    //rather than to use RTTI ?
    
    //Should we have a switch here 
    // if TextBlock, it must be  added to _textBlocks,
    // if ImageBlock, it must be added to _imageBlocks
    //...

    DocTextBlock* current = dynamic_cast<DocTextBlock*>(_currentElement);
    if (current != nullptr) {
        current->setOffset(0);
    }

    setCurrentBlock(e);
    _otherBlocks.append(e);
}

void Page::removeTextBlocks()
{
  DocTextBlock* current = dynamic_cast<DocTextBlock*>(_currentElement);
  if (current != nullptr) {
    current = nullptr; //B:TODO:USELESS/BUG ?
  }
  _textBlocks.clear();
}
  
void Page::add(DocTextBlock* e)
{
  if (_textBlocks.contains(e)) {
        return;
  }

    DocTextBlock* current = dynamic_cast<DocTextBlock*>(_currentElement);
    if (current != nullptr) {
        current->setOffset(0);
    }

    setCurrentBlock(e);
    _textBlocks.append(e);
}

void Page::add(DocImageBlock* e)
{
  if (_imageBlocks.contains(e)) {
        return;
  }

    DocTextBlock* current = dynamic_cast<DocTextBlock*>(_currentElement);//???? For image block why define new text block
    //DocImageBlock* current = dynamic_cast<DocImageBlock*>(_currentElement);
    if (current != nullptr) {
        current->setOffset(0);
    }

    setCurrentBlock(e);
    _imageBlocks.append(e);
}

void Page::add(DocTestBlock* e)
{
  if (_testBlocks.contains(e)) {
        return;
  }

    DocTextBlock* current = dynamic_cast<DocTextBlock*>(_currentElement);//???? For image block why define new text block
    //DocImageBlock* current = dynamic_cast<DocImageBlock*>(_currentElement);
    if (current != nullptr) {
        current->setOffset(0);
    }

    setCurrentBlock(e);
    _testBlocks.append(e);
}

void Page::add(DocComponentBlock *e){

  if(_componentBlocks.contains(e)) {
      return;
  }

    DocTextBlock* current = dynamic_cast<DocTextBlock*>(_currentElement);

    if(current != nullptr) {
      current->setOffset(0);
    }

    setCurrentBlock(e);
    _componentBlocks.append(e);
}

void Page::remove(Block* e)
{
  if (_currentElement == e) {
        _currentElement = nullptr;
  }
    DocTextBlock* text = dynamic_cast<DocTextBlock*>(e);
    if (_textBlocks.contains(text)) {
        _textBlocks.removeAll(text);
    }

    DocImageBlock* image = dynamic_cast<DocImageBlock*>(e);
    if (_imageBlocks.contains(image)) {
        _imageBlocks.removeAll(image);
    }
    DocTestBlock* test = dynamic_cast<DocTestBlock*>(e);
    if (_testBlocks.contains(test)) {
        _testBlocks.removeAll(test);
    }
    DocComponentBlock* component = dynamic_cast<DocComponentBlock*>(e);
    if (_componentBlocks.contains(component)) {
        _componentBlocks.removeAll(component);
    }
    if (_otherBlocks.contains(e)) {
        _otherBlocks.removeAll(e);
    }
}

void Page::setCurrentBlock(Block* e)
{
    _currentElement = e;
}

Block* Page::currentBlock()
{
  return _currentElement;
}

void Page::add(DocParagraph* e)
{
    DocTextBlock* current = dynamic_cast<DocTextBlock*>(_currentElement);
    if (current == nullptr) {
      add(new DocTextBlock(_document)); //B return;
    }
    assert(current);
    current->add(e);
}

void Page::add(const QList<DocParagraph*> &e)
{
    DocTextBlock* current = dynamic_cast<DocTextBlock*>(_currentElement);
    if (current == nullptr) {
        add(new DocTextBlock(_document)); //B return;
    }
    assert(current);
    current->add(e);
}

void Page::add(DocString* e)
{
    DocTextBlock* current = dynamic_cast<DocTextBlock*>(_currentElement);
    if (current == nullptr) {
        add(new DocTextBlock(_document)); //B return;
    }
    assert(current);
    current->add(e);
}

void Page::add(const QList<DocString*> &e)
{
    DocTextBlock* current = dynamic_cast<DocTextBlock*>(_currentElement);
    if (current == nullptr) {
        add(new DocTextBlock(_document)); //B return;
    }
    assert(current);
    current->add(e);
}

void Page::add(DocCharacter* e)
{
    DocTextBlock* current = dynamic_cast<DocTextBlock*>(_currentElement);
    if (current == nullptr) {
        add(new DocTextBlock(_document)); //B return;
    }
    assert(current);
    current->add(e);
}

void Page::add(const QList<DocCharacter*> &e)
{
    DocTextBlock* current = dynamic_cast<DocTextBlock*>(_currentElement);
    if (current == nullptr) {
        add(new DocTextBlock(_document)); //B return;
    }
    assert(current);
    current->add(e);
}

void Page::add(DocComponent *e){
    DocComponentBlock* current = dynamic_cast<DocComponentBlock*>(_currentElement);
    if(current == nullptr)  {
      add(new DocComponentBlock(_document)); //B return;
    }
    assert(current);
    current->add(e);
}

void Page::add(const QList<DocComponent*> &e){
    DocComponentBlock* current = dynamic_cast<DocComponentBlock*>(_currentElement);
    if(current == nullptr) {
      add(new DocComponentBlock(_document)); //B return;
    }
    assert(current);
    current->add(e);
}

void Page::add(DocZone *e){
    DocComponentBlock* current = dynamic_cast<DocComponentBlock*>(_currentElement);
    if(current == nullptr)  {
      add(new DocComponentBlock(_document)); //B return;
    }
    assert(current);
    current->add(e);
}

void Page::add(const QList<DocZone*> &e){
    DocComponentBlock* current = dynamic_cast<DocComponentBlock*>(_currentElement);
    if(current == nullptr)  {
      add(new DocComponentBlock(_document)); //B return;
    }
    assert(current);
    current->add(e);
}
}//namespace Doc
