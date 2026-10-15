#include <catch2/catch_test_macros.hpp>

#include <QString>

#include "models/doc/doccharacter.h"
#include "models/doc/docparagraph.h"
#include "models/doc/docstring.h"
#include "models/doc/docstyle.h"
#include "models/doc/document.h"
#include "models/doc/doctextblock.h"
#include "models/doc/page.h"

/*
  The cursor machinery of the document model -- Node, NodeOfLeafs and
  NodeOfNodes, and the DocString and DocParagraph built on them -- had no
  test at all, although it is pure logic and holds the state every edit
  goes through: an insertion point (the offset), a selection, and a length
  that has to stay equal to the sum of the lengths of the children.

  The invariants checked below are the ones the rest of the model relies
  on:

   - length() is the sum of the children's lengths;
   - offset() stays within [0; length()], whatever is asked for;
   - removeBeforeCursor()/removeAfterCursor() return exactly what they took
     off the length, and return 0 instead of reaching past an end;
   - splitAtPosition() gives two parts whose lengths add up to the whole.
*/

namespace {

  int
  sumOfLengths(const QList<Doc::DocCharacter *> &characters)
  {
    int sum = 0;
    for (const Doc::DocCharacter *c : characters)
      sum += c->length();
    return sum;
  }

  int
  sumOfLengths(const QList<Doc::DocString *> &strings)
  {
    int sum = 0;
    for (const Doc::DocString *s : strings)
      sum += s->length();
    return sum;
  }

  /*
    @return a new style registered with @a document.

    DocStyleManager deletes the styles it holds, so a style handed to a
    document has to be allocated and must not be deleted here.
  */
  Doc::DocStyle *
  makeStyle(Doc::Document &document, const QString &name)
  {
    auto *style = new Doc::DocStyle(name, name + QStringLiteral("Font"));
    document.addStyle(style);
    return style;
  }

  //@return a string of @a text, in @a style, belonging to @a document
  Doc::DocString *
  makeString(Doc::Document &document, Doc::DocStyle *style, const QString &text)
  {
    auto *s = new Doc::DocString(style, &document);
    for (const QChar &c : text)
      s->add(new Doc::DocCharacter(QString(c), 0, &document));
    return s;
  }

} //namespace

