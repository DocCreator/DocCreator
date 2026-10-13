/*
  measureText() and splitParagraphsToFitColumns() were the two blocks that
  the three document builders of ImageGenerationFromDirDialog each held a
  copy of. Out of the dialog, they can be run.
*/
#include <fstream>
#include <memory>

#include <catch2/catch_test_macros.hpp>

#include <QImage>

#include "models/character.h"
#include "models/characterdata.h"
#include "models/font.h"

#include "DocumentTextLayout.hpp"

namespace {

  const char *FILENAME = "tmp_testDocumentTextLayout.txt";

  /*
    @return a font whose character @a c is @a width pixels wide and @a
    height high, for each (c, width, height) of @a characters.
  */
  std::unique_ptr<Models::Font>
  makeFont(const std::vector<std::tuple<char, int, int>> &characters)
  {
    auto font = std::make_unique<Models::Font>(QStringLiteral("test"));
    for (const auto &c : characters) {
      QImage img(std::get<1>(c), std::get<2>(c), QImage::Format_ARGB32);
      img.fill(Qt::black);
      auto *ch = new Models::Character(QString(QChar(std::get<0>(c))), 0, 100, 0, 100);
      ch->add(new Models::CharacterData(img, 0));
      font->addCharacter(ch);
    }
    return font;
  }

  QString writeText(const std::string &text)
  {
    std::ofstream out(FILENAME);
    REQUIRE( out.good() );
    out << text;
    out.close();
    return QString::fromLatin1(FILENAME);
  }

} //namespace

TEST_CASE("measureText() measures the lines of a file")
{
  //'a' is 10x20, 'b' is 30x40, ' ' is 5x20
  const std::unique_ptr<Models::Font> font = makeFont({ {'a', 10, 20}, {'b', 30, 40}, {' ', 5, 20} });

  SECTION("an empty file")
  {
    const TextMeasurement m = measureText(writeText(""), *font);
    REQUIRE( m.isEmpty() );
    REQUIRE( m.lines.isEmpty() );
    REQUIRE( m.numCharacters == 0 );
    //and no division by zero on the way out
    REQUIRE( m.meanCharHeight == 0 );
  }

  SECTION("a file that does not exist")
  {
    const TextMeasurement m = measureText(QStringLiteral("no_such_file.txt"), *font);
    REQUIRE( m.isEmpty() );
    REQUIRE( m.lines.isEmpty() );
  }

  SECTION("one line")
  {
    const TextMeasurement m = measureText(writeText("aab\n"), *font);
    REQUIRE( m.lines.size() == 1 );
    REQUIRE( m.rawLines.size() == 1 );
    REQUIRE( m.lines[0].charList.size() == 3 );
    //10 + 10 + 30
    REQUIRE( m.lines[0].Long == 50 );
    REQUIRE( m.maxLineLong == 50 );
    REQUIRE( m.totalLineLong == 50 );
    REQUIRE( m.maxCharHeight == 40 );
    REQUIRE( m.numCharacters == 3 );
    //(20 + 20 + 40) / 3
    REQUIRE( m.meanCharHeight == 26 );
  }

  SECTION("several lines")
  {
    const TextMeasurement m = measureText(writeText("a\nbb\nab\n"), *font);
    REQUIRE( m.lines.size() == 3 );
    REQUIRE( m.lines[0].Long == 10 );
    REQUIRE( m.lines[1].Long == 60 );
    REQUIRE( m.lines[2].Long == 40 );
    REQUIRE( m.maxLineLong == 60 );
    REQUIRE( m.totalLineLong == 110 );
  }

  SECTION("the characters the font does not hold are left out")
  {
    //'Z' is not in the font
    const TextMeasurement m = measureText(writeText("aZb\n"), *font);
    REQUIRE( m.lines.size() == 1 );
    REQUIRE( m.lines[0].charList.size() == 2 );
    REQUIRE( m.lines[0].Long == 40 ); //10 + 30, 'Z' adds nothing

    /*
      B: numCharacters counts the characters of the file, 'Z' included,
      while the sum of the heights only counts those the font holds. The
      mean is then smaller than any of the heights it averages: (20 + 40)
      divided by 3 rather than by 2. This is what the three builders did
      and what this kept; it is a bug of its own.
    */
    REQUIRE( m.numCharacters == 3 );
    REQUIRE( m.meanCharHeight == 20 );
  }
}

TEST_CASE("splitParagraphsToFitColumns() cuts what does not fit")
{
  const std::unique_ptr<Models::Font> font = makeFont({ {'a', 10, 20}, {' ', 5, 20} });

  SECTION("a paragraph that fits is left alone")
  {
    TextMeasurement m = measureText(writeText("aa aa\n"), *font);
    const int before = m.lines.size();
    splitParagraphsToFitColumns(m.lines, *font, 1000, 1000, 24, 20);
    REQUIRE( m.lines.size() == before );
    REQUIRE( m.lines[0].charList.size() == 5 );
  }

  SECTION("a paragraph that does not fit becomes two")
  {
    TextMeasurement m = measureText(writeText("aa aa aa aa aa aa\n"), *font);
    REQUIRE( m.lines.size() == 1 );

    //a column one character wide and one line high: nothing fits
    splitParagraphsToFitColumns(m.lines, *font, 10, 1, 24, 20);
    REQUIRE( m.lines.size() == 2 );

    /*
      B: the two halves hold 19 characters for an input of 17.

      The walk over the words runs "for (i = spaceIndex; i <= spaceIndexNext;
      ++i)" and then sets spaceIndex = spaceIndexNext, so the space at each
      boundary is copied twice, once as the end of a word and once as the
      start of the next; and the characters after the last space are never
      copied at all. "aa aa aa aa aa aa" has five spaces, so the walk copies
      "aa " then " aa " four times, 19 characters for 17, and never copies
      the last "aa".

      This is what the three builders did, and this records it rather than
      changing it: the dialog is not built (cf software/DocCreator/
      CMakeLists.txt:420) and there is no way to check a new behaviour
      against the old one.
    */
    QString all;
    for (const LineText &l : m.lines) {
      for (const QString &ch : l.charList) {
	all += ch;
      }
    }
    REQUIRE( all.size() == 19 );
    REQUIRE( all == QStringLiteral("aa  aa  aa  aa  aa ") );
  }

  SECTION("an empty list")
  {
    QList<LineText> none;
    splitParagraphsToFitColumns(none, *font, 100, 100, 24, 20);
    REQUIRE( none.isEmpty() );
  }
}
