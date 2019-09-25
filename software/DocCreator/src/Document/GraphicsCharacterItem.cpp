#include "GraphicsCharacterItem.hpp"

#include <cassert>

#include "Utils/CharacterListView.hpp"
#include "Utils/CharacterListWidgetItem.hpp"
#include "models/character.h"
#include "models/characterdata.h"
#include <QGraphicsSceneMouseEvent>

//B:TODO:OPTIM? is it necessary to store _img_highlighted ?
// Maybe we could compute it on the fly ?
// (Here we compute it only when it is required/accessed the first time.
//  It should be beneficial in batch generation mode, where highlighted image is
//  never accessed)
//

//B:DESIGN: I added "mustDeleteCharacter" parameter to be able to delete characters that are not associated with a character present in the font
// cf graphicstextblockitem.cpp, it is used for "\n" character.

GraphicsCharacterItem::GraphicsCharacterItem(Models::Character *character,
                                             int id,
                                             int index,
                                             bool mustDeleteCharacter)
  : QGraphicsPixmapItem()
  , _character(character)
  , _img()
  , _img_highlighted()
  , _cId(id)
  , _index(index)
  , _mustDeleteCharacter(mustDeleteCharacter)
{
  assert(_character);
  const Models::CharacterData *charData = _character->getCharacterData(_cId);
  assert(charData);
  _img = charData->getImage();
  //buildHighlightedImage(); //we don't do it here. We do it on demand.

  setPixmap(QPixmap::fromImage(_img));

  setCursor(Qt::IBeamCursor);
  setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
}

GraphicsCharacterItem::~GraphicsCharacterItem()
{
  if (_mustDeleteCharacter) {
    delete _character;
  }
}

void
GraphicsCharacterItem::buildHighlightedImage()
{
  _img_highlighted = QImage(_img);
  _img_highlighted.invertPixels(QImage::InvertRgba);
}

QImage
GraphicsCharacterItem::getHighlightedImage()
{
  if (_img_highlighted.isNull()) {
    buildHighlightedImage();
  }
  return _img_highlighted;
}

void
GraphicsCharacterItem::setHighlighted(bool highlighted)
{
  if (highlighted)
    setPixmap(QPixmap::fromImage(getHighlightedImage()));
  else
    setPixmap(QPixmap::fromImage(_img));
}

/* Character choice */
void
GraphicsCharacterItem::openCharacterChoice(QPoint pos)
{
  auto characterList = new CharacterListView(_character, pos, nullptr);

  connect(characterList,
          SIGNAL(characterItemClicked(CharacterListWidgetItem*)),
          this,
          SLOT(onCharacterItemClicked(CharacterListWidgetItem*)));

  characterList->show();
}

void
GraphicsCharacterItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  /*if(event->button() == Qt::RightButton)
    openCharacterChoice(event->scenePos().toPoint());
    else*/
  QGraphicsPixmapItem::mousePressEvent(event);
}

//Slots
void
GraphicsCharacterItem::onCharacterItemClicked(
  CharacterListWidgetItem * /*charListItem*/)
{
  //Models::CharacterData *charData = charListItem->getCharacterData();

  //TODO: change char id
}
