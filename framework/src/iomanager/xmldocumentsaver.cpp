#include "xmldocumentsaver.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif
#include <QXmlStreamWriter>
#include <map>

namespace IOManager {

XMLDocumentSaver::XMLDocumentSaver(Doc::Document *doc)
  : DocumentSaver(doc)
  , _writer(nullptr)
{
  _writer = new QXmlStreamWriter(&_output);
  _isDIGIDOC = false;
}

XMLDocumentSaver::~XMLDocumentSaver()
{
  delete _writer;
}

void
XMLDocumentSaver::createNewDocumentOutput()
{
  delete _writer;

  DocumentSaver::createNewDocumentOutput();
  _writer = new QXmlStreamWriter(&_output);
  _writer->setAutoFormatting(true);
  _writer->setAutoFormattingIndent(2);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  _writer->setCodec(QTextCodec::codecForName("utf8"));
#endif

}

void
XMLDocumentSaver::buildStartDocument()
{
  if (!_isDIGIDOC) {
    _writer->writeStartDocument();
    _writer->writeStartElement(QStringLiteral("document"));
    _writer->writeAttribute(
      QStringLiteral("width"),
      QString::number(DocumentSaver::_input->pageWidth()));
    _writer->writeAttribute(
      QStringLiteral("height"),
      QString::number(DocumentSaver::_input->pageHeight()));
  } else {
    _writer->writeStartDocument();
    _writer->writeStartElement(QStringLiteral("DIGIDOC"));  // <DIGIDOC>
    buildHeaderDocument();                                  // HEADER
    _writer->writeStartElement(QStringLiteral("document")); // <document>
    buildDocumentInfos();
  }
}

void
XMLDocumentSaver::buildDocumentInfos()
{
  _writer->writeAttribute(QStringLiteral("ID"), QStringLiteral("0"));
  _writer->writeAttribute(QStringLiteral("URL"), QString());
  _writer->writeAttribute(QStringLiteral("width"),
                          QString::number(DocumentSaver::_input->pageWidth()));
  _writer->writeAttribute(QStringLiteral("height"),
                          QString::number(DocumentSaver::_input->pageHeight()));

  _writer->writeTextElement(QStringLiteral("Usage"),
                            QStringLiteral("Copyright"));

  _writer->writeStartElement(QStringLiteral("Production"));
  _writer->writeTextElement(QStringLiteral("Accquisition"),
                            QStringLiteral("Date, Company, material..."));
  _writer->writeEndElement();

  _writer->writeTextElement(
    QStringLiteral("Notice"),
    QStringLiteral("Printer, title, publisher, author, collation..."));

  _writer->writeStartElement(QStringLiteral("Statistics"));
  _writer->writeTextElement(QStringLiteral("NumberOfPages"),
                            QStringLiteral("1"));
  _writer->writeTextElement(QStringLiteral("NumberOfEocs"),
                            QStringLiteral("74"));
  _writer->writeTextElement(QStringLiteral("NumberOfWords"),
                            QStringLiteral("1024"));
  _writer->writeTextElement(QStringLiteral("Redundancy"),
                            QStringLiteral("0.7"));
  _writer->writeEndElement();
}

void
XMLDocumentSaver::buildHeaderDocument()
{
  _writer->writeStartElement(QStringLiteral("Header"));
  _writer->writeTextElement(QStringLiteral("MeasurementUnit"),
                            QStringLiteral("pixel"));
  _writer->writeTextElement(QStringLiteral("Company"), QStringLiteral("BNF"));
  _writer->writeTextElement(QStringLiteral("CreatedDate"),
                            QStringLiteral("30/05/2012")); //B:TODO:UGLY?
  _writer->writeEndElement();
}

void
XMLDocumentSaver::buildEndDocument()
{
  if (!_isDIGIDOC) {
    _writer->writeEndElement(); //</document>
    _writer->writeEndDocument();
  } else {
    _writer->writeEndElement(); //</document>
    _writer->writeEndElement(); // </DIGIDOC>
    _writer->writeEndDocument();
  }
}

void
XMLDocumentSaver::buildStyles()
{
  _writer->writeStartElement(QStringLiteral("styles"));
  for (Doc::DocStyle *s : _input->getStyles()) {
    buildStyle(s);
  }
  _writer->writeEndElement(); //</styles>
}

void
XMLDocumentSaver::buildContent()
{
  _writer->writeStartElement(QStringLiteral("content"));
  int id = 0;
  for (Doc::Page *p : _input->getPages()) {
    if (_isDIGIDOC)
      buildPage(p, id);
    else
      buildPage(p);
    ++id;
  }
  _writer->writeEndElement(); //</content>
}

//protected:
void
XMLDocumentSaver::buildMediaAttributeOfPage()
{
  _writer->writeStartElement(QStringLiteral("Media"));
  _writer->writeTextElement(QStringLiteral("Supports"),
                            QStringLiteral("Adobe Reader"));
  _writer->writeTextElement(QStringLiteral("files"), QStringLiteral("PDF"));
  _writer->writeTextElement(QStringLiteral("URL"), QString());
  _writer->writeEndElement();
}

void
XMLDocumentSaver::buildAcquisitionAttributeOfPage()
{
  _writer->writeStartElement(QStringLiteral("Acquisition"));
  _writer->writeTextElement(QStringLiteral("Material"), QStringLiteral("wood"));
  _writer->writeTextElement(QStringLiteral("date"), QStringLiteral("1876"));
  _writer->writeTextElement(QStringLiteral("user"), QString());
  _writer->writeStartElement(QStringLiteral("Parameters"));
  _writer->writeTextElement(QStringLiteral("ResolutionScaleFactor"),
                            QStringLiteral("100%"));
  _writer->writeTextElement(QStringLiteral("Contrast"), QString());
  _writer->writeTextElement(QStringLiteral("Luminosity"), QString());
  _writer->writeTextElement(QStringLiteral("speed"), QString());
  _writer->writeTextElement(QStringLiteral("Duration"), QString());
  _writer->writeEndElement();
  _writer->writeEndElement();
}

void
XMLDocumentSaver::buildBackgroundInfosOfPage()
{
  _writer->writeStartElement(QStringLiteral("Background"));
  _writer->writeTextElement(QStringLiteral("ColorHistogram"), QString());
  _writer->writeTextElement(QStringLiteral("Texture"), QString());
  _writer->writeTextElement(QStringLiteral("Blur"), QString());
  _writer->writeEndElement();
}

void
XMLDocumentSaver::buildLayoutInfosOfPage()
{
  _writer->writeStartElement(QStringLiteral("Layout"));
  _writer->writeStartElement(QStringLiteral("Spatial_Organization"));

  _writer->writeEndElement();
  _writer->writeStartElement(QStringLiteral("Complexity_Measurement"));

  _writer->writeEndElement();
  _writer->writeStartElement(QStringLiteral("Homogeneity_Measurement"));

  _writer->writeEndElement();

  _writer->writeEndElement();
}

void
XMLDocumentSaver::buildMediaAttributeOfEOCs()
{
  _writer->writeStartElement(QStringLiteral("Media"));
  _writer->writeTextElement(QStringLiteral("Supports"),
                            QStringLiteral("Adobe Reader"));
  _writer->writeTextElement(QStringLiteral("files"), QStringLiteral("PDF"));
  _writer->writeTextElement(QStringLiteral("URL"), QString());
  _writer->writeEndElement();
}

void
XMLDocumentSaver::buildAcquisitionAttributeOfEOCs()
{
  _writer->writeStartElement(QStringLiteral("Acquisition"));
  _writer->writeTextElement(QStringLiteral("Material"), QStringLiteral("wood"));
  _writer->writeTextElement(QStringLiteral("date"), QStringLiteral("1876"));
  _writer->writeTextElement(QStringLiteral("user"), QString());
  _writer->writeStartElement(QStringLiteral("Parameters"));
  _writer->writeTextElement(QStringLiteral("ResolutionScaleFactor"),
                            QStringLiteral("100%"));
  _writer->writeTextElement(QStringLiteral("Contrast"), QString());
  _writer->writeTextElement(QStringLiteral("Luminosity"), QString());
  _writer->writeTextElement(QStringLiteral("speed"), QString());
  _writer->writeTextElement(QStringLiteral("Duration"), QString());
  _writer->writeEndElement();
  _writer->writeEndElement();
}

void
XMLDocumentSaver::buildPositionOfEOCs()
{
  _writer->writeStartElement(QStringLiteral("scanline"));
  _writer->writeEndElement();
  _writer->writeStartElement(QStringLiteral("polyline"));
  _writer->writeEndElement();
}

void
XMLDocumentSaver::buildShapeInfosOfEOCs(int x, int y, int w, int h)
{
  _writer->writeStartElement(QStringLiteral("shape_information"));
  _writer->writeAttribute(QStringLiteral("x"), QString::number(x));
  _writer->writeAttribute(QStringLiteral("y"), QString::number(y));
  _writer->writeAttribute(QStringLiteral("width"), QString::number(w));
  _writer->writeAttribute(QStringLiteral("height"), QString::number(h));

  _writer->writeEndElement();
}

void
XMLDocumentSaver::buildStyle(Doc::DocStyle *s)
{
  _writer->writeStartElement(QStringLiteral("style"));
  _writer->writeAttribute(QStringLiteral("name"), s->getName());
  _writer->writeTextElement(QStringLiteral("font"), s->getName());
  _writer->writeEndElement(); //</style>
}

void
XMLDocumentSaver::groundtruthGlobalModel()
{

  _writer->writeStartElement(QStringLiteral("groundtruth"));
  _writer->writeStartElement(QStringLiteral("model"));
  _writer->writeAttribute(QStringLiteral("name"),
                          QStringLiteral("Global Deformation Model"));
  _writer->writeAttribute(QStringLiteral("level"), QStringLiteral("global"));
  _writer->writeAttribute(QStringLiteral("degradationlevel"),
                          QString::number(4));

  _writer->writeStartElement(QStringLiteral("parametre"));
  _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("ro"));
  _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("2200"));
  _writer->writeEndElement();

  _writer->writeStartElement(QStringLiteral("parametre"));
  _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("theta"));
  _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("6"));
  _writer->writeEndElement();

  _writer->writeStartElement(QStringLiteral("parametre"));
  _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("f"));
  _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("80"));
  _writer->writeEndElement();

  _writer->writeStartElement(QStringLiteral("parametre"));
  _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("delta"));
  _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("20"));
  _writer->writeEndElement();

  _writer->writeStartElement(QStringLiteral("parametre"));
  _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("l0"));
  _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("15"));
  _writer->writeEndElement();

  _writer->writeStartElement(QStringLiteral("parametre"));
  _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("k"));
  _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("8"));
  _writer->writeEndElement();

  _writer->writeEndElement();
  _writer->writeEndElement();
}

