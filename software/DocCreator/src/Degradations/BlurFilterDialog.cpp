#include "BlurFilterDialog.hpp"
#include "ui_BlurFilterDialog.h"
#include "BlurFilterQ.hpp"

#include <cassert>

#include <opencv2/imgproc/imgproc.hpp>

#include <QDir>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>

#include "Utils/ImageUtils.hpp"
#include "Utils/convertor.h"

#include "appconstants.h"
#include "core/configurationmanager.h"

static const int IMG_WIDTH = 180;
static const int IMG_HEIGHT = 300;
static const int BLURRED_WIDTH = 220;
static const int BLURRED_HEIGHT = 220;
static const int BLUR_X = 480;
static const int BLUR_Y = 420;
//static const int X_ORIGIN_PART = 200;
//static const int Y_ORIGIN_PART = 100;

static const int EXAMPLE_WIDTH = 130;
static const int EXAMPLE_HEIGHT = 200;
static const int HEIGHT_WINDOW = 700;
//static const int NUMBER_POINT_INIT = 20;


//MIN ET MAX TO SET FOR COEFF SLIDER, AND DEFAULT COEFF TO SET WHEN THE FUNCTION IS CHOSEN
static const int MIN_COEFF_LINEAR = 0;
static const int MAX_COEFF_LINEAR = 10;
static const float DEFAULT_COEFF_LINEAR = 1.f;
static const int MIN_COEFF_LOG = 0;
static const int MAX_COEFF_LOG = 300;
static const float DEFAULT_COEFF_LOG = 150.f;
static const int MIN_COEFF_PARABOLA = 0;
static const int MAX_COEFF_PARABOLA = 1000;
static const float DEFAULT_COEFF_PARABOLA = 0.001f;
static const int MIN_COEFF_SINUS = 10;
static const int MAX_COEFF_SINUS = 400;
static const float DEFAULT_COEFF_SINUS = 250.f;
static const int MIN_COEFF_ELLIPSE = -500;
static const int MAX_COEFF_ELLIPSE = 1000;
static const float DEFAULT_COEFF_ELLIPSE = 0.f;
static const int MIN_COEFF_HYPERBOLA = -150;
static const int MAX_COEFF_HYPERBOLA = 300;
static const float DEFAULT_COEFF_HYPERBOLA = 0.f;


QString
BlurFilterDialog::getBlurImagesPath()
{
  return Core::ConfigurationManager::get(AppConfigMainGroup,
                                         AppConfigBlurImagesFolderKey)
    .toString();
}

QString
BlurFilterDialog::getBlurPatternsPath()
{
  return QDir(BlurFilterDialog::getBlurImagesPath())
    .absoluteFilePath(QStringLiteral("blurPatterns"));
}

QString
BlurFilterDialog::getBlurExamplesPath()
{
  return QDir(BlurFilterDialog::getBlurImagesPath())
    .absoluteFilePath(QStringLiteral("blurExamples"));
}

/*
  Get list of files in directory @a dirName.
  Directories "." and ".." are removed from the list.
*/
QStringList
BlurFilterDialog::getDirectoryList(const QString &dirName)
{
  QStringList list = QDir(dirName).entryList();
  if (!list.empty()) {
    assert(list.size() >= 2);
    list.removeFirst();
    list.removeFirst();
  }
  return list;
}

