#ifndef DOCUMENTCONTROLLER_H
#define DOCUMENTCONTROLLER_H

#include "DocRenderFlags.hpp"
#include "mvc/icontroller.h"
#include "patterns/observable.h"
#include "patterns/observer.h"
#include <QImage>
class DocumentView;
class QPrinter;
namespace Doc {
class Block;
class Document;
}

class DocumentController
  : public Mvc::IController
  , public Patterns::Observer
  , public Patterns::Observable
{
public:
  DocumentController();
  explicit DocumentController(Doc::Document *document);

  void setDocument(Doc::Document *document);

  inline virtual Doc::Document *getDocument() { return _document; }

  /* Getters */
  int getOffset() const;
  int getPageWidth() const;
  int getPageHeight() const;
  int getParagraphLineSpacing() const;
  int getBlockMarginTop() const;
  int getBlockMarginBottom() const;
  int getBlockMarginLeft() const;
  int getBlockMarginRight() const;
  bool baseLineVisibility() const { return _baseLineVisibility; }

  /* Setters */
  void setView(DocumentView *view);
  void setOffset(int value);
  void setPageWidth(int pageWidth);
  void setPageHeight(int pageHeight);
  void setParagraphLineSpacing(int lineSpacing);
  void setBlockMarginTop(int marginTop);
  void setBlockMarginBottom(int marginBottom);
  void setBlockMarginLeft(int marginLeft);
  void setBlockMarginRight(int marginRight);
  void setBlockGeometry(int x, int y, int width, int height);
  void setBaseLineVisibility(bool baseLineVisibility);

  void resetCurrentTextBlockCursor();

  /* Operations on document */
  void setCurrentBlock(Doc::Block *block);

  void addCharacter(const QString &s, int id = -1);
  void addCharacters(const QList<QString> &charList);
  void addParagraph();
  void addTextBlock();
  void addTextBlock(int x, int y, int w, int h);
  void addString(const QString &s);
  void addImageBlock(const QString &imagePath);
  void addTestBlock(const QString &imagePath, int x, int y, int w, int h);
  void addComponentBlock(const QString &imagePath);
  void addComponentBlock(const QImage &image);
  void removeAfterCursor();
  void removeBeforeCursor();
  void removeSelection();
  void removeCurrentBlock();

  void copy();
  void cut();
  void paste();

  void exportToImage(const QString &imagePath);
  QImage toQImage(DocRenderFlags flags);
  QSize getImageSize();
  void printDocument(QPrinter *printer);

  void randomize();

  /* Observer method */
  void update() override;

  void
  setModified(); // Notify the application that a modification has taken effect

private:
  /* Initializer */
  void initialize(DocumentView *view, Doc::Document *document);

private:
  DocumentView *_view;
  Doc::Document *_document;
  bool _baseLineVisibility;

  qreal _cursorBaseLine;
  qreal _cursorRightLine;
};

#endif /* DOCUMENTCONTROLLER_H */