void
XMLDocumentSaver::groundtruthLuminosityModel()
{
  _writer->writeStartElement(QStringLiteral("groundtruth"));
  _writer->writeStartElement(QStringLiteral("model"));
  _writer->writeAttribute(QStringLiteral("name"),
                          QStringLiteral("Defect Luminosity Model"));
  _writer->writeAttribute(QStringLiteral("level"), QStringLiteral("global"));
  _writer->writeAttribute(QStringLiteral("degradationlevel"),
                          QString::number(4));

  _writer->writeStartElement(QStringLiteral("parametre"));
  _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("d0"));
  _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("6000"));
  _writer->writeEndElement();

  _writer->writeStartElement(QStringLiteral("parametre"));
  _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("a"));
  _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("0.0007"));
  _writer->writeEndElement();

  _writer->writeStartElement(QStringLiteral("parametre"));
  _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("theta"));
  _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("80"));
  _writer->writeEndElement();

  _writer->writeEndElement();
  _writer->writeEndElement();
}

void
XMLDocumentSaver::buildPage(Doc::Page *p, int id)
{
  if (!_isDIGIDOC) {
    _writer->writeStartElement(QStringLiteral("page"));
    _writer->writeAttribute(QStringLiteral("backgroundFileName"),
                            p->getBackgroundFileName());
    for (Doc::DocTextBlock *tb : p->getTextBlocks()) {
      buildTextBlock(tb);
    }
    for (Doc::DocImageBlock *ib : p->getImageBlocks()) {
      buildImageBlock(ib);
    }
    _writer->writeEndElement(); //</page>
  } else {
    _writer->writeStartElement(QStringLiteral("page"));
    _writer->writeAttribute(QStringLiteral("ID"), QString::number(id));
    _writer->writeAttribute(QStringLiteral("backgroundFileName"),
                            p->getBackgroundFileName());

    buildMediaAttributeOfPage();
    buildAcquisitionAttributeOfPage();
    buildBackgroundInfosOfPage();
    buildLayoutInfosOfPage();

    // Degradation model informations

    groundtruthGlobalModel();
    groundtruthLuminosityModel();
    /**/
    //    MapDegradationModels models = p->getListDegradationsModels();
    //    MapDegradationModels::const_iterator i = models.constBegin();
    //    while (i != models.constEnd()) {
    //        _writer->writeStartElement("degradationModel");
    //        _writer->writeAttribute("name", i.key());

    //        MapDegradationModelParameters paras = i.value();

    //        MapDegradationModelParameters::const_iterator j =
    //        paras.constBegin();
    //        while (j != paras.constEnd()) {
    //            _writer->writeAttribute(j.key(),j.value());
    //            ++j;
    //        }
    //        _writer->writeEndElement();
    //        ++i;
    //    }
    //
    /**/
    int idTextBloc = 0;
    for (Doc::DocTextBlock *tb : p->getTextBlocks()) {
      buildTextBlock(tb, idTextBloc);
      ++idTextBloc;
    }
    int idImageBloc = 0;
    for (Doc::DocImageBlock *ib : p->getImageBlocks()) {
      buildImageBlock(ib, idImageBloc);
      ++idImageBloc;
    }
    _writer->writeEndElement(); //</page>
    /**/
  }
}

