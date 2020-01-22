#ifndef CHARACTERLISTVIEW_H
#define CHARACTERLISTVIEW_H

#include <QListWidget>

namespace Models {
class Character;
}
class CharacterListWidgetItem;

class CharacterListView : public QListWidget
{
  Q_OBJECT

public:
  CharacterListView(const Models::Character *character,
                    QPoint pos,
                    QWidget *parent = nullptr);

public slots:
  void onItemClicked(QListWidgetItem *item);

signals:
  void characterItemClicked(CharacterListWidgetItem *charItem);

private:
  const Models::Character *_character;
};

#endif // CHOICECHARACTERLISTVIEW_H
