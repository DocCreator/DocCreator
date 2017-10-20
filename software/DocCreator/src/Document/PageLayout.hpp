#ifndef PAGELAYOUT_HPP
#define PAGELAYOUT_HPP

#include <QObject>
#include <QRect>

#include "models/doc/doctextblock.h"
#include "models/doc/document.h"

//#include <QVector>

//template<class T>
//class Vector2D : public QVector< QVector<T> >
//{
//    Vector2D():
//      QVector< QVector<T> >()
//    {
//    }
//    Vector2D(int rows, int columns) :
//            QVector< QVector<T> >(rows)
//    {
//        for(int i = 0; i < size(); i++)
//            this[i].resize(columns);
//    }
//    virtual ~Vector2D() {}
//};

class PageLayout : public QObject
{
  Q_OBJECT

public:
  explicit PageLayout(Doc::Document *doc, QObject *parent = nullptr)
    : QObject(parent)
    , _height(doc->pageHeight())
    , _width(doc->pageWidth())
    , page(QPoint(0, 0), QPoint(_width, _height))
    , leftMargin(0)
    , rightMargin(0)
    , topMargin(0)
    , bottomMargin(0)
    , _doc(doc)
  {}

  virtual Doc::DocTextBlock *newTextBlock(const int blockNumber = -1) = 0;

  virtual inline void setRightMargin(const int pRightMargin)
  {
    rightMargin = pRightMargin;
    page.setWidth(_width - rightMargin);
  }

  virtual inline void setLeftMargin(const int pLeftMargin)
  {
    leftMargin = pLeftMargin;
    page.setX(leftMargin);
  }

  virtual inline void setBottomMargin(const int pBottomMargin)
  {
    bottomMargin = pBottomMargin;
    page.setHeight(_height - bottomMargin);
  }

  virtual inline void setTopMargin(const int pTopMargin)
  {
    topMargin = pTopMargin;
    page.setY(topMargin);
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
  QRect page;
  int leftMargin;
  int rightMargin;
  int topMargin;
  int bottomMargin;
  Doc::Document *_doc;
};

#endif // PAGELAYOUT_HPP