void
XMLDocumentSaver::buildImageBlock(Doc::DocImageBlock *ib, int id)
{
  if (!_isDIGIDOC) {
    _writer->writeStartElement(QStringLiteral("imageBlock"));
    _writer->writeAttribute(QStringLiteral("x"), QString::number(ib->x()));
    _writer->writeAttribute(QStringLiteral("y"), QString::number(ib->y()));
    _writer->writeAttribute(QStringLiteral("width"),
                            QString::number(ib->width()));
    _writer->writeAttribute(QStringLiteral("height"),
                            QString::number(ib->height()));
    _writer->writeAttribute(QStringLiteral("filePath"), ib->filePath());
    _writer->writeEndElement(); //</imageBlock>
  } else {
    _writer->writeStartElement(QStringLiteral("EoC"));
    _writer->writeAttribute(QStringLiteral("ID"), QString::number(id));
    _writer->writeAttribute(QStringLiteral("type"),
                            QStringLiteral("imageBlock"));

    buildMediaAttributeOfEOCs();
    buildAcquisitionAttributeOfEOCs();
    buildPositionOfEOCs();
    buildShapeInfosOfEOCs(ib->x(), ib->y(), ib->width(), ib->height());

    _writer->writeStartElement(QStringLiteral("path"));
    _writer->writeAttribute(QStringLiteral("filePath"), ib->filePath());
    _writer->writeEndElement(); //path
    _writer->writeEndElement(); //</EoC>
  }
}

