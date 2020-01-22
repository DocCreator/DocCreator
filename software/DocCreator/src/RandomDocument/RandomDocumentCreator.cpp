#include "RandomDocumentCreator.hpp"

#include <cassert>
//#define TIMING 1
#ifdef TIMING
#include <chrono> //DEBUG
#endif            //TIMING

#include <QCursor>
#include <QFile>
#include <QGuiApplication>
#include <QImageReader>

#include "context/CurrentRandomDocumentContext.h"
#include "context/backgroundcontext.h"
#include "context/documentcontext.h"
#include "context/fontcontext.h"
#include "core/configurationmanager.h"
#include "models/doc/page.h"
#include <Lipsum4Qt.hpp>

#include "RandomDocumentParameters.hpp"

#include "Document/DocumentController.hpp"
#include "Document/GridPageLayout.hpp"
#include "Utils/FontUtils.hpp" //computeBestLineSpacing
#include "Utils/RandomElement.hpp"

#include "appconstants.h"

RandomDocumentCreator::RandomDocumentCreator(
  DocumentController *ctrl,
  const RandomDocumentParameters &params,
  QObject *parent)
  : QObject(parent)
  , _ctrl(ctrl)
  , _params(params)
{}

static QString
loadText(const QString &filename)
{
  QString text;
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    text = "Unable to read text file: " + filename;
    std::cerr << "UNABLE TO LOAD text filename: " << filename.toStdString()
              << std::endl;
  } else {
    std::cout << "LOAD text filename: " << filename.toStdString() << std::endl;
    QTextStream in(&file);
    text =
      in.readAll(); //B:TODO:OPTIM: cache ? we re-read text file each time...
  }
  return text;
}

/*
  if (_params.lineSpacingType == RandomDocumentParameters::RandomLineSpacing), @a lineSpacing will not be used.

  if (! _params.textList.empty() && useRandomTextFile), @a textIndex will not be used.

 */

