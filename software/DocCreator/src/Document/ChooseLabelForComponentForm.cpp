#include "ChooseLabelForComponentForm.hpp"
#include "ui_ChooseLabelForComponentForm.h"

#include <algorithm>
#include <cassert>

#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QTableWidget>

#include "DocumentController.hpp"
#include "Utils/ImageUtils.hpp" //getReadImageFilterList()
#include "appconstants.h"
#include "context/backgroundcontext.h"
#include "core/configurationmanager.h"
#include "iomanager/fontfilemanager.h"
#include "models/character.h"
#include "models/characterdata.h"
#include "models/doc/doccomponent.h"
#include "models/doc/doccomponentblock.h"
#include "models/doc/docimageblock.h"
#include "models/doc/document.h"
#include "models/doc/page.h"

//B:TODO: this code is quite ugly... lots of "new"...

ChooseLabelForComponentForm::ChooseLabelForComponentForm(
  DocumentController *docController,
  Models::Font *currentFont,
  const QList<Doc::DocComponentBlock *> &componentBlocks,
  QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::ChooseLabelForComponentForm)
  , _listWidgetSmall(nullptr)
  , _listWidgetMedium(nullptr)
  , _listWidgetBig(nullptr)
  , _tableOfLabeledComponents(nullptr)
  , _chkOneLettre(nullptr)
  , _cbPositions(nullptr)
{
  // add UI component
  ui->setupUi(this);

  _listWidgetSmall = new QListWidget(this);
  _listWidgetSmall->setGeometry(10, 50, 150, 500);
  _listWidgetSmall->setEnabled(true);

  _listWidgetMedium = new QListWidget(this);
  _listWidgetMedium->setGeometry(180, 50, 150, 500);
  _listWidgetMedium->setEnabled(true);

  _listWidgetBig = new QListWidget(this);
  _listWidgetBig->setGeometry(350, 50, 150, 500);
  _listWidgetBig->setEnabled(true);

  _tableOfLabeledComponents = new QTableWidget(this);
  _tableOfLabeledComponents->setGeometry(700, 50, 500, 500);
  _tableOfLabeledComponents->setEnabled(true);

  _chkOneLettre = new QCheckBox(tr("One lettre"), this);
  _chkOneLettre->setGeometry(520, 10, 101, 22);
  _chkOneLettre->setEnabled(true);
  _chkOneLettre->setChecked(false);

  _cbPositions = new QComboBox(this);
  _cbPositions->setGeometry(700, 10, 50, 22);
  _cbPositions->setEnabled(true);
  _cbPositions->addItem(tr("Top"));
  _cbPositions->addItem(tr("Right"));
  _cbPositions->addItem(tr("Bottom"));
  _cbPositions->addItem(tr("Left"));

  QLabel *lblPosition = new QLabel(tr("Position"), this);
  lblPosition->setGeometry(760, 10, 100, 22);
  lblPosition->setEnabled(true);
  // connected components
  _componentBlocks = componentBlocks;

  //normal character image position
  _imageBlockID = 0;
  _upLine = 0;
  _baseLine = 100;
  _leftLine = 0;
  _rightLine = 100;

  //
  _currentFont = currentFont;
  _docController = docController;

  //loadFontNamesFromTmpFolder();
  //loading document image
  const QString path =
    Context::BackgroundContext::instance()->getPath() +
    Context::BackgroundContext::instance()->getCurrentBackground();
  QImage tmp_bg(path);
  //const QSize size = docController->toQImage().size(); //B: we want only image size !!!!!
  const QSize size = docController->getImageSize();
  _originalImage = tmp_bg.scaled(size.width(), size.height());

  connect(_listWidgetSmall,
          SIGNAL(itemClicked(QListWidgetItem*)),
          this,
          SLOT(smallItemClicked(QListWidgetItem*)));
  connect(_listWidgetMedium,
          SIGNAL(itemClicked(QListWidgetItem*)),
          this,
          SLOT(mediumItemClicked(QListWidgetItem*)));
  connect(_listWidgetBig,
          SIGNAL(itemClicked(QListWidgetItem*)),
          this,
          SLOT(bigItemClicked(QListWidgetItem*)));

  connect(ui->btnLabeling, SIGNAL(clicked()), this, SLOT(btnLabelingClicked()));
  connect(ui->btnCreateFont, SIGNAL(clicked()), this, SLOT(createNewFont()));

  connect(ui->btnBackGround, SIGNAL(clicked()), this, SLOT(isBackGround()));
  connect(ui->btnImageBlock, SIGNAL(clicked()), this, SLOT(isImageBlock()));
  connect(ui->btnSelectAll, SIGNAL(clicked()), this, SLOT(isSelectAll()));

  // split the list of connected components into small, medium, and big lists
  // following their bounding box size
  splitListComponentBlocks();
  initialize();
  createListItems();
  loadTable();
}

void
ChooseLabelForComponentForm::splitListComponentBlocks()
{
  _smallCBlocks.clear();
  _MediumCBlocks.clear();
  _bigCBlocks.clear();
  const QList<Doc::DocComponentBlock *> &componentBlocks = _componentBlocks;
  const size_t sz = componentBlocks.size();
  if (sz > 0) {
    std::vector<float> squares;
    squares.reserve(sz);
    for (Doc::DocComponentBlock *dcb : componentBlocks) {
      squares.push_back(dcb->width() * dcb->height());
    }
    assert(squares.size() == sz);
    std::sort(squares.begin(), squares.end());

    _smallCBlocks.reserve(sz / 3);  //arbitrary
    _MediumCBlocks.reserve(sz / 3); //arbitrary
    _bigCBlocks.reserve(sz / 3);    //arbitrary

    const float small = squares.at(sz / 3);
    const float medium = squares.at((2 * sz) / 3);
    for (Doc::DocComponentBlock *dcb : componentBlocks) {
      const float area = (dcb->width() * dcb->height());
      if (area <= small)
        _smallCBlocks.push_back(dcb);
      else if (area <= medium) //&& area > small
        _MediumCBlocks.push_back(dcb);
      else
        _bigCBlocks.push_back(dcb);
    }
  }
}