void
XMLDocumentSaver::buildTextBlock(Doc::DocTextBlock *tb, int id)
{
  if (!_isDIGIDOC) {
    _writer->writeStartElement(QStringLiteral("textBlock"));
    _writer->writeAttribute(QStringLiteral("x"), QString::number(tb->x()));
    _writer->writeAttribute(QStringLiteral("y"), QString::number(tb->y()));
    _writer->writeAttribute(QStringLiteral("width"),
                            QString::number(tb->width()));
    _writer->writeAttribute(QStringLiteral("height"),
                            QString::number(tb->height()));
    _writer->writeAttribute(QStringLiteral("marginTop"),
                            QString::number(tb->marginTop()));
    _writer->writeAttribute(QStringLiteral("marginBottom"),
                            QString::number(tb->marginBottom()));
    _writer->writeAttribute(QStringLiteral("marginLeft"),
                            QString::number(tb->marginLeft()));
    _writer->writeAttribute(QStringLiteral("marginRight"),
                            QString::number(tb->marginRight()));
    // re-order paragraphs by y-position
    std::map<int, Doc::DocParagraph *> listReOrdre;

    for (Doc::DocParagraph *p : tb->getParagraphs()) {
      listReOrdre.insert(std::pair<int, Doc::DocParagraph *>(
        p->getStrings().at(0)->getCharacters().at(0)->y(), p));
    }

    for (const auto &p : listReOrdre) {
      buildParagraph(p.second);
    }
    //        for (DocParagraph *p : tb->getParagraphs())
    //        {
    //            buildParagraph(p);
    //        }
    _writer->writeEndElement(); //</textBlock>

  } else {

    _writer->writeStartElement(QStringLiteral("EoC"));
    _writer->writeAttribute(QStringLiteral("ID"), QString::number(id));
    _writer->writeAttribute(QStringLiteral("type"),
                            QStringLiteral("textBlock"));

    _writer->writeAttribute(QStringLiteral("marginTop"),
                            QString::number(tb->marginTop()));
    _writer->writeAttribute(QStringLiteral("marginBottom"),
                            QString::number(tb->marginBottom()));
    _writer->writeAttribute(QStringLiteral("marginLeft"),
                            QString::number(tb->marginLeft()));
    _writer->writeAttribute(QStringLiteral("marginRight"),
                            QString::number(tb->marginRight()));
    buildMediaAttributeOfEOCs();
    buildAcquisitionAttributeOfEOCs();
    //    buildPositionOfEOCs();

    buildShapeInfosOfEOCs(tb->x(), tb->y(), tb->width(), tb->height());
    int idP = 0;
    for (Doc::DocParagraph *p : tb->getParagraphs()) {
      buildParagraph(p, idP);
      ++idP;
    }
    _writer->writeEndElement(); //</EoC>
  }
}
void
XMLDocumentSaver::buildParagraph(Doc::DocParagraph *p, int id)
{
  if (!_isDIGIDOC) {
    _writer->writeStartElement(QStringLiteral("paragraph"));
    _writer->writeAttribute(QStringLiteral("lineSpacing"),
                            QString::number(p->lineSpacing()));
    _writer->writeAttribute(QStringLiteral("tabulationSize"),
                            QString::number(p->tabulationSize()));
    QList<Doc::DocString *> strings = p->getStrings();
    strings
      .pop_back(); //On ne sauvegarde pas le caractre de retour de ligne ("\n")
    for (Doc::DocString *s : strings) {
      buildString(s);
    }
    _writer->writeEndElement(); //</paragraph>
  } else {
    _writer->writeStartElement(QStringLiteral("EoC"));
    _writer->writeAttribute(QStringLiteral("ID"), QString::number(id));
    _writer->writeAttribute(QStringLiteral("type"),
                            QStringLiteral("paragraph"));
    _writer->writeAttribute(QStringLiteral("lineSpacing"),
                            QString::number(p->lineSpacing()));
    _writer->writeAttribute(QStringLiteral("tabulationSize"),
                            QString::number(p->tabulationSize()));

    buildMediaAttributeOfEOCs();
    buildAcquisitionAttributeOfEOCs();
    //    buildPositionOfEOCs();
    buildShapeInfosOfEOCs(0, 0, 0, 0);

    QList<Doc::DocString *> strings = p->getStrings();
    int idS = 0;
    strings
      .pop_back(); //On ne sauvegarde pas le caractre de retour de ligne ("\n")
    for (Doc::DocString *s : strings) {
      buildString(s, idS);
      ++idS;
    }

    _writer->writeEndElement(); //</EoC>
  }
}

