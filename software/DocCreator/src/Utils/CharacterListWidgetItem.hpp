#ifndef CHARACTERLISTWIDGETITEM_H
#define CHARACTERLISTWIDGETITEM_H

#include <QListWidgetItem>
namespace Models {
class CharacterData;
}

class CharacterListWidgetItem : public QListWidgetItem
{

public:
  explicit CharacterListWidgetItem(Models::CharacterData *charData);

  Models::CharacterData *getCharacterData() { return _charData; }

private:
  Models::CharacterData *_charData;
};

#endif // CHARACTERLISTWIDGETITEM_H