void
RandomDocumentCreator::create_aux(
  Context::CurrentRandomDocumentContext *randDoc,
  const QString &fontName,
  int lineSpacing,
  int textIndex,
  bool useRandomTextFile)
{
#ifdef TIMING
  auto t0 = std::chrono::steady_clock::now();
#endif //TIMING

  if (_params.lineSpacingType ==
      RandomDocumentParameters::FontHeightAdaptedLineSpacing) {
    Models::Font *font = Context::FontContext::instance()->getFont(fontName);
    assert(font);
    lineSpacing = computeBestLineSpacing(*font);
    randDoc->addProperty(QStringLiteral("lineSpacing"),
                         QString::number(lineSpacing));
  }

  auto document = new Doc::Document();

  auto style = new Doc::DocStyle(fontName, fontName);
  document->addStyle(style);
  //B:TODO: change style for each paragraph ? each page ? or only each document ? It coule be a parameter
  auto p = new Doc::Page(document);

  document->add(p);

  Context::DocumentContext::instance()->setCurrentDocument(document);
  assert(_ctrl);
  _ctrl->setDocument(document);

  Doc::Document *currentDoc = _ctrl->getDocument();
  assert(currentDoc == document);

  Doc::Page *currentPage = currentDoc->currentPage();
  assert(currentPage == p);

  const int col = RandomElement().randomInt(_params.nbBlocksPerColMin,
                                            _params.nbBlocksPerColMax);
  const int row = RandomElement().randomInt(_params.nbBlocksPerRowMin,
                                            _params.nbBlocksPerRowMax);

  randDoc->addProperty(QStringLiteral("nbColumn"), QString::number(col));
  randDoc->addProperty(QStringLiteral("nbRow"), QString::number(row));

  const int leftMargin =
    RandomElement().randomInt(_params.leftMarginMin, _params.leftMarginMax);
  const int rightMargin =
    RandomElement().randomInt(_params.rightMarginMin, _params.rightMarginMax);
  const int topMargin =
    RandomElement().randomInt(_params.topMarginMin, _params.topMarginMax);
  const int bottomMargin =
    RandomElement().randomInt(_params.bottomMarginMin, _params.bottomMarginMax);

  PageLayout *layout = new GridPageLayout(currentDoc, col, row);
  layout->setLeftMargin(leftMargin);
  randDoc->addProperty(QStringLiteral("leftMargin"),
                       QString::number(leftMargin)); //B: why french ?
  layout->setRightMargin(rightMargin);
  randDoc->addProperty(QStringLiteral("rightMargin"),
                       QString::number(rightMargin));
  layout->setTopMargin(topMargin);
  randDoc->addProperty(QStringLiteral("topMargin"), QString::number(topMargin));
  layout->setBottomMargin(bottomMargin);
  randDoc->addProperty(QStringLiteral("bottomMargin"),
                       QString::number(bottomMargin));

  const int nbBlocks = col * row;

  QString text;
  //If possible, we load the text here to avoid to have to load it for every block
  bool textLoaded = false;
  if (useRandomTextFile && _params.textList.size() == 1) {
    //always same text would be chosen (the first in the list), so we load it only once
    text = loadText(_params.textList.at(0));
    textLoaded = true;
  } else if (!useRandomTextFile && !_params.textList.empty() && nbBlocks == 1) {
    text = loadText(_params.textList.at(textIndex));
    textLoaded = true;
  }

  for (int i = 0; i < nbBlocks; ++i) {

    Doc::DocTextBlock *textBlock = layout->newTextBlock(i);
    assert(textBlock != nullptr);

    //std::cerr<<"RDC: block "<<i<<": x="<<textBlock->x()<<" y="<<textBlock->y()<<" w="<<textBlock->width()<<" h="<<textBlock->height()<<"\n";

    currentPage->add(textBlock);
    //currentPage->setCurrentBlock(textBlock);
    _ctrl->resetCurrentTextBlockCursor();

    _ctrl->setModified();

    //std::cerr<<"RDC: _ctrl->update()"<<std::endl;
    //_ctrl->update();

    auto para = new Doc::DocParagraph(currentDoc);
    currentDoc->add(para);
    Doc::DocParagraph *currentParagraph =
      currentDoc
        ->currentParagraph(); //B:TODO: why call currentParagraph() when we alreay have para ?
    currentParagraph->setLineSpacing(lineSpacing);

    if (RandomElement().randomInt(1, 100) <=
        100 - _params.percentOfEmptyBlocks) {

      if (!textLoaded) {
        if (_params.textList.empty()) {
          //use random text generator
          Lipsum4Qt lipsumGenerator(
            Lipsum4Qt::
              SYNC); //B: we have to recreate this objet each time to get a new text !
          text = lipsumGenerator.Lipsum;
        } else {
          if (useRandomTextFile) {
            const int index =
              RandomElement().randomInt(0, _params.textList.size() - 1);
            assert(index >= 0 && index < _params.textList.size());
            text = loadText(_params.textList.at(index));
          } else {
            const int index =
              (textIndex + i) %
              _params.textList
                .size(); //B: we want to use a different text file for each block !
            assert(index >= 0 && index < _params.textList.size());
            text = loadText(_params.textList.at(index));
          }
        }
      }

      //add "text" in the area
      _ctrl->addString(text);
    }
  }
  _ctrl->setModified();

  _ctrl->update();

  //B: we do not set the image in CurrentRandomDocumentContext
  //   it will be done by RandomDocumentExporter if connected...
  //randDoc->setImage(_ctrl->toQImage(Color | WithTextBlocks | WithImageBlocks));

  //qDebug() << metaObject()->className() << ":::" << "READY";

#ifdef TIMING
  auto t1 = std::chrono::steady_clock::now();
  auto time1 = t1 - t0;
  std::cerr << "RandomDocumentCreator::create_aux() time="
            << std::chrono::duration<double, std::milli>(time1).count()
            << "ms\n";
#endif //TIMING

  emit imageReady();

  Context::DocumentContext::instance()->setCurrentDocument(nullptr);
  _ctrl->setDocument(nullptr);

  //delete randDoc;
  //delete QImage ???
  delete document;
  delete layout;

  qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

static void
saveFonts(QString &prevCurrFontName, QList<Models::Font *> &prevFonts)
{
  prevCurrFontName = Context::FontContext::instance()->getCurrentFontName();
  QStringList prevFontList = Context::FontContext::instance()->getFontNames();
  for (const QString &fontName : prevFontList) {
    prevFonts.push_back(Context::FontContext::instance()->getFont(fontName));
    Context::FontContext::instance()->removeFont(fontName);
  }
}

static void
restoreFonts(const QString &prevCurrFontName,
             const QList<Models::Font *> &prevFonts)
{
  for (Models::Font *f : prevFonts) {
    Context::FontContext::instance()->addFont(f);
  }
  if (!prevCurrFontName.isEmpty()) {
    Context::FontContext::instance()->setCurrentFont(prevCurrFontName);
  }
}

static std::pair<int, int>
saveViewSize()
{
  const int w =
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigPageSizeX)
      .toInt();
  const int h =
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigPageSizeY)
      .toInt();
  return std::pair<int, int>(w, h);
}

