#include "ImageGenerationFromDirDialog.hpp"

#include "ui_ImageGenerationFromDirDialog.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <vector>

#include <opencv2/core/core.hpp>

#include <QDebug>
#include <QDir>
#include <QFileDialog>

#include "Degradations/GrayCharacterDegradationModelQ.hpp"
#include "Document/DocumentController.hpp"
#include "context/backgroundcontext.h"
#include "context/documentcontext.h"
#include "context/fontcontext.h"
#include "iomanager/documentfilemanager.h"
#include "models/character.h"
#include "models/characterdata.h"
#include "models/doc/doctextblock.h"
#include "models/doc/document.h"
#include "models/doc/page.h"
#include "models/font.h"

ImageGenerationFromDirDialog::ImageGenerationFromDirDialog(
  QWidget *parent,
  DocumentController *docController)
  : QDialog(parent)
  , ui(new Ui::ImageGenerationFromDirDialog)
{
  _docController = docController;
  ui->setupUi(this);
  createFontSelector();
  createBackgroundSelector();

  connect(ui->btnBrowserTextDir,
          SIGNAL(clicked()),
          this,
          SLOT(chooseTextDirectory()));

  this->ui->progressBar->hide();

  connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(generate()));

  connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(accepted()));
}

ImageGenerationFromDirDialog::~ImageGenerationFromDirDialog()
{
  delete ui;
}

void
ImageGenerationFromDirDialog::accepted()
{
  this->done(1);
}

void
ImageGenerationFromDirDialog::degrade(
  QDir folder)
{

  QStringList filters;
  filters << QStringLiteral("*.png"); //B: why only this file format ???
  folder.setNameFilters(filters);
  QStringList listFileNames = folder.entryList();

  const int level = this->ui->hsSliderLevel->value();

  //int pageCounter = 1;
  for (const QString &f : listFileNames) {

    QString filePath = folder.absoluteFilePath(f);
    QImage input(filePath);

    dc::GrayscaleCharsDegradationModelQ cdg(input);
    QImage out = cdg.degradateByLevel(level);

    QString fileImageOut = filePath;
    QString str_degradation_level =
      "_degradation_level_" +
      QString::number(this->ui->hsSliderLevel->value()) + ".png";
    fileImageOut.replace(QStringLiteral(".png"), str_degradation_level);

    out.save(fileImageOut);

    //++pageCounter;
  }
}

QStringList
ImageGenerationFromDirDialog::fileSplit(const QString &filePath)
{

  int pageMarginTop = 100;
  int pageMarginLeft = 100;
  int pageMarginRight = 100;
  int pageMarginBottom = 150;
  int columnSpacing = 10;
  // background
  QString bgName = this->ui->cbBackground->currentText();
  QString bgPath = Context::BackgroundContext::instance()->getPath() + bgName;
  _bg = QImage(bgPath);

  int width = _bg.width();
  int height = _bg.height();
  // Font
  QString fontName = this->ui->cbFontType->currentText();
  _font = Context::FontContext::instance()->getFont(fontName);

  // Document layout
  int nbColumns = this->ui->cbColumn->value();

  int columnsWidth = (width - pageMarginLeft - pageMarginRight -
                      columnSpacing * (nbColumns - 1)) /
                     nbColumns;
  int columnsHeight = (height - pageMarginTop - pageMarginBottom);

  std::string line;
  std::ifstream file(filePath.toStdString().c_str());

  const int normalBaseLine = 100;
  int detaMaxBaseLine = 0;
  int meanCharHeight = 0;
  int maxCharHeight = 0;
  size_t charCounter = 0;
  int totalLineLong = 0;
  int maxLineLong = 0;

  QVector<std::string> lines;
  if (file.is_open()) {
    while (file.good()) {
      getline(file, line);

      const size_t lineSize = line.size();
      int lineLong = 0;
      charCounter += lineSize;

      lines.push_back(line);

      for (size_t i = 0; i < lineSize; ++i) {

        QString charDisplay(line.at(i));
        const Models::Character *character = _font->getCharacter(charDisplay);

        if (character != nullptr &&
            !charDisplay.isEmpty()) { // if the character exist in the font, get
                                      // this character.
          if (detaMaxBaseLine < fabs(normalBaseLine - character->getBaseLine()))
            detaMaxBaseLine = fabs(normalBaseLine - character->getBaseLine());

          const Models::CharacterData *charData =
            character->getRandomCharacterData();
          const QImage charImage = charData->getImage();

          if (maxCharHeight < charImage.height())
            maxCharHeight = charImage.height();

          meanCharHeight += charImage.height();
          lineLong += charImage.width();
        }
      }
      if (maxLineLong < lineLong)
        maxLineLong = lineLong;

      totalLineLong += lineLong;
    }
  }
  file.close();
  // check if height of all paragraph > columns height
  assert(charCounter > 0); //B
  meanCharHeight =
    meanCharHeight / charCounter; //B: do we really want integer division ???
  const int lineSpacing = (maxCharHeight * 1.2f); //B: implicit cast ???

  if (maxLineLong < columnsWidth) { // re-form background and columns width
    //        int delta = columnsWidth - maxLineLong;
    columnsWidth = maxLineLong + 100;
    //        width = width - nbColumns*(delta - 100);
  }

  const int nbLines = totalLineLong / columnsWidth + 1;
  int heightOfAllLines = nbLines * (lineSpacing + meanCharHeight);

  if (heightOfAllLines < columnsHeight) {
    height = heightOfAllLines + 100 + pageMarginTop + pageMarginBottom;
    columnsHeight = (height - pageMarginTop - pageMarginBottom);
  }

  int nbNeededPages = heightOfAllLines / (columnsHeight * nbColumns) + 1;
  // split process
  int linesPerPage = lines.size() / nbNeededPages;
  int pageCounter = 0;
  int lineStart = 0;
  int pagesPerFile = 2;
  qDebug() << nbNeededPages << " total lines = " << lines.size();

  QStringList listFilesOut;
  if (nbNeededPages > pagesPerFile) {
    while (lineStart < lines.size()) {
      QVector<std::string> f;
      if ((lineStart + pagesPerFile * linesPerPage) < lines.size())
        f = lines.mid(lineStart, pagesPerFile * linesPerPage);
      else
        f = lines.mid(lineStart, lines.size() - lineStart);
      lineStart += pagesPerFile * linesPerPage;

      //write to file
      ++pageCounter;
      std::ofstream openFileOut;
      QString fileOut = filePath;
      fileOut.replace(QStringLiteral(".txt"),
                      "_p" + QString::number(pageCounter) + ".txt");
      listFilesOut.push_back(fileOut);
      openFileOut.open(fileOut.toStdString().c_str());
      if (openFileOut.is_open()) {
        for (const std::string &l : f)
          openFileOut << l << std::endl;
      }
      openFileOut.close();
    }
  }
  lines.clear();

  return listFilesOut;
}