BlurFilterDialog::BlurFilterDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::BlurFilterDialog)
  , _originalLabel(nullptr)
  , _blurredLabel(nullptr)
{
  ui->setupUi(this);
  _kernelSize = ui->kernelSizeSlider->value();
  _coeff = ui->coeffSlider->value();
  _vertical = ui->verticalSlider->value();
  _horizontal = ui->horizontalSlider->value();
  _radius = ui->radiusSlider->value();
  _method = static_cast<dc::BlurFilter::Method>( ui->methodComboBox->currentIndex() );
  _mode = static_cast<dc::BlurFilter::Mode>( ui->modeComboBox->currentIndex() );
  _area = static_cast<dc::BlurFilter::Area>( ui->areaComboBox->currentIndex() );
  _function = dc::BlurFilter::Function::ELLIPSE;
  _currentExample = 1;
  _currentPattern = 4;
  _patternImg = QImage(QDir(getBlurPatternsPath())
                         .absoluteFilePath(QStringLiteral("patternArea5.png")));
  this->resize(this->width(), HEIGHT_WINDOW);

  _patterns = getDirectoryList(getBlurPatternsPath());
  _examples = getDirectoryList(getBlurExamplesPath());

  _exampleChosen = -1;
  ui->fitLabel->setVisible(false);

  hideSettings();
  hideAdvancedOptions();

  connect(ui->kernelSizeSlider,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(kernelSizeChanged(int)));
  connect(
    ui->coeffSlider, SIGNAL(valueChanged(int)), this, SLOT(coeffChanged(int)));
  connect(ui->verticalSlider,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(verticalChanged(int)));
  connect(ui->horizontalSlider,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(horizontalChanged(int)));
  connect(ui->radiusSlider,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(radiusChanged(int)));
  connect(ui->methodComboBox,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(methodChanged(int)));
  connect(ui->modeComboBox,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(modeChanged(int)));
  connect(ui->areaComboBox,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(areaChanged(int)));
  connect(ui->functionComboBox,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(functionChanged(int)));
  connect(ui->advancedOptionsLink,
          SIGNAL(linkActivated(QString)),
          this,
          SLOT(advancedOptionsClicked()));
  connect(
    ui->settingButton, SIGNAL(clicked()), this, SLOT(moreSettingClicked()));
  connect(ui->previousButton, SIGNAL(clicked()), this, SLOT(previousClicked()));
  connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(nextClicked()));
  connect(ui->exampleButton, SIGNAL(clicked()), this, SLOT(exampleChosen()));
  connect(ui->previousPatternButton,
          SIGNAL(clicked()),
          this,
          SLOT(previousPatternClicked()));
  connect(
    ui->nextPatternButton, SIGNAL(clicked()), this, SLOT(nextPatternClicked()));
  connect(ui->patternButton, SIGNAL(clicked()), this, SLOT(patternChosen()));
  connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(savePattern()));
}

BlurFilterDialog::~BlurFilterDialog()
{
  delete ui;
}

static QImage
takePart(const QImage &img)
{
  int x = std::max(0, std::min(BLUR_X, img.width() - BLURRED_WIDTH));
  int y = std::max(0, std::min(BLUR_Y, img.height() - BLURRED_HEIGHT));
  int w = std::min(BLURRED_WIDTH, img.width());
  int h = std::min(BLURRED_HEIGHT, img.height());

  return img.copy(QRect(x, y, w, h));
}

void
BlurFilterDialog::previousClicked()
{
  --_currentExample;

  if (_currentExample < 0)
    _currentExample = _examples.size() - 1;

  updateExamples();
}

void
BlurFilterDialog::nextClicked()
{
  ++_currentExample;

  if (_currentExample > _examples.size() - 1)
    _currentExample = 0;

  updateExamples();
}

void
BlurFilterDialog::updateExamples()
{
  if (_currentExample < 0 || _currentExample > _examples.size())
    return;

  QString path =
    QDir(getBlurExamplesPath()).absoluteFilePath(_examples.at(_currentExample));

  QImage exampleImg(path);

  ui->exampleLabel->setPixmap(QPixmap::fromImage(takePart(exampleImg)));
}

void
BlurFilterDialog::exampleChosen()
{
  if (_currentExample < 0 || _currentExample > _examples.size())
    return;

  _exampleChosen = _currentExample;

  QString path =
    QDir(getBlurExamplesPath()).absoluteFilePath(_examples.at(_currentExample));
  QImage exampleImg(path);

  _kernelSize = dc::BlurFilter::searchFitFourier(_originalImg, dc::BlurFilter::getRadiusFourier(exampleImg));

  _method = dc::BlurFilter::Method::GAUSSIAN;
  ui->kernelSizeSlider->setValue(_kernelSize);
  ui->kernelSizeSlider->setSliderPosition(_kernelSize);
  kernelSizeChanged(_kernelSize);
  ui->methodComboBox->setCurrentIndex(static_cast<int>(_method));
  updateBlurredImage();
}

bool
BlurFilterDialog::fileExists(const QString &path, const QString &name)
{
  QString filename = name;
  if (!name.contains(QStringLiteral(".png")) &&
      !name.contains(QStringLiteral(".jpg"))) {
    filename = name + ".png";
  }

#if 0
  //B:TODO:OPTIM: we could use an iterator instead...
  QStringList patterns = getDirectoryList(path); 
  
  for (int i = 0; i < patterns.size(); ++i)
    {
      if (patterns.at(i) == filename)
	return true;
    }

  return false;
#else

#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)) //Qt 5.2
  return QFileInfo::exists(QDir(path).absoluteFilePath(filename));
#else
  return QFileInfo(QDir(path).absoluteFilePath(filename)).exists();
#endif //Qt 5.2

#endif
}

