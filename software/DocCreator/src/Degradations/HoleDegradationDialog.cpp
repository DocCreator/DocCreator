#include "HoleDegradationDialog.hpp"
#include "ui_HoleDegradationDialog.h"
#include "HoleDegradationQ.hpp"

#include <cassert>
#include <ctime> //time

#include <QColorDialog>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>

#include "appconstants.h"
#include "core/configurationmanager.h"

#include "Utils/ImageUtils.hpp"
#include "Utils/convertor.h"

#include <iostream> //DEBUG

static const int IMG_WIDTH =
  180; //Size of image displayed in dialog (origin & result)
static const int IMG_HEIGHT = 300;
static const int ZOOM_WIDTH = 200; //Size of the zoom image
static const int ZOOM_HEIGHT = 200;

static const int DEFAULT_WIDTH = 5; //default with of the shadow in the hole

static const int NB_TRY_MAX =
  20; //Define how many try we make before give up when we try to generate a random hole and when we don't find any place (to prevent of to big pattern for example)

static const int MARGIN =
  50; //Default margin that define the minimum length that the hole has to have in image (it can be a little outside, but has to at least have a number of pixel equal to margin)

QString
HoleDegradationDialog::getHolePatternsPath()
{
  return Core::ConfigurationManager::get(AppConfigMainGroup,
                                         AppConfigHolePatternsFolderKey)
    .toString();
}

QString
HoleDegradationDialog::getCenterHolePatternsPath()
{
  return QDir(HoleDegradationDialog::getHolePatternsPath())
    .absoluteFilePath(QStringLiteral("centerHoles"));
}

QString
HoleDegradationDialog::getBorderHolePatternsPath()
{
  return QDir(HoleDegradationDialog::getHolePatternsPath())
    .absoluteFilePath(QStringLiteral("borderHoles"));
}

QString
HoleDegradationDialog::getCornerHolePatternsPath()
{
  return QDir(HoleDegradationDialog::getHolePatternsPath())
    .absoluteFilePath(QStringLiteral("cornerHoles"));
}

/*
  Get list of files in directory @a dirName.
  Directories "." and ".." are removed from the list.
*/
QStringList
HoleDegradationDialog::getDirectoryList(const QString &dirName)
{
  QStringList list = QDir(dirName).entryList();
  if (!list.empty()) {
    assert(list.size() >= 2);
    list.removeFirst();
    list.removeFirst();
  }
  return list;
}

