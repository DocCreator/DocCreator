#include <catch2/catch_test_macros.hpp>

#include <QImage>

#include "models/character.h"
#include "models/characterdata.h"

namespace {

  /*
    @return a character holding one image per id of @a ids, each filled with
    a gray level that identifies it.
  */
  Models::Character *
  makeCharacter(const QString &value, const std::vector<int> &ids)
  {
    auto *ch = new Models::Character(value, 0, 100, 0, 100);
    for (int id : ids) {
      QImage img(4, 4, QImage::Format_ARGB32);
      img.fill(qRgb(id, id, id));
      ch->add(new Models::CharacterData(img, id));
    }
    return ch;
  }

  int grayOf(const Models::CharacterData &data)
  {
    return qRed(data.getImage().pixel(0, 0));
  }

} //namespace

TEST_CASE("Character::hasCharacterData()")
{
  Models::Character empty;
  REQUIRE_FALSE( empty.hasCharacterData() );
  REQUIRE( empty.getCharacterDataCount() == 0 );
  REQUIRE( empty.getRandomCharacterData() == nullptr );

  Models::Character *ch = makeCharacter(QStringLiteral("a"), { 7 });
  REQUIRE( ch->hasCharacterData() );
  REQUIRE( ch->getCharacterDataCount() == 1 );
  delete ch;
}

TEST_CASE("Character::getCharacterData() tells a missing id from a held one")
{
  //an id is not an index: these are the ids a font built from image files
  //or from the OCR dialog gives
  Models::Character *ch = makeCharacter(QStringLiteral("a"), { 3, 12, 40 });

  REQUIRE( ch->getCharacterData(3) != nullptr );
  REQUIRE( ch->getCharacterData(12) != nullptr );
  REQUIRE( ch->getCharacterData(40) != nullptr );

  //not an index, and not present
  REQUIRE( ch->getCharacterData(0) == nullptr );
  REQUIRE( ch->getCharacterData(1) == nullptr );
  REQUIRE( ch->getCharacterData(2) == nullptr );
  REQUIRE( ch->getCharacterData(-1) == nullptr );
  REQUIRE( ch->getCharacterData(41) == nullptr );

  delete ch;
}

TEST_CASE("Character::getCharacterDataOrFirst() replaces a missing id")
{
  /*
    A document stores the id of the version of the character it was written
    with. A font edited since may not hold that version anymore, and the
    call sites that draw the document used to dereference the null pointer
    getCharacterData() returned for it.
  */
  Models::Character *ch = makeCharacter(QStringLiteral("a"), { 3, 12, 40 });

  SECTION("an id the character holds is returned")
  {
    REQUIRE( grayOf(ch->getCharacterDataOrFirst(3)) == 3 );
    REQUIRE( grayOf(ch->getCharacterDataOrFirst(12)) == 12 );
    REQUIRE( grayOf(ch->getCharacterDataOrFirst(40)) == 40 );
  }

  SECTION("a missing id gives the first version")
  {
    REQUIRE( grayOf(ch->getCharacterDataOrFirst(0)) == 3 );
    REQUIRE( grayOf(ch->getCharacterDataOrFirst(41)) == 3 );
    REQUIRE( grayOf(ch->getCharacterDataOrFirst(-1)) == 3 );
  }

  SECTION("the replacement is the same on every call")
  {
    //unlike getRandomCharacterData(): a document must be drawn the same way
    //every time it is opened
    const Models::CharacterData &first = ch->getCharacterDataOrFirst(999);
    for (int i = 0; i < 20; ++i) {
      REQUIRE( &ch->getCharacterDataOrFirst(999) == &first );
    }
  }

  SECTION("it never returns the address of a version that was not asked for"
	  " when the id is held")
  {
    REQUIRE( &ch->getCharacterDataOrFirst(40)
	     == ch->getCharacterData(40) );
  }

  delete ch;
}

TEST_CASE("Character::getCharacterDataOrFirst() on a character with one version")
{
  Models::Character *ch = makeCharacter(QStringLiteral("b"), { 5 });

  REQUIRE( grayOf(ch->getCharacterDataOrFirst(5)) == 5 );
  REQUIRE( grayOf(ch->getCharacterDataOrFirst(6)) == 5 );

  delete ch;
}