void
BlurFilterDialog::savePattern()
{
  bool ok = false;
  int newPattern = 1;

  QString path = getBlurPatternsPath();
  QString defaultName = "newPattern" + QString::number(newPattern) + ".png";

  while (fileExists(path, defaultName)) {
    ++newPattern;
    defaultName = "newPattern" + QString::number(newPattern) + ".png";
  }

  QString name =
    QInputDialog::getText(this,
                          tr("Name"),
                          tr("How do you want to name your pattern ?"),
                          QLineEdit::Normal,
                          defaultName,
                          &ok);

  if (ok && !name.isEmpty()) {
    QString savePath = QDir(path).absoluteFilePath(name);
    if (!name.contains(QStringLiteral(".png")) &&
        !name.contains(QStringLiteral(".jpg"))) {
      savePath = savePath + ".png";
    }

    if (fileExists(path, name)) {
      if (QMessageBox::question(
            this,
            tr("Are you sure ?"),
            tr("The file %1 already exists, do you want to overwrite it ?")
              .arg(name),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        _patternImg.save(savePath);
      }
      else {
        savePattern();
      }
    }
    else {
      _patternImg.save(savePath);
    }
  }
  else {
    QMessageBox::critical(
      this, tr("Warning"), tr("No specified name, saving cancelled"));
  }
}

void
BlurFilterDialog::previousPatternClicked()
{
  --_currentPattern;

  if (_currentPattern < 0) {
    _currentPattern = _patterns.size() - 1;
  }

  changePatterns();
}

void
BlurFilterDialog::nextPatternClicked()
{
  ++_currentPattern;

  if (_currentPattern > _patterns.size() - 1)
    _currentPattern = 0;

  changePatterns();
}

void
BlurFilterDialog::changePatterns()
{
  if (_currentPattern < 0 || _currentPattern > _patterns.size())
    return;

  QString path =
    QDir(getBlurPatternsPath()).absoluteFilePath(_patterns.at(_currentPattern));
  _patternImg = QImage(path);
  ui->patternLabel->setPixmap(
    QPixmap::fromImage(_patternImg.scaled(EXAMPLE_WIDTH,
                                          EXAMPLE_HEIGHT,
                                          Qt::KeepAspectRatio,
                                          Qt::FastTransformation)));

}

void
BlurFilterDialog::updatePatterns()
{
  _patternImg = dc::BlurFilter::makePattern(_originalImg,
					    _function, _area,
					    _coeff, _vertical, _horizontal, _radius);
  ui->patternLabel->setPixmap(
    QPixmap::fromImage(_patternImg.scaled(EXAMPLE_WIDTH,
                                          EXAMPLE_HEIGHT,
                                          Qt::KeepAspectRatio,
                                          Qt::FastTransformation)));

}

void
BlurFilterDialog::patternChosen()
{
  //_function = _currentPattern;
  //ui->functionComboBox->setCurrentIndex(_currentPattern);

  updateSliders();
}

void
BlurFilterDialog::updateParameters()
{
  //TO ADAPT THE BOUND OF AREA WITH THE ACTUAL IMAGE
  ui->verticalSlider->setMinimum(int(-_originalImg.height() / 2));
  ui->verticalSlider->setMaximum(int(_originalImg.height()));
  _vertical = 1;
  ui->horizontalSlider->setMinimum(int(-_originalImg.width() / 2));
  ui->horizontalSlider->setMaximum(int(_originalImg.width()));
  _horizontal = 1;
}

void
BlurFilterDialog::setOriginalImage(const QImage &img)
{
  _originalImg = img;
  _originalImgSmall = img.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation);

  _originalImgPart = takePart(img);
  //_originalImgPart = _originalImgPart;

  updateParameters();

  setupGUIImages();
}