void
ChooseLabelForComponentForm::reset()
{}

void
ChooseLabelForComponentForm::loadTable()
{
  _tableOfLabeledComponents->clear();
  _tableOfLabeledComponents->setRowCount(_map_display_block_image.size() + 1);
  _tableOfLabeledComponents->setColumnCount(10);

  QTableWidgetItem *item =
    new QTableWidgetItem(QStringLiteral("Characters/components"));
  _tableOfLabeledComponents->setItem(0, 0, item);

  QMap<QString, QMap<Doc::DocComponentBlock *, QImage> *>::const_iterator iter =
    _map_display_block_image.constBegin();
  int row = 1;
  while (iter != _map_display_block_image.constEnd()) {

    int colIndex = 0;
    QTableWidgetItem *itemDisplay = new QTableWidgetItem(iter.key());
    _tableOfLabeledComponents->setItem(row, colIndex, itemDisplay);

    QMap<Doc::DocComponentBlock *, QImage> *components_images = iter.value();
    QMap<Doc::DocComponentBlock *, QImage>::const_iterator iter1 =
      components_images->constBegin();
    while (iter1 != components_images->constEnd()) {
      ++colIndex;
      QTableWidgetItem *itm = new QTableWidgetItem();
      QIcon icon(QPixmap::fromImage(iter1.value()));
      itm->setIcon(icon);

      _tableOfLabeledComponents->setItem(row, colIndex, itm);
      ++iter1;
    }

    ++iter;
    ++row;
  }
}

void
ChooseLabelForComponentForm::isSelectAll()
{
  for (QListWidgetItem *item : _listWidgetSmall->findItems(
         QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard))
    if (item->checkState() == Qt::Unchecked) {
      item->setCheckState(Qt::Checked);
      item->setSelected(true);
    }

  for (QListWidgetItem *item : _listWidgetMedium->findItems(
         QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard))
    if (item->checkState() == Qt::Unchecked) {
      item->setCheckState(Qt::Checked);
      item->setSelected(true);
    }

  for (QListWidgetItem *item : _listWidgetBig->findItems(
         QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard))
    if (item->checkState() == Qt::Unchecked) {
      item->setCheckState(Qt::Checked);
      item->setSelected(true);
    }
}

void
ChooseLabelForComponentForm::fillScanlineToImage(QImage &image,
                                                 const Doc::DocComponent *dc,
                                                 int dx,
                                                 int dy)
{
  const QMap<int, std::vector<int>> &sls = dc->getScanlines();
  QMap<int, std::vector<int>>::const_iterator it = sls.constBegin();
  const int X = dc->x();
  const int Y = dc->y();

  //B:TODO:OPTIM: remove checks from loops & take into account that scanlines are ordered !!!???
  while (it != sls.constEnd()) {
    const int y = it.key();

    if ((y - Y + dy) <= image.height()) {

      const std::vector<int> &Xs = it.value();
      if (Xs.size() > 1) {
        for (size_t m = 1; m < Xs.size();
             m += 2) { //start from 1 ; incremented of 2 because scanline
          const int x0 = Xs[m - 1] - X;
          const int x1 = Xs[m] - X;

          for (int k = x0; k <= x1; ++k) {
            if (k + dx <= image.width())
              image.setPixel(
                k + dx, y - Y + dy, _originalImage.pixel(k + X, y));
          }
        }
      }
    }

    ++it;
  }
  //    image->save("fillscanlineToimage.png");
}

