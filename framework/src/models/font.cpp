#include "font.h"

#include "models/character.h"

#include <iostream>

namespace Models {
Font::Font()
{
  initialize(QStringLiteral("no_name"));
}

Font::Font(const QString &name)
{
  initialize(name);
}

Font::~Font()
{
  CharacterMap::iterator it = _characterMap.begin();
  const CharacterMap::iterator itEnd = _characterMap.end();
  for (; it != itEnd; ++it) {
    delete it.value();
  }
  _characterMap.clear();
}


bool
Font::contains(Character *c) const
{
  auto it = _characterMap.begin();
  const auto itEnd = _characterMap.end();
  for (; it != itEnd; ++it) {
    if (it.value() == c) {
      return true;
    }
  }
  return false;
}

bool
Font::addCharacter(Character *c)
{
  if (contains(c)) {
    return false;
  }

  auto it = _characterMap.find(c->getCharacterValue());
  if (it != _characterMap.end())
      delete it.value();

  _characterMap.insert(c->getCharacterValue(), c);

  return true;
}

const Character *
Font::getCharacter(const QString &charValue) const
{
  auto it = _characterMap.find(charValue);
  if (it != _characterMap.end())
    return it.value();

  return nullptr;
}

Character *
Font::getEditableCharacter(const QString &charValue) const
{
  auto it = _characterMap.find(charValue);
  if (it != _characterMap.end())
    return it.value();

  return nullptr;
}

void
Font::addCharacter(const QString &charValue, CharacterData *cd)
{
  auto it = _characterMap.find(charValue);
  if (it == _characterMap.end()) {
    it =_characterMap.insert(charValue, new Models::Character(charValue));
  }
  it.value()->add(cd);
}

/* Setters */
void
Font::setName(const QString &name)
{
  _name = name;
}

/* Getters */
QString
Font::getName() const
{
  return _name;
}

const CharacterMap &
Font::getCharacters() const
{
  return _characterMap;
}

/* Private methods */
void
Font::initialize(const QString &name)
{
  _name = name;
}
} //namespace Models
