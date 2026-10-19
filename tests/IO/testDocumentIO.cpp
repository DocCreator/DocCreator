#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include <QFile>
#include <QString>

#include "iomanager/documentfilemanager.h"
#include "iomanager/xmldocumentloader.h"
#include "models/doc/document.h"
#include "models/doc/page.h"

/*
  The geometry of a document was read from the file and used as is: the
  sizes end up in the allocations of the generation path, and a non
  numerical value silently gave 0.
*/

namespace {

  const int MAX = IOManager::XMLDocumentLoader::MAX_PAGE_SIZE;

  Doc::Document *
  loadXml(const QString &filename, const QString &content)
  {
    QFile file(filename);
    REQUIRE( file.open(QFile::WriteOnly | QFile::Truncate) );
    file.write(content.toUtf8());
    file.close();

    return IOManager::DocumentFileManager::documentFromXml(filename);
  }

  QString
  makeDocument(const QString &documentAttributes,
	       const QString &charAttributes)
  {
    return QStringLiteral(
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<document %1>\n"
      "  <styles>\n"
      "    <style name=\"default\"><font>JulesVerne</font></style>\n"
      "  </styles>\n"
      "  <content>\n"
      "    <page>\n"
      "      <textBlock x=\"10\" y=\"20\" width=\"100\" height=\"200\""
      " marginTop=\"1\" marginBottom=\"2\" marginLeft=\"3\" marginRight=\"4\">\n"
      "        <paragraph lineSpacing=\"5\" tabulationSize=\"6\">\n"
      "          <string style=\"default\">\n"
      "            <char %2/>\n"
      "          </string>\n"
      "        </paragraph>\n"
      "      </textBlock>\n"
      "    </page>\n"
      "  </content>\n"
      "</document>\n").arg(documentAttributes, charAttributes);
  }

} //namespace

TEST_CASE( "Testing document XML loading" )
{
  SECTION("a well formed document")
  {
    Doc::Document *doc =
      loadXml(QStringLiteral("tmp_doc_ok.xml"),
	      makeDocument(QStringLiteral("width=\"1240\" height=\"1754\""),
			   QStringLiteral("id=\"3\" display=\"a\" x=\"12\""
					  " y=\"34\" width=\"8\" height=\"16\"")));
    REQUIRE( doc != nullptr );
    REQUIRE( doc->pageWidth() == 1240 );
    REQUIRE( doc->pageHeight() == 1754 );
    REQUIRE( doc->getPages().size() == 1 );
    delete doc;
  }

  SECTION("an absurd page size is clamped")
  {
    //<document width="2000000000" height="2000000000"> used to reach
    //setPageWidth()/setPageHeight() as is
    Doc::Document *doc =
      loadXml(QStringLiteral("tmp_doc_huge.xml"),
	      makeDocument(QStringLiteral("width=\"2000000000\""
					  " height=\"2000000000\""),
			   QStringLiteral("id=\"0\" display=\"a\" x=\"0\""
					  " y=\"0\" width=\"8\" height=\"16\"")));
    REQUIRE( doc != nullptr );
    REQUIRE( doc->pageWidth() == MAX );
    REQUIRE( doc->pageHeight() == MAX );
    delete doc;
  }

  SECTION("a negative page size is clamped")
  {
    Doc::Document *doc =
      loadXml(QStringLiteral("tmp_doc_negative.xml"),
	      makeDocument(QStringLiteral("width=\"-1240\" height=\"-1754\""),
			   QStringLiteral("id=\"0\" display=\"a\" x=\"0\""
					  " y=\"0\" width=\"8\" height=\"16\"")));
    REQUIRE( doc != nullptr );
    REQUIRE( doc->pageWidth() == 0 );
    REQUIRE( doc->pageHeight() == 0 );
    delete doc;
  }

  SECTION("a page size that is not a number")
  {
    Doc::Document *doc =
      loadXml(QStringLiteral("tmp_doc_nan.xml"),
	      makeDocument(QStringLiteral("width=\"2e9\" height=\"many\""),
			   QStringLiteral("id=\"0\" display=\"a\" x=\"0\""
					  " y=\"0\" width=\"8\" height=\"16\"")));
    REQUIRE( doc != nullptr );
    REQUIRE( doc->pageWidth() == 0 );
    REQUIRE( doc->pageHeight() == 0 );
    delete doc;
  }

  SECTION("an absurd character geometry is clamped")
  {
    Doc::Document *doc =
      loadXml(QStringLiteral("tmp_doc_hugechar.xml"),
	      makeDocument(QStringLiteral("width=\"1240\" height=\"1754\""),
			   QStringLiteral("id=\"0\" display=\"a\""
					  " x=\"-2000000000\" y=\"2000000000\""
					  " width=\"2000000000\""
					  " height=\"2000000000\"")));
    REQUIRE( doc != nullptr );
    REQUIRE( doc->pageWidth() == 1240 );
    delete doc;
  }
}

