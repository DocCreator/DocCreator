#include "font.h"

#include "models/character.h"

namespace Models {
Font::Font()
{
  initialize(QStringLiteral("no_name"));
}

Font::Font(const QString &name)
{
  initialize(name);
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
  if (contains(c))
    return false;

  _characterMap.insert(c->getCharacterValue(), c);

  return true;
}

Character *
Font::getCharacter(const QString &charValue) const
{
  auto it = _characterMap.find(charValue);
  if (it != _characterMap.end())
    return it.value();

  return nullptr;
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
