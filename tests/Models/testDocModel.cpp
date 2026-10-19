#include <catch2/catch_test_macros.hpp>

#include <QString>

#include <memory>

#include "models/doc/block.h"
#include "models/doc/doccharacter.h"
#include "models/doc/doccomponent.h"
#include "models/doc/doccomponentblock.h"
#include "models/doc/docimageblock.h"
#include "models/doc/docparagraph.h"
#include "models/doc/docstring.h"
#include "models/doc/docstyle.h"
#include "models/doc/doctestblock.h"
#include "models/doc/doctextblock.h"
#include "models/doc/document.h"
#include "models/doc/doczone.h"
#include "models/doc/page.h"

/*
  The containers of the document model above the paragraph: Document,
  Page, DocTextBlock and DocComponentBlock. testDocNode tests the nodes and
  the cursor machinery; this tests what the editor and the loaders build
  with them -- pages, blocks of each kind, the current element at each
  level, styles, and copies.

  Ownership, as the code has it: a document deletes its pages, a page its
  blocks, a block its paragraphs or zones. Document::remove() and
  Page::remove() do not delete.
*/

namespace {

  Doc::DocStyle *
  makeStyle(Doc::Document &document, const QString &name)
  {
    auto *style = new Doc::DocStyle(name, name + QStringLiteral("Font"));
    document.addStyle(style);
    return style;
  }

  Doc::DocParagraph *
  makeParagraph(Doc::Document &document, Doc::DocStyle *style, const QString &text)
  {
    auto *p = new Doc::DocParagraph(&document);
    p->changeStyle(style);
    for (const QChar &c : text)
      p->add(new Doc::DocCharacter(QString(c), 0, &document));
    return p;
  }

  Doc::DocComponent *
  makeComponent(Doc::Document &document, int id)
  {
    return new Doc::DocComponent(Doc::Scanlines(), id, &document);
  }

} //namespace


TEST_CASE( "Document: pages" )
{
  Doc::Document document;

  SECTION("a new document has no page and no current element")
  {
    REQUIRE( document.getPages().empty() );
    REQUIRE( document.currentPage() == nullptr );
    REQUIRE( document.currentBlock() == nullptr );
    REQUIRE( document.currentParagraph() == nullptr );
    REQUIRE( document.currentString() == nullptr );
    REQUIRE( document.currentCharacter() == nullptr );
    REQUIRE( document.getSelection() == nullptr );
  }

  SECTION("a page added becomes the current one, once")
  {
    auto *p1 = new Doc::Page(&document);
    auto *p2 = new Doc::Page(&document);
    document.add(p1);
    document.add(p2);
    document.add(p2);
    REQUIRE( document.getPages().size() == 2 );
    REQUIRE( document.currentPage() == p2 );

    document.setCurrentPage(p1);
    REQUIRE( document.currentPage() == p1 );
  }

  SECTION("a page removed is no longer current, and is not deleted")
  {
    auto *p1 = new Doc::Page(&document);
    auto *p2 = new Doc::Page(&document);
    document.add(p1);
    document.add(p2);
    document.remove(p2);
    REQUIRE( document.getPages().size() == 1 );
    REQUIRE( document.currentPage() == nullptr );
    delete p2;

    document.removePages();
    REQUIRE( document.getPages().empty() );
    delete p1;
  }

  SECTION("geometry")
  {
    document.setPageWidth(600);
    document.setPageHeight(800);
    document.setPageMarginLeft(1);
    document.setPageMarginRight(2);
    document.setPageMarginTop(3);
    document.setPageMarginBottom(4);
    REQUIRE( document.pageWidth() == 600 );
    REQUIRE( document.pageHeight() == 800 );
    REQUIRE( document.pageMarginLeft() == 1 );
    REQUIRE( document.pageMarginRight() == 2 );
    REQUIRE( document.pageMarginTop() == 3 );
    REQUIRE( document.pageMarginBottom() == 4 );
  }
}