TEST_CASE( "Testing DocString, a node of leafs" )
{
  Doc::Document document;
  Doc::DocStyle *style = makeStyle(document, QStringLiteral("default"));

  SECTION("an empty string")
  {
    Doc::DocString s(style, &document);

    REQUIRE( s.isEmpty() );
    REQUIRE( s.length() == 0 );
    REQUIRE( s.offset() == 0 );
    REQUIRE( s.isAtBeginning() );
    REQUIRE( s.isAtEnd() );
    REQUIRE( s.getCharacters().isEmpty() );

    //removing past either end takes nothing and does not move the cursor
    REQUIRE( s.removeBeforeCursor() == 0 );
    REQUIRE( s.removeAfterCursor() == 0 );
    REQUIRE( s.length() == 0 );
    REQUIRE( s.offset() == 0 );

    //and there is nothing to select
    s.select(0, 0);
    REQUIRE( s.getSelection() == nullptr );
    REQUIRE( s.removeSelection() == 0 );
  }

  SECTION("the length follows the characters added")
  {
    Doc::DocString *s = makeString(document, style, QStringLiteral("abcde"));

    REQUIRE_FALSE( s->isEmpty() );
    REQUIRE( s->getCharacters().size() == 5 );
    REQUIRE( s->length() == 5 );
    REQUIRE( s->length() == sumOfLengths(s->getCharacters()) );
    //every character has been added at the cursor, which is at the end
    REQUIRE( s->offset() == 5 );
    REQUIRE( s->isAtEnd() );
    REQUIRE_FALSE( s->isAtBeginning() );

    delete s;
  }

  SECTION("the offset is clamped to the length")
  {
    Doc::DocString *s = makeString(document, style, QStringLiteral("abcde"));

    s->setOffset(-10);
    REQUIRE( s->offset() == 0 );
    REQUIRE( s->isAtBeginning() );

    s->setOffset(1000);
    REQUIRE( s->offset() == 5 );
    REQUIRE( s->isAtEnd() );

    s->setOffset(3);
    REQUIRE( s->offset() == 3 );
    REQUIRE_FALSE( s->isAtBeginning() );
    REQUIRE_FALSE( s->isAtEnd() );

    delete s;
  }

  SECTION("removing on either side of the cursor")
  {
    Doc::DocString *s = makeString(document, style, QStringLiteral("abcde"));
    s->setOffset(3);

    REQUIRE( s->removeBeforeCursor() == 1 );
    REQUIRE( s->length() == 4 );
    REQUIRE( s->offset() == 2 );
    REQUIRE( s->length() == sumOfLengths(s->getCharacters()) );

    REQUIRE( s->removeAfterCursor() == 1 );
    REQUIRE( s->length() == 3 );
    REQUIRE( s->offset() == 2 );
    REQUIRE( s->length() == sumOfLengths(s->getCharacters()) );

    //what is left is "abe"
    QString left;
    for (const Doc::DocCharacter *c : s->getCharacters())
      left += c->getDisplay();
    REQUIRE( left == QLatin1String("abe") );

    //at the beginning, removing before takes nothing
    s->setOffset(0);
    REQUIRE( s->removeBeforeCursor() == 0 );
    REQUIRE( s->length() == 3 );

    //at the end, removing after takes nothing
    s->setOffset(s->length());
    REQUIRE( s->removeAfterCursor() == 0 );
    REQUIRE( s->length() == 3 );

    delete s;
  }

  SECTION("emptying the string one character at a time")
  {
    Doc::DocString *s = makeString(document, style, QStringLiteral("abcde"));

    int removed = 0;
    while (! s->isEmpty() && removed < 100) {
      s->setOffset(s->length());
      removed += s->removeBeforeCursor();
      REQUIRE( s->length() == sumOfLengths(s->getCharacters()) );
    }
    REQUIRE( removed == 5 );
    REQUIRE( s->isEmpty() );
    REQUIRE( s->length() == 0 );

    delete s;
  }

  SECTION("a selection holds the characters between the two offsets")
  {
    Doc::DocString *s = makeString(document, style, QStringLiteral("abcde"));

    s->select(1, 4);
    Doc::DocString *selection = s->getSelection();
    REQUIRE( selection != nullptr );
    REQUIRE( selection->length() == 3 );
    QString selected;
    for (const Doc::DocCharacter *c : selection->getCharacters())
      selected += c->getDisplay();
    REQUIRE( selected == QLatin1String("bcd") );
    //the selection is a copy: the string is untouched
    REQUIRE( s->length() == 5 );
    delete selection;

    //removing it takes exactly those characters
    s->select(1, 4);
    REQUIRE( s->removeSelection() == 3 );
    REQUIRE( s->length() == 2 );
    REQUIRE( s->length() == sumOfLengths(s->getCharacters()) );

    delete s;
  }

  SECTION("splitting at a position gives two parts of the same total length")
  {
    for (int position = 0; position <= 5; ++position) {
      Doc::DocString *s = makeString(document, style, QStringLiteral("abcde"));

      const QList<Doc::DocString *> parts = s->splitAtPosition(position);
      REQUIRE( parts.size() == 2 );
      REQUIRE( parts.at(0)->length() + parts.at(1)->length() == 5 );
      REQUIRE( parts.at(0)->length() == position );

      delete parts.at(0);
      delete parts.at(1);
      delete s;
    }
  }

  SECTION("splitting an empty string gives nothing")
  {
    //NodeOfNodes::add() relied on the two halves being there and used
    //list.at(0) without looking.
    Doc::DocString s(style, &document);
    REQUIRE( s.splitAtPosition(0).isEmpty() );
  }

  SECTION("a clone is an independent copy")
  {
    Doc::DocString *s = makeString(document, style, QStringLiteral("abcde"));
    s->setOffset(2);

    Doc::DocString *copy = s->clone();
    REQUIRE( copy->length() == s->length() );
    REQUIRE( copy->offset() == s->offset() );

    copy->setOffset(copy->length());
    copy->removeBeforeCursor();
    REQUIRE( copy->length() == 4 );
    REQUIRE( s->length() == 5 ); //the original is untouched

    delete copy;
    delete s;
  }

  SECTION("the same character cannot be added twice")
  {
    //insertElement() called exit() on a duplicate, which threw away the
    //document being edited; it refuses the insertion instead.
    Doc::DocString s(style, &document);
    auto *c = new Doc::DocCharacter(QStringLiteral("a"), 0, &document);
    s.add(c);
    REQUIRE( s.length() == 1 );
    s.add(c);
    REQUIRE( s.length() == 1 );
    REQUIRE( s.getCharacters().size() == 1 );
  }
}