HoleDegradationDialog::HoleDegradationDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::HoleDegradationDialog)
  , _originalLabel(nullptr)
  , _resultLabel(nullptr)
{
  ui->setupUi(this);
  _color = QColor(0, 0, 0, 255); //Default color : Black and no transparent
  ui->colorLabel->setStyleSheet(
    QStringLiteral("QLabel { background-color: rgba(0, 0, 0, "
                   "255); }")); //initialise also the label in dialog
  _pageBelow = QImage();
  _pattern = 0;
  _patternImg = QImage(QDir(getCenterHolePatternsPath())
                         .absoluteFilePath(QStringLiteral("pattern1.png")));
  _horizontal = ui->horizontalSlider->value();
  _vertical = ui->verticalSlider->value();
  _size = ui->sizeSlider->value();
  _type = static_cast<dc::HoleDegradation::HoleType>( ui->typeComboBox->currentIndex() );
  _side = static_cast<int>(dc::HoleDegradation::Border::TOP);
  _width = 0;
  _intensity = ui->intensitySlider->value();
  _holes = QList<Hole>();
  ui->borderGroupBox->setVisible(false);
  ui->cornerGroupBox->setVisible(false);

  //QString dirName = getCenterHolePatternsPath();

  _patterns = getDirectoryList(getCenterHolePatternsPath());

  hideAdvancedOptions();

  connect(ui->horizontalSlider,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(horizontalChanged(int)));
  connect(ui->verticalSlider,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(verticalChanged(int)));
  connect(
    ui->sizeSlider, SIGNAL(valueChanged(int)), this, SLOT(sizeChanged(int)));
  connect(
    ui->widthSlider, SIGNAL(valueChanged(int)), this, SLOT(widthChanged(int)));
  connect(ui->intensitySlider,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(intensityChanged(int)));
  connect(ui->typeComboBox,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(typeChanged(int)));
  connect(ui->browseButton, SIGNAL(clicked()), this, SLOT(choosePageBelow()));
  connect(ui->shadowCheckBox, SIGNAL(clicked()), this, SLOT(chooseShadow()));
  connect(
    ui->belowLineEdit, SIGNAL(editingFinished()), this, SLOT(belowChanged()));
  connect(
    ui->transparencyButton, SIGNAL(clicked()), this, SLOT(setTransparent()));
  connect(ui->colorButton, SIGNAL(clicked()), this, SLOT(chooseColor()));
  connect(ui->advancedOptionsLink,
          SIGNAL(linkActivated(QString)),
          this,
          SLOT(advancedOptionsClicked()));
  connect(ui->previousButton, SIGNAL(clicked()), this, SLOT(previousClicked()));
  connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(nextClicked()));
  connect(ui->topButton, SIGNAL(clicked()), this, SLOT(borderButtonClicked()));
  connect(
    ui->bottomButton, SIGNAL(clicked()), this, SLOT(borderButtonClicked()));
  connect(
    ui->rightButton, SIGNAL(clicked()), this, SLOT(borderButtonClicked()));
  connect(ui->leftButton, SIGNAL(clicked()), this, SLOT(borderButtonClicked()));
  connect(
    ui->topLeftButton, SIGNAL(clicked()), this, SLOT(cornerButtonClicked()));
  connect(
    ui->topRightButton, SIGNAL(clicked()), this, SLOT(cornerButtonClicked()));
  connect(
    ui->bottomLeftButton, SIGNAL(clicked()), this, SLOT(cornerButtonClicked()));
  connect(ui->bottomRightButton,
          SIGNAL(clicked()),
          this,
          SLOT(cornerButtonClicked()));
  connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(setHole()));
  connect(ui->randomButton, SIGNAL(clicked()), this, SLOT(generateHoles()));
  connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(removeHole()));
}

HoleDegradationDialog::~HoleDegradationDialog()
{
  delete ui;
}

void
HoleDegradationDialog::previousClicked()
{
  --_pattern;

  if (_pattern < 0)
    _pattern = _patterns.size() - 1;

  updatePatterns();
}

void
HoleDegradationDialog::nextClicked()
{
  ++_pattern;

  if (_pattern > _patterns.size() - 1)
    _pattern = 0;

  updatePatterns();
}

void
HoleDegradationDialog::updatePatterns()
{
  if (_pattern < 0) { //happens when no pattern was loaded
    return;
  }

  QString path;

  if (_type == dc::HoleDegradation::HoleType::CENTER) {
    path = getCenterHolePatternsPath();
  }
  else if (_type == dc::HoleDegradation::HoleType::BORDER) {
    path = getBorderHolePatternsPath();
  }
  else if (_type == dc::HoleDegradation::HoleType::CORNER) {
    path = getCornerHolePatternsPath();
  }

  _patterns = getDirectoryList(path);

  const int numPatterns = _patterns.size();
  if (numPatterns == 0) {
    return;
  }

  if (_pattern >= numPatterns) {
    _pattern = 0;
  }

  assert(_pattern >= 0 && _pattern < numPatterns);
  path = QDir(path).absoluteFilePath(_patterns.at(_pattern));

  _patternImg = QImage(path);
  ui->patternLabel->setPixmap(QPixmap::fromImage(
    _patternImg.scaled(130, 200, Qt::KeepAspectRatio, Qt::FastTransformation)));

  sideChanged(_side); // will update constraints and updateSliders();
  this->updateGeometry();

  if (!_originalImgSmall.isNull()) {
    updateResultImage();
  }
}