void
ImageGenerationFromDirDialog::generate()
{

  this->ui->progressBar->show();

  QString txtTextDir = this->ui->txtTextDirPath->text();

  if (txtTextDir.length() != 0 && !txtTextDir.isEmpty()) {

    QDir folder(this->ui->txtTextDirPath->text());

    QStringList filters;                // filtre : only *.txt
    filters << QStringLiteral("*.txt"); //
    folder.setNameFilters(filters);     //

    QStringList listFileNames = folder.entryList(); // get files from folder

    //        QStringList listFilesTmp;
    //        QStringList listBigFiles;
    //        if (listFileNames.size()>0)
    //            for (const QString &file : listFileNames) {
    //                QString filePath = folder.absoluteFilePath(file);
    //                // split big files
    //                listFilesTmp = fileSplit(filePath);
    //                if (listFilesTmp.size()>0) listBigFiles.push_back(file);
    //            }

    //        // reload list files
    //        listFileNames.clear();
    //        folder.refresh();
    //        listFileNames = folder.entryList();
    //        // remove big files
    //        for (const QString &bigFile : listBigFiles)
    //        listFileNames.removeOne(bigFile);
    //        listBigFiles.clear();

    int per = 100 / listFileNames.size();
    if (!listFileNames.empty())
      for (const QString &file : listFileNames) {
        QString filePath = folder.absoluteFilePath(file);

        qDebug() << file << " : build XML";

        QStringList listXMLs =
          createDocument(filePath); // create XMLs for each page

        //QStringList listXMLs = createNewBigDocument(filePath);

        buildImagesFromXML(listXMLs); // build an image for each page

        qDebug() << file << " : degradation";

        //degrade(folder, file);                                        // degrade this image

        this->ui->progressBar->setValue(per);
        this->ui->progressBar->update();
        per += 100 / listFileNames.size();

        listXMLs.clear();
      }
  }
  this->close();
}

void
ImageGenerationFromDirDialog::buildImagesFromXML(const QStringList &xmls)
{
  for (const QString &file : xmls) {
    buildImage(file);
    //xmls.removeOne(file); //B: was wrong ????????????
  }
}

QImage
ImageGenerationFromDirDialog::buildImage(const QString &file)
{

  Doc::Document *document =
    IOManager::DocumentFileManager::documentFromXml(file);

  QImage out;

  if (_Char_Format < 7 && _Char_Format >= 1) {
    out = _bg.convertToFormat(_Char_Format);
  }
  else {
    out = _bg.convertToFormat(QImage::Format_ARGB32);
  }

  if (document != nullptr && !document->getPages().empty())
    for (Doc::Page *page : document->getPages()) {

      if (!page->getTextBlocks().empty())
        for (Doc::DocTextBlock *tb : page->getTextBlocks()) {
          const int tb_posX = tb->x();
          int tb_posY = tb->y(); // adjust the text block position

          int counter_lines = 0;
          int checker_width = 0;

          if (!tb->getParagraphs().empty()) {
            tb_posY += tb->getParagraphs().at(0)->lineSpacing() /
                       3; // adjust the text block position

            for (Doc::DocParagraph *p : tb->getParagraphs()) {
              int wo = 0, ho = 0;

              if (!p->getStrings().empty()) {
                for (Doc::DocString *s : p->getStrings()) {
                  if (!s->getCharacters().empty())
                    for (Doc::DocCharacter *ch : s->getCharacters()) {
                      const Models::Character *character =
                        _font->getCharacter(ch->getDisplay());
                      const QString chDisplay = ch->getDisplay();
                      if (character == nullptr ||
                          chDisplay == QStringLiteral(" ") ||
                          chDisplay.isEmpty())
                        continue;

                      const Models::CharacterData *charData =
                        character->getCharacterData(ch->getId());
                      const QImage charImage = charData->getImage();

                      wo = charImage.width();
                      ho = charImage.height();
                      if (wo > 0 && ho > 0)
                        break;
                    }

                  if (wo > 0 && ho > 0)
                    break;
                }

                for (Doc::DocString *s : p->getStrings()) {

                  if (!s->getCharacters().empty())
                    for (Doc::DocCharacter *c : s->getCharacters()) {
                      int posX = c->x();

                      if (checker_width > c->x())
                        ++counter_lines; // count the number of lines

                      int posY = c->y() - counter_lines * p->lineSpacing() /
                                            2; // adjust the character position

                      const QString cDisplay = c->getDisplay();
                      const Models::Character *character =
                        _font->getCharacter(cDisplay);

                      if (character != nullptr && !cDisplay.isEmpty()) {
                        const Models::CharacterData *charData =
                          character->getCharacterData(c->getId());
                        const QImage charImage = charData->getImage();

                        if (ho > 0)
                          posY += ho - charImage.height();
                        posY += (1 - character->getBaseLine() / (float)100) *
                                charImage.height();

                        /** DEBUG **
                            if (id ==2) qDebug() << c->getDisplay() << " char w " << charImage->width() << " h " << charImage->height() <<
                            " char x y "  << posX << " " << posY <<
                            " tb x y " << tb_posX << " " << tb_posY <<
                            " out w h " << out.width() << " " << out.height() <<
                            " total x y " << (charImage->width() + posX + tb_posX) << " " << (charImage->height() + posY + tb_posY);
                        */

                        //B:TODO:OPTIM: remove checks from inside of loops !
                        //B:TODO:OPTIM: do not use pixel()/setPixel() !
                        const int w = charImage.width();
                        const int h = charImage.height();
                        for (int y = 0; y < h; ++y) {
                          for (int x = 0; x < w; ++x) {
                            const int alpha = qAlpha(charImage.pixel(x, y));
                            if ((x + tb_posX + posX) < out.width() &&
                                (y + tb_posY + posY) < out.height()) {
                              if (alpha != 0) {
                                out.setPixel(x + tb_posX + posX,
                                             y + tb_posY + posY,
                                             charImage.pixel(x, y));
                              }
                            }
                          }
                        }
                      }
                      checker_width = c->x();
                    }
                }
              }
            }
          }
        }

      out.save(file + ".png");
    }

  delete document;

  //out = QImage();
  return out;
}