/*
  The reads and the writes used to ignore whether the file could be
  opened: documentFromXml() returned a document built from an empty
  string, and documentToXml() reported nothing at all.
*/
TEST_CASE( "Testing document IO failures" )
{
  SECTION("reading a file that does not exist")
  {
    REQUIRE(
      IOManager::DocumentFileManager::documentFromXml(
	QStringLiteral("no_such_directory/no_such_document.xml")) == nullptr );
  }

  SECTION("writing into a directory that does not exist")
  {
    Doc::Document doc;
    REQUIRE_FALSE(
      IOManager::DocumentFileManager::documentToXml(
	&doc, QStringLiteral("no_such_directory/doc.xml")) );

    IOManager::DocumentFileManager manager;
    REQUIRE_FALSE(
      manager.documentToTxt(&doc,
			    QStringLiteral("no_such_directory/doc.txt")) );
  }

  SECTION("a document written back can be read again")
  {
    Doc::Document *doc =
      loadXml(QStringLiteral("tmp_doc_roundtrip.xml"),
	      makeDocument(QStringLiteral("width=\"640\" height=\"480\""),
			   QStringLiteral("id=\"0\" display=\"a\" x=\"1\""
					  " y=\"2\" width=\"8\" height=\"16\"")));
    REQUIRE( doc != nullptr );

    const QString saved = QStringLiteral("tmp_doc_roundtrip_saved.xml");
    REQUIRE( IOManager::DocumentFileManager::documentToXml(doc, saved) );
    delete doc;

    Doc::Document *doc2 =
      IOManager::DocumentFileManager::documentFromXml(saved);
    REQUIRE( doc2 != nullptr );
    REQUIRE( doc2->pageWidth() == 640 );
    REQUIRE( doc2->pageHeight() == 480 );
    REQUIRE( doc2->getPages().size() == 1 );
    delete doc2;
  }
}

/*
  save -> load -> save has to give the same bytes twice: that pins the
  format, and it is the only thing that tells whether the loader reads back
  everything the saver writes.
*/

#include "iomanager/documentsaverdirector.h"
#include "iomanager/xmldocumentsaver.h"

namespace {

  QString
  toXml(Doc::Document *document)
  {
    IOManager::XMLDocumentSaver saver(document);
    IOManager::DocumentSaverDirector director(&saver);
    director.constructDocumentOutput();
    return director.getDocumentOutput();
  }

  QString
  makeRichDocument()
  {
    return QStringLiteral(
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<document width=\"1240\" height=\"1754\" marginTop=\"10\""
      " marginBottom=\"20\" marginLeft=\"30\" marginRight=\"40\">\n"
      "  <styles>\n"
      "    <style name=\"first\"><font>JulesVerne</font></style>\n"
      "    <style name=\"second\"><font>Arabic</font></style>\n"
      "  </styles>\n"
      "  <content>\n"
      "    <page>\n"
      "      <textBlock x=\"10\" y=\"20\" width=\"100\" height=\"200\""
      " marginTop=\"1\" marginBottom=\"2\" marginLeft=\"3\" marginRight=\"4\">\n"
      "        <paragraph lineSpacing=\"5\" tabulationSize=\"6\">\n"
      "          <string style=\"first\">\n"
      "            <char id=\"1\" display=\"a\" x=\"1\" y=\"2\""
      " width=\"8\" height=\"16\"/>\n"
      "            <char id=\"2\" display=\"b\" x=\"9\" y=\"2\""
      " width=\"8\" height=\"16\"/>\n"
      "          </string>\n"
      "          <string style=\"second\">\n"
      "            <char id=\"3\" display=\"c\" x=\"17\" y=\"2\""
      " width=\"8\" height=\"16\"/>\n"
      "          </string>\n"
      "        </paragraph>\n"
      "        <paragraph lineSpacing=\"7\" tabulationSize=\"8\">\n"
      "          <string style=\"first\">\n"
      "            <char id=\"4\" display=\"d\" x=\"1\" y=\"20\""
      " width=\"8\" height=\"16\"/>\n"
      "          </string>\n"
      "        </paragraph>\n"
      "      </textBlock>\n"
      "    </page>\n"
      "    <page>\n"
      "      <textBlock x=\"5\" y=\"6\" width=\"50\" height=\"60\""
      " marginTop=\"0\" marginBottom=\"0\" marginLeft=\"0\" marginRight=\"0\">\n"
      "        <paragraph lineSpacing=\"9\" tabulationSize=\"0\">\n"
      "          <string style=\"second\">\n"
      "            <char id=\"5\" display=\"e\" x=\"0\" y=\"0\""
      " width=\"8\" height=\"16\"/>\n"
      "          </string>\n"
      "        </paragraph>\n"
      "      </textBlock>\n"
      "    </page>\n"
      "  </content>\n"
      "</document>\n");
  }

} //namespace