void
HoleDegradationDialog::setHole()
{
  if (QMessageBox::question(this,
                            tr("Are you sure ?"),
                            tr("Do you want to apply this hole definitely ?"),
                            QMessageBox::Yes | QMessageBox::No) ==
      QMessageBox::Yes) {
    _degradedImg = _resultImg;
    _degradedImg.setAlphaChannel(_resultImg.alphaChannel());
    // _originalImgSmall = _originalImg.scaled(IMG_WIDTH, IMG_HEIGHT,
    // Qt::KeepAspectRatio, Qt::FastTransformation);

    ui->horizontalSlider->setMaximum(_originalImg.width());
    ui->verticalSlider->setMaximum(_originalImg.height());

    _holes.append(Hole(_horizontal, _vertical, _size, _color, _patternImg));
    // _originalLabel->setPixmap(QPixmap::fromImage(_originalImgSmall));
  }
}

void
HoleDegradationDialog::setOriginalImage(const QImage &img)
{
  _originalImg = img;
  _originalImg.setAlphaChannel(img.alphaChannel());
  _originalImgSmall = img.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation);

  ui->horizontalSlider->setMaximum(img.width());
  ui->verticalSlider->setMaximum(img.height());

  _degradedImg = _originalImg;
  _degradedImg.setAlphaChannel(_originalImg.alphaChannel());

  updateSliders();

  setupGUIImages();
}

void
HoleDegradationDialog::setupGUIImages()
{
  QLabel *pattern = ui->patternLabel;
  pattern->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  pattern->setText(tr("pattern"));

  QImage _tmpPatternImg =
    QImage(QDir(getCenterHolePatternsPath())
             .absoluteFilePath(QStringLiteral("pattern1.png")));
  pattern->setPixmap(QPixmap::fromImage(_tmpPatternImg.scaled(
    130, 200, Qt::KeepAspectRatio, Qt::FastTransformation)));

  if (!_originalImgSmall.isNull()) {

    _originalLabel = ui->originalLabel;
    _originalLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _originalLabel->setText(tr("original"));
    _originalLabel->setPixmap(QPixmap::fromImage(_originalImgSmall));

    _resultLabel = ui->resultLabel;
    _resultLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _resultLabel->setText(tr("result"));
    _resultLabel->setPixmap(QPixmap::fromImage(_originalImgSmall));

    updateResultImage();

    this->updateGeometry();
  }
}

void
HoleDegradationDialog::updateResultImage()
{
  if (_holes.isEmpty())
    _resultImg = dc::HoleDegradation::holeDegradation(_originalImg,
                                 _patternImg,
                                 _horizontal,
                                 _vertical,
                                 _size,
                                 _type,
                                 _side,
                                 _color,
                                 _pageBelow,
                                 _width,
                                 _intensity);
  else
    _resultImg = dc::HoleDegradation::holeDegradation(_degradedImg,
                                 _patternImg,
                                 _horizontal,
                                 _vertical,
                                 _size,
                                 _type,
                                 _side,
                                 _color,
                                 _pageBelow,
                                 _width,
                                 _intensity);

  _resultImgSmall = _resultImg.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation);

  _resultLabel->setPixmap(QPixmap::fromImage(_resultImgSmall));
  updateZoom();
}

void
HoleDegradationDialog::updateZoom()
{
  int x = std::max(0, _horizontal);
  int y = std::max(0, _vertical);
  //int w = std::min(_patternImg.width()+_size, _resultImg.width());
  //int h = std::min(_patternImg.height()+_size, _resultImg.height());
  int w, h;
  if (_type == dc::HoleDegradation::HoleType::BORDER
      && (_side == static_cast<int>(dc::HoleDegradation::Border::LEFT) ||
	  _side == static_cast<int>(dc::HoleDegradation::Border::RIGHT))) {
    w = std::min(_patternImg.height() + _size, _resultImg.width());
    h = std::min(_patternImg.width() + _size, _resultImg.height());
  } else {
    w = std::min(_patternImg.width() + _size, _resultImg.width());
    h = std::min(_patternImg.height() + _size, _resultImg.height());
  }

  QImage zoomImg = _resultImg.copy(QRect(x, y, w, h));

  ui->zoomLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->zoomLabel->setText(tr("zoom"));
  ui->zoomLabel->setPixmap(QPixmap::fromImage(zoomImg.scaled(
    ZOOM_WIDTH, ZOOM_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation)));
}