void
ImageGenerationFromDirDialog::createNewDocument(const QString &filePath)
{
  int pageMarginTop = 100; //B: Why are these values hard-coded ???
  int pageMarginLeft = 100;
  int pageMarginRight = 100;
  int pageMarginBottom = 150;
  int columnSpacing = 10;
  // background
  loadBackground();
  int width = _bg.width();
  int height = _bg.height();
  // Font
  QString fontName = this->ui->cbFontType->currentText();
  _font = Context::FontContext::instance()->getFont(fontName);

  // Document layout
  int nbColumns = this->ui->cbColumn->value();

  int columnsWidth = (width - pageMarginLeft - pageMarginRight -
                      columnSpacing * (nbColumns - 1)) /
                     nbColumns;
  int columnsHeight = (height - pageMarginTop - pageMarginBottom);
  // create new document
  auto document = new Doc::Document();
  Context::DocumentContext::instance()->setCurrentDocument(document);
  if (_docController == nullptr) {
    qDebug() << "ERROR: file not saved because _docController is NULL: "
             << filePath;
    return; //B
  }
  _docController->setDocument(document);

  // add font style
  auto style = new Doc::DocStyle(fontName);
  document->addStyle(style);
  // read string line from txt file
  std::string line;
  std::ifstream file(filePath.toStdString().c_str());
  QList<LineText *> text;
  const int normalBaseLine = 100;
  int detaMaxBaseLine = 0;
  int meanCharHeight = 0;
  int maxCharHeight = 0;
  int itemCounter = 0;
  int totalLineLong = 0;
  int maxLineLong = 0;

  if (file.is_open()) {
    while (file.good()) {
      getline(file, line);
      LineText *tmpLine = new LineText;
      tmpLine->Height = 0;
      tmpLine->Long = 0;
      tmpLine->meanCharHeight = 0;
      int lineLong = 0;
      const size_t lineSize = line.size();
      itemCounter += lineSize;

      for (size_t i = 0; i < lineSize; ++i) {

        QString charDisplay(line.at(i));
        const Models::Character *character = _font->getCharacter(charDisplay);

        if (character != nullptr &&
            !charDisplay.isEmpty()) { // if the character exist in the font, get
                                      // this character.
          if (detaMaxBaseLine < fabs(normalBaseLine - character->getBaseLine()))
            detaMaxBaseLine = fabs(normalBaseLine - character->getBaseLine());

          const Models::CharacterData *charData =
            character->getRandomCharacterData();
          const QImage charImage = charData->getImage();

          const int h = charImage.height();
          if (maxCharHeight < h)
            maxCharHeight = h;

          meanCharHeight += h;
          tmpLine->meanCharHeight += h;
          lineLong += charImage.width();
          tmpLine->charList.push_back(charDisplay);
        }
      }
      if (maxLineLong < lineLong)
        maxLineLong = lineLong;
      tmpLine->Long = lineLong;
      tmpLine->meanCharHeight = tmpLine->meanCharHeight / line.size();

      totalLineLong += lineLong;
      text.push_back(tmpLine);
    }
  }
  file.close();
  // check if height of all paragraph > columns height
  assert(itemCounter > 0); //B
  meanCharHeight = meanCharHeight / itemCounter;
  int lineSpacing = (maxCharHeight * 1.2f);

  // check fitted column width with text input
  if (maxLineLong < columnsWidth) { // re-form background and columns width

    int delta = columnsWidth - maxLineLong;
    columnsWidth = maxLineLong + 100;
    _bg = _bg.scaled(width - nbColumns * (delta - 100), height);
    width = _bg.width();
  }

  const int nbLines = totalLineLong / columnsWidth + 1;
  const int heightOfAllLines = nbLines * (lineSpacing + meanCharHeight);
  // check fitted column height with text input
  qDebug() << "max height of text = " << heightOfAllLines << " height "
           << columnsHeight;

  if (heightOfAllLines < columnsHeight) {
    _bg = _bg.scaled(width,
                     heightOfAllLines + 100 + pageMarginTop + pageMarginBottom);
    height = _bg.height();

    columnsHeight = (height - pageMarginTop - pageMarginBottom);
  }
  //    qDebug() << " bg w h " << _bg.width() << " " << _bg.height();
  const int nbNeededPages = heightOfAllLines / (columnsHeight * nbColumns) + 1;

  qDebug() << "Create page nb of pages = " << nbNeededPages
           << " nb of lines = " << nbLines << " linespacing =" << lineSpacing
           << " mean char height = " << meanCharHeight << " nbcol "
           << nbColumns;

  // add the first page
  auto page = new Doc::Page(document);
  page->setBackgroundFileName(_bgName);
  document->add(page);
  document->setPageWidth(_bg.width());
  document->setPageHeight(_bg.height());
  document->setCurrentPage(page);

  qDebug() << " build first page";
  // add textbox for the first page
  _docController->addTextBlock(
    pageMarginLeft, pageMarginTop, columnsWidth, columnsHeight);

  const bool isBalance = true; //B:useless?
  if (isBalance) {
    int heightFilledTextBlock = 0;
    QList<LineText *> tmpParagraphs;
    tmpParagraphs.reserve(text.size());
    for (LineText *tmpP : text) {

      // calculate paragraph height
      int nbLinesOfParagraph = tmpP->Long / columnsWidth + 1;
      tmpP->Height =
        (nbLinesOfParagraph - 1) * (tmpP->meanCharHeight + lineSpacing);
      tmpP->Height = tmpP->Height * 0.85;

      int heightChecker = heightFilledTextBlock + tmpP->Height - lineSpacing;

      // at the end of text block
      if (heightChecker > columnsHeight) { // split this paragraph into 2
        LineText *tmp1 = new LineText;
        tmp1->Long = 0;
        tmp1->Height = 0;
        tmp1->meanCharHeight = tmpP->meanCharHeight;
        LineText *tmp2 = new LineText;
        tmp2->Long = 0;
        tmp2->Height = 0;
        tmp2->meanCharHeight = tmpP->meanCharHeight;

        int delta_height = columnsHeight - heightFilledTextBlock;

        //find the splitted position
        int spaceIndex = 0;
        int spaceIndexNext = tmpP->charList.indexOf(QStringLiteral(" "));
        while (spaceIndexNext != -1) {
          // check word long
          int wordLong = 0;
          for (int i = spaceIndex; i <= spaceIndexNext; ++i) {
            const Models::Character *character =
              _font->getCharacter(tmpP->charList.at(i));
            if (character != nullptr) {
              const Models::CharacterData *charData =
                character->getRandomCharacterData();
              const QImage charImage = charData->getImage();
              wordLong += charImage.width();
            }
          }
          const int numLines = (tmp1->Long + wordLong) / columnsWidth + 1;
          const int checkerH = numLines * (meanCharHeight + lineSpacing);
          if (checkerH < delta_height) {
            for (int i = spaceIndex; i <= spaceIndexNext; ++i) {
              tmp1->charList << tmpP->charList.at(i);
            }
            tmp1->Long += wordLong;
            tmp1->Height = checkerH;
          }
	  else {
            for (int i = spaceIndex; i <= spaceIndexNext; ++i) {
              tmp2->charList << tmpP->charList.at(i);
            }
            tmp2->Long += wordLong;
          }
          // move to next word
          spaceIndex = spaceIndexNext;
          spaceIndexNext =
            tmpP->charList.indexOf(QStringLiteral(" "), spaceIndex + 1);
        }

        tmpParagraphs.push_back(tmp1);
        tmpParagraphs.push_back(tmp2);
        // Reset heightchecker
        heightFilledTextBlock =
          tmpP->Height - tmp1->Height; // update the height of filled text.

      } else { // continue to add paragraph to the current textblock
        heightFilledTextBlock =
          heightChecker; // update the height of filled text.
        tmpParagraphs.push_back(tmpP);
      }
    }
    text = tmpParagraphs;
  }
  //

  qDebug() << " build next page nb of paragraphs " << text.size();

  int columnCounter = 1;
  int heightFilledTextBlock = 0;
  for (LineText *tmpParagraph : text) {

    // calculate paragraph height
    int nbLinesOfParagraph = tmpParagraph->Long / columnsWidth + 1;

    if (nbLinesOfParagraph > 1)
      tmpParagraph->Height =
        (nbLinesOfParagraph - 1) * (tmpParagraph->meanCharHeight + lineSpacing);
    else
      tmpParagraph->Height =
        (nbLinesOfParagraph) * (tmpParagraph->meanCharHeight + lineSpacing);

    tmpParagraph->Height = tmpParagraph->Height * 0.85;

    int heightChecker = heightFilledTextBlock + tmpParagraph->Height;
    //        qDebug() << " checker = " << heightChecker << " col height = " <<
    //        columnsHeight << " nbLines = " << nbLinesOfParagraph << " para
    //        height = " << tmpParagraph->Height;

    if (heightChecker > columnsHeight) {
      heightFilledTextBlock =
        0; // Reset heightchecker  //B:useless ? set at line 613.
      if (columnCounter < nbColumns) { // and new column
        _docController->addTextBlock(
          (pageMarginLeft + (columnsWidth + columnSpacing) * columnCounter),
          pageMarginTop,
          columnsWidth,
          columnsHeight);
        ++columnCounter; //update column counter
      }
      else {           // add new page + new column
        Doc::Page *newPage = new Doc::Page(document);
        newPage->setBackgroundFileName(_bgName);
        document->add(newPage);
        document->setPageWidth(_bg.width());
        document->setPageHeight(_bg.height());
        document->setCurrentPage(newPage);
        // add textbox for the page
        _docController->addTextBlock(
          pageMarginLeft, pageMarginTop, columnsWidth, columnsHeight);
        columnCounter = 1; // Reset column counter for the new page.
      }
      heightFilledTextBlock =
        tmpParagraph->Height; // update the height of filled text.

    } else { // continue to add paragraph to the current textblock
      heightFilledTextBlock =
        heightChecker; // update the height of filled text.
    }
    // add the current paragraph to the current textblock of page.
    _docController->addParagraph();
    _docController->setParagraphLineSpacing(lineSpacing);
    _docController->addCharacters(tmpParagraph->charList);
  }

  // save
  QString filePathSave = filePath;
  filePathSave.replace(QStringLiteral(".txt"), QStringLiteral(".xml"));
  IOManager::DocumentFileManager::documentToXml(document, filePathSave);

  for (LineText *p : text)
    delete p;
  text.clear();
}

