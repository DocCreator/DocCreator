#ifndef GRAPHICSTEXTBLOCKITEM_H
#define GRAPHICSTEXTBLOCKITEM_H

#include "ADocumentView.hpp"
#include "GraphicsBlockItem.hpp"

namespace Doc {
class DocCharacter;
class DocParagraph;
class DocString;
class DocStyle;
class DocTextBlock;
}
namespace Models {
class Character;
}

class GraphicsCharacterItem;
class GraphicsCursorItem;

class GraphicsTextBlockItem
  : public GraphicsBlockItem
  , public ADocumentView<Doc::DocTextBlock>
{
  Q_OBJECT
public:
  explicit GraphicsTextBlockItem(Mvc::IController *controller,
                                 QGraphicsItem *parent = nullptr);

  void clear() override;
  void setOffset(int value) override;

  void hide(bool transparent = false) override;
  void show() override;

  //Events
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  void keyPressEvent(QKeyEvent * /*event*/) override {}
  void keyReleaseEvent(QKeyEvent * /*event*/) override {}

public slots:
  void updatePosition() override;

protected:
  void load() override;
  void draw(bool complete) override;

  void addCharacterItem(GraphicsCharacterItem *charItem);
  void removeCharacterItem(GraphicsCharacterItem *charItem);
  void removeAllCharacterItem();
  void setCursorOnCharacter(GraphicsCharacterItem *charItem, bool cursorBefore);
  bool clickOnFirstPart(GraphicsCharacterItem *charItem, QPointF pos);
  void setCursorFromPosition(QPointF pos);

  qreal findUpPosition(const Models::Character *character,
                       int characterImageWidth,
                       int characterImageHeight) const;
  qreal findBasePosition(const Models::Character *character,
                         int characterImageWidth,
                         int characterImageHeight) const;
  qreal findLeftPosition(const Models::Character *character,
                         int characterImageWidth,
                         int characterImageHeight) const;
  qreal findRightPosition(const Models::Character *character,
                          int characterImageWidth,
                          int characterImageHeight) const;

  void goNextLine(Doc::DocParagraph *p = nullptr);

  void drawParagraph(Doc::DocParagraph *p, int offset);
  void drawString(Doc::DocString *s,
                  int offset,
                  Doc::DocParagraph *p = nullptr);
  void drawCharacter(Doc::DocCharacter *c,
                     int offset,
                     Doc::DocStyle *style,
                     Doc::DocParagraph *p = nullptr);
  void hightlightSelection(int startOffset, int endOffset);

protected:
  qreal _cursorBaseLine;
  qreal _cursorRightLine;

  QList<GraphicsCharacterItem *> _charItems;
  GraphicsCursorItem *_cursorItem;

  bool _mousePressed;
  int _startSelectionOffset;
};

#endif // GRAPHICSTEXTBLOCKITEM_H