void
BlurFilterDialog::setupGUIImages()
{
  QLabel *example = ui->exampleLabel;
  example->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  example->setText(tr("example"));
  QImage _tmpExampleImg =
    QImage(QDir(getBlurExamplesPath())
             .absoluteFilePath(QStringLiteral("blurGenerated1.png")));
  example->setPixmap(QPixmap::fromImage(takePart(_tmpExampleImg)));

  //Patterns for area
  QLabel *pattern = ui->patternLabel;
  pattern->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  pattern->setText(tr("pattern"));

  _tmpExampleImg =
    QImage(QDir(getBlurPatternsPath())
             .absoluteFilePath(QStringLiteral("patternArea5.png")));
  pattern->setPixmap(
    QPixmap::fromImage(_tmpExampleImg.scaled(EXAMPLE_WIDTH,
                                             EXAMPLE_HEIGHT,
                                             Qt::KeepAspectRatio,
                                             Qt::FastTransformation)));

  if (!_originalImgSmall.isNull()) {

    _originalLabel = ui->originalLabel;
    _originalLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _originalLabel->setText(tr("original"));
    _originalLabel->setPixmap(QPixmap::fromImage(_originalImgPart));

    _blurredLabel = ui->blurredLabel;
    _blurredLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _blurredLabel->setText(tr("result"));
    exampleChosen(); //to set a default blur to the result picture
    this->updateGeometry();
  }
}

void
BlurFilterDialog::updateBlurredImage()
{
  if (_mode == dc::BlurFilter::Mode::COMPLETE) {
    _blurredImg = dc::BlurFilter::blur(_originalImg, _method, _kernelSize);
  }
  else {
    _blurredImg = dc::BlurFilter::applyPattern(_originalImg, _patternImg, _method, _kernelSize);
  //_blurredImg = dc::BlurFilter::blur(_originalImg, _method,  _kernelSize, _mode, _function, _area, _coeff,  _vertical, _horizontal, _radius);
  }

  _blurredImgSmall = _blurredImg.scaled(IMG_WIDTH, IMG_HEIGHT,
					Qt::KeepAspectRatio,
					Qt::FastTransformation);
  _blurredImgPart = takePart(_blurredImg);

  if (_mode == dc::BlurFilter::Mode::COMPLETE)
    _blurredLabel->setPixmap(QPixmap::fromImage(_blurredImgPart));
  else
    _blurredLabel->setPixmap(QPixmap::fromImage(_blurredImgSmall));
}

void
BlurFilterDialog::moreSettingClicked()
{
  if (ui->coeffSlider->isVisible())
    hideSettings();
  else
    showSettings();
}

void
BlurFilterDialog::hideSettings()
{
  ui->functionLabel->setVisible(false);
  ui->functionComboBox->setVisible(false);
  ui->areaLabel->setVisible(false);
  ui->areaComboBox->setVisible(false);
  ui->saveButton->setVisible(false);

  ui->coeffLabel->setVisible(false);
  ui->coeffSlider->setVisible(false);
  ui->verticalLabel->setVisible(false);
  ui->verticalSlider->setVisible(false);
  ui->horizontalLabel->setVisible(false);
  ui->horizontalSlider->setVisible(false);
  ui->radiusLabel->setVisible(false);
  ui->radiusSlider->setVisible(false);
}

void
BlurFilterDialog::showSettings()
{
  ui->functionLabel->setVisible(true);
  ui->functionComboBox->setVisible(true);
  ui->areaLabel->setVisible(true);
  ui->areaComboBox->setVisible(true);
  ui->saveButton->setVisible(true);

  ui->coeffLabel->setVisible(true);
  ui->coeffSlider->setVisible(true);
  ui->verticalLabel->setVisible(true);
  ui->verticalSlider->setVisible(true);
  ui->horizontalLabel->setVisible(true);
  ui->horizontalSlider->setVisible(true);

  updateSliders();
}

void
BlurFilterDialog::advancedOptionsClicked()
{
  if (ui->modeComboBox->isVisible())
    hideAdvancedOptions();
  else
    showAdvancedOptions();
}

void
BlurFilterDialog::hideAdvancedOptions()
{
  ui->modeLabel->setVisible(false);
  ui->modeComboBox->setVisible(false);
  ui->methodLabel->setVisible(false);
  ui->methodComboBox->setVisible(false);
  ui->areaLabel->setVisible(false);
  ui->areaComboBox->setVisible(false);
  ui->functionLabel->setVisible(false);
  ui->functionComboBox->setVisible(false);
  ui->settingButton->setVisible(false);

  ui->kernelSizeLabel->setVisible(false);
  ui->kernelSizeSlider->setVisible(false);
  ui->coeffLabel->setVisible(false);
  ui->coeffSlider->setVisible(false);
  ui->verticalLabel->setVisible(false);
  ui->verticalSlider->setVisible(false);
  ui->horizontalLabel->setVisible(false);
  ui->horizontalSlider->setVisible(false);
  ui->radiusLabel->setVisible(false);
  ui->radiusSlider->setVisible(false);

  ui->patternLabel->setVisible(false);
  ui->previousPatternButton->setVisible(false);
  ui->nextPatternButton->setVisible(false);
  ui->patternButton->setVisible(false);

  ui->modeComboBox->setCurrentIndex(0);
  modeChanged(static_cast<int>(dc::BlurFilter::Mode::COMPLETE));

  this->adjustSize();
}