void
HoleDegradationDialog::chooseColor()
{
  QColor color = QColorDialog::getColor(Qt::black,
                                        this,
                                        tr("Choose the color of background"),
                                        QColorDialog::ShowAlphaChannel);

  ui->colorLabel->setAutoFillBackground(true);
  QString values = QString::number(color.red()) + ", " +
                   QString::number(color.green()) + ", " +
                   QString::number(color.blue()) + ", " +
                   QString::number(color.alpha());

  ui->colorLabel->setStyleSheet("QLabel { background-color: rgba(" + values +
                                "); }");

  if (color.alpha() == 0)
    ui->colorLabel->setVisible(false);
  else
    ui->colorLabel->setVisible(true);

  colorChanged(color);
}

void
HoleDegradationDialog::choosePageBelow()
{
  QString path =
    QFileDialog::getOpenFileName(this,
                                 tr("Open File"),
                                 QStringLiteral("/DocCreator/Image"),
                                 getReadImageFilter());
  ui->belowLineEdit->setText(path); //no tr because no traduction for path

  setTransparent();
  belowChanged();
}

void
HoleDegradationDialog::chooseShadow()
{
  if (ui->shadowCheckBox->isChecked()) {
    widthChanged(DEFAULT_WIDTH);
    ui->widthSlider->setValue(_width);
    ui->widthSlider->setSliderPosition(_width);
    if (ui->verticalLabel->isVisible()) {
      ui->shadowLabel->setVisible(true);
      ui->widthLabel->setVisible(true);
      ui->widthSlider->setVisible(true);
      ui->intensityLabel->setVisible(true);
      ui->intensitySlider->setVisible(true);
    }
  } else {
    widthChanged(0); //reset the width if we don't use the shadow
    if (ui->verticalLabel->isVisible()) {
      ui->shadowLabel->setVisible(false);
      ui->widthLabel->setVisible(false);
      ui->widthSlider->setVisible(false);
      ui->intensityLabel->setVisible(false);
      ui->intensitySlider->setVisible(false);
    }
  }
}

void
HoleDegradationDialog::advancedOptionsClicked()
{
  if (ui->horizontalLabel->isVisible())
    hideAdvancedOptions();
  else
    showAdvancedOptions();
}

void
HoleDegradationDialog::hideAdvancedOptions()
{
  ui->horizontalLabel->setVisible(false);
  ui->horizontalSlider->setVisible(false);
  ui->verticalLabel->setVisible(false);
  ui->verticalSlider->setVisible(false);
  ui->sizeLabel->setVisible(false);
  ui->sizeSlider->setVisible(false);
  ui->shadowLabel->setVisible(false);
  ui->widthLabel->setVisible(false);
  ui->widthSlider->setVisible(false);
  ui->intensityLabel->setVisible(false);
  ui->intensitySlider->setVisible(false);

  this->adjustSize();
}

void
HoleDegradationDialog::showAdvancedOptions()
{
  ui->horizontalLabel->setVisible(true);
  ui->horizontalSlider->setVisible(true);
  ui->verticalLabel->setVisible(true);
  ui->verticalSlider->setVisible(true);
  ui->sizeLabel->setVisible(true);
  ui->sizeSlider->setVisible(true);

  if (ui->shadowCheckBox->isChecked()) {
    ui->shadowLabel->setVisible(true);
    ui->widthLabel->setVisible(true);
    ui->widthSlider->setVisible(true);
    ui->intensityLabel->setVisible(true);
    ui->intensitySlider->setVisible(true);
  }

  this->updateGeometry();
}

