#ifndef FONT_H
#define FONT_H

#include "framework_global.h"
#include <QMap>
#include <QString>

namespace Models {
class Character;

using CharacterMap = QMap<QString, Character *>;

class FRAMEWORK_EXPORT Font
{
public:
  Font();
  explicit Font(const QString &name);

  /**
     Check if a Character @a c is already present in font.
  */
  bool contains(Character *c) const;

  /**
     Add a Character @a c if not already present in font.
     Return true if character was added to font, false otherwise.
  */
  bool addCharacter(Character *c);

  Character *getCharacter(const QString &charValue) const;

  /* Setters */
  void setName(const QString &name);

  /* Getters */
  QString getName() const;
  const CharacterMap &getCharacters() const;

private:
  void initialize(const QString &name);

private:
  QString _name;
  CharacterMap _characterMap;
};
} // Models

#endif // FONT_H