TEST_CASE( "Testing the document XML round trip" )
{
  SECTION("saving a loaded document twice gives the same bytes")
  {
    Doc::Document *doc =
      loadXml(QStringLiteral("tmp_doc_rich.xml"), makeRichDocument());
    REQUIRE( doc != nullptr );

    const QString first = toXml(doc);
    REQUIRE_FALSE( first.isEmpty() );
    delete doc;

    Doc::Document *doc2 =
      loadXml(QStringLiteral("tmp_doc_rich_1.xml"), first);
    REQUIRE( doc2 != nullptr );

    const QString second = toXml(doc2);
    REQUIRE( second == first );
    delete doc2;
  }

  SECTION("the geometry and the content survive the round trip")
  {
    Doc::Document *doc =
      loadXml(QStringLiteral("tmp_doc_rich_2.xml"), makeRichDocument());
    REQUIRE( doc != nullptr );
    const int pageWidth = doc->pageWidth();
    const int pageHeight = doc->pageHeight();
    const int numPages = doc->getPages().size();
    const int numStyles = doc->getStyles().size();
    const QString saved = toXml(doc);
    delete doc;

    Doc::Document *doc2 =
      loadXml(QStringLiteral("tmp_doc_rich_3.xml"), saved);
    REQUIRE( doc2 != nullptr );
    REQUIRE( doc2->pageWidth() == pageWidth );
    REQUIRE( doc2->pageHeight() == pageHeight );
    REQUIRE( doc2->getPages().size() == numPages );
    REQUIRE( doc2->getStyles().size() == numStyles );
    delete doc2;
  }
}

/*
  The saver ordered the paragraphs of a text block by the y-position of
  their first character with a std::map, which kept one paragraph per y and
  dropped the others. It read that character without checking that the
  first string had one.
*/

namespace {

  QString
  makeTextBlockDocument(const QString &paragraphs)
  {
    return QStringLiteral(
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<document width=\"1240\" height=\"1754\">\n"
      "  <styles>\n"
      "    <style name=\"first\"><font>JulesVerne</font></style>\n"
      "  </styles>\n"
      "  <content>\n"
      "    <page>\n"
      "      <textBlock x=\"10\" y=\"20\" width=\"100\" height=\"200\""
      " marginTop=\"1\" marginBottom=\"2\" marginLeft=\"3\" marginRight=\"4\">\n"
      "%1"
      "      </textBlock>\n"
      "    </page>\n"
      "  </content>\n"
      "</document>\n").arg(paragraphs);
  }

  QString
  makeParagraph(const QString &display, int y)
  {
    return QStringLiteral(
      "        <paragraph lineSpacing=\"0\" tabulationSize=\"0\">\n"
      "          <string style=\"first\">\n"
      "            <char id=\"1\" display=\"%1\" x=\"1\" y=\"%2\""
      " width=\"8\" height=\"16\"/>\n"
      "          </string>\n"
      "        </paragraph>\n").arg(display).arg(y);
  }

  //the display attributes of the characters of @a xml, in their order
  QString
  displays(const QString &xml)
  {
    QString result;
    int from = 0;
    const QString key = QStringLiteral("display=\"");
    for (;;) {
      const int i = xml.indexOf(key, from);
      if (i < 0)
        break;
      const int start = i + key.size();
      const int end = xml.indexOf(QLatin1Char('"'), start);
      result += xml.mid(start, end - start);
      from = end;
    }
    return result;
  }

} //namespace

