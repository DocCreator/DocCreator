#ifndef FONT_H
#define FONT_H

#include "framework_global.h"
#include <QMap>
#include <QString>

namespace Models {
class Character;
class CharacterData;

using CharacterMap = QMap<QString, Character *>;

class FRAMEWORK_EXPORT Font
{
public:
  Font();
  explicit Font(const QString &name);

  ~Font();


  /**
     Check if a Character @a c is already present in font.
  */
  bool contains(Character *c) const;

  /**
     Add a Character @a c if pointer not already present in font.
     If letter of @a c is already present in font, corresponding Character
     will first be deleted and then @a c added.
     Return true if character @a c was added to font, false otherwise.

     @warning take ownership of Character @a c.
  */
  bool addCharacter(Character *c);

  /**
     Add Character corresponding to @a charValue to font if it does not exist.
     Add CharacterData @a cd to Character corresponding to @a charValue.

     @warning Character corresponding to @a charValue takes ownership of @a cd.
   */
  void addCharacter(const QString &charValue, CharacterData *cd);

  /**
     Get Character corresponding to @a charValue if it exists in font.
     Return nullptr otherwise.
   */
  const Character *getCharacter(const QString &charValue) const;

  /**
     Get Character corresponding to @a charValue if it exists in font.
     Return nullptr otherwise.

     @warning font keeps ownership of the Character.
   */
  Character *getEditableCharacter(const QString &charValue) const;


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