void
BlurFilterDialog::showAdvancedOptions()
{
  ui->modeLabel->setVisible(true);
  ui->modeComboBox->setVisible(true);
  ui->methodLabel->setVisible(true);
  ui->methodComboBox->setVisible(true);

  ui->kernelSizeLabel->setVisible(true);
  ui->kernelSizeSlider->setVisible(true);

  modeChanged(
    static_cast<int>(_mode)); //to show sliders and combo box that we need

  this->updateGeometry();
}

void
BlurFilterDialog::kernelSizeChanged(int kernelSize)
{
  if (kernelSize % 2 == 1) {
    _kernelSize = kernelSize;
  }
  else {
    _kernelSize = kernelSize - 1;
  }

  ui->kernelSizeSlider->setValue(_kernelSize);

  if (ui->modeLabel->isVisible() &&
      _exampleChosen !=
        -1) //if advanced options are used and an example is chosen
  {
    const int radiusFourier = dc::BlurFilter::getRadiusFourier(_blurredImg);
    const QString path = QDir(getBlurExamplesPath())
                           .absoluteFilePath(_examples.at(_exampleChosen));
    const QImage exampleImg = QImage(path);
    const int radiusExample = dc::BlurFilter::getRadiusFourier(exampleImg);

    if (radiusFourier > radiusExample - dc::BlurFilter::INTERVAL_FOURIER &&
        radiusFourier < radiusExample + dc::BlurFilter::INTERVAL_FOURIER) {
      ui->fitLabel->setVisible(true);
    }
    else {
      ui->fitLabel->setVisible(false);
    }
  }

  if (!_originalImgSmall.isNull()) {
    updateBlurredImage();
  }
}

void
BlurFilterDialog::coeffChanged(int coeff)
{
  _coeff = coeff;

  if (_function == dc::BlurFilter::Function::PARABOLA) {
    _coeff = coeff * 0.001f;
    //to get a three-decimal float
  }

  if (!_originalImgSmall.isNull()) {
    updatePatterns();
    updateBlurredImage();
  }
}

void
BlurFilterDialog::verticalChanged(int vertical)
{
  _vertical = vertical;

  if (!_originalImgSmall.isNull()) {
    updatePatterns();
    updateBlurredImage();
  }
}

void
BlurFilterDialog::horizontalChanged(int horizontal)
{
  _horizontal = horizontal;

  if (!_originalImgSmall.isNull()) {
    updatePatterns();
    updateBlurredImage();
  }
}

void
BlurFilterDialog::radiusChanged(int radius)
{
  _radius = radius;

  if (!_originalImgSmall.isNull()) {
    updatePatterns();
    updateBlurredImage();
  }
}

void
BlurFilterDialog::methodChanged(int method)
{
  _method = static_cast<dc::BlurFilter::Method>(method);

  if (!_originalImgSmall.isNull()) {
    updatePatterns();
    updateBlurredImage();
  }
}

void
BlurFilterDialog::modeChanged(int mode)
{
  _mode = static_cast<dc::BlurFilter::Mode>(mode);

  if (_mode == dc::BlurFilter::Mode::COMPLETE) {
    ui->exampleButton->setVisible(true);
    if (_originalLabel != nullptr) {
      _originalLabel->setPixmap(QPixmap::fromImage(_originalImgPart));
      _blurredLabel->setPixmap(QPixmap::fromImage(_blurredImgPart));
    }
    ui->patternLabel->setVisible(false);
    ui->previousPatternButton->setVisible(false);
    ui->nextPatternButton->setVisible(false);
    ui->patternButton->setVisible(false);
    ui->settingButton->setVisible(false);
    hideSettings();
  } else //AREA
  {
    ui->previousPatternButton->setVisible(true);
    ui->nextPatternButton->setVisible(true);
    ui->patternButton->setVisible(true);
    ui->settingButton->setVisible(true);
    ui->patternLabel->setVisible(true);

    ui->exampleButton->setVisible(false);
    _originalLabel->setPixmap(QPixmap::fromImage(_originalImgSmall));
    _blurredLabel->setPixmap(QPixmap::fromImage(_blurredImgSmall));
  }

  if (!_originalImgSmall.isNull()) {
    updateBlurredImage();
  }
}

