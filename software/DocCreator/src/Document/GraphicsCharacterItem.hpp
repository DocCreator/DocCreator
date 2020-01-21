#ifndef GRAPHICSCHARACTERITEM_H
#define GRAPHICSCHARACTERITEM_H

#include <QGraphicsPixmapItem>

namespace Models {
class Character;
}

class CharacterListWidgetItem;
class QGraphicsSceneMouseEvent;

class GraphicsCharacterItem
  : public QObject
  , public QGraphicsPixmapItem
{
  Q_OBJECT

public:
  GraphicsCharacterItem(const Models::Character *character,
                        int id,
                        int index,
                        bool mustDeleteCharacter = false);
  ~GraphicsCharacterItem();

  const Models::Character *getCharacter() { return _character; }

  void setIndex(int value) { _index = value; }
  int index() const { return _index; }

  void setHighlighted(bool highlighted = true);

  /* Character choice dialog */
  void openCharacterChoice(QPoint pos);

  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

public slots:
  void onCharacterItemClicked(CharacterListWidgetItem *charListItem);

protected:
  void buildHighlightedImage();
  QImage getHighlightedImage();

protected:
  //Attributes
  const Models::Character *_character;
  QImage _img;
  QImage _img_highlighted;

  int _cId;
  int _index;
  bool _mustDeleteCharacter;
};

#endif // GRAPHICSCHARACTERITEM_H