void
XMLDocumentSaver::buildString(Doc::DocString *s, int id)
{
  if (!_isDIGIDOC) {

    _writer->writeStartElement(QStringLiteral("string"));

    Doc::DocStyle *style = s->getStyle();
    _writer->writeAttribute(QStringLiteral("style"),
                            style != nullptr ? style->getName()
                                             : QStringLiteral("vesale"));
    //        if(style!=nullptr) {

    //            _writer->writeAttribute("style",style->getName());
    for (Doc::DocCharacter *c : s->getCharacters()) {
      buildCharacter(c);
    }
    _writer->writeEndElement(); //</string>
    //        }

  } else {
    _writer->writeStartElement(QStringLiteral("EoC"));
    _writer->writeAttribute(QStringLiteral("ID"), QString::number(id));
    _writer->writeAttribute(QStringLiteral("type"), QStringLiteral("string"));
    Doc::DocStyle *style = s->getStyle();
    _writer->writeAttribute(QStringLiteral("style"),
                            style != nullptr ? style->getName()
                                             : QStringLiteral("defaultStyle"));
    buildMediaAttributeOfEOCs();
    buildAcquisitionAttributeOfEOCs();
    //    buildPositionOfEOCs();
    buildShapeInfosOfEOCs(0, 0, 0, 0);
    int idC = 0;
    for (Doc::DocCharacter *c : s->getCharacters()) {
      buildCharacter(c, idC);
      ++idC;
    }
    _writer->writeEndElement(); //</EoC>
  }
}