void
ImageGenerationFromDirDialog::loadBackground()
{
  _bgName = this->ui->cbBackground->currentText();
  QString bgPath = Context::BackgroundContext::instance()->getPath() + _bgName;
  _bg = QImage(bgPath);
}

void
ImageGenerationFromDirDialog::loadFont()
{
  _fontName = this->ui->cbFontType->currentText();
  _font = Context::FontContext::instance()->getFont(_fontName);

  const Models::CharacterMap charsMap = _font->getCharacters();

  //QMap<QString, Character*>
  Models::CharacterMap::const_iterator it = charsMap.constBegin();

  int counter = 0;
  const Models::CharacterMap::const_iterator itEnd = charsMap.constEnd();
  for (; it != itEnd; ++it) {

    Models::Character *ch = it.value();
    Models::CharacterDataList charsDataList = ch->getAllCharacterData();

    for (Models::CharacterData *charData : charsDataList) {
      if (charData != nullptr) {
        const QImage charImage = charData->getImage();
        if (!charImage.isNull()) {
          _word_height += charImage.height();
          _Char_Format = charImage.format();

          ++counter;
        }
      }
    }
  }

  _word_height = _word_height / counter;

  _word_height = round(1.5 * _word_height);

  qDebug() << _word_height;
}

void
ImageGenerationFromDirDialog::setupPageParameters()
{

  _pageMarginTop = 100; // Document layout
  _pageMarginLeft = 100;
  _pageMarginRight = 100;
  _pageMarginBottom = 150;
  _columnSpacing = 10;
  _lineSpacing = this->ui->sbLinespacing->value();
  _word_height = 0;

  loadBackground(); // background
  _width = _bg.width();
  _height = _bg.height();

  loadFont(); // Font

  _nbColumns = this->ui->cbColumn->value(); // text block layout
  _columnsWidth = (_width - _pageMarginLeft - _pageMarginRight -
                   _columnSpacing * (_nbColumns - 1)) /
                  _nbColumns;
  _columnsHeight = (_height - _pageMarginTop - _pageMarginBottom);
}