void
BlurFilterDialog::functionChanged(int function)
{
  _function = static_cast<dc::BlurFilter::Function>(function);

  _currentPattern = function + 1;
  updatePatterns();

  updateSliders();

  if (!_originalImgSmall.isNull()) {
    updateBlurredImage();
  }
}

void
BlurFilterDialog::areaChanged(int area)
{
  _area = static_cast<dc::BlurFilter::Area>(area);

  updateSliders();

  if (!_originalImgSmall.isNull()) {
    updatePatterns();
    updateBlurredImage();
  }
}

void
BlurFilterDialog::updateSliders()
{
  if (ui->functionComboBox
        ->isVisible()) //to not show if more Setting are not available
  {
    ui->horizontalLabel->setVisible(true);
    ui->horizontalSlider->setVisible(true);
  }

  switch (_function) {
  case dc::BlurFilter::Function::LINEAR:
      _coeff = DEFAULT_COEFF_LINEAR;
      _vertical = _originalImg.height() / 5;
      ui->coeffSlider->setMinimum(MIN_COEFF_LINEAR);
      ui->coeffSlider->setMaximum(MAX_COEFF_LINEAR);
      ui->horizontalLabel->setVisible(false);
      ui->horizontalSlider->setVisible(false);
      break;

  case dc::BlurFilter::Function::LOG:
      _coeff = DEFAULT_COEFF_LOG;
      _vertical = _originalImg.height() / 10;
      ui->coeffSlider->setMinimum(MIN_COEFF_LOG);
      ui->coeffSlider->setMaximum(MAX_COEFF_LOG);
      ui->horizontalLabel->setVisible(false);
      ui->horizontalSlider->setVisible(false);
      break;

  case dc::BlurFilter::Function::PARABOLA:
      _coeff = DEFAULT_COEFF_PARABOLA;
      _vertical = ui->verticalSlider->minimum();
      ui->coeffSlider->setMinimum(MIN_COEFF_PARABOLA);
      ui->coeffSlider->setMaximum(MAX_COEFF_PARABOLA);
      _horizontal = 0;
      break;

  case dc::BlurFilter::Function::SINUS:
      _coeff = DEFAULT_COEFF_SINUS;
      _vertical = _originalImg.height() / 2;
      ui->coeffSlider->setMinimum(MIN_COEFF_SINUS);
      ui->coeffSlider->setMaximum(MAX_COEFF_SINUS);
      _horizontal = 0;
      break;

  case dc::BlurFilter::Function::ELLIPSE:
      _vertical = 0;
      _horizontal = 0;
      _coeff = DEFAULT_COEFF_ELLIPSE;
      ui->coeffSlider->setMinimum(MIN_COEFF_ELLIPSE);
      ui->coeffSlider->setMaximum(MAX_COEFF_ELLIPSE);
      break;

  case dc::BlurFilter::Function::HYPERBOLA:
      _vertical = 0;
      _horizontal = 0;
      _coeff = DEFAULT_COEFF_HYPERBOLA;
      ui->coeffSlider->setMinimum(MIN_COEFF_HYPERBOLA);
      ui->coeffSlider->setMaximum(MAX_COEFF_HYPERBOLA);
      break;
  }

  if (_area == dc::BlurFilter::Area::CENTERED) {
    ui->radiusLabel->setVisible(true);
    ui->radiusSlider->setVisible(true);
  } else {
    ui->radiusLabel->setVisible(false);
    ui->radiusSlider->setVisible(false);
  }

  ui->coeffSlider->setValue(_coeff);
  ui->coeffSlider->setSliderPosition(_coeff);
  ui->verticalSlider->setValue(_vertical);
  ui->verticalSlider->setSliderPosition(_vertical);
  ui->horizontalSlider->setValue(_horizontal);
  ui->horizontalSlider->setSliderPosition(_horizontal);

  updateBlurredImage();
  this->adjustSize();
}

void
BlurFilterDialog::changeEvent(QEvent *e)
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