static void
setViewSize(int w, int h)
{
  Core::ConfigurationManager::set(AppConfigMainGroup, AppConfigPageSizeX, w);
  Core::ConfigurationManager::set(AppConfigMainGroup, AppConfigPageSizeY, h);
}

static void
restoreViewSize(std::pair<int, int> wh)
{
  setViewSize(wh.first, wh.second);
}

static void
adaptViewSizeToBackgroundSize()
{
  //Here, we just need the image size. We do not want to load it completely.

  const QString backgroundFilename =
    Context::BackgroundContext::instance()->getPath() +
    Context::BackgroundContext::instance()->getCurrentBackground();
  const QImageReader rdr(backgroundFilename);
  const int w = rdr.size().width();
  const int h = rdr.size().height();

  setViewSize(w, h);
}

/*
  Create N documents with one random font, background and text among those available in _params.
 */
void
RandomDocumentCreator::create()
{
  if (_params.nbPages > 0 && !_params.fontList.isEmpty()) {

    QGuiApplication::setOverrideCursor(Qt::BusyCursor);

    assert(_params.percentOfEmptyBlocks >= 0 &&
           _params.percentOfEmptyBlocks <= 100);

    const std::pair<int, int> wh = saveViewSize();
    if (_params.imageSizeUniform) {
      setViewSize(_params.imageWidth, _params.imageHeight);
    }

    assert(_ctrl->getDocument() != nullptr);

    Doc::Document *previousDoc = _ctrl->getDocument();

    Context::CurrentRandomDocumentContext *randDoc =
      Context::CurrentRandomDocumentContext::instance();

    //RandomElement elem;

    int lineSpacing = 0;
    if (_params.lineSpacingType ==
        RandomDocumentParameters::RandomLineSpacing) {
      lineSpacing = RandomElement().randomInt(_params.lineSpacingMin,
                                              _params.lineSpacingMax);
      std::cerr << "RandomDocumentCreator::create() randomLineSpacing="
                << lineSpacing << " in [" << _params.lineSpacingMin << "; "
                << _params.lineSpacingMax << "]\n";
      randDoc->addProperty(QStringLiteral("lineSpacing"),
                           QString::number(lineSpacing));
    }

    //save previous fonts & load selected fonts
    QString prevCurrFontName;
    QList<Models::Font *> prevFonts;
    saveFonts(prevCurrFontName, prevFonts);

    Context::FontContext::instance()->addFontsFromXmlFiles(_params.fontList);
    QStringList fontList =
      Context::FontContext::instance()->getFontNames();
    //may be different than _params.fontList
    assert(fontList.size() > 0);
    Context::FontContext::instance()->setCurrentFont(fontList.back());

    //B: should we have _params.nbDocs && _params.nbPages ???
    // Is it possible to render several images (pages) in the same document !
    //TODO: for now we consider that each document contains only one page
    const int nbPages = _params.nbPages;

    std::cerr << "###### will create " << nbPages << " documents\n";

    const int unused = 0;
    const bool useRandomTextFile = (!_params.textList.empty());

    for (int k = 0; k < nbPages; ++k) {

      std::cerr << "øøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøøø$ Document "
                << k << " start\n";

      //auto document = new Doc::Document();

      //set random background if available
      if (!_params.backgroundList.empty()) {
        const int index =
          RandomElement().randomInt(0, _params.backgroundList.size() - 1);
        const QString &backgroundName = _params.backgroundList.at(index);
        Context::BackgroundContext::instance()->setCurrentBackground(
          backgroundName);

        randDoc->addProperty(QStringLiteral("background"), backgroundName);

        if (!_params.imageSizeUniform) {
          adaptViewSizeToBackgroundSize();
        }
      }

      //choose one font randomly
      const int fontIndex = RandomElement().randomInt(0, fontList.count() - 1);
      assert(fontIndex < fontList.count());
      const QString &fontName = fontList.at(fontIndex);
      //std::cout << "FONT LOAD: " << fontName.toStdString() << std::endl;

      create_aux(randDoc, fontName, lineSpacing, unused, useRandomTextFile);
    }

    //QString fontpath = Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigFontFolderKey).toString();
    //QString fontext = Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigFontExtKey).toString();
    //Context::FontContext::instance()->initialize(fontpath, fontext);

    Context::DocumentContext::instance()->setCurrentDocument(nullptr);
    _ctrl->setDocument(nullptr);

    for (const QString &fontName : fontList) {
      Context::FontContext::instance()->removeFont(fontName);
    }

    restoreFonts(prevCurrFontName, prevFonts);

    restoreViewSize(wh);

    Context::DocumentContext::instance()->setCurrentDocument(previousDoc);
    _ctrl->setDocument(previousDoc);

    QGuiApplication::restoreOverrideCursor();
  }
}