TEST_CASE( "Document: adding text where there is no page yet" )
{
  //Document::add() of an element that lives in a block creates the page,
  //and Page::add() the block, that are missing
  Doc::Document document;
  Doc::DocStyle *style = makeStyle(document, QStringLiteral("default"));

  SECTION("a paragraph")
  {
    document.add(makeParagraph(document, style, QStringLiteral("abc")));
    REQUIRE( document.getPages().size() == 1 );
    Doc::Page *page = document.currentPage();
    REQUIRE( page != nullptr );
    REQUIRE( page->getTextBlocks().size() == 1 );
    REQUIRE( document.currentBlock() == page->getTextBlocks().at(0) );
    REQUIRE( document.currentParagraph() != nullptr );
    REQUIRE( page->getTextBlocks().at(0)->content().contains(QStringLiteral("abc")) );
  }

  SECTION("characters, then a string")
  {
    document.add(new Doc::DocCharacter(QStringLiteral("x"), 0, &document));
    document.add(QList<Doc::DocCharacter *>{ new Doc::DocCharacter(QStringLiteral("y"), 0, &document) });
    REQUIRE( document.currentCharacter() != nullptr );
    REQUIRE( document.currentString() != nullptr );
    const QString content = document.currentPage()->getTextBlocks().at(0)->content();
    REQUIRE( content.contains(QStringLiteral("xy")) );
  }

  SECTION("components and zones go in a component block")
  {
    document.add(makeComponent(document, 1));
    document.add(QList<Doc::DocComponent *>{ makeComponent(document, 2) });
    Doc::Page *page = document.currentPage();
    REQUIRE( page != nullptr );
    REQUIRE( page->getComponentBlocks().size() == 1 );
    const Doc::DocComponentBlock *block = page->getComponentBlocks().at(0);
    REQUIRE( block->getZones().size() == 1 );
    REQUIRE( block->getZones().at(0)->getComponents().size() == 2 );
  }
}

TEST_CASE( "Page: blocks of each kind" )
{
  Doc::Document document;
  auto *page = new Doc::Page(&document);
  document.add(page);

  auto *text = new Doc::DocTextBlock(&document, 100, 50, 10, 20);
  auto *image = new Doc::DocImageBlock(QStringLiteral("image.png"));
  auto *test = new Doc::DocTestBlock(QStringLiteral("test.png"));
  auto *components = new Doc::DocComponentBlock(&document, 30, 40, 1, 2);

  page->add(text);
  page->add(image);
  page->add(test);
  page->add(components);
  //once only
  page->add(text);
  page->add(components);

  REQUIRE( page->getTextBlocks().size() == 1 );
  REQUIRE( page->getImageBlocks().size() == 1 );
  REQUIRE( page->getTestBlocks().size() == 1 );
  REQUIRE( page->getComponentBlocks().size() == 1 );
  //the last one added is the current one
  REQUIRE( page->currentBlock() == components );

  REQUIRE( text->width() == 100 );
  REQUIRE( text->height() == 50 );
  REQUIRE( text->x() == 10 );
  REQUIRE( text->y() == 20 );

  SECTION("removed blocks are taken out of their list, not deleted")
  {
    page->remove(components);
    REQUIRE( page->getComponentBlocks().empty() );
    REQUIRE( page->currentBlock() == nullptr );
    page->remove(text);
    page->remove(image);
    page->remove(test);
    REQUIRE( page->getTextBlocks().empty() );
    REQUIRE( page->getImageBlocks().empty() );
    REQUIRE( page->getTestBlocks().empty() );
    delete components;
    delete text;
    delete image;
    delete test;
  }

  SECTION("removeTextBlocks() empties the list only")
  {
    page->removeTextBlocks();
    REQUIRE( page->getTextBlocks().empty() );
    delete text;
  }

  SECTION("background and degradation models")
  {
    page->setBackgroundFileName(QStringLiteral("paper.png"));
    REQUIRE( page->getBackgroundFileName() == QStringLiteral("paper.png") );
    page->setAppliedDegradationModel(QStringLiteral("blur"), {});
    REQUIRE( page->getListDegradationsModels().contains(QStringLiteral("blur")) );
  }
}

TEST_CASE( "DocTextBlock: content, style and copies" )
{
  Doc::Document document;
  Doc::DocStyle *style = makeStyle(document, QStringLiteral("default"));
  Doc::DocStyle *bold = makeStyle(document, QStringLiteral("bold"));

  auto *block = new Doc::DocTextBlock(&document);
  block->add(makeParagraph(document, style, QStringLiteral("hello")));
  block->add(makeParagraph(document, style, QStringLiteral("world")));

  REQUIRE( block->getParagraphs().size() == 2 );
  const QString content = block->content();
  REQUIRE( content.contains(QStringLiteral("hello")) );
  REQUIRE( content.contains(QStringLiteral("world")) );
  REQUIRE( content.indexOf(QStringLiteral("hello")) < content.indexOf(QStringLiteral("world")) );

  SECTION("a clone is a copy, in other paragraphs, and the block is unchanged")
  {
    //clone() added the copies of the paragraphs to this block, and left
    //the copy empty
    const QString before = block->content();
    Doc::DocTextBlock *copy = block->clone();
    REQUIRE( copy != block );
    REQUIRE( block->content() == before );
    REQUIRE( block->getParagraphs().size() == 2 );
    REQUIRE( copy->content() == block->content() );
    REQUIRE( copy->length() == block->length() );
    REQUIRE( copy->getParagraphs().size() == block->getParagraphs().size() );
    for (int i = 0; i < copy->getParagraphs().size(); ++i)
      REQUIRE( copy->getParagraphs().at(i) != block->getParagraphs().at(i) );
    delete copy;
  }

  SECTION("a selection is a copy of what is selected")
  {
    block->select(1, 4); //"ell"
    Doc::DocTextBlock *selection = block->getSelection();
    REQUIRE( selection != nullptr );
    REQUIRE( selection->content().contains(QStringLiteral("ell")) );
    REQUIRE_FALSE( selection->content().contains(QStringLiteral("world")) );
    delete selection;
  }

  SECTION("changing the style")
  {
    block->setOffset(2);
    block->changeStyle(bold);
    REQUIRE( block->getStyle() != nullptr );
  }

  delete block;
}