void
XMLDocumentSaver::buildCharacter(Doc::DocCharacter *c, int id)
{
  if (!_isDIGIDOC) {
    _writer->writeStartElement(QStringLiteral("char"));
    _writer->writeAttribute(QStringLiteral("display"), c->getDisplay());
    _writer->writeAttribute(QStringLiteral("id"), QString::number(c->getId()));
    _writer->writeAttribute(QStringLiteral("x"), QString::number(c->x()));
    _writer->writeAttribute(QStringLiteral("y"), QString::number(c->y()));
    _writer->writeAttribute(QStringLiteral("width"),
                            QString::number(c->width()));
    _writer->writeAttribute(QStringLiteral("height"),
                            QString::number(c->height()));
    _writer->writeEndElement(); //</char>

  } else {
    _writer->writeStartElement(QStringLiteral("EoC"));
    _writer->writeAttribute(QStringLiteral("ID"), QString::number(id));
    _writer->writeAttribute(QStringLiteral("type"), QStringLiteral("char"));
    _writer->writeStartElement(QStringLiteral("groundtruth"));
    _writer->writeStartElement(QStringLiteral("model"));
    _writer->writeAttribute(QStringLiteral("name"),
                            QStringLiteral("grayscale character degradation"));
    _writer->writeAttribute(QStringLiteral("level"), QStringLiteral("local"));
    if (c->getDisplay() != QLatin1String("\n") &&
        c->getDisplay() != QLatin1String(" ")) {
      _writer->writeAttribute(QStringLiteral("degradationlevel"),
                              QString::number(0));

#ifdef STORE_SEEDPOINTS
      if ((c->getSeedPointsBF().size() + c->getSeedPointsFB().size()) > 0)
        _writer->writeAttribute(
          QStringLiteral("seedPoints"),
          QString::number((c->getSeedPointsFB()).size()) + "+" +
            QString::number((c->getSeedPointsBF()).size()));
#endif //STORE_SEEDPOINTS
    }
    _writer->writeStartElement(QStringLiteral("parametre"));
    _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("alpha"));
    _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("2"));
    _writer->writeEndElement();

    _writer->writeStartElement(QStringLiteral("parametre"));
    _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("beta"));
    _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("2"));
    _writer->writeEndElement();

    _writer->writeStartElement(QStringLiteral("parametre"));
    _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("a0"));
    _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("3.5"));
    _writer->writeEndElement();

    _writer->writeStartElement(QStringLiteral("parametre"));
    _writer->writeAttribute(QStringLiteral("name"), QStringLiteral("g"));
    _writer->writeAttribute(QStringLiteral("value"), QStringLiteral("0.6"));
    _writer->writeEndElement();

    _writer->writeEndElement();
    _writer->writeEndElement();
    _writer->writeStartElement(QStringLiteral("char"));
    _writer->writeAttribute(QStringLiteral("display"), c->getDisplay());
    _writer->writeAttribute(QStringLiteral("id"), QString::number(c->getId()));
    //    if(c->getDisplay() !="\n" && c->getDisplay()!=" ") {
    //        if((c->getSeedPointsBF().size()+c->getSeedPointsFB().size())>0)
    //        _writer->writeAttribute("seedPoints",
    //        QString::number((c->getSeedPointsFB()).size()) + "+" +
    //        QString::number((c->getSeedPointsBF()).size()));

    //        _writer->writeAttribute("avg",
    //        QString::number(c->getDegradationLevel()));}
    _writer->writeEndElement(); //</char>
    buildMediaAttributeOfEOCs();
    buildAcquisitionAttributeOfEOCs();
    buildPositionOfEOCs();
    buildShapeInfosOfEOCs(c->x(), c->y(), c->width(), c->height());

    _writer->writeEndElement(); //</EoC>
  }
}

} //namespace IOManager