void
HoleDegradationDialog::borderButtonClicked()
{
  if (ui->topButton->isChecked())
    sideChanged(static_cast<int>(dc::HoleDegradation::Border::TOP));
  else if (ui->rightButton->isChecked())
    sideChanged(static_cast<int>(dc::HoleDegradation::Border::RIGHT));
  else if (ui->bottomButton->isChecked())
    sideChanged(static_cast<int>(dc::HoleDegradation::Border::BOTTOM));
  else if (ui->leftButton->isChecked())
    sideChanged(static_cast<int>(dc::HoleDegradation::Border::LEFT));
}

void
HoleDegradationDialog::cornerButtonClicked()
{
  if (ui->topLeftButton->isChecked())
    sideChanged(static_cast<int>(dc::HoleDegradation::Corner::TOPLEFT));
  else if (ui->topRightButton->isChecked())
    sideChanged(static_cast<int>(dc::HoleDegradation::Corner::TOPRIGHT));
  else if (ui->bottomRightButton->isChecked())
    sideChanged(static_cast<int>(dc::HoleDegradation::Corner::BOTTOMRIGHT));
  else if (ui->bottomLeftButton->isChecked())
    sideChanged(static_cast<int>(dc::HoleDegradation::Corner::BOTTOMLEFT));
}

void
HoleDegradationDialog::findBound(dc::HoleDegradation::HoleType type,
                                 int &minH,
                                 int &maxH,
                                 int &minV,
                                 int &maxV,
                                 int side,
                                 const QImage &patternImg)
{
  switch (type) {
  case dc::HoleDegradation::HoleType::CENTER:
      minH = 0;
      maxH = _originalImg.width() - patternImg.width(); // + MARGIN;
      minV = 0;
      maxV = _originalImg.height() - patternImg.height(); // + MARGIN;
      break;

  case dc::HoleDegradation::HoleType::BORDER:
    switch (static_cast<dc::HoleDegradation::Border>(side)) {
      case dc::HoleDegradation::Border::TOP:
          minH = -patternImg.width() + MARGIN;
          maxH = _originalImg.width() - MARGIN;
          minV = -patternImg.height() + MARGIN;
          maxV = 0;
          break;
      case dc::HoleDegradation::Border::RIGHT:
          minH = _originalImg.width() - patternImg.height();
          maxH = _originalImg.width() - MARGIN;
          minV = -patternImg.width() + MARGIN;
          maxV = _originalImg.width() - MARGIN;
          break;
      case dc::HoleDegradation::Border::BOTTOM:
          minH = -patternImg.width() + MARGIN;
          maxH = _originalImg.width() - MARGIN;
          minV = _originalImg.height() - patternImg.height();
          maxV = _originalImg.height() - MARGIN;
          break;
      case dc::HoleDegradation::Border::LEFT:
          minH = -patternImg.height() + MARGIN;
          maxH = 0;
          minV = -patternImg.width() + MARGIN;
          maxV = _originalImg.width() - MARGIN;
          break;
      }
      break;

  case dc::HoleDegradation::HoleType::CORNER:
    switch (static_cast<dc::HoleDegradation::Corner>(side)) {
      case dc::HoleDegradation::Corner::TOPLEFT:
          minH = -patternImg.width() + MARGIN;
          maxH = 0;
          minV = -patternImg.height() + MARGIN;
          maxV = 0;
          break;
      case dc::HoleDegradation::Corner::TOPRIGHT:
          minH = _originalImg.width() - patternImg.width();
          maxH = _originalImg.width() - MARGIN;
          minV = -patternImg.height() + MARGIN;
          maxV = 0;
          break;
      case dc::HoleDegradation::Corner::BOTTOMRIGHT:
          minH = _originalImg.width() - patternImg.width();
          maxH = _originalImg.width() - MARGIN;
          minV = _originalImg.height() - patternImg.height();
          maxV = _originalImg.height() - MARGIN;
          break;
      case dc::HoleDegradation::Corner::BOTTOMLEFT:
          minH = -patternImg.width() + MARGIN;
          maxH = 0;
          minV = _originalImg.height() - patternImg.height();
          maxV = _originalImg.height() - MARGIN;
          break;
      }
      break;

  case dc::HoleDegradation::HoleType::NUM_HOLE_TYPES:
    default:
      minH = 0;
      maxH = 0;
      minV = 0;
      maxV = 0;
      break;
  }
}