TEST_CASE( "DocComponentBlock: content and copies" )
{
  Doc::Document document;

  auto *block = new Doc::DocComponentBlock(&document);
  for (int id = 0; id < 3; ++id)
    block->add(makeComponent(document, id));
  REQUIRE( block->getZones().size() == 1 );
  REQUIRE( block->currentZone() == block->getZones().at(0) );
  REQUIRE( block->length() == 3 );

  SECTION("a clone holds the same components, in other zones")
  {
    //clone() shared the zones, which the two blocks then deleted twice
    Doc::DocComponentBlock *copy = block->clone();
    REQUIRE( copy->getZones().size() == 1 );
    REQUIRE( copy->getZones().at(0) != block->getZones().at(0) );
    REQUIRE( copy->getZones().at(0)->getComponents().size() == 3 );
    REQUIRE( copy->length() == block->length() );
    delete copy;
  }

  SECTION("a list of components goes in the current zone")
  {
    block->add(QList<Doc::DocComponent *>{ makeComponent(document, 7), makeComponent(document, 8) });
    REQUIRE( block->getZones().at(0)->getComponents().size() == 5 );
    REQUIRE( block->length() == 5 );
  }

  delete block;
}

TEST_CASE( "DocString: a copy into another document outlives its source" )
{
  //what pasting does: the strings of a document loaded from the
  //clipboard go into the edited document, and the loaded one is deleted.
  //Adding the strings themselves left the edited document with freed
  //strings, whose document and style were freed too.
  Doc::Document target;
  Doc::DocStyle *targetDefault = makeStyle(target, QStringLiteral("default"));

  auto source = std::make_unique<Doc::Document>();
  auto *sourceDefault = new Doc::DocStyle(QStringLiteral("default"), QStringLiteral("sourceFont"));
  source->addStyle(sourceDefault);
  Doc::DocStyle *sourceOther = makeStyle(*source, QStringLiteral("other"));

  auto makeString = [&](Doc::DocStyle *style, const QString &text) {
    auto s = std::make_unique<Doc::DocString>(style, source.get());
    for (const QChar &c : text)
      s->add(new Doc::DocCharacter(QString(c), 7, source.get()));
    return s;
  };
  auto first = makeString(sourceDefault, QStringLiteral("ab"));
  auto second = makeString(sourceOther, QStringLiteral("xyz"));

  std::unique_ptr<Doc::DocString> firstCopy(first->cloneInto(&target));
  std::unique_ptr<Doc::DocString> secondCopy(second->cloneInto(&target));

  first.reset();
  second.reset();
  source.reset();

  REQUIRE( firstCopy->getDocument() == &target );
  REQUIRE( firstCopy->length() == 2 );
  REQUIRE( firstCopy->getCharacters().at(1)->getDisplay() == QStringLiteral("b") );
  REQUIRE( firstCopy->getCharacters().at(1)->getId() == 7 );
  REQUIRE( firstCopy->getCharacters().at(1)->getDocument() == &target );

  SECTION("a style the target has is the target's")
  {
    REQUIRE( firstCopy->getStyle() == targetDefault );
    REQUIRE( firstCopy->getStyle()->getFontName() == QStringLiteral("defaultFont") );
  }

  SECTION("a style the target does not have is copied into it")
  {
    REQUIRE( secondCopy->length() == 3 );
    Doc::DocStyle *style = secondCopy->getStyle();
    REQUIRE( style != nullptr );
    REQUIRE( style != sourceOther );
    REQUIRE( style == target.getStyle(QStringLiteral("other")) );
    REQUIRE( style->getFontName() == QStringLiteral("otherFont") );
  }
}

/*
  ~DocString() dropped its style with changeStyle(style, nullptr), which
  also reset the current style of the document to nullptr: any string
  destroyed made the document lose its style.
*/
TEST_CASE( "Document: destroying a string keeps the current style" )
{
  Doc::Document document;
  auto *style = new Doc::DocStyle(QStringLiteral("default"), QStringLiteral("defaultFont"));
  document.addStyle(style);
  document.changeStyle(QString(), style);
  REQUIRE( document.getStyle() == style );

  auto *s = new Doc::DocString(style, &document);
  s->add(new Doc::DocCharacter(QStringLiteral("a"), 0, &document));
  delete s;

  REQUIRE( document.getStyle() == style );
}