/*
  Create 1*1*T documents with the 1 random font, 1 random background and T texts available in _params.

  _params.nbPages is not used.
 */
void
RandomDocumentCreator::createAllTextsOneFontBackground()
{
  if (_params.nbPages > 0 && !_params.fontList.isEmpty()) {

    QGuiApplication::setOverrideCursor(Qt::BusyCursor);

    assert(_params.percentOfEmptyBlocks >= 0 &&
           _params.percentOfEmptyBlocks <= 100);

    const std::pair<int, int> wh = saveViewSize();
    if (_params.imageSizeUniform) {
      setViewSize(_params.imageWidth, _params.imageHeight);
    }

    assert(_ctrl->getDocument() != nullptr);

    Doc::Document *previousDoc = _ctrl->getDocument();

    Context::CurrentRandomDocumentContext *randDoc =
      Context::CurrentRandomDocumentContext::instance();

    int lineSpacing = 0;
    if (_params.lineSpacingType ==
        RandomDocumentParameters::RandomLineSpacing) {
      lineSpacing = RandomElement().randomInt(_params.lineSpacingMin,
                                              _params.lineSpacingMax);
      std::cerr << "RandomDocumentCreator::create() randomLineSpacing="
                << lineSpacing << " in [" << _params.lineSpacingMin << "; "
                << _params.lineSpacingMax << "]\n";
      randDoc->addProperty(QStringLiteral("lineSpacing"),
                           QString::number(lineSpacing));
    }

    //save previous fonts & load selected fonts
    QString prevCurrFontName;
    QList<Models::Font *> prevFonts;
    saveFonts(prevCurrFontName, prevFonts);

    Context::FontContext::instance()->addFontsFromXmlFiles(_params.fontList);
    QStringList fontList =
      Context::FontContext::instance()
        ->getFontNames(); //may be different than _params.fontList
    if (Context::FontContext::instance()->getCurrentFont() == nullptr
	&& ! fontList.isEmpty())
      Context::FontContext::instance()->setCurrentFont(fontList.back());


    //std::cerr<<"++++ fontList.size()="<<fontList.size()<<"\n";
    //std::cerr<<"_params.backgroundList.size()="<<_params.backgroundList.size()<<"\n";
    //std::cerr<<"++++ currentFont="<<Context::FontContext::instance()->getCurrentFont()<<"\n";

    //set random background if available
    if (_params.backgroundList.size() == 1) {
      const QString &backgroundName = _params.backgroundList.at(0);
      Context::BackgroundContext::instance()->setCurrentBackground(
        backgroundName);
      randDoc->addProperty(QStringLiteral("background"), backgroundName);

      if (!_params.imageSizeUniform) {
        adaptViewSizeToBackgroundSize();
      }
    }

    assert(!_params.fontList.isEmpty());
    QString fontName = fontList.at(0);

    if (!_params.textList.empty()) {
      for (int textIndex = 0; textIndex < _params.textList.size();
           ++textIndex) {

        if (_params.fontList.size() > 1) {
          const int fontIndex =
            RandomElement().randomInt(0, _params.fontList.size() - 1);
          fontName = fontList.at(fontIndex);
        }
        if (_params.backgroundList.size() > 1) {
          const int index =
            RandomElement().randomInt(0, _params.backgroundList.size() - 1);
          const QString &backgroundName = _params.backgroundList.at(index);
          Context::BackgroundContext::instance()->setCurrentBackground(
            backgroundName);
          randDoc->addProperty(QStringLiteral("background"), backgroundName);

          if (!_params.imageSizeUniform) {
            adaptViewSizeToBackgroundSize();
          }
        }

        const bool useRandomTextFile = false;
        create_aux(
          randDoc, fontName, lineSpacing, textIndex, useRandomTextFile);
      }
    } else {
      if (_params.fontList.size() > 1) {
        const int fontIndex =
          RandomElement().randomInt(0, _params.fontList.size() - 1);
        fontName = fontList.at(fontIndex);
      }
      if (_params.backgroundList.size() > 1) {
        const int index =
          RandomElement().randomInt(0, _params.backgroundList.size() - 1);
        const QString &backgroundName = _params.backgroundList.at(index);
        Context::BackgroundContext::instance()->setCurrentBackground(
          backgroundName);
        randDoc->addProperty(QStringLiteral("background"), backgroundName);

        if (!_params.imageSizeUniform) {
          adaptViewSizeToBackgroundSize();
        }
      }

      const bool useRandomTextFile = true;
      create_aux(randDoc, fontName, lineSpacing, 0, useRandomTextFile);
    }

    Context::DocumentContext::instance()->setCurrentDocument(nullptr);
    _ctrl->setDocument(nullptr);

    for (const QString &font : fontList) {
      Context::FontContext::instance()->removeFont(font);
    }

    restoreFonts(prevCurrFontName, prevFonts);

    restoreViewSize(wh);

    Context::DocumentContext::instance()->setCurrentDocument(previousDoc);
    _ctrl->setDocument(previousDoc);

    QGuiApplication::restoreOverrideCursor();
  }
}