Doc::Document *
ImageGenerationFromDirDialog::createADocument()
{

  setupPageParameters(); // set up document

  auto document = new Doc::Document(); // create new document

  auto style = new Doc::DocStyle(_fontName); // add font style
  document->addStyle(style);

  return document;
}

Doc::Page *
ImageGenerationFromDirDialog::createAPage(Doc::Document *document)
{
  auto page = new Doc::Page(document);
  page->setBackgroundFileName(_bgName);
  document->add(page);
  document->setPageWidth(_width);
  document->setPageHeight(_height);
  document->setCurrentPage(page);
  return page;
}

Doc::DocTextBlock *
ImageGenerationFromDirDialog::createATextBox(Doc::Document *document,
                                             int x,
                                             int y)
{
  auto tb = new Doc::DocTextBlock(document);
  tb->setMarginTop(0);
  tb->setMarginBottom(0);
  tb->setMarginLeft(0);
  tb->setMarginRight(0);

  tb->setHeight(_columnsHeight);
  tb->setWidth(_columnsWidth);
  tb->setX(x);
  tb->setY(y);

  return tb;
}

QImage
ImageGenerationFromDirDialog::getCharacterImage(const Models::Character *ch)
{
  const Models::CharacterData *charData = ch->getRandomCharacterData();
  const QImage charImage = charData->getImage();
  return charImage;
}

void
ImageGenerationFromDirDialog::bindTextForATextBox(QStringList &textLines,
                                                  Doc::DocTextBlock *tb)
{
  cv::Point char_pos(0, 0);
  bool isFullBlock = false;

  while (!textLines.isEmpty() && !isFullBlock) {

    if ((char_pos.y + _word_height) > _columnsHeight) {
      isFullBlock = true;
      break;
    } // if the block is full -> out

    QString line = textLines.first(); // get first line

    auto p = new Doc::DocParagraph(tb->getDocument()); // create a new paragraph
    p->setLineSpacing(_lineSpacing);
    tb->add(p); // add the current paragraph to the current textblock of page.

    while (!line.isEmpty()) {

      if ((char_pos.y + _word_height) > _columnsHeight) {
        isFullBlock = true;
        break;
      } // IF the block is full -> out

      QChar c = line.at(0); // Get the current character
      const Models::Character *character = _font->getCharacter(QString(c));

      if (character != nullptr) {

        const int char_ID = character->getRandomCharacterData()->getId();
        const QImage charImage = getCharacterImage(character);

        // int lx=0, rx=0, by = 0;
        // lx = (0 - character->getLeftLine()/100)* charImage->width();
        // rx = (character->getRightLine()/100 - 1)* charImage->width();

        if ((char_pos.x + charImage.width()) >
            (_columnsWidth)) { // vérifier le retour de la ligne
          char_pos.x = 0;
          char_pos.y =
            char_pos.y + _word_height + _lineSpacing; //retour à la ligne
        }

        Doc::DocCharacter *docChar =
          new Doc::DocCharacter(QString(c), char_ID, tb->getDocument());
        docChar->setX(char_pos.x);
        docChar->setY(char_pos.y);
        docChar->setWidth(charImage.width());
        docChar->setHeight(charImage.height());
        p->add(docChar);

        /** DEBUG **

                cv::rectangle(_bind_text_checker, cv::Rect(char_pos.x + tb->x(), char_pos.y + tb->y(),
                                                          charImage.width(),
                                                          charImage.height()),
                                                          cv::Scalar(255, 0, 0)); // Debug: draw a rectangle of a character.


                cv::putText(_bind_text_checker, QString(c).toStdString(),
                            cv::Point(char_pos.x + tb->x(),
                                      char_pos.y + tb->y()),
                            cv::FONT_HERSHEY_PLAIN,1, cv::Scalar::all(255));     // Debug: draw text
                */

        char_pos.x = char_pos.x + charImage.width(); //update next position
      }
      line.remove(0, 1); // remove the current character
    }

    if (isFullBlock)
      break;                 // IF the block is full -> out
                             //else
    textLines.removeFirst(); // remove the first line

    char_pos.x = 0;                                        // retour de ligne
    char_pos.y = char_pos.y + _word_height + _lineSpacing; // retour de ligne

    if (textLines.isEmpty())
      isFullBlock = true; // Text line is empty
  }
}

