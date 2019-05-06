#include "CharacterListView.hpp"

#include "CharacterListWidgetItem.hpp"
#include "models/character.h"
#include "models/characterdata.h"
#include <QImage>

CharacterListView::CharacterListView(Models::Character *character,
                                     QPoint pos,
                                     QWidget *parent)
  : QListWidget(parent)
{
  setFlow(QListView::LeftToRight);

  _character = character;

  Models::CharacterDataList charDatas = _character->getAllCharacterData();

  int width = 0;
  int height = 0;

  for (Models::CharacterData *charData : charDatas) {
    addItem(new CharacterListWidgetItem(charData));

    width = width + charData->getImage().width();

    const int charImgHeight = charData->getImage().height();
    if (height < charImgHeight)
      height = charImgHeight;
  }

  const int minHeight = 60;
  height = std::max(minHeight, height);

  setGeometry(pos.x(), pos.y(), width, height);

  connect(this,
          SIGNAL(itemClicked(QListWidgetItem*)),
          this,
          SLOT(onItemClicked(QListWidgetItem*)));
}

//Slots
void
CharacterListView::onItemClicked(QListWidgetItem *item)
{
  CharacterListWidgetItem *charItem =
    static_cast<CharacterListWidgetItem *>(item);
  emit characterItemClicked(charItem);
  this->close();
}
