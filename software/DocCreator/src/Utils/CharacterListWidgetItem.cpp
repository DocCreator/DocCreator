#include "CharacterListWidgetItem.hpp"

#include <QIcon>
#include <QPixmap>
#include <models/characterdata.h>

CharacterListWidgetItem::CharacterListWidgetItem(
  Models::CharacterData *charData)
  : QListWidgetItem()
{
  _charData = charData;
  setIcon(QIcon(QPixmap::fromImage(charData->getImage())));
}