Doc::Page *
ImageGenerationFromDirDialog::bindTextForAPage(Doc::Document *document,
                                               QStringList &TEXT_lines)
{

  Doc::Page *page = createAPage(document); // add new page

  int tb_x = _pageMarginLeft;
  int tb_y = _pageMarginTop;

  for (int i = 0; i < _nbColumns; ++i) { // add textbox

    if (TEXT_lines.isEmpty())
      break; // check text lines firstly

    Doc::DocTextBlock *tb = createATextBox(document, tb_x, tb_y);
    bindTextForATextBox(TEXT_lines, tb); // bind characters to textbox

    page->add(tb);             // add the textbox to the page
    page->setCurrentBlock(tb); // set this textbox as current block

    /** DEBUG **
        cv::rectangle(_bind_text_checker, cv::Rect(tb_x, tb_y,_columnsWidth,
                                                  _columnsHeight), cv::Scalar(0,255, 0));     // debug : draw text block boundary
        */

    tb_x += _columnsWidth + _columnSpacing;
  }

  return page;
}

QStringList
ImageGenerationFromDirDialog::readTextFile(const QString &filePath)
{
  std::string line;
  std::ifstream file(filePath.toStdString().c_str());
  QStringList textLines;
  if (file.is_open()) {
    while (file.good()) {
      getline(file, line);
      if (!line.empty())
        textLines.push_back(QString::fromStdString(line));
    }
  }
  file.close();
  return textLines;
}

QStringList
ImageGenerationFromDirDialog::createDocument(const QString &filePath)
{
  QStringList listXMLs;

  Doc::Document *document = createADocument(); // Create a new document

  QStringList textLines =
    readTextFile(filePath); // read text lines from file *.txt

  int pageCounter = 0;
  while (!textLines.isEmpty()) {

    /** DEBUG **
          _bind_text_checker = cv::Mat::zeros(_height,_width, cV_8UC3);  // check binding text process
      */

    Doc::Page *page =
      bindTextForAPage(document, textLines); // bind text for a page

    QString filePathSave = filePath; // save this page
    filePathSave.replace(QStringLiteral(".txt"),
                         "_p" + QString::number(pageCounter) + ".xml");    //
    listXMLs.push_back(filePathSave);                                      //
    IOManager::DocumentFileManager::documentToXml(document, filePathSave); //

    /** DEBUG **
          filePathSave.append("_checker.png");
          cv::imwrite(filePathSave.toStdString().c_str(),_bind_text_checker);
      */

    ++pageCounter; // increate the nb of pages

    document->remove(page); //B

    delete page;
  }

  delete document;

  return listXMLs;
}

