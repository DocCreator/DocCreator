#ifndef PAGELAYOUT_HPP
#define PAGELAYOUT_HPP

#include <QObject>
#include <QRect>

#include "models/doc/doctextblock.h"
#include "models/doc/document.h"


class PageLayout : public QObject
{
  Q_OBJECT

public:
  explicit PageLayout(Doc::Document *doc,
		      QObject *parent = nullptr,
		      int leftMargin = 0, int rightMargin = 0,
		      int topMargin = 0, int bottomMargin = 0)
    : QObject(parent)
    , _height(doc->pageHeight())
    , _width(doc->pageWidth())
    , _page(QPoint(0, 0), QPoint(_width, _height))
    , _leftMargin(leftMargin)
    , _rightMargin(rightMargin)
    , _topMargin(topMargin)
    , _bottomMargin(bottomMargin)
    , _doc(doc)
  {
  }

  virtual Doc::DocTextBlock *takeTextBlock(int blockNumber) = 0;

  virtual inline void setRightMargin(int pRightMargin)
  {
    _rightMargin = pRightMargin;
    _page.setWidth(_width - _rightMargin - _leftMargin);
  }

  virtual inline void setLeftMargin(int pLeftMargin)
  {
    _leftMargin = pLeftMargin;
    _page.setX(_leftMargin);
    _page.setWidth(_width - _rightMargin - _leftMargin);
  }

  virtual inline void setBottomMargin(int pBottomMargin)
  {
    _bottomMargin = pBottomMargin;
    _page.setHeight(_height - _bottomMargin - _topMargin);
  }

  virtual inline void setTopMargin(int pTopMargin)
  {
    _topMargin = pTopMargin;
    _page.setY(_topMargin);
    _page.setHeight(_height - _bottomMargin - _topMargin);
  }

protected:

  virtual Doc::DocTextBlock *creatTextBlock(int x, int y, int w, int h) const
  {
    Doc::DocTextBlock *textBlock = new Doc::DocTextBlock(_doc);

    textBlock->setX(x);
    textBlock->setY(y);
    textBlock->setMarginTop(0);
    textBlock->setMarginBottom(0);
    textBlock->setMarginLeft(0);
    textBlock->setMarginRight(0);
    textBlock->setHeight(h);
    textBlock->setWidth(w);

    return textBlock;
  }

protected:

  int _height;
  int _width;
  QRect _page;
  int _leftMargin;
  int _rightMargin;
  int _topMargin;
  int _bottomMargin;
  Doc::Document *_doc;
};

#endif // PAGELAYOUT_HPP
