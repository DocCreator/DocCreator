#include "document.h"

#include <cassert>
#include "docparagraph.h"
#include "docstylemanager.h"
#include "doctextblock.h"
#include "page.h"

namespace Doc
{
  Document::Document() :
    _currentPage(nullptr),
    _currentStyle(nullptr),
    _styleManager(new DocStyleManager()), //B:TODO: unique_ptr ???
    _pagesMarginLeft(0), 
    _pagesMarginRight(0), 
    _pagesMarginTop(0), 
    _pagesMarginBottom(0),
    _pageWidth(0), 
    _pageHeight(0),
    _defaultBlockMarginLeft(0),
    _defaultBlockMarginRight(0),
    _defaultBlockMarginBottom(0),
    _defaultBlockMarginTop(0)
  {

  }

   Document::~Document()
   {
     const auto &pages = _pages;
     for (Page* p : pages) {
	delete p;
      }
     //_pages.clear();

     delete _styleManager; //will delete all added styles
   }


  Document* Document::getSelection() //B:rename in "copySelection" ?
    {
        Page* p = currentPage();
        if (p == nullptr)
            return nullptr;

        Page* selection = p->getSelection();
        if (selection == nullptr)
            return nullptr;

        auto result = new Document();
        result->add(selection);

        return result;
    }

    void Document::add(Page* e)
    {
      if (_pages.contains(e))
	return;
      _currentPage = e;
      _pages.append(_currentPage);

      assert(_pages.contains(e));
    }

    void Document::remove(Page* e)
    {
      if (_currentPage == e)
	_currentPage = nullptr;

      const int ind = _pages.indexOf(e);
      if (ind != -1)
	_pages.removeAt(ind);

      assert(! _pages.contains(e));
    }

    void Document::removePages()
    {
      _currentPage = nullptr;
      _pages.clear();
    }
  
    void Document::setCurrentPage(Page* p)
    {
      _currentPage = p; //B: we can set a page that is not in _pages !!!
    }

    Page* Document::currentPage()
    {
      return _currentPage;
    }

    Block* Document::currentBlock()
    {
      Page* current = this->currentPage();
      if (current == nullptr)
	return nullptr;
      return current->currentBlock();
    }

    DocParagraph* Document::currentParagraph()
    {
        DocTextBlock* current = dynamic_cast<DocTextBlock*>(this->currentBlock());
        if (current == nullptr)
            return nullptr;

        return current->currentParagraph();
    }

    DocString* Document::currentString()
    {
        DocParagraph* current = this->currentParagraph();
        if (current == nullptr)
            return nullptr;

        return current->currentString();
    }

    DocCharacter* Document::currentCharacter()
    {
        DocString* current = this->currentString();
        if (current == nullptr)
            return nullptr;

        return current->currentCharacter();
    }

    QString Document::addStyle(DocStyle* newStyle)
    {
      assert(_styleManager != nullptr);

      _currentStyle = newStyle;
      return _styleManager->addReference(newStyle);
    }

    QString Document::changeStyle(const QString &olfStyleKey, DocStyle* newStyle)
    {
      assert(_styleManager != nullptr);

      _styleManager->removeReference(olfStyleKey);
      
      _currentStyle = newStyle;
      return _styleManager->addReference(newStyle);
    }

    DocStyle* Document::getStyle(const QString &style)
    {
      assert(_styleManager != nullptr);

      return _styleManager->getStyle(style);
    }

    DocStyle* Document::getStyle()
    {
        return _currentStyle;
    }

    QList<DocStyle*> Document::getStyles()
    {
      assert(_styleManager != nullptr);

      return _styleManager->getStyles();
    }

    void Document::add(Block* e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;

      _currentPage->add(e);
    }

    void Document::add(DocImageBlock* e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
      
      _currentPage->add(e);
    }

    void Document::add(DocTestBlock* e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
      
      _currentPage->add(e);
    }

    void Document::add(DocTextBlock* e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
      
      _currentPage->add(e);
    }

    void Document::add(DocComponentBlock* e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
      
      _currentPage->add(e);
    }

    void Document::add(DocParagraph* e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
      
      _currentPage->add(e);
    }

    void Document::add(const QList<DocParagraph*> &e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
	
      _currentPage->add(e);
    }

    void Document::add(DocString* e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
      
      _currentPage->add(e);
    }

    void Document::add(const QList<DocString*> &e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
      
      _currentPage->add(e);
    }

    void Document::add(DocCharacter* e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
      
      _currentPage->add(e);
    }

    void Document::add(const QList<DocCharacter*> &e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
      
      _currentPage->add(e);
    }

    void Document::add(DocComponent* e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
      
      _currentPage->add(e);
    }

    void Document::add(const QList<DocComponent*> &e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //return;
      
      _currentPage->add(e);
    }

    void Document::add(DocZone* e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
      
      _currentPage->add(e);
    }

    void Document::add(const QList<DocZone*> &e)
    {
      if (_currentPage == nullptr)
	add(new Page(this)); //B return;
      
      _currentPage->add(e);
    }

} //namespace Doc