QStringList
ImageGenerationFromDirDialog::createNewBigDocument(const QString &filePath)
{

  QStringList listXMLs;
  // set up document
  setupPageParameters();
  // create new document
  auto document = new Doc::Document();
  // add font style
  auto style = new Doc::DocStyle(_fontName);
  document->addStyle(style);
  // read string line from txt file
  std::string line;
  std::ifstream file(filePath.toStdString().c_str());
  QList<LineText *> textLines;
  const int normalBaseLine = 100;
  int detaMaxBaseLine = 0;
  int meanCharHeight = 0;
  int maxCharHeight = 0;
  int itemCounter = 0;
  int totalLineLong = 0;
  int maxLineLong = 0;

  if (file.is_open()) {
    while (file.good()) {
      getline(file, line);
      LineText *tmpLine = new LineText;
      tmpLine->Height = 0;
      tmpLine->Long = 0;
      tmpLine->meanCharHeight = 0;
      int lineLong = 0;
      const size_t lineSize = line.size();
      itemCounter += lineSize;

      for (size_t i = 0; i < lineSize; ++i) {

        QString charDisplay(line.at(i));
        const Models::Character *character = _font->getCharacter(charDisplay);

        if (character != nullptr &&
            !charDisplay.isEmpty()) { // if the character exist in the font, get
                                      // this character.
          if (detaMaxBaseLine < fabs(normalBaseLine - character->getBaseLine()))
            detaMaxBaseLine = fabs(normalBaseLine - character->getBaseLine());

          const Models::CharacterData *charData =
            character->getRandomCharacterData();
          const QImage charImage = charData->getImage();

          const int h = charImage.height();
          if (maxCharHeight < h)
            maxCharHeight = h;

          meanCharHeight += h;
          tmpLine->meanCharHeight += h;
          lineLong += charImage.width();
          tmpLine->charList.push_back(charDisplay);
        }
      }
      if (maxLineLong < lineLong)
        maxLineLong = lineLong;
      tmpLine->Long = lineLong;
      tmpLine->meanCharHeight = tmpLine->meanCharHeight / line.size();

      totalLineLong += lineLong;
      textLines.push_back(tmpLine);
    }
  }
  file.close();
  // check if height of all paragraph > columns height
  assert(itemCounter > 0); //B
  meanCharHeight = meanCharHeight / itemCounter;
  const int lineSpacing = (maxCharHeight * 1.2f);

  // check fitted column width with text input
  if (maxLineLong < _columnsWidth) { // re-form background and columns width

    const int delta = _columnsWidth - maxLineLong;
    _columnsWidth = maxLineLong + 100;
    _bg = _bg.scaled(_width - _nbColumns * (delta - 100), _height);
    _width = _bg.width();
  }

  const int nbLines = totalLineLong / _columnsWidth + 1;
  const int heightOfAllLines = nbLines * (lineSpacing + meanCharHeight);
  // check fitted column height with text input
  qDebug() << "max height of text = " << heightOfAllLines << " height "
           << _columnsHeight;

  if (heightOfAllLines < _columnsHeight) {
    _bg = _bg.scaled(
      _width, heightOfAllLines + 100 + _pageMarginTop + _pageMarginBottom);
    _height = _bg.height();

    _columnsHeight = (_height - _pageMarginTop - _pageMarginBottom);
  }

  const int nbNeededPages = heightOfAllLines / (_columnsHeight * _nbColumns) + 1;

  qDebug() << "Create page nb of pages = " << nbNeededPages
           << " nb of lines = " << nbLines << " linespacing =" << lineSpacing
           << " mean char height = " << meanCharHeight << " nbcol "
           << _nbColumns;

  // add the first page
  auto firstPage = new Doc::Page(document);
  firstPage->setBackgroundFileName(_bgName);
  document->add(firstPage);
  document->setPageWidth(_bg.width());
  document->setPageHeight(_bg.height());
  document->setCurrentPage(firstPage);

  qDebug() << " build first page";
  // add textbox for the first page
  auto firstTextBlock = new Doc::DocTextBlock(document);
  firstTextBlock->setMarginTop(0);
  firstTextBlock->setMarginBottom(0);
  firstTextBlock->setMarginLeft(10);
  firstTextBlock->setMarginRight(0);
  firstTextBlock->setHeight(_columnsHeight);
  firstTextBlock->setWidth(_columnsWidth);
  firstTextBlock->setX(_pageMarginLeft);
  firstTextBlock->setY(_pageMarginTop);

  firstPage->add(firstTextBlock);
  firstPage->setCurrentBlock(firstTextBlock);

  //_docController->addTextBlock(pageMarginLeft, pageMarginTop, columnsWidth, columnsHeight);

  const bool isBalance = true; //B:useless?
  if (isBalance) {
    int heightFilledTextBlock = 0;
    QList<LineText *> tmpLines;
    tmpLines.reserve(textLines.size());
    for (LineText *tmpLine : textLines) {

      // calculate paragraph height
      const int nbLinesOfParagraph = tmpLine->Long / _columnsWidth + 1;
      tmpLine->Height =
        (nbLinesOfParagraph - 1) * (tmpLine->meanCharHeight + lineSpacing);
      tmpLine->Height = tmpLine->Height * 0.85;

      const int heightChecker = heightFilledTextBlock + tmpLine->Height - lineSpacing;

      // at the end of text block
      if (heightChecker > _columnsHeight) { // split this paragraph into 2
        LineText *tmp1 = new LineText;
        tmp1->Long = 0;
        tmp1->Height = 0;
        tmp1->meanCharHeight = tmpLine->meanCharHeight;
        LineText *tmp2 = new LineText;
        tmp2->Long = 0;
        tmp2->Height = 0;
        tmp2->meanCharHeight = tmpLine->meanCharHeight;

        const int delta_height = _columnsHeight - heightFilledTextBlock;

        //find the splitted position
        int spaceIndex = 0;
        int spaceIndexNext = tmpLine->charList.indexOf(QStringLiteral(" "));
        while (spaceIndexNext != -1) {
          // check word long
          int wordLong = 0;
          for (int i = spaceIndex; i <= spaceIndexNext; ++i) {
            const Models::Character *character =
              _font->getCharacter(tmpLine->charList.at(i));
            if (character != nullptr) {
              const Models::CharacterData *charData =
                character->getRandomCharacterData();
              const QImage charImage = charData->getImage();
              wordLong += charImage.width();
            }
          }
          const int numLines = (tmp1->Long + wordLong) / _columnsWidth + 1;
          const int checkerH = numLines * (meanCharHeight + lineSpacing);
          if (checkerH < delta_height) {
            for (int i = spaceIndex; i <= spaceIndexNext; ++i) {
              tmp1->charList << tmpLine->charList.at(i);
            }
            tmp1->Long += wordLong;
            tmp1->Height = checkerH;
          }
	  else {
            for (int i = spaceIndex; i <= spaceIndexNext; ++i) {
              tmp2->charList << tmpLine->charList.at(i);
            }
            tmp2->Long += wordLong;
          }
          // move to next word
          spaceIndex = spaceIndexNext;
          spaceIndexNext =
            tmpLine->charList.indexOf(QStringLiteral(" "), spaceIndex + 1);
        }

        tmpLines.push_back(tmp1);
        tmpLines.push_back(tmp2);
        // Reset heightchecker
        heightFilledTextBlock =
          tmpLine->Height - tmp1->Height; // update the height of fille text.

      } else { // continue to add paragraph to the current textblock
        heightFilledTextBlock =
          heightChecker; // update the height of filled text.
        tmpLines.push_back(tmpLine);
      }
    }
    textLines = tmpLines;
  }
  //

  qDebug() << " build next page nb of paragraphs " << textLines.size();

  int columnCounter = 1;
  int heightFilledTextBlock = 0;
  int dy = 0, dh = 0, pageCounter = 1;

  for (LineText *tmpLine : textLines) {

    // calculate paragraph height
    const int nbLinesOfParagraph = tmpLine->Long / _columnsWidth + 1;

    if (nbLinesOfParagraph > 1)
      tmpLine->Height =
        (nbLinesOfParagraph - 1) * (tmpLine->meanCharHeight + lineSpacing);
    else
      tmpLine->Height =
        (nbLinesOfParagraph) * (tmpLine->meanCharHeight + lineSpacing);

    tmpLine->Height = tmpLine->Height * 0.85;

    const int heightChecker = heightFilledTextBlock + tmpLine->Height;

    if (heightChecker > _columnsHeight) {
      heightFilledTextBlock = 0;        // Reset heightchecker
      if (columnCounter < _nbColumns) { // and new column
        Doc::DocTextBlock *nextTextBlock = new Doc::DocTextBlock(document);
        nextTextBlock->setMarginTop(0);
        nextTextBlock->setMarginBottom(0);
        nextTextBlock->setMarginLeft(10);
        nextTextBlock->setMarginRight(0);
        nextTextBlock->setHeight(_columnsHeight);
        nextTextBlock->setWidth(_columnsWidth);
        nextTextBlock->setX(_pageMarginLeft +
                            (_columnsWidth + _columnSpacing) * columnCounter);
        nextTextBlock->setY(_pageMarginTop);
        document->currentPage()->add(nextTextBlock);
        document->currentPage()->setCurrentBlock(nextTextBlock);

        ++columnCounter; //update column counter
        // reset dy, dh
        dy = 0;
      }
      else {
        // save previous page
        QString filePathSave = filePath;
        filePathSave.replace(QStringLiteral(".txt"),
                             "_p" + QString::number(pageCounter) + ".xml");
        listXMLs.push_back(filePathSave);
        IOManager::DocumentFileManager::documentToXml(document, filePathSave);
        // remove previous page + textblocks
        for (Doc::Page *page : document->getPages()) {
          for (Doc::DocTextBlock *tb : page->getTextBlocks())
            delete tb;
          page->removeTextBlocks();
          delete page;
        }
        document->removePages();
        // reset dy, dh
        dy = 0;
        dh = 0;
        // add new page + new column
        ++pageCounter;
        Doc::Page *nextPage = new Doc::Page(document);
        nextPage->setBackgroundFileName(_bgName);
        document->add(nextPage);
        document->setPageWidth(_bg.width());
        document->setPageHeight(_bg.height());
        document->setCurrentPage(nextPage);
        // add textbox for the page
        Doc::DocTextBlock *tb = new Doc::DocTextBlock(document);
        tb->setMarginTop(0);
        tb->setMarginBottom(0);
        tb->setMarginLeft(10);
        tb->setMarginRight(0);
        tb->setHeight(_columnsHeight);
        tb->setWidth(_columnsWidth);
        tb->setX(_pageMarginLeft);
        tb->setY(_pageMarginTop);
        nextPage->add(tb);
        nextPage->setCurrentBlock(tb);

        columnCounter = 1; // Reset column counter for the new page.
      }
      heightFilledTextBlock =
        tmpLine->Height; // update the height of fille text.

    } else { // continue to add paragraph to the current textblock
      heightFilledTextBlock =
        heightChecker; // update the height of filled text.
    }

    // add the current paragraph to the current textblock of page.
    Doc::DocParagraph *p = new Doc::DocParagraph(document);
    p->setLineSpacing(lineSpacing);
    Doc::DocTextBlock *currentTextBlock =
      dynamic_cast<Doc::DocTextBlock *>(document->currentBlock());
    assert(currentTextBlock != nullptr);
    currentTextBlock->add(p);

    int dx = 0;

    for (const QString &s : tmpLine->charList) {

      int x = dx;
      int y = dy;
      const Models::Character *character = _font->getCharacter(s);

      if (character != nullptr) {
        const Models::CharacterData *charData =
          character->getRandomCharacterData();

        const QImage charImage = charData->getImage();

        int lx = (0 - character->getLeftLine() / 100) * charImage.width();
        int rx = (character->getRightLine() / 100 - 1) * charImage.width();

        Doc::DocCharacter *docChar =
          new Doc::DocCharacter(s, charData->getId(), document);
        docChar->setX(x + lx);
        docChar->setY(y);
        docChar->setWidth(charImage.width());
        docChar->setHeight(charImage.height());
        p->add(docChar);

        if (dh < charImage.height())
          dh = charImage.height();
        if ((dx + charImage.width() + lx + rx) < _columnsWidth) {
          dx += charImage.width() + lx + rx;
        } else {
          dx = 0;
          dy += lineSpacing + dh;
        }
      }
    }
    dy += lineSpacing;
  }

  // save
  if (pageCounter >= nbNeededPages) {
    QString filePathSave = filePath;
    filePathSave.replace(QStringLiteral(".txt"),
                         "_p" + QString::number(pageCounter) + ".xml");
    listXMLs.push_back(filePathSave);
    IOManager::DocumentFileManager::documentToXml(document, filePathSave);
  }

  for (LineText *p : textLines)
    delete p;
  textLines.clear();

  //remove document
  for (Doc::Page *page : document->getPages()) {
    for (Doc::DocTextBlock *tb : page->getTextBlocks())
      delete tb;
    page->removeTextBlocks();
    delete page;
  }
  document->removePages();
  delete document;
  qDebug() << " OK xml";

  return listXMLs;
}