/*
  Create F*B*T documents with the F fonts, B backgrounds and T texts available in _params.

  _params.nbPages is not used.
 */
void
RandomDocumentCreator::createAllTexts()
{
  if (_params.nbPages > 0 && !_params.fontList.isEmpty()) {

    QGuiApplication::setOverrideCursor(Qt::BusyCursor);

    assert(_params.percentOfEmptyBlocks >= 0 &&
           _params.percentOfEmptyBlocks <= 100);

    const std::pair<int, int> wh = saveViewSize();
    if (_params.imageSizeUniform) {
      setViewSize(_params.imageWidth, _params.imageHeight);
    }

    assert(_ctrl->getDocument() != nullptr);

    Doc::Document *previousDoc = _ctrl->getDocument();

    Context::CurrentRandomDocumentContext *randDoc =
      Context::CurrentRandomDocumentContext::instance();

    int lineSpacing = 0;
    if (_params.lineSpacingType ==
        RandomDocumentParameters::RandomLineSpacing) {
      lineSpacing = RandomElement().randomInt(_params.lineSpacingMin,
                                              _params.lineSpacingMax);
      std::cerr << "RandomDocumentCreator::create() randomLineSpacing="
                << lineSpacing << " in [" << _params.lineSpacingMin << "; "
                << _params.lineSpacingMax << "]\n";
      randDoc->addProperty(QStringLiteral("lineSpacing"),
                           QString::number(lineSpacing));
    }

    //save previous fonts & load selected fonts
    QString prevCurrFontName;
    QList<Models::Font *> prevFonts;
    saveFonts(prevCurrFontName, prevFonts);

    Context::FontContext::instance()->addFontsFromXmlFiles(_params.fontList);
    QStringList fontList =
      Context::FontContext::instance()
        ->getFontNames(); //may be different than _params.fontList

    //std::cerr<<"fontList.size()="<<fontList.size()<<"\n";
    //std::cerr<<"_params.backgroundList.size()="<<_params.backgroundList.size()<<"\n";

    /*
    //set random background if available
    if (! _params.backgroundList.empty()) {
      const int index = RandomElement().randomInt(0, _params.backgroundList.size()-1);
      const QString &backgroundName = _params.backgroundList.at(index);
      Context::BackgroundContext::instance()->setCurrentBackground(backgroundName);

      randDoc->addProperty("background", backgroundName);
    }
    */

    if (!_params.backgroundList.empty()) {

      //OPTIM: background change must be the outside loop (as loading background images is quite costly...)
      for (int backgroundIndex = 0;
           backgroundIndex < _params.backgroundList.size();
           ++backgroundIndex) {

        const QString &backgroundName =
          _params.backgroundList.at(backgroundIndex);
        Context::BackgroundContext::instance()->setCurrentBackground(
          backgroundName);
        randDoc->addProperty(QStringLiteral("background"), backgroundName);

        if (!_params.imageSizeUniform) {
          adaptViewSizeToBackgroundSize();
        }

        for (int fontIndex = 0; fontIndex < fontList.size(); ++fontIndex) {

          const QString &fontName = fontList.at(fontIndex);

          if (!_params.textList.empty()) {
            for (int textIndex = 0; textIndex < _params.textList.size();
                 ++textIndex) {

              const bool useRandomTextFile = false;
              create_aux(
                randDoc, fontName, lineSpacing, textIndex, useRandomTextFile);
            }
          } else {
            //must generate one random text with Lipsum4Qt
            const bool useRandomTextFile = true;
            create_aux(randDoc, fontName, lineSpacing, 0, useRandomTextFile);
          }
        }
      }
    } else {
      //background not changed

      for (int fontIndex = 0; fontIndex < fontList.size(); ++fontIndex) {

        const QString &fontName = fontList.at(fontIndex);

        if (!_params.textList.empty()) {
          for (int textIndex = 0; textIndex < _params.textList.size();
               ++textIndex) {

            const bool useRandomTextFile = false;
            create_aux(
              randDoc, fontName, lineSpacing, textIndex, useRandomTextFile);
          }
        } else {
          //must generate one random text with Lipsum4Qt
          const bool useRandomTextFile = true;
          create_aux(randDoc, fontName, lineSpacing, 0, useRandomTextFile);
        }
      }
    }

    Context::DocumentContext::instance()->setCurrentDocument(nullptr);
    _ctrl->setDocument(nullptr);

    for (const QString &fontName : fontList) {
      Context::FontContext::instance()->removeFont(fontName);
    }

    restoreFonts(prevCurrFontName, prevFonts);

    restoreViewSize(wh);

    Context::DocumentContext::instance()->setCurrentDocument(previousDoc);
    _ctrl->setDocument(previousDoc);

    QGuiApplication::restoreOverrideCursor();
  }
}