void
ChooseLabelForComponentForm::isImageBlock()
{
  // small list
  for (QListWidgetItem *item : _listWidgetSmall->findItems(
         QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
    if (item->checkState() == Qt::Checked) {

      Doc::DocComponentBlock *componentBlock =
        _map_Items_SmallComponentBlocks.value(item);
      QImage imgSmallBlockData =
        _map_SmallComponent_Imagedata.value(componentBlock);

      _map_SmallComponentBlock_DisplayForListWidget[componentBlock] = false;

      //Remove block inside the block
      const QList<Doc::DocComponentBlock *> &smallCBlocks = _smallCBlocks;
      for (Doc::DocComponentBlock *bloc : smallCBlocks) {
        if (bloc->x() > componentBlock->x() &&
            bloc->x() < (componentBlock->x() + componentBlock->width()) &&
            bloc->y() > componentBlock->y() &&
            bloc->y() < (componentBlock->y() + componentBlock->height())) {

          for (Doc::DocZone *zone : bloc->getZones())
            for (Doc::DocComponent *com : zone->getComponents()) {
              const int dx = bloc->x() - componentBlock->x();
              const int dy = bloc->y() - componentBlock->y();
              fillScanlineToImage(imgSmallBlockData, com, dx, dy);
            }

          _map_SmallComponentBlock_DisplayForListWidget[bloc] = false;
        }
      }

      QString path = Core::ConfigurationManager::get(AppConfigMainGroup,
                                                     AppConfigImageFolderKey)
                       .toString();
      path.append("imageblock_extracted_");
      path.append(QString::number(_imageBlockID));
      path.append(".png");
      imgSmallBlockData.save(path);
      //
      Doc::DocImageBlock *imageBlock =
        new Doc::DocImageBlock(path,
                               imgSmallBlockData.width(),
                               imgSmallBlockData.height(),
                               componentBlock->x(),
                               componentBlock->y());
      _docController->getDocument()->currentPage()->add(imageBlock);
      ++_imageBlockID;
    }
  }
  // medium list
  for (QListWidgetItem *item : _listWidgetMedium->findItems(
         QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
    if (item->checkState() == Qt::Checked) {

      Doc::DocComponentBlock *componentBlock =
        _map_Items_MediumComponentBlocks.value(item);
      QImage imgMediumBlockData =
        _map_MediumComponent_Imagedata.value(componentBlock);

      _map_MediumComponentBlock_DisplayForListWidget[componentBlock] = false;

      //Remove block inside the block
      const QList<Doc::DocComponentBlock *> &MediumCBlocks = _MediumCBlocks;
      for (Doc::DocComponentBlock *bloc : MediumCBlocks) {
        if (bloc->x() > componentBlock->x() &&
            bloc->x() < (componentBlock->x() + componentBlock->width()) &&
            bloc->y() > componentBlock->y() &&
            bloc->y() < (componentBlock->y() + componentBlock->height())) {

          for (Doc::DocZone *zone : bloc->getZones())
            for (Doc::DocComponent *com : zone->getComponents()) {
              int dx = bloc->x() - componentBlock->x();
              int dy = bloc->y() - componentBlock->y();
              fillScanlineToImage(imgMediumBlockData, com, dx, dy);
            }

          _map_MediumComponentBlock_DisplayForListWidget[bloc] = false;
        }
      }
      QString path = Core::ConfigurationManager::get(AppConfigMainGroup,
                                                     AppConfigImageFolderKey)
                       .toString();
      path.append("imageblock_extracted_");
      path.append(QString::number(_imageBlockID));
      path.append(".png");
      imgMediumBlockData.save(path);
      //
      Doc::DocImageBlock *imageBlock =
        new Doc::DocImageBlock(path,
                               imgMediumBlockData.width(),
                               imgMediumBlockData.height(),
                               componentBlock->x(),
                               componentBlock->y());
      _docController->getDocument()->currentPage()->add(imageBlock);
      ++_imageBlockID;
    }
  }
  // big list
  for (QListWidgetItem *item : _listWidgetBig->findItems(
         QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
    if (item->checkState() == Qt::Checked) {

      Doc::DocComponentBlock *componentBlock =
        _map_Items_BigComponentBlocks.value(item);
      QImage imgBigBlockData =
        _map_BigComponent_Imagedata.value(componentBlock);

      _map_BigComponentBlock_DisplayForListWidget[componentBlock] = false;

      //Remove block inside the block
      const QList<Doc::DocComponentBlock *> &bigCBlocks = _bigCBlocks;
      for (Doc::DocComponentBlock *bloc : bigCBlocks)
        if (bloc->x() > componentBlock->x() &&
            bloc->x() < (componentBlock->x() + componentBlock->width()) &&
            bloc->y() > componentBlock->y() &&
            bloc->y() < (componentBlock->y() + componentBlock->height())) {

          for (Doc::DocZone *zone : bloc->getZones())
            for (Doc::DocComponent *com : zone->getComponents()) {
              const int dx = bloc->x() - componentBlock->x();
              const int dy = bloc->y() - componentBlock->y();
              fillScanlineToImage(imgBigBlockData, com, dx, dy);
            }

          _map_BigComponentBlock_DisplayForListWidget[bloc] = false;
        }
      QString path = Core::ConfigurationManager::get(AppConfigMainGroup,
                                                     AppConfigImageFolderKey)
                       .toString();
      path.append("imageblock_extracted_");
      path.append(QString::number(_imageBlockID));
      path.append(".png");
      imgBigBlockData.save(path);
      //
      Doc::DocImageBlock *imageBlock =
        new Doc::DocImageBlock(path,
                               imgBigBlockData.width(),
                               imgBigBlockData.height(),
                               componentBlock->x(),
                               componentBlock->y());
      _docController->getDocument()->currentPage()->add(imageBlock);
      ++_imageBlockID;
    }
  }

  createListItems();
  _listWidgetSmall->repaint();
  _listWidgetMedium->repaint();
  _listWidgetBig->repaint();
  // update at parent windown
  _docController->setModified();
}

void
ChooseLabelForComponentForm::isBackGround()
{
  for (QListWidgetItem *item : _listWidgetSmall->findItems(
         QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
    if (item->checkState() == Qt::Unchecked)
      continue;
    Doc::DocComponentBlock *componentBlock =
      _map_Items_SmallComponentBlocks.value(item);
    _map_SmallComponentBlock_DisplayForListWidget[componentBlock] = false;
  }
  for (QListWidgetItem *item : _listWidgetMedium->findItems(
         QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
    if (item->checkState() == Qt::Unchecked)
      continue;
    Doc::DocComponentBlock *componentBlock =
      _map_Items_MediumComponentBlocks.value(item);

    _map_MediumComponentBlock_DisplayForListWidget[componentBlock] = false;
  }
  for (QListWidgetItem *item : _listWidgetBig->findItems(
         QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
    if (item->checkState() == Qt::Unchecked)
      continue;
    Doc::DocComponentBlock *componentBlock =
      _map_Items_BigComponentBlocks.value(item);
    _map_BigComponentBlock_DisplayForListWidget[componentBlock] = false;
  }
  createListItems();
  _listWidgetSmall->repaint();
  _listWidgetMedium->repaint();
  _listWidgetBig->repaint();
  _docController->setModified();
}

void
ChooseLabelForComponentForm::close()
{
  //qDebug() << " closed window";

  QWidget::close();
}

void
ChooseLabelForComponentForm::smallItemClicked(QListWidgetItem *item)
{
  if (item->checkState() == Qt::Unchecked) {
    item->setCheckState(Qt::Checked);
    item->setSelected(true);
  } else {
    item->setCheckState(Qt::Unchecked);
    item->setSelected(false);
  }
  Doc::DocComponentBlock *dcb = _map_Items_SmallComponentBlocks.value(item);
  _docController->setCurrentBlock(dcb);
  _docController->setModified();
}

void
ChooseLabelForComponentForm::mediumItemClicked(QListWidgetItem *item)
{
  if (item->checkState() == Qt::Unchecked) {
    item->setCheckState(Qt::Checked);
    item->setSelected(true);
  } else {
    item->setCheckState(Qt::Unchecked);
    item->setSelected(false);
  }
  Doc::DocComponentBlock *dcb = _map_Items_MediumComponentBlocks.value(item);
  _docController->setCurrentBlock(dcb);
  _docController->setModified();
}

void
ChooseLabelForComponentForm::bigItemClicked(QListWidgetItem *item)
{
  if (item->checkState() == Qt::Unchecked) {
    item->setCheckState(Qt::Checked);
    item->setSelected(true);
  } else {
    item->setCheckState(Qt::Unchecked);
    item->setSelected(false);
  }
  Doc::DocComponentBlock *dcb = _map_Items_BigComponentBlocks.value(item);
  _docController->setCurrentBlock(dcb);
  _docController->setModified();
}

//void ChooseLabelForComponentForm::loadFontNamesFromTmpFolder() {
//    QString char_image_path =
//    ConfigurationManager::get(AppConfigMainGroup,AppConfigFontFolderKey).toString();
//    char_image_path.append("char_image_tmp/");
//    QDir imageCharFolder(char_image_path);
//    QStringList filters;
//    filters << "*.jpg"; filters << "*.png";
//    filters << "*.bmp"; filters << "*.jpeg";
//    filters << "*.PNG"; filters << "*.JPG";
//    imageCharFolder.setNameFilters(filters);
//    QStringList lstFileNames = imageCharFolder.entryList();

//    QMap<QString, int> mapFontNames;
//    for (QString file : lstFileNames) {
//        //format: [fontname]_U+[code]_[order].*
//        QString fontName = file;
//        fontName.remove(fontName.lastIndexOf("_"),fontName.length()-fontName.lastIndexOf("_")+1);
//        fontName.remove(fontName.lastIndexOf("_"),fontName.length()-fontName.lastIndexOf("_")+1);
//        mapFontNames.insert(fontName,1);
//    }
//    this->ui->cbFontNames->addItem("Unknown");
//    QMap<QString, int>::const_iterator iter = mapFontNames.constBegin();
//    while(iter!=mapFontNames.constEnd()) {
//        this->ui->cbFontNames->addItem(iter.key());
//        ++iter;
//    }

//}

void
ChooseLabelForComponentForm::createNewFont()
{
  _currentFont->setName(this->ui->txtFontName->toPlainText());

  QString char_image_path =
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigFontFolderKey)
      .toString();

  QDir char_image_pathDir(char_image_path);
  bool ok = char_image_pathDir.mkdir(this->ui->txtFontName->toPlainText());
  if (!ok) {
    qDebug() << "ERROR: unable to create subdir "
             << this->ui->txtFontName->toPlainText() << " in directory "
             << char_image_path;
    //B::TODO:UGLY...
  }

  QMap<QString, QMap<Doc::DocComponentBlock *, QImage> *>::const_iterator iter =
    _map_display_block_image.constBegin();
  while (iter != _map_display_block_image.constEnd()) {
    const QString character_unicode = iter.key();
    const int pos = character_unicode.indexOf(QStringLiteral("_"));
    const QString display = character_unicode.mid(0, pos);
    const unsigned long unicode =
      character_unicode.midRef(pos + 1, character_unicode.size()).toInt();

    Models::Character *character =
      new Models::Character(display, _upLine, _baseLine, _leftLine, _rightLine);

    Doc::Page *currentPage = _docController->getDocument()->currentPage();
    Q_ASSERT(currentPage != nullptr);

    QMap<Doc::DocComponentBlock *, QImage> *mapComponentBlockWithImage =
      iter.value();
    QMap<Doc::DocComponentBlock *, QImage>::const_iterator iter1 =
      mapComponentBlockWithImage->constBegin();
    //format: [fontname]_U+[code]_[order].*
    QString char_name;
    char_name.append(_currentFont->getName());
    char_name.append("_");
    char hex_code_point[128];
    sprintf(hex_code_point, "U+%.4lx", unicode); //B:TODO:UGLY ???
    char_name.append(QString::fromStdString(hex_code_point));
    char_name.append("_");

    while (iter1 != mapComponentBlockWithImage->constEnd()) {

      const QImage imageOfBlock = iter1.value();
      // create new character and data then add to font.
      QString char_ID;
      const int ID = getExistingID(char_name, char_image_path) + 1;
      char_ID.append(QString::number(ID));
      char_ID.append(".png");
      QString saveFolderPath = char_image_path + char_name + char_ID;
      imageOfBlock.save(saveFolderPath);

      Models::CharacterData *charData =
        new Models::CharacterData(imageOfBlock, ID);
      Q_ASSERT(charData != nullptr);
      character->add(charData);
      ++iter1;
    }
    _currentFont->addCharacter(character);
    ++iter;
  }
  // add space
  const int w_space = 10;
  QImage space(w_space, 3, QImage::Format_ARGB32); //B???
  space.fill(qRgba(0, 0, 0, 0));
  auto charSpaceData = new Models::CharacterData(space, 1);
  auto charSpace = new Models::Character(
    QStringLiteral(" "), _upLine, _baseLine, _leftLine, _rightLine);
  charSpace->add(charSpaceData);
  _currentFont->addCharacter(charSpace);

  //
  _docController->setModified();

  QString fontNamePath =
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigFontFolderKey)
      .toString();
  fontNamePath.append('/' + _currentFont->getName() + ".of");
  IOManager::FontFileManager::fontToXml(_currentFont, fontNamePath);
}

int
ChooseLabelForComponentForm::getExistingID(const QString &file_code,
                                           const QString &filePath) const
{
  QDir imageCharFolder(filePath);
  imageCharFolder.setNameFilters(getReadImageFilterList());
  const QStringList lstFileNames = imageCharFolder.entryList();
  int id = 0;
  for (const QString &file : lstFileNames) {
    if (file.contains(file_code)) {
      const int pos = file.lastIndexOf(QStringLiteral("_"));
      const QString strID =
        file.mid(pos + 1, file.indexOf(QStringLiteral("."), pos + 1) - pos - 1);
      const int currentID = strID.toInt();
      if (id < currentID)
        id = currentID;
    }
  }
  return id;
}

void
ChooseLabelForComponentForm::btnLabelingClicked()
{
  QString display = ui->txtLabel->toPlainText();
  unsigned long unicode = display.unicode()->unicode();
  QString code = ui->txtCodePoint->toPlainText();
  if ((display.isEmpty() || display.length() == 0) &&
      code.length() == 4) { // unicode 16 bit
    unicode = strtoul(code.toStdString().c_str(), nullptr, 16);
    const QChar qchar = QChar(static_cast<int>(unicode));
    display = QString(qchar);
  }
  QString character_unicode(display);

  character_unicode.append("_");
  character_unicode.append(QString::number(unicode));

  QMap<Doc::DocComponentBlock *, QImage> *map_block_images;
  auto it = _map_display_block_image.find(character_unicode);
  if (it != _map_display_block_image.end()) {
    map_block_images = it.value();
  } else {
    map_block_images = new QMap<Doc::DocComponentBlock *, QImage>();
    _map_display_block_image.insert(character_unicode, map_block_images);
  }

  // if the character have many componants
  if (_chkOneLettre->isChecked()) {

    qDebug() << "\t\t if the character have many componants ";
    QList<QListWidgetItem *> listCheckedItems;

    for (QListWidgetItem *item : _listWidgetSmall->findItems(
           QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
      if (item->checkState() == Qt::Checked) {
        listCheckedItems.push_back(item);
      }
    }
    for (QListWidgetItem *item : _listWidgetMedium->findItems(
           QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
      if (item->checkState() == Qt::Checked) {
        listCheckedItems.push_back(item);
      }
    }
    for (QListWidgetItem *item : _listWidgetBig->findItems(
           QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
      if (item->checkState() == Qt::Checked) {
        listCheckedItems.push_back(item);
      }
    }

    if (listCheckedItems.size() == 2) { //A character of two componants
      QListWidgetItem *first_item = listCheckedItems.at(0);
      QListWidgetItem *second_item = listCheckedItems.at(1);
      bool chk_big_1 = false, chk_medium_1 = false, chk_small_1 = false;
      bool chk_big_2 = false, chk_medium_2 = false, chk_small_2 = false;
      int char_x = 0, char_y = 0, char_w = 0, char_h = 0;

      Doc::DocComponentBlock *first_cb =
        _map_Items_BigComponentBlocks.value(first_item);
      QImage img_CharData_1 = _map_BigComponent_Imagedata.value(first_cb);

      if (first_cb == nullptr) {
        first_cb = _map_Items_MediumComponentBlocks.value(first_item);
        img_CharData_1 = _map_MediumComponent_Imagedata.value(first_cb);
      } else {
        chk_big_1 = true;
      }
      if (first_cb == nullptr) {
        first_cb = _map_Items_SmallComponentBlocks.value(first_item);
        img_CharData_1 = _map_SmallComponent_Imagedata.value(first_cb);
      } else {
        chk_medium_1 = true;
      }
      if (first_cb != nullptr)
        chk_small_1 = true;

      Doc::DocComponentBlock *second_cb =
        _map_Items_BigComponentBlocks.value(second_item);
      QImage img_CharData_2 = _map_BigComponent_Imagedata.value(second_cb);
      if (second_cb == nullptr) {
        second_cb = _map_Items_MediumComponentBlocks.value(second_item);
        img_CharData_2 = _map_MediumComponent_Imagedata.value(second_cb);
      } else {
        chk_big_2 = true;
      }
      if (second_cb == nullptr) {
        second_cb = _map_Items_SmallComponentBlocks.value(second_item);
        img_CharData_2 = _map_SmallComponent_Imagedata.value(second_cb);
      } else {
        chk_medium_2 = true;
      }
      if (second_cb != nullptr)
        chk_small_2 = true;
      //

      //B:TODO:OPTIM: In the following code, it seems that we copy blocks of images
      // could we use QImage.copy(Rect()) ?

      QImage img_FinalCharData;
      if (_cbPositions->currentText() == QLatin1String("Top")) { // top position
        int delta_h = img_CharData_1.height() / 3;
        if (img_CharData_1.height() > img_CharData_2.height())
          delta_h = img_CharData_2.height() / 3;

        int h = delta_h + img_CharData_1.height() + img_CharData_2.height();
        int w = img_CharData_1.width();
        if (w < img_CharData_2.width())
          w = img_CharData_2.width();

        img_FinalCharData = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
        img_FinalCharData.fill(
          Qt::
            transparent); //B:TODO:OPTIM: USELESS ? as we fill with qRgba(0, 0, 0, 0) afterwards ???
        img_FinalCharData.fill(qRgba(0, 0, 0, 0));

        //B:TODO:OPTIM: do not use pixel()/setPixel()
        if (img_CharData_1.width() * img_CharData_1.height() <
            img_CharData_2.width() * img_CharData_2.height()) {

          for (int y = 0; y < img_CharData_1.height(); ++y)
            for (int x = 0; x < img_CharData_1.width(); ++x)
              img_FinalCharData.setPixel((x + (w - img_CharData_1.width()) / 2),
                                         y,
                                         img_CharData_1.pixel(x, y));

          for (int y = 0; y < img_CharData_2.height(); ++y)
            for (int x = 0; x < img_CharData_2.width(); ++x)
              img_FinalCharData.setPixel(x + (w - img_CharData_2.width()) / 2,
                                         y + img_CharData_1.height() + delta_h,
                                         img_CharData_2.pixel(x, y));

        } else {

          for (int y = 0; y < img_CharData_2.height(); ++y)
            for (int x = 0; x < img_CharData_2.width(); ++x)
              img_FinalCharData.setPixel(x + (w - img_CharData_2.width()) / 2,
                                         y,
                                         img_CharData_2.pixel(x, y));

          for (int y = 0; y < img_CharData_1.height(); ++y)
            for (int x = 0; x < img_CharData_1.width(); ++x)
              img_FinalCharData.setPixel((x + (w - img_CharData_1.width()) / 2),
                                         y + img_CharData_2.height() + delta_h,
                                         img_CharData_1.pixel(x, y));
        }
        char_w = img_FinalCharData.width();
        char_h = img_FinalCharData.height();

      } else if (_cbPositions->currentText() ==
                 QLatin1String("Right")) { //right position
        ;
      } else if (_cbPositions->currentText() ==
                 QLatin1String("Bottom")) { //bottom position
        int delta_h = img_CharData_1.height() / 3;
        if (img_CharData_1.height() > img_CharData_2.height())
          delta_h = img_CharData_2.height() / 3;

        int h = delta_h + img_CharData_1.height() + img_CharData_2.height();
        int w = img_CharData_1.width();
        if (w < img_CharData_2.width())
          w = img_CharData_2.width();

        img_FinalCharData = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
        img_FinalCharData.fill(
          Qt::
            transparent); //B:TODO:OPTIM: USELESS ? as we fill with qRgba(0, 0, 0, 0) afterwards ???
        img_FinalCharData.fill(qRgba(0, 0, 0, 0));

        //B:TODO:OPTIM: do not use pixel()/setPixel()
        if (img_CharData_1.width() * img_CharData_1.height() <
            img_CharData_2.width() * img_CharData_2.height()) {
          for (int y = 0; y < img_CharData_1.height(); ++y)
            for (int x = 0; x < img_CharData_1.width(); ++x)
              img_FinalCharData.setPixel((x + (w - img_CharData_1.width()) / 2),
                                         y + img_CharData_2.height() + delta_h,
                                         img_CharData_1.pixel(x, y));

          for (int y = 0; y < img_CharData_2.height(); ++y)
            for (int x = 0; x < img_CharData_2.width(); ++x)
              img_FinalCharData.setPixel(x + (w - img_CharData_2.width()) / 2,
                                         y,
                                         img_CharData_2.pixel(x, y));

        } else {

          for (int y = 0; y < img_CharData_2.height(); ++y)
            for (int x = 0; x < img_CharData_2.width(); ++x)
              img_FinalCharData.setPixel(x + (w - img_CharData_2.width()) / 2,
                                         y + img_CharData_1.height() + delta_h,
                                         img_CharData_2.pixel(x, y));

          for (int y = 0; y < img_CharData_1.height(); ++y)
            for (int x = 0; x < img_CharData_1.width(); ++x)
              img_FinalCharData.setPixel((x + (w - img_CharData_1.width()) / 2),
                                         y,
                                         img_CharData_1.pixel(x, y));
        }
        char_w = img_FinalCharData.width();
        char_h = img_FinalCharData.height();
      } else { // left position
      }

      char_x = first_cb->x();
      if (first_cb->x() > second_cb->x())
        char_x = second_cb->x();
      char_y = first_cb->y();
      if (first_cb->y() > second_cb->y())
        char_y = second_cb->y();

      if (img_CharData_1.width() * img_CharData_1.height() <
          img_CharData_2.width() * img_CharData_2.height()) {
        // update image for component.
        second_cb->setWidth(char_w);
        second_cb->setHeight(char_h);
        second_cb->setX(char_x);
        second_cb->setY(char_y);

        if (chk_big_2) {
          _map_BigComponent_Imagedata[second_cb] = img_FinalCharData;
          map_block_images->insert(
            second_cb, _map_BigComponent_Imagedata.value(second_cb));
          _map_BigComponentBlock_DisplayForListWidget[second_cb] = false;
        } else if (chk_medium_2) {
          _map_MediumComponent_Imagedata[second_cb] = img_FinalCharData;
          map_block_images->insert(
            second_cb, _map_MediumComponent_Imagedata.value(second_cb));
          _map_MediumComponentBlock_DisplayForListWidget[second_cb] = false;
        } else {
          _map_SmallComponent_Imagedata[second_cb] = img_FinalCharData;
          map_block_images->insert(
            second_cb, _map_SmallComponent_Imagedata.value(second_cb));
          _map_SmallComponentBlock_DisplayForListWidget[second_cb] = false;
        }
        if (chk_big_1)
          _map_BigComponentBlock_DisplayForListWidget[first_cb] = false;
        if (chk_medium_1)
          _map_MediumComponentBlock_DisplayForListWidget[first_cb] = false;
        if (chk_small_1)
          _map_SmallComponentBlock_DisplayForListWidget[first_cb] = false;
      } else {

        first_cb->setWidth(char_w);
        first_cb->setHeight(char_h);
        first_cb->setX(char_x);
        first_cb->setY(char_y);

        if (chk_big_1) {
          _map_BigComponent_Imagedata[first_cb] = img_FinalCharData;
          map_block_images->insert(first_cb,
                                   _map_BigComponent_Imagedata.value(first_cb));
          _map_BigComponentBlock_DisplayForListWidget[first_cb] = false;
        } else if (chk_medium_1) {
          _map_MediumComponent_Imagedata[first_cb] = img_FinalCharData;
          map_block_images->insert(
            first_cb, _map_MediumComponent_Imagedata.value(first_cb));
          _map_MediumComponentBlock_DisplayForListWidget[first_cb] = false;
        } else {
          _map_SmallComponent_Imagedata[first_cb] = img_FinalCharData;
          map_block_images->insert(
            first_cb, _map_SmallComponent_Imagedata.value(first_cb));
          _map_SmallComponentBlock_DisplayForListWidget[first_cb] = false;
        }
        if (chk_big_2)
          _map_BigComponentBlock_DisplayForListWidget[second_cb] = false;
        if (chk_medium_2)
          _map_MediumComponentBlock_DisplayForListWidget[second_cb] = false;
        if (chk_small_2)
          _map_SmallComponentBlock_DisplayForListWidget[second_cb] = false;
      }
    }

  } else {
    // check separated characters.

    // for small items
    for (QListWidgetItem *item : _listWidgetSmall->findItems(
           QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
      if (item->checkState() == Qt::Checked) {
        Doc::DocComponentBlock *componentBlock =
          _map_Items_SmallComponentBlocks.value(item);
        map_block_images->insert(
          componentBlock, _map_SmallComponent_Imagedata.value(componentBlock));
        _map_SmallComponentBlock_DisplayForListWidget[componentBlock] = false;
      }
    }

    // for medium items
    for (QListWidgetItem *item : _listWidgetMedium->findItems(
           QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
      if (item->checkState() == Qt::Checked) {
        Doc::DocComponentBlock *componentBlock =
          _map_Items_MediumComponentBlocks.value(item);
        map_block_images->insert(
          componentBlock, _map_MediumComponent_Imagedata.value(componentBlock));
        _map_MediumComponentBlock_DisplayForListWidget[componentBlock] = false;
      }
    }
    // for big items
    for (QListWidgetItem *item : _listWidgetBig->findItems(
           QStringLiteral("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
      if (item->checkState() == Qt::Checked) {
        Doc::DocComponentBlock *componentBlock =
          _map_Items_BigComponentBlocks.value(item);

        map_block_images->insert(
          componentBlock, _map_BigComponent_Imagedata.value(componentBlock));
        _map_BigComponentBlock_DisplayForListWidget[componentBlock] = false;
      }
    }
    //
  }
  createListItems();
  _listWidgetSmall->repaint();
  _listWidgetMedium->repaint();
  _listWidgetBig->repaint();

  loadTable();
  _tableOfLabeledComponents->repaint();
}

/*
QImage ChooseLabelForComponentForm::getCharacterDataOf_display(int &x_i, int &y_i, QListWidgetItem *main_item, const QString &display,
                                                                const QMap<QListWidgetItem*, Doc::DocComponentBlock*> &map_Items_ComponentBlocks,
                                                                const QMap<Doc::DocComponentBlock*, QImage> &map_Component_Imagedata,
                                                                const QMap<Doc::DocComponentBlock*, bool> &map_ComponentBlock_DisplayForListWidget)
{

  QImage charData_specialCharacter;

  Doc::DocComponentBlock* componentBlock_specialCharacter = map_Items_ComponentBlocks.value(main_item);

  QImage imageOfComponent_i_0 = map_Component_Imagedata.value(componentBlock_specialCharacter);
  int y0 = componentBlock_specialCharacter->y() - imageOfComponent_i_0.height()*2/3;

  if (display!="i" && display!="?")
    y0 = componentBlock_specialCharacter->y() - 2*imageOfComponent_i_0.height();
  if (display=="?")
    y0 = componentBlock_specialCharacter->y() - 10*imageOfComponent_i_0.height();

  int x0 = componentBlock_specialCharacter->x();

  int w =0, h=0;

  QMap<QListWidgetItem*, Doc::DocComponentBlock*>::const_iterator iter = map_Items_ComponentBlocks.constBegin();
  bool found = false;
  while( !found && iter != map_Items_ComponentBlocks.end()) {

    Doc::DocComponentBlock* dcb = iter.value();

    if (dcb->x()>=x0 && dcb->x()<=(x0+imageOfComponent_i_0.width()) &&
        dcb->y()>y0 && dcb->y()<componentBlock_specialCharacter->y())
      {
        //qDebug() << " \t char i : Found the dot x, y =" << dcb->x() << ", " << dcb->y()  ;
        int delta_x = dcb->x()-componentBlock_specialCharacter->x();
        int delta_y = componentBlock_specialCharacter->y() - dcb->y();
        if (imageOfComponent_i_0.width() >= ( delta_x + dcb->width()))
          w = imageOfComponent_i_0.width();
        else
          w = delta_x + dcb->width();
        h =  delta_y + imageOfComponent_i_0.height();

        QImage imageOfComponent_i_1 = map_Component_Imagedata.value(dcb);

        //
        charData_specialCharacter = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
        charData_specialCharacter.fill(Qt::transparent); //B:TODO:OPTIM: USELESS ? as we fill with qRgba(0, 0, 0, 0) afterwards ???
        charData_specialCharacter.fill(qRgba(0, 0, 0, 0));

        //B:TODO:OPTIM: do no use setPixel()/pixel() & remove checks from inside loops
        for(int y = 0; y<h; ++y) {
          for(int x=0; x<w; ++x) {
            if (y< imageOfComponent_i_1.height() &&
                x > delta_x && x< (delta_x+imageOfComponent_i_1.width()))
              charData_specialCharacter.setPixel(x, y, imageOfComponent_i_1.pixel(x - delta_x, y));
            if (y>delta_y && y<(delta_y+imageOfComponent_i_0.height()) && x<imageOfComponent_i_0.width())
              charData_specialCharacter.setPixel(x, y, imageOfComponent_i_0.pixel(x, y-delta_y));
          }
        }

        // update position of char i
        x_i = componentBlock_specialCharacter->x();
        y_i = dcb->y();
        //qDebug() << " char i : x_i, y_i =" << x_i << " , "  << y_i;

        // remove item + componentblock;
        (_docController->getDocument()->currentPage())->remove(dcb);
        _componentBlocks.removeOne(dcb);
        map_ComponentBlock_DisplayForListWidget[dcb] = false;
        found =true;
      }
    ++iter;
  }
  if (!found) {
    // qDebug() << " \t char i: NOT found";
    QString dot_path = Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigFontFolderKey).toString() + "the_dot_tmp.png";//B:TODO:UGLY !!!!
    QImage imageOfComponent_i_1(dot_path);
    int delta_x = imageOfComponent_i_0.width()/2  - imageOfComponent_i_1.width()/2;
    int delta_y = 3*imageOfComponent_i_1.height()/2;
    //int x1 = componentBlock_specialCharacter->x() + delta_x;
    int y1 = componentBlock_specialCharacter->y() - 2*delta_y;
    x_i = componentBlock_specialCharacter->x();
    y_i = y1;

    w= componentBlock_specialCharacter->width();
    h = componentBlock_specialCharacter->height() + delta_y;

    charData_specialCharacter = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
    charData_specialCharacter.fill(Qt::transparent);//B:TODO:OPTIM: USELESS ? as we fill with qRgba(0, 0, 0, 0) afterwards ???
    charData_specialCharacter.fill(qRgba(0, 0, 0, 0));

    //B:TODO:OPTIM: do no use setPixel()/pixel() & remove checks from inside loops
    for(int y = 0; y<h; ++y) {
      for(int x=0; x<w; ++x) {

        if (y< imageOfComponent_i_1.height() &&
            x > delta_x && x< (delta_x+imageOfComponent_i_1.width()))
          charData_specialCharacter.setPixel(x, y, imageOfComponent_i_1.pixel(x - delta_x, y));

        if (y>delta_y && y<(delta_y+imageOfComponent_i_0.height()) && x<imageOfComponent_i_0.width())
          charData_specialCharacter.setPixel(x, y, imageOfComponent_i_0.pixel(x, y-delta_y));
      }
    }

  }

  return charData_specialCharacter;
}
*/

void
ChooseLabelForComponentForm::initialize()
{

  //B:TODO: lots of CODE DUPLICATION here !

  const QList<Doc::DocComponentBlock *> &smallCBlocks = _smallCBlocks;
  for (Doc::DocComponentBlock *dcb : smallCBlocks) {

    _map_SmallComponentBlock_DisplayForListWidget.insert(dcb, true);
    for (Doc::DocZone *dz : dcb->getZones())
      for (Doc::DocComponent *dc : dz->getComponents()) {

        QImage imageCharData(
          dc->width(), dc->height(), QImage::Format_ARGB32_Premultiplied);
        imageCharData.fill(
          Qt::
            transparent); //B:TODO:USELESS because filled with qRgba(0, 0, 0, 0) afterwards ???
        imageCharData.fill(qRgba(0, 0, 0, 0));
        fillScanlineToImage(imageCharData, dc);
        _map_SmallComponent_Imagedata.insert(dcb, imageCharData);
      }
  }

  const QList<Doc::DocComponentBlock *> &MediumCBlocks = _MediumCBlocks;
  for (Doc::DocComponentBlock *dcb : MediumCBlocks) {

    _map_MediumComponentBlock_DisplayForListWidget.insert(dcb, true);
    for (Doc::DocZone *dz : dcb->getZones())
      for (Doc::DocComponent *dc : dz->getComponents()) {

        QImage imageCharData(
          dc->width(), dc->height(), QImage::Format_ARGB32_Premultiplied);
        imageCharData.fill(
          Qt::
            transparent); //B:TODO:USELESS because filled with qRgba(0, 0, 0, 0) afterwards ???
        imageCharData.fill(qRgba(0, 0, 0, 0));
        fillScanlineToImage(imageCharData, dc);
        _map_MediumComponent_Imagedata.insert(dcb, imageCharData);
      }
  }

  const QList<Doc::DocComponentBlock *> &bigCBlocks = _bigCBlocks;
  for (Doc::DocComponentBlock *dcb : bigCBlocks) {

    _map_BigComponentBlock_DisplayForListWidget.insert(dcb, true);
    for (Doc::DocZone *dz : dcb->getZones())
      for (Doc::DocComponent *dc : dz->getComponents()) {

        QImage imageCharData(
          dc->width(), dc->height(), QImage::Format_ARGB32_Premultiplied);
        imageCharData.fill(
          Qt::
            transparent); //B:TODO:USELESS because filled with qRgba(0, 0, 0, 0) afterwards ???
        imageCharData.fill(qRgba(0, 0, 0, 0));
        fillScanlineToImage(imageCharData, dc);
        _map_BigComponent_Imagedata.insert(dcb, imageCharData);
      }
  }
}

void
ChooseLabelForComponentForm::createListItems()
{

  //loading small charachers
  _listWidgetSmall->clear();
  QMap<Doc::DocComponentBlock *, bool>::const_iterator iter =
    _map_SmallComponentBlock_DisplayForListWidget.constBegin();
  _map_Items_SmallComponentBlocks.clear();
  while (iter != _map_SmallComponentBlock_DisplayForListWidget.constEnd()) {
    Doc::DocComponentBlock *dcb = iter.key();
    if (iter.value()) {

      QListWidgetItem *item = new QListWidgetItem(_listWidgetSmall);
      // drawing icon
      const QIcon icon =
        QIcon(QPixmap::fromImage(_map_SmallComponent_Imagedata.value(dcb)));
      item->setIcon(icon);
      item->setText(tr("Unknown"));
      item->setFlags(Qt::ItemIsUserCheckable);
      item->setFlags(Qt::ItemIsEnabled);
      item->setSelected(false);
      item->setCheckState(Qt::Unchecked);

      _listWidgetSmall->addItem(item);
      _map_Items_SmallComponentBlocks.insert(item, dcb);
    }
    ++iter;
  }
  //loading medium charachers
  _listWidgetMedium->clear();
  QMap<Doc::DocComponentBlock *, bool>::const_iterator iterMedium =
    _map_MediumComponentBlock_DisplayForListWidget.constBegin();
  _map_Items_MediumComponentBlocks.clear();
  while (iterMedium !=
         _map_MediumComponentBlock_DisplayForListWidget.constEnd()) {
    Doc::DocComponentBlock *dcb = iterMedium.key();
    if (iterMedium.value()) {

      QListWidgetItem *item = new QListWidgetItem(_listWidgetMedium);
      // drawing icon
      const QIcon icon =
        QIcon(QPixmap::fromImage(_map_MediumComponent_Imagedata.value(dcb)));
      item->setIcon(icon);
      item->setText(tr("Unknown"));
      item->setFlags(Qt::ItemIsUserCheckable);
      item->setFlags(Qt::ItemIsEnabled);
      item->setSelected(false);
      item->setCheckState(Qt::Unchecked);

      _listWidgetMedium->addItem(item);
      _map_Items_MediumComponentBlocks.insert(item, dcb);
    }
    ++iterMedium;
  }
  //loading big charachers
  _listWidgetBig->clear();
  QMap<Doc::DocComponentBlock *, bool>::const_iterator iterBig =
    _map_BigComponentBlock_DisplayForListWidget.constBegin();
  _map_Items_BigComponentBlocks.clear();
  while (iterBig != _map_BigComponentBlock_DisplayForListWidget.constEnd()) {
    Doc::DocComponentBlock *dcb = iterBig.key();
    if (iterBig.value()) {

      QListWidgetItem *item = new QListWidgetItem(_listWidgetBig);
      // drawing icon
      const QIcon icon =
        QIcon(QPixmap::fromImage(_map_BigComponent_Imagedata.value(dcb)));
      item->setIcon(icon);
      item->setText(tr("Unknown"));
      item->setFlags(Qt::ItemIsUserCheckable);
      item->setFlags(Qt::ItemIsEnabled);
      item->setSelected(false);
      item->setCheckState(Qt::Unchecked);

      _listWidgetBig->addItem(item);
      _map_Items_BigComponentBlocks.insert(item, dcb);
    }
    ++iterBig;
  }
}
void
ChooseLabelForComponentForm::loadItems()
{
  //    _listWidget->clear();
  //    for (QListWidgetItem* item : _listItem)
  //        _listWidget->addItem(item);
}

ChooseLabelForComponentForm::~ChooseLabelForComponentForm()
{
  delete ui;
  delete _currentFont;
  delete _currentCharacterMap;
}