TEST_CASE( "Testing the order of the saved paragraphs" )
{
  SECTION("paragraphs at the same y are all saved")
  {
    Doc::Document *doc =
      loadXml(QStringLiteral("tmp_doc_same_y.xml"),
              makeTextBlockDocument(makeParagraph(QStringLiteral("a"), 2) +
                                    makeParagraph(QStringLiteral("b"), 2) +
                                    makeParagraph(QStringLiteral("c"), 2)));
    REQUIRE( doc != nullptr );
    const QString saved = toXml(doc);
    delete doc;
    REQUIRE( saved.count(QStringLiteral("<paragraph")) == 3 );
    REQUIRE( displays(saved) == QStringLiteral("abc") );
  }

  SECTION("paragraphs are saved in the order of their y")
  {
    Doc::Document *doc =
      loadXml(QStringLiteral("tmp_doc_y_order.xml"),
              makeTextBlockDocument(makeParagraph(QStringLiteral("c"), 40) +
                                    makeParagraph(QStringLiteral("a"), 2) +
                                    makeParagraph(QStringLiteral("b"), 20)));
    REQUIRE( doc != nullptr );
    const QString saved = toXml(doc);
    delete doc;
    REQUIRE( saved.count(QStringLiteral("<paragraph")) == 3 );
    REQUIRE( displays(saved) == QStringLiteral("abc") );
  }

  SECTION("a paragraph whose first string has no character is saved")
  {
    const QString emptyFirstString = QStringLiteral(
      "        <paragraph lineSpacing=\"0\" tabulationSize=\"0\">\n"
      "          <string style=\"first\"/>\n"
      "          <string style=\"first\">\n"
      "            <char id=\"1\" display=\"b\" x=\"1\" y=\"20\""
      " width=\"8\" height=\"16\"/>\n"
      "          </string>\n"
      "        </paragraph>\n");
    const QString onlyEmptyString = QStringLiteral(
      "        <paragraph lineSpacing=\"0\" tabulationSize=\"0\">\n"
      "          <string style=\"first\"/>\n"
      "        </paragraph>\n");
    Doc::Document *doc =
      loadXml(QStringLiteral("tmp_doc_empty_string.xml"),
              makeTextBlockDocument(makeParagraph(QStringLiteral("a"), 2) +
                                    emptyFirstString +
                                    onlyEmptyString +
                                    makeParagraph(QStringLiteral("c"), 40)));
    REQUIRE( doc != nullptr );
    const QString saved = toXml(doc);
    delete doc;
    REQUIRE( displays(saved) == QStringLiteral("abc") );

    //and what is saved can be read again
    Doc::Document *doc2 =
      loadXml(QStringLiteral("tmp_doc_empty_string_1.xml"), saved);
    REQUIRE( doc2 != nullptr );
    REQUIRE( displays(toXml(doc2)) == QStringLiteral("abc") );
    delete doc2;
  }
}


#include "models/doc/doccharacter.h"
#include "models/doc/docparagraph.h"
#include "models/doc/docstyle.h"
#include "models/doc/doctextblock.h"

namespace {

  QString
  readText(const QString &filename)
  {
    QFile file(filename);
    REQUIRE( file.open(QFile::ReadOnly) );
    return QString::fromUtf8(file.readAll());
  }

  Doc::DocParagraph *
  paragraphOf(Doc::Document &document, Doc::DocStyle *style, const QString &text)
  {
    auto *p = new Doc::DocParagraph(&document);
    p->changeStyle(style);
    for (const QChar &c : text)
      p->add(new Doc::DocCharacter(QString(c), 0, &document));
    return p;
  }

} //namespace

TEST_CASE( "Testing the text export of a document" )
{
  Doc::Document document;
  auto *style = new Doc::DocStyle(QStringLiteral("default"), QStringLiteral("defaultFont"));
  document.addStyle(style);
  IOManager::DocumentFileManager manager;
  const QString filename = QStringLiteral("tmp_doc_export.txt");

  SECTION("an empty document gives an empty file")
  {
    REQUIRE( manager.documentToTxt(&document, filename) );
    REQUIRE( readText(filename).isEmpty() );
  }

  SECTION("the characters, in their order")
  {
    document.add(paragraphOf(document, style, QStringLiteral("héllo, wörld")));
    REQUIRE( manager.documentToTxt(&document, filename) );
    REQUIRE( readText(filename) == QStringLiteral("héllo, wörld") );
  }

  SECTION("every paragraph and every block")
  {
    //the end of a paragraph is not written, cf TxtDocumentSaver::
    //buildParagraph(): whether paragraphs are separated is not checked
    //here, only that nothing is lost or reordered
    document.add(paragraphOf(document, style, QStringLiteral("first")));
    document.add(paragraphOf(document, style, QStringLiteral("second")));
    auto *block = new Doc::DocTextBlock(&document);
    block->add(paragraphOf(document, style, QStringLiteral("third")));
    document.currentPage()->add(block);

    REQUIRE( manager.documentToTxt(&document, filename) );
    QString text = readText(filename);
    text.remove(QLatin1Char('\n'));
    REQUIRE( text == QStringLiteral("firstsecondthird") );
  }

  QFile::remove(filename);
}

