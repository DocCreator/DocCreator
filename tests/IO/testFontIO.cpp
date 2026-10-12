#include <functional>
#include <iostream>

#include <QDataStream>
#include <QFile>
#include <QImage>

#include <catch2/catch_test_macros.hpp>

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


namespace {

  const uint32_t BOF_HEADER = ('B' << 24 | 'O' << 16 | 'F' << 8);

  /*
    Writes a .bof file whose header is valid and whose content is given by
    @a fill, and returns its path.
  */
  QString
  writeBOF(const QString &filename,
	   const std::function<void (QDataStream &)> &fill)
  {
    QFile file(filename);
    REQUIRE( file.open(QFile::WriteOnly | QFile::Truncate) );
    QDataStream out(&file);
    out << BOF_HEADER;
    out.setVersion(QDataStream::Qt_4_0);
    fill(out);
    file.close();
    return filename;
  }

} //namespace

static
void
testCorrupt()
{
  /*
    A count read from a font file used to be trusted: a 70-byte file
    announcing 0xFFFFFFFF versions made the reader allocate until the
    process was killed (it did not complete in 20s under a 2GB limit).
  */

  SECTION("a file that is not a font")
  {
    const QString filename = QStringLiteral("tmp_corrupt_notafont.bof");
    QFile file(filename);
    REQUIRE( file.open(QFile::WriteOnly | QFile::Truncate) );
    file.write("this is not a font at all", 25);
    file.close();

    REQUIRE( ! IOManager::FontFileManager::isBOFFile(filename) );
    REQUIRE( IOManager::FontFileManager::fontFromBinary(filename) == nullptr );
  }

  SECTION("a file that holds nothing but a header")
  {
    const QString filename = writeBOF(QStringLiteral("tmp_corrupt_header.bof"),
				      [](QDataStream &) {});
    REQUIRE( IOManager::FontFileManager::isBOFFile(filename) );
    REQUIRE( IOManager::FontFileManager::fontFromBinary(filename) == nullptr );
  }

  SECTION("a huge number of characters announced")
  {
    const QString filename =
      writeBOF(QStringLiteral("tmp_corrupt_numcharacters.bof"),
	       [](QDataStream &out) {
		 out << QStringLiteral("corrupt");
		 out << uint32_t(0xFFFFFFFF);
	       });
    REQUIRE( IOManager::FontFileManager::fontFromBinary(filename) == nullptr );
  }

  SECTION("a huge number of versions announced for a letter")
  {
    const QString filename =
      writeBOF(QStringLiteral("tmp_corrupt_numversions.bof"),
	       [](QDataStream &out) {
		 out << QStringLiteral("corrupt");
		 out << uint32_t(1);            //one character
		 out << QStringLiteral("a");    //its value
		 out << qreal(10) << qreal(20) << qreal(0) << qreal(10);
		 out << uint32_t(0xFFFFFFFF);   //its number of versions
	       });
    REQUIRE( IOManager::FontFileManager::fontFromBinary(filename) == nullptr );
  }

  SECTION("a file truncated in the middle of a letter")
  {
    const QString filename =
      writeBOF(QStringLiteral("tmp_corrupt_truncated.bof"),
	       [](QDataStream &out) {
		 out << QStringLiteral("corrupt");
		 out << uint32_t(4);            //four characters announced
		 out << QStringLiteral("a");
		 out << qreal(10) << qreal(20); //and then nothing
	       });
    REQUIRE( IOManager::FontFileManager::fontFromBinary(filename) == nullptr );
  }

  SECTION("the same letter twice")
  {
    //both letters are read, the second one is rejected and deleted
    const QString filename =
      writeBOF(QStringLiteral("tmp_corrupt_duplicate.bof"),
	       [](QDataStream &out) {
		 for (int i = 0; i < 2; ++i) {
		   if (i == 0) {
		     out << QStringLiteral("duplicate");
		     out << uint32_t(2);        //two characters
		   }
		   out << QStringLiteral("a");
		   out << qreal(10) << qreal(20) << qreal(0) << qreal(10);
		   out << uint32_t(1);          //one version
		   out << int(0);               //its id
		   out << QImage(4, 4, QImage::Format_ARGB32);
		 }
	       });
    Models::Font *font = IOManager::FontFileManager::fontFromBinary(filename);
    REQUIRE( font != nullptr );
    REQUIRE( font->getCharacters().size() == 1 );
    delete font;
  }
}

TEST_CASE( "Testing IO" )
{ 

  SECTION("Testing font IO")
  {
    testSimple();
  }
  
}

TEST_CASE( "Testing a corrupt binary font file" )
{
  testCorrupt();
}

