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
