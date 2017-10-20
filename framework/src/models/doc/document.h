#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QString>
#include <QList>
#include <framework_global.h>


namespace Doc
{
  class Block;
  class DocCharacter;
  class DocComponent;
  class DocComponentBlock;
  class DocImageBlock;
  class DocParagraph;
  class DocString;
  class DocStyle;
  class DocStyleManager;
  class DocTextBlock;
  class DocTestBlock;
  class DocZone;
  class Page;
  
  class FRAMEWORK_EXPORT Document
    {
    public:
        Document();

	~Document();

        Document* getSelection();

	/**
	 * Add page @a e to document if not already present.
	 *
	 * Set @a e as current page.
	 */
        void add(Page* e);

	/**
	 * Remove page @a e from document.
	 * 
	 * It is client responsibility to delete the page.
	 */
        void remove(Page* e);

	/**
	 * Remove all pages from document.
	 * 
	 * It is client responsibility to delete the pages.
	 */
        void removePages();
	
        void setCurrentPage(Page* p);
        Page* currentPage();
        QList<Page*> getPages() { return _pages; }

        Block* currentBlock();
        DocParagraph* currentParagraph();
        DocString* currentString();
        DocCharacter* currentCharacter();

	//B:TODO:API: remove "get" in methods names : getStyle(), getStyles(), getPages();
	// because we have pageHeight() and not getPageHeight()

	//B:TODO:API:  Why getStyle() & getStyles() ??? We can have several styles ?
	//B:TODO:API: changeStyle() weird ?!
	//B:TODO:API: we cannot remove a style !?
        QString addStyle(DocStyle* newStyle);
        QString changeStyle(const QString &olfStyleKey, DocStyle* newStyle);
        DocStyle* getStyle(const QString &style);
        DocStyle* getStyle();
        QList<DocStyle*> getStyles();


	//B:TODO:API: we can add *Blocks, but we cannot remove them !!!
	//B:TODO:API: we cannot access the list of blocks ! 

	//B:TODO:API: does the document take ownership of blocks ??? If not, who ?

	/**
	 * @warning block is not added if there is no page.
	 */

        void add(Block* e);
        void add(DocImageBlock* e);        
        void add(DocTestBlock* e);
        void add(DocTextBlock* e);
        void add(DocParagraph* e);
        void add(const QList<DocParagraph*> &e);
        void add(DocString* e);
        void add(const QList<DocString*> &e);
        void add(DocCharacter* e);
        void add(const QList<DocCharacter*> &e);
        // added by kvcuong 09/05/2012
        void add(DocComponentBlock* e);

        void add(DocComponent* e);
        void add(const QList<DocComponent*> &e);
        void add(DocZone* e);
        void add(const QList<DocZone*> &e);

	void setPageWidth(int pageWidth){ _pageWidth = pageWidth; }
	int pageWidth() const { return _pageWidth; }
	void setPageHeight(int pageHeight){ _pageHeight = pageHeight; }
	int pageHeight() const { return _pageHeight; }

        void setPageMarginLeft(int value) { _pagesMarginLeft = value; }
        int pageMarginLeft() { return _pagesMarginLeft; }
        void setPageMarginRight(int value) { _pagesMarginRight = value; }
        int pageMarginRight() const { return _pagesMarginRight; }
        void setPageMarginTop(int value) { _pagesMarginTop = value; }
        int pageMarginTop() const { return _pagesMarginTop; }
        void setPageMarginBottom(int value) { _pagesMarginBottom = value; }
        int pageMarginBottom() const { return _pagesMarginBottom; }

    private:
	Document(const Document &);
	Document &operator=(const Document &);

    private:
        QList<Page*> _pages;
        Page* _currentPage;

        DocStyle* _currentStyle;
        DocStyleManager* _styleManager;

        int _pagesMarginLeft;
        int _pagesMarginRight;
        int _pagesMarginTop;
        int _pagesMarginBottom;

	int _pageWidth;
	int _pageHeight;

        int _defaultBlockMarginLeft;
        int _defaultBlockMarginRight;
        int _defaultBlockMarginBottom;
        int _defaultBlockMarginTop;
    };
}

#endif // DOCUMENT_H