TEST_CASE( "Testing DocParagraph, a node of nodes" )
{
  Doc::Document document;
  Doc::DocStyle *style = makeStyle(document, QStringLiteral("default"));

  //A paragraph always holds its end-of-paragraph string, of length 1, which
  //getStrings() returns but which is not one of the elements.
  const int END_OF_PARAGRAPH = 1;

  SECTION("a new paragraph holds only its end of paragraph")
  {
    Doc::DocParagraph p(&document);

    REQUIRE( p.isEmpty() );
    REQUIRE( p.length() == END_OF_PARAGRAPH );
    REQUIRE( p.getStrings().size() == 1 );
    REQUIRE( p.length() == sumOfLengths(p.getStrings()) );

    REQUIRE( p.removeBeforeCursor() == 0 );
    REQUIRE( p.removeAfterCursor() == 0 );
    REQUIRE( p.length() == END_OF_PARAGRAPH );
  }

  SECTION("the length follows the characters added")
  {
    Doc::DocParagraph p(&document);
    p.changeStyle(style);
    for (const QChar &c : QStringLiteral("hello"))
      p.add(new Doc::DocCharacter(QString(c), 0, &document));

    REQUIRE_FALSE( p.isEmpty() );
    REQUIRE( p.length() == 5 + END_OF_PARAGRAPH );
    REQUIRE( p.length() == sumOfLengths(p.getStrings()) );
    //one string, since the style never changed
    REQUIRE( p.getStrings().size() == 1 + 1 );
  }

  SECTION("a style change starts a new string")
  {
    Doc::DocStyle *other = makeStyle(document, QStringLiteral("other"));

    Doc::DocParagraph p(&document);
    p.changeStyle(style);
    p.add(new Doc::DocCharacter(QStringLiteral("a"), 0, &document));
    p.add(new Doc::DocCharacter(QStringLiteral("b"), 0, &document));
    p.changeStyle(other);
    p.add(new Doc::DocCharacter(QStringLiteral("c"), 0, &document));

    REQUIRE( p.getStrings().size() == 2 + 1 );
    REQUIRE( p.length() == 3 + END_OF_PARAGRAPH );
    REQUIRE( p.length() == sumOfLengths(p.getStrings()) );
  }

  SECTION("the offset is clamped and reaches every character")
  {
    Doc::DocParagraph p(&document);
    p.changeStyle(style);
    for (const QChar &c : QStringLiteral("hello"))
      p.add(new Doc::DocCharacter(QString(c), 0, &document));

    p.setOffset(-10);
    REQUIRE( p.offset() == 0 );
    REQUIRE( p.isAtBeginning() );

    p.setOffset(1000);
    REQUIRE( p.offset() <= p.length() );
    REQUIRE( p.isAtEnd() );

    for (int o = 0; o <= 5; ++o) {
      p.setOffset(o);
      REQUIRE( p.offset() == o );
      REQUIRE( p.length() == 5 + END_OF_PARAGRAPH );
    }
  }

  SECTION("removing on either side of the cursor")
  {
    Doc::DocParagraph p(&document);
    p.changeStyle(style);
    for (const QChar &c : QStringLiteral("hello"))
      p.add(new Doc::DocCharacter(QString(c), 0, &document));

    p.setOffset(3);
    REQUIRE( p.removeBeforeCursor() == 1 );
    REQUIRE( p.length() == 4 + END_OF_PARAGRAPH );
    REQUIRE( p.length() == sumOfLengths(p.getStrings()) );

    REQUIRE( p.removeAfterCursor() == 1 );
    REQUIRE( p.length() == 3 + END_OF_PARAGRAPH );
    REQUIRE( p.length() == sumOfLengths(p.getStrings()) );

    p.setOffset(0);
    REQUIRE( p.removeBeforeCursor() == 0 );
    REQUIRE( p.length() == 3 + END_OF_PARAGRAPH );
  }

  SECTION("emptying the paragraph one character at a time")
  {
    Doc::DocParagraph p(&document);
    p.changeStyle(style);
    for (const QChar &c : QStringLiteral("hello"))
      p.add(new Doc::DocCharacter(QString(c), 0, &document));

    int removed = 0;
    for (int i = 0; i < 20 && ! p.isEmpty(); ++i) {
      p.setOffset(p.length());
      const int took = p.removeBeforeCursor();
      if (took == 0)
	break;
      removed += took;
      REQUIRE( p.length() == sumOfLengths(p.getStrings()) );
    }
    REQUIRE( removed == 5 );
    REQUIRE( p.isEmpty() );
    REQUIRE( p.length() == END_OF_PARAGRAPH );
  }

  SECTION("inserting a string in the middle splits the one there")
  {
    //NodeOfNodes::add() splits the element under the cursor in two and
    //inserts between the halves; the element it replaced was left behind.
    Doc::DocParagraph p(&document);
    p.changeStyle(style);
    for (const QChar &c : QStringLiteral("abcdef"))
      p.add(new Doc::DocCharacter(QString(c), 0, &document));
    REQUIRE( p.length() == 6 + END_OF_PARAGRAPH );

    p.setOffset(3);
    p.add(makeString(document, style, QStringLiteral("XY")));

    REQUIRE( p.length() == 8 + END_OF_PARAGRAPH );
    REQUIRE( p.length() == sumOfLengths(p.getStrings()) );

    QString text;
    for (const Doc::DocString *s : p.getStrings())
      for (const Doc::DocCharacter *c : s->getCharacters())
	text += c->getDisplay();
    REQUIRE( text == QLatin1String("abcXYdef\n") );
  }

  SECTION("splitting at a position gives two parts of the same total length")
  {
    for (int position = 0; position <= 5; ++position) {
      Doc::DocParagraph p(&document);
      p.changeStyle(style);
      for (const QChar &c : QStringLiteral("hello"))
	p.add(new Doc::DocCharacter(QString(c), 0, &document));

      const QList<Doc::DocParagraph *> parts = p.splitAtPosition(position);
      REQUIRE( parts.size() == 2 );
      //each part carries its own end of paragraph
      REQUIRE( parts.at(0)->length() + parts.at(1)->length() ==
	       5 + 2 * END_OF_PARAGRAPH );
      REQUIRE( parts.at(0)->length() == position + END_OF_PARAGRAPH );
      REQUIRE( parts.at(0)->length() == sumOfLengths(parts.at(0)->getStrings()) );
      REQUIRE( parts.at(1)->length() == sumOfLengths(parts.at(1)->getStrings()) );

      delete parts.at(0);
      delete parts.at(1);
    }
  }

  SECTION("splitting an empty paragraph gives nothing")
  {
    Doc::DocParagraph p(&document);
    REQUIRE( p.splitAtPosition(0).isEmpty() );
  }

  SECTION("a selection holds the characters between the two offsets")
  {
    Doc::DocParagraph p(&document);
    p.changeStyle(style);
    for (const QChar &c : QStringLiteral("hello"))
      p.add(new Doc::DocCharacter(QString(c), 0, &document));

    p.select(1, 4);
    Doc::DocParagraph *selection = p.getSelection();
    REQUIRE( selection != nullptr );
    QString selected;
    for (const Doc::DocString *s : selection->getStrings())
      for (const Doc::DocCharacter *c : s->getCharacters())
	selected += c->getDisplay();
    REQUIRE( selected == QLatin1String("ell\n") );
    delete selection;

    //and the paragraph is untouched by the copy
    REQUIRE( p.length() == 5 + END_OF_PARAGRAPH );

    p.select(1, 4);
    REQUIRE( p.removeSelection() == 3 );
    REQUIRE( p.length() == 2 + END_OF_PARAGRAPH );
    REQUIRE( p.length() == sumOfLengths(p.getStrings()) );
  }

  SECTION("a clone is an independent copy")
  {
    Doc::DocParagraph p(&document);
    p.changeStyle(style);
    for (const QChar &c : QStringLiteral("hello"))
      p.add(new Doc::DocCharacter(QString(c), 0, &document));
    p.setOffset(2);
    p.setLineSpacing(42);
    p.setTabulationSize(7);

    Doc::DocParagraph *copy = p.clone();
    REQUIRE( copy->length() == p.length() );
    REQUIRE( copy->offset() == p.offset() );
    REQUIRE( copy->lineSpacing() == 42 );
    REQUIRE( copy->tabulationSize() == 7 );

    copy->setOffset(copy->length());
    copy->removeBeforeCursor();
    REQUIRE( copy->length() == 4 + END_OF_PARAGRAPH );
    REQUIRE( p.length() == 5 + END_OF_PARAGRAPH );

    delete copy;
  }

  SECTION("the line spacing and the tabulation size are not negative")
  {
    Doc::DocParagraph p(&document);
    p.setLineSpacing(-1);
    REQUIRE( p.lineSpacing() == 0 );
    p.setTabulationSize(-1);
    REQUIRE( p.tabulationSize() == 0 );
  }
}


TEST_CASE( "Testing Page::add() with no current block" )
{
  //Page::add() of a paragraph, a string or a character creates the text
  //block to hold it when the page has none. It used to add that block, then
  //add the element to the null pointer it had found before.
  Doc::Document document;

  SECTION("a paragraph goes in a new text block")
  {
    Doc::Page page(&document);
    REQUIRE( page.getTextBlocks().empty() );

    auto paragraph = new Doc::DocParagraph(&document);
    page.add(paragraph);

    REQUIRE( page.getTextBlocks().size() == 1 );
    REQUIRE( page.currentBlock() == page.getTextBlocks().at(0) );
    REQUIRE( page.getTextBlocks().at(0)->getParagraphs().contains(paragraph) );
  }

  SECTION("a second paragraph goes in the same block")
  {
    Doc::Page page(&document);
    page.add(new Doc::DocParagraph(&document));
    page.add(new Doc::DocParagraph(&document));

    REQUIRE( page.getTextBlocks().size() == 1 );
  }
}
