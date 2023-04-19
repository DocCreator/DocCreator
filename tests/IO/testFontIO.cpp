#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "models/font.h"
#include "models/character.h"
#include "models/characterdata.h"
#include "iomanager/fontfilemanager.h"
#include "Degradations/FileUtils.hpp"

#include "paths.hpp"

static
bool
checkEqual(const Models::Font *font1, const Models::Font *font2)
{
  if (font1 == font2) {
    return true;
  }
  if (font1 == nullptr || font2 == nullptr) {
    return false;
  }  
 
  if (font1->getName() != font2->getName()) {
    return false;
  }

  const Models::CharacterMap &characters1 = font1->getCharacters();
  const Models::CharacterMap &characters2 = font2->getCharacters();
  const size_t numCharacters = characters1.size();
  if (numCharacters != static_cast<size_t>(characters2.size())) {
    return false;
  }

  Models::CharacterMap::const_iterator i1 = characters1.constBegin();
  Models::CharacterMap::const_iterator i2 = characters2.constBegin();
  for ( ; i1 != characters1.constEnd() && i2 != characters2.constEnd() ; ++i1, ++i2) {

    assert(i1.key() == i2.key());
    const Models::Character *c1 = i1.value();
    const Models::Character *c2 = i2.value();

    if (c1->getCharacterValue() != c2->getCharacterValue()) {
      return false;
    }

    if (c1->getBaseLine() != c2->getBaseLine()) {
      return false;
    }
    if (c1->getUpLine() != c2->getUpLine()) {
      return false;
    }
    if (c1->getLeftLine() != c2->getLeftLine()) {
      return false;
    }
    if (c1->getRightLine() != c2->getRightLine()) {
      return false;
    }

    const Models::CharacterDataList &cl1 = c1->getAllCharacterData();
    const Models::CharacterDataList &cl2 = c2->getAllCharacterData();
    const size_t numVersions = cl1.size();
    if (numVersions != static_cast<size_t>(cl2.size())) {
      return false;
    }
    for (size_t j=0; j<numVersions; ++j) {
      const Models::CharacterData *cd1 = cl1[j];
      const Models::CharacterData *cd2 = cl2[j];
      if (cd1->getId() != cd2->getId()) {
	return false;
      }
      if (cd1->width() != cd2->width()) {
	return false;
      }
      if (cd1->height() != cd2->height()) {
	return false;
      }
      //TODO: check pixel by pixel !!!
      
    }
        
  }

  return true;
}

static
void
testSimple()
{
  //Check equality between a xml font and binary font 

  const char *fonts[] = {"JulesVerne.bof", "manto.bof", "montaigne.bof"};

  const QString tmpFilename1 = QString::fromStdString("tmp_font.of");
  const QString tmpFilename2 = QString::fromStdString("tmp_font.bof");
  
  const size_t numFonts = sizeof(fonts)/sizeof(const char *);

  for (size_t i=0; i<numFonts; ++i) {

    const std::string fontPath = dc::makePath(FONT_DATA_SRC_PATH, fonts[i]);
    Models::Font *font = IOManager::FontFileManager::fontFromBinary(QString::fromStdString(fontPath));
    REQUIRE( font != nullptr );

    const bool writeOk1 = IOManager::FontFileManager::fontToXml(font, tmpFilename1);
    REQUIRE( writeOk1 );

    Models::Font *font2 = IOManager::FontFileManager::fontFromXml(tmpFilename1);
    REQUIRE( font2 != nullptr );
    REQUIRE( checkEqual(font, font2) ); //check XML I/O

    const bool writeOk2 = IOManager::FontFileManager::fontToBinary(font, tmpFilename2);
    REQUIRE( writeOk2 );

    Models::Font *font3 = IOManager::FontFileManager::fontFromXml(tmpFilename1);
    REQUIRE( font3 != nullptr );
    REQUIRE( checkEqual(font, font3) ); //check binary I/O
    
  }
  
}


TEST_CASE( "Testing IO" )
{ 

  SECTION("Testing font IO")
  {
    testSimple();
  }
  
}