void
HoleDegradationDialog::updateSliders()
{
  int minH, maxH, minV, maxV;
  findBound(_type, minH, maxH, minV, maxV, _side, _patternImg);

  ui->horizontalSlider->setMinimum(minH);
  ui->horizontalSlider->setMaximum(maxH);
  ui->verticalSlider->setMinimum(minV);
  ui->verticalSlider->setMaximum(maxV);

  ui->verticalSlider->setValue(_vertical);
  ui->verticalSlider->setSliderPosition(_vertical);
  ui->horizontalSlider->setValue(_horizontal);
  ui->horizontalSlider->setSliderPosition(_horizontal);
}

void
HoleDegradationDialog::sideChanged(int side)
{
  _side = side;

  switch (_side) {
    case 0: //TOP OR TOPLEFT
      _horizontal = 0;
      _vertical = 0;
      break;
    case 1: //RIGHT OR TOPRIGHT
      _horizontal = _originalImg.width() - _patternImg.width();
      _vertical = 0;
      break;
    case 2: //BOTTOM OR BOTTOMRIGHT
      if (_type == dc::HoleDegradation::HoleType::BORDER) {
        _horizontal = 0;
      }
      else if (_type == dc::HoleDegradation::HoleType::CORNER) {
        _horizontal = _originalImg.width() - _patternImg.width();
      }
      _vertical = _originalImg.height() - _patternImg.height();
      break;
    case 3: //LEFT OR BOTTOM LEF
      _horizontal = 0;

      if (_type == dc::HoleDegradation::HoleType::BORDER) {
        _vertical = 0;
      }
      else if (_type == dc::HoleDegradation::HoleType::CORNER) {
        _vertical = _originalImg.height() - _patternImg.height();
      }

      break;
  }

  updateSliders();

  if (!_originalImgSmall.isNull()) {
    updateResultImage();
  }
}

void
HoleDegradationDialog::setTransparent()
{
  _color = QColor(255, 255, 255, 0);
  ui->colorLabel->setVisible(false);

  if (!_originalImgSmall.isNull()) {
    updateResultImage();
  }
}

void
HoleDegradationDialog::colorChanged(QColor color)
{
  _color = color;

  if (!_originalImgSmall.isNull()) {
    updateResultImage();
  }
}

void
HoleDegradationDialog::belowChanged()
{
  QString path;
  if (!ui->belowLineEdit->text().isEmpty()) {
    path = ui->belowLineEdit->text();
  }
  else {
    path = QString();
  }

  _pageBelow = QImage(path);

  if (!_originalImgSmall.isNull()) {
    updateResultImage();
  }
}

void
HoleDegradationDialog::horizontalChanged(int horizontal)
{
  _horizontal = horizontal;

  if (!_originalImgSmall.isNull()) {
    updateResultImage();
  }
}

void
HoleDegradationDialog::verticalChanged(int vertical)
{
  _vertical = vertical;

  if (!_originalImgSmall.isNull()) {
    updateResultImage();
  }
}

void
HoleDegradationDialog::sizeChanged(int size)
{
  _size = size;

  if (!_originalImgSmall.isNull()) {
    updateResultImage();
  }
}

void
HoleDegradationDialog::widthChanged(int width)
{
  _width = width;

  if (!_originalImgSmall.isNull()) {
    updateResultImage();
  }
}

void
HoleDegradationDialog::intensityChanged(int intensity)
{
  _intensity = -intensity * 0.1;

  if (!_originalImgSmall.isNull()) {
    updateResultImage();
  }
}