void
ImageGenerationFromDirDialog::chooseTextDirectory()
{
  QString path =
    QFileDialog::getExistingDirectory(this, tr("Choose a directory"));
  this->ui->txtTextDirPath->setText(path);
}

void
ImageGenerationFromDirDialog::createFontSelector()
{
  QStringList fontNames = Context::FontContext::instance()->getFontNames();
  for (const QString &fontName : fontNames)
    this->ui->cbFontType->addItem(fontName);

  //    this->ui->cbFontType->addItem("Randomly");

  this->ui->cbFontType->setCurrentIndex(this->ui->cbFontType->findText(
    Context::FontContext::instance()->getCurrentFont()->getName()));

  connect(this->ui->cbFontType,
          SIGNAL(currentIndexChanged(QString)),
          this,
          SLOT(changeFont(QString)));
}

void
ImageGenerationFromDirDialog::changeFont(const QString &fontName)
{
  Context::FontContext::instance()->setCurrentFont(fontName);
}
void
ImageGenerationFromDirDialog::changeBackground(const QString &background)
{
  Context::BackgroundContext::instance()->setCurrentBackground(background);
}

void
ImageGenerationFromDirDialog::createBackgroundSelector()
{

  Context::BackgroundList backgroundList =
    Context::BackgroundContext::instance()->getBackgrounds();
  for (const QString &background : backgroundList) {
    QString path = Context::BackgroundContext::instance()->getPath();
    QPixmap pixmap(path + background);
    this->ui->cbBackground->addItem(QIcon(pixmap.scaled(21, 30)), background);
  }
  //    this->ui->cbBackground->addItem("Randomly");
  this->ui->cbBackground->setCurrentIndex(this->ui->cbBackground->findText(
    Context::BackgroundContext::instance()->getCurrentBackground()));

  connect(this->ui->cbBackground,
          SIGNAL(currentIndexChanged(QString)),
          this,
          SLOT(changeBackground(QString)));
}