/*
  @return the path of a .of file holding one letter whose image announces
  @a width, @a height and @a format, and holds @a numValues pixel values.
*/
static
QString
writeOF(const QString &filename, int width, int height, int format, int numValues)
{
  QString data;
  for (int i = 0; i < numValues; ++i) {
    if (i != 0) {
      data += QLatin1String(",");
    }
    data += QString::number(0xff804020u);
  }

  const QString xml =
    QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		   "<font name=\"corrupt\">\n"
		   "  <letter char=\"a\">\n"
		   "    <anchor><upLine>0</upLine><baseLine>100</baseLine>"
		   "<leftLine>0</leftLine><rightLine>100</rightLine></anchor>\n"
		   "    <picture id=\"0\">\n"
		   "      <imageData>\n"
		   "        <width>%1</width>\n"
		   "        <height>%2</height>\n"
		   "        <format>%3</format>\n"
		   "        <data>%4</data>\n"
		   "      </imageData>\n"
		   "    </picture>\n"
		   "  </letter>\n"
		   "</font>\n")
      .arg(width).arg(height).arg(format).arg(data);

  QFile file(filename);
  REQUIRE( file.open(QFile::WriteOnly | QFile::Truncate) );
  file.write(xml.toUtf8());
  file.close();

  return filename;
}

TEST_CASE( "Testing a corrupt XML font file" )
{
  /*
    Everything the reader took from the file used to be used as it was. A
    file announcing more pixels than it holds aborted the process with a
    std::bad_alloc, and so did one whose width times height overflowed an
    int.
  */

  SECTION("a well formed file, as a control")
  {
    const QString filename =
      writeOF(QStringLiteral("tmp_corrupt_ok.of"), 4, 4, QImage::Format_ARGB32, 16);
    Models::Font *font = IOManager::FontFileManager::fontFromXml(filename);
    REQUIRE( font != nullptr );
    REQUIRE( font->getCharacters().size() == 1 );

    const Models::CharacterDataList &versions =
      font->getCharacters().constBegin().value()->getAllCharacterData();
    REQUIRE( versions.size() == 1 );
    const QImage img = versions.at(0)->getImage();
    REQUIRE( img.width() == 4 );
    REQUIRE( img.height() == 4 );
    REQUIRE( img.format() == QImage::Format_ARGB32 );
    delete font;
  }

  SECTION("fewer pixel values than the announced size")
  {
    //on the QImage::Format_ARGB32 path, which writes through scanLine()
    const QString filename =
      writeOF(QStringLiteral("tmp_corrupt_short.of"), 4, 4, QImage::Format_ARGB32, 3);
    REQUIRE( IOManager::FontFileManager::fontFromXml(filename) == nullptr );
  }

  SECTION("fewer pixel values than the announced size, on the setPixel path")
  {
    const QString filename =
      writeOF(QStringLiteral("tmp_corrupt_short2.of"), 4, 4,
	      QImage::Format_ARGB32_Premultiplied, 3);
    REQUIRE( IOManager::FontFileManager::fontFromXml(filename) == nullptr );
  }

  SECTION("more pixel values than the announced size")
  {
    const QString filename =
      writeOF(QStringLiteral("tmp_corrupt_long.of"), 2, 2, QImage::Format_ARGB32, 16);
    REQUIRE( IOManager::FontFileManager::fontFromXml(filename) == nullptr );
  }

  SECTION("a size whose product overflows an int")
  {
    const QString filename =
      writeOF(QStringLiteral("tmp_corrupt_overflow.of"), 65536, 65536,
	      QImage::Format_ARGB32, 4);
    REQUIRE( IOManager::FontFileManager::fontFromXml(filename) == nullptr );
  }

  SECTION("a null or negative size")
  {
    REQUIRE( IOManager::FontFileManager::fontFromXml(
	       writeOF(QStringLiteral("tmp_corrupt_zero.of"), 0, 4,
		       QImage::Format_ARGB32, 0)) == nullptr );
    REQUIRE( IOManager::FontFileManager::fontFromXml(
	       writeOF(QStringLiteral("tmp_corrupt_negative.of"), -4, 4,
		       QImage::Format_ARGB32, 16)) == nullptr );
  }

  SECTION("an image format that does not exist")
  {
    REQUIRE( IOManager::FontFileManager::fontFromXml(
	       writeOF(QStringLiteral("tmp_corrupt_format0.of"), 4, 4,
		       QImage::Format_Invalid, 16)) == nullptr );
    REQUIRE( IOManager::FontFileManager::fontFromXml(
	       writeOF(QStringLiteral("tmp_corrupt_format255.of"), 4, 4,
		       255, 16)) == nullptr );
  }
}