/*
  The saver wrote the name of each style as its font: the font of a style
  whose name differs from its font was lost on a round trip. The two
  successive saves of the round trip test above agreed, both wrong.
*/
TEST_CASE( "Testing the font of the saved styles" )
{
  Doc::Document *doc =
    loadXml(QStringLiteral("tmp_doc_style_font.xml"), makeRichDocument());
  REQUIRE( doc != nullptr );
  const QString saved = toXml(doc);
  delete doc;

  REQUIRE( saved.contains(QStringLiteral("<font>JulesVerne</font>")) );
  REQUIRE( saved.contains(QStringLiteral("<font>Arabic</font>")) );
  REQUIRE_FALSE( saved.contains(QStringLiteral("<font>first</font>")) );
  REQUIRE_FALSE( saved.contains(QStringLiteral("<font>second</font>")) );

  Doc::Document *doc2 =
    loadXml(QStringLiteral("tmp_doc_style_font_1.xml"), saved);
  REQUIRE( doc2 != nullptr );
  QString first, second;
  for (const Doc::DocStyle *s : doc2->getStyles()) {
    if (s->getName() == QLatin1String("first"))
      first = s->getFontName();
    else if (s->getName() == QLatin1String("second"))
      second = s->getFontName();
  }
  REQUIRE( first == QStringLiteral("JulesVerne") );
  REQUIRE( second == QStringLiteral("Arabic") );
  delete doc2;
}

/*
  The styles were read up to </styles>: without a <styles> element, or
  with it after <content>, the whole document was read with them, and the
  document was loaded empty. And the file was decoded as UTF-8 whatever its
  XML declaration said.
*/
namespace {

  const char *CONTENT =
    "  <content>\n"
    "    <page>\n"
    "      <textBlock x=\"10\" y=\"20\" width=\"100\" height=\"200\""
    " marginTop=\"0\" marginBottom=\"0\" marginLeft=\"0\" marginRight=\"0\">\n"
    "        <paragraph lineSpacing=\"0\" tabulationSize=\"0\">\n"
    "          <string style=\"first\">\n"
    "            <char id=\"1\" display=\"a\" x=\"1\" y=\"2\" width=\"8\" height=\"16\"/>\n"
    "            <char id=\"2\" display=\"b\" x=\"9\" y=\"2\" width=\"8\" height=\"16\"/>\n"
    "          </string>\n"
    "        </paragraph>\n"
    "      </textBlock>\n"
    "    </page>\n"
    "  </content>\n";

  const char *STYLES =
    "  <styles>\n"
    "    <style name=\"first\"><font>JulesVerne</font></style>\n"
    "  </styles>\n";

} //namespace

TEST_CASE( "Testing document XML loading, in any order" )
{
  SECTION("the styles after the content")
  {
    Doc::Document *doc = loadXml(QStringLiteral("tmp_doc_styles_last.xml"),
      QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<document width=\"100\" height=\"100\">\n")
      + QLatin1String(CONTENT) + QLatin1String(STYLES) + QStringLiteral("</document>\n"));
    REQUIRE( doc != nullptr );
    REQUIRE( doc->getPages().size() == 1 );
    REQUIRE( displays(toXml(doc)) == QStringLiteral("ab") );
    REQUIRE( toXml(doc).contains(QStringLiteral("<font>JulesVerne</font>")) );
    delete doc;
  }

  SECTION("no styles at all")
  {
    Doc::Document *doc = loadXml(QStringLiteral("tmp_doc_no_styles.xml"),
      QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<document width=\"100\" height=\"100\">\n")
      + QLatin1String(CONTENT) + QStringLiteral("</document>\n"));
    REQUIRE( doc != nullptr );
    REQUIRE( doc->getPages().size() == 1 );
    REQUIRE( displays(toXml(doc)) == QStringLiteral("ab") );
    delete doc;
  }

  SECTION("an ISO-8859-1 document")
  {
    QByteArray bytes =
      "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<document width=\"100\" height=\"100\">\n";
    bytes += STYLES;
    bytes += QByteArray(CONTENT).replace("display=\"a\"", "display=\"\xe9\"");
    bytes += "</document>\n";
    const QString filename = QStringLiteral("tmp_doc_latin1.xml");
    QFile file(filename);
    REQUIRE( file.open(QFile::WriteOnly | QFile::Truncate) );
    file.write(bytes);
    file.close();
    Doc::Document *doc = IOManager::DocumentFileManager::documentFromXml(filename);
    REQUIRE( doc != nullptr );
    //U+00E9, written as one byte in the file
    REQUIRE( displays(toXml(doc)) == QString(QChar(0xE9)) + QStringLiteral("b") );
    delete doc;
  }
}