void
HoleDegradationDialog::typeChanged(int type)
{
  _type = static_cast<dc::HoleDegradation::HoleType>(type);

  ui->borderGroupBox->setVisible(false);
  ui->cornerGroupBox->setVisible(false);

  if (_type == dc::HoleDegradation::HoleType::BORDER) {
    ui->borderGroupBox->setVisible(true);
    ui->topButton->setChecked(true);
    sideChanged(static_cast<int>(dc::HoleDegradation::Border::TOP));
  }
  else if (_type == dc::HoleDegradation::HoleType::CORNER) {
    ui->cornerGroupBox->setVisible(true);
    ui->topLeftButton->setChecked(true);
    sideChanged(static_cast<int>(dc::HoleDegradation::Corner::TOPLEFT));
  }

  updatePatterns();

  if (!_originalImgSmall.isNull()) {
    updateResultImage();
  }
}

void
HoleDegradationDialog::changeEvent(QEvent *e)
{
  QDialog::changeEvent(e);
  switch (e->type()) {
    case QEvent::LanguageChange:
      ui->retranslateUi(this);
      break;
    default:
      break;
  }
}

void
HoleDegradationDialog::removeHole()
{
  if (!_holes.isEmpty()) {
    Hole hole = _holes.last();
    _holes.removeLast();

    cv::Mat tmpMat = Convertor::getCvMat(_originalImg);
    cv::Mat resultMat = Convertor::getCvMat(_resultImg);

    for (int y = hole.getY(); y < hole.getY() + hole.getHeight(); y++) {
      for (int x = hole.getX(); x < hole.getX() + hole.getWidth(); x++) {
        if (y > 0 && y < resultMat.rows && x > 0 && x < resultMat.cols) {
          resultMat.at<cv::Vec3b>(y, x) = tmpMat.at<cv::Vec3b>(y, x);
	}
      }
    }

    _degradedImg = Convertor::getQImage(resultMat);

    updateResultImage();
  }
}

//RANDOM

static int
countHole(dc::HoleDegradation::HoleType type)
{
  QString dirName;

  switch (type) {
  case dc::HoleDegradation::HoleType::CENTER:
      dirName = HoleDegradationDialog::getCenterHolePatternsPath();
      break;
  case dc::HoleDegradation::HoleType::BORDER:
      dirName = HoleDegradationDialog::getBorderHolePatternsPath();
      break;
  case dc::HoleDegradation::HoleType::CORNER:
      dirName = HoleDegradationDialog::getCornerHolePatternsPath();
      break;
  case dc::HoleDegradation::HoleType::NUM_HOLE_TYPES:
      break;
  }

  QStringList patterns = HoleDegradationDialog::getDirectoryList(dirName);

  return patterns.size();
}

/*
Allow to get the path of a pattern just with the type of hole and the number of the pattern
 */
static QString
getPath(dc::HoleDegradation::HoleType type, int pattern)
{
  QString path;

  if (type == dc::HoleDegradation::HoleType::CENTER) {
    path = HoleDegradationDialog::getCenterHolePatternsPath();
  }
  else if (type == dc::HoleDegradation::HoleType::BORDER) {
    path = HoleDegradationDialog::getBorderHolePatternsPath();
  }
  else if (type == dc::HoleDegradation::HoleType::CORNER) {
    path = HoleDegradationDialog::getCornerHolePatternsPath();
  }

  const QStringList patterns = HoleDegradationDialog::getDirectoryList(path);

  path = QDir(path).absoluteFilePath(patterns.at(pattern));

  return path;
}

bool
HoleDegradationDialog::containsHole(const QList<Hole> &holes,
             const QImage &pattern,
             int horizontal,
             int vertical,
             int size)
{
  const QPoint topLeft = QPoint(horizontal, vertical);
  const QPoint topRight = QPoint(horizontal + pattern.width() + size, vertical);
  const QPoint bottomLeft =
    QPoint(horizontal, vertical + pattern.height() + size);

  for (const Hole &hole : holes) {
    const int x = hole.getX();
    const int y = hole.getY();
    const int w = hole.getWidth();
    const int h = hole.getHeight();

    if (x < topRight.x() &&
	topLeft.x() < x + w &&
	y < bottomLeft.y() &&
        topLeft.y() < y + h) {
      return true;
    }
  }

  return false;
}

void
HoleDegradationDialog::generateHoles()
{
  // Random : How many hole :
  // int min = 1, max = 6;
  // int number = rand()%(max-min) + min;
  // for (int i = 0; i<number; i++)

  srand(time(nullptr)); // initialisation
  dc::HoleDegradation::HoleType type = dc::HoleDegradation::HoleType::CENTER;
  int hole = 0;
  int horizontal = 0, vertical = 0;
  int minH, maxH, minV, maxV;
  int side = static_cast<int>(dc::HoleDegradation::Border::TOP);
  int size = 0;
  int minSize = -100;
  int maxSize =
    100; //bound between which we will choose the size : the variation of size that the pattern will undergo

  QImage pattern;
  bool contain;

  do {
    const int tmpType = rand() % 3; //random type

    if (tmpType == 0) { //Can't cast directly ? (error -fpermissive)
      type = dc::HoleDegradation::HoleType::CENTER;
    }
    else if (tmpType == 1) {
      type = dc::HoleDegradation::HoleType::BORDER;
    }
    else if (tmpType == 2) {
      type = dc::HoleDegradation::HoleType::CORNER;
    }

    const int numberOfHole = countHole(type);

    if (numberOfHole == 0) { //no pattern was loaded
      return;
    }

    hole = rand() % numberOfHole; //Select random hole among these that we count

    pattern = QImage(getPath(type, hole));

    if (type == dc::HoleDegradation::HoleType::BORDER
	|| type == dc::HoleDegradation::HoleType::CORNER) {
      side = rand() % 4;
    }

    findBound(type, minH, maxH,
	      minV, maxV, side, pattern); //set the right bound
    int nbRound = 0;
    do {
      horizontal = rand() % (maxH - minH) + minH;
      vertical = rand() % (maxV - minV) + minV;
      size = rand() % (maxSize - minSize) + minSize;
      ++nbRound;
      contain = containsHole(_holes, pattern, horizontal, vertical, size);
    } while (contain && nbRound < NB_TRY_MAX);
    //While we don't find a place and that is not too much try

  } while (contain); //If we didn't have found a place, we generate a new hole

  //Update parameters
  _type = type;
  ui->typeComboBox->setCurrentIndex(static_cast<int>(type));

  _horizontal = horizontal;
  ui->horizontalSlider->setValue(horizontal);
  _vertical = vertical;
  ui->verticalSlider->setValue(vertical);
  _size = size;
  ui->sizeSlider->setValue(size);

  _pattern = hole;
  updatePatterns();

  sideChanged(side);
  //Allow to set new side, update radio button, update result image in same time ans also set constraints

  //update dialog (IHM)
  if (type == dc::HoleDegradation::HoleType::BORDER) {
    dc::HoleDegradation::Border border = static_cast<dc::HoleDegradation::Border>(side);
    if (border == dc::HoleDegradation::Border::TOP) {
      ui->topButton->setChecked(true);
    }
    else if (border == dc::HoleDegradation::Border::RIGHT) {
      ui->rightButton->setChecked(true);
    }
    else if (border == dc::HoleDegradation::Border::BOTTOM) {
      ui->bottomButton->setChecked(true);
    }
    else if (border == dc::HoleDegradation::Border::LEFT) {
      ui->leftButton->setChecked(true);
    }
  }
  else if (type == dc::HoleDegradation::HoleType::CORNER) {
    dc::HoleDegradation::Corner corner = static_cast<dc::HoleDegradation::Corner>(side);
    if (corner == dc::HoleDegradation::Corner::TOPLEFT) {
      ui->topLeftButton->setChecked(true);
    }
    else if (corner == dc::HoleDegradation::Corner::TOPRIGHT) {
      ui->topRightButton->setChecked(true);
    }
    else if (corner == dc::HoleDegradation::Corner::BOTTOMRIGHT) {
      ui->bottomRightButton->setChecked(true);
    }
    else if (corner == dc::HoleDegradation::Corner::BOTTOMLEFT) {
      ui->bottomLeftButton->setChecked(true);
    }
  }
}
