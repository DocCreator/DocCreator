#include "BlurFilterDialog.hpp"
#include "ui_BlurFilterDialog.h"

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

static const int MIN_BLUR_FOURIER = 1;
static const int MAX_BLUR_FOURIER = 21;

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

/*
  Compute the Fourier transform of image to measure the blur
  Method taken from Emile Vinsonneau's thesis, section 3.4.2 (descriptor D5).
  Thesis here : http://www.theses.fr/s131479

  Method to get Fourier transform in OpenCV documentation :
  http://docs.opencv.org/doc/tutorials/core/discrete_fourier_transform/discrete_fourier_transform.html

*/
static float
getRadiusFourier(const QImage &original)
{
  cv::Mat originalMatColor = Convertor::getCvMat(original);
  cv::Mat originalMat;
  cv::cvtColor(originalMatColor,
               originalMat,
               cv::COLOR_BGR2GRAY); //To get a grayscale image

  cv::Mat padded; //expand input image to optimal size
  int m = cv::getOptimalDFTSize(originalMat.rows);
  int n = cv::getOptimalDFTSize(originalMat.cols);
  copyMakeBorder(originalMat,
                 padded,
                 0,
                 m - originalMat.rows,
                 0,
                 n - originalMat.cols,
                 cv::BORDER_CONSTANT,
                 cv::Scalar::all(0));

  cv::Mat planes[] = { cv::Mat_<float>(padded),
                       cv::Mat::zeros(padded.size(), CV_32F) };
  cv::Mat complexI;
  cv::Mat_<float> tmpT = cv::Mat_<float>(padded);
  cv::Mat temp = cv::Mat::zeros(padded.size(), CV_32F);
  cv::merge(
    planes, 2, complexI); // Add to the expanded another plane with zeros
  cv::dft(complexI, complexI);

  cv::split(complexI, planes); // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
  cv::magnitude(planes[0], planes[1], planes[0]); // planes[0] = magnitude
  cv::Mat magI = planes[0];

  magI += cv::Scalar::all(1); // switch to logarithmic scale
  cv::log(magI, magI);

  magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));
  int cx = magI.cols / 2;
  int cy = magI.rows / 2;

  cv::Mat q0(magI,
             cv::Rect(0, 0, cx, cy)); // Top-Left - Create a ROI per quadrant
  cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));  // Top-Right
  cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));  // Bottom-Left
  cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

  cv::Mat tmp; // swap quadrants (Top-Left with Bottom-Right)
  q0.copyTo(tmp);
  q3.copyTo(q0);
  tmp.copyTo(q3);

  q1.copyTo(tmp); // swap quadrant (Top-Right with Bottom-Left)
  q2.copyTo(q1);
  tmp.copyTo(q2);

  cv::normalize(magI, magI, 0, 255, cv::NORM_MINMAX); // Transform the matrix
                                                      // with float values into
                                                      // a viewable image form
                                                      // (float between values 0
                                                      // and 1).
  cv::threshold(magI, magI, 127, 255, cv::THRESH_BINARY);
  int erosion_size = 1;
  cv::Mat element = cv::getStructuringElement(
    cv::MORPH_RECT,
    cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
    cv::Point(
      erosion_size,
      erosion_size)); //http://docs.opencv.org/doc/tutorials/imgproc/erosion_dilatation/erosion_dilatation.html

  // to compute the radius
  QImage tmpImg = Convertor::getQImage(
    magI); //To get a mat that can use cvtColor (not works if we clone directly the mat magI, [...]
  cv::Mat calcRayon = Convertor::getCvMat(
    tmpImg); //  [...] the conversion function make the operation we need)
  cv::cvtColor(calcRayon, calcRayon, cv::COLOR_BGR2GRAY);

  int pixelX = calcRayon.cols / 2;
  int pixelY = calcRayon.rows / 2;
  bool found = false;
  //start at the center of image, and search in diagonal the end of binarization
  while (!found && pixelX > 0 && pixelY > 0) {
    if (calcRayon.at<uchar>(pixelY, pixelX) == 0) {
      found = true;
    } else {
      --pixelX;
      --pixelY;
    }
  }

  //compute the diagonal
  return sqrt(
    (((calcRayon.rows / 2) - pixelY) * ((calcRayon.rows / 2) - pixelY)) +
    (((calcRayon.cols / 2) - pixelX) * ((calcRayon.cols / 2) - pixelX)));
}

/*
  Search the size of filter (represented here by intensity) that we have to apply to fit with the example selected.

  QImage original is the image on which we want to apply blur.
  radiusExample is the radius (computed by the fonction getRadiusFourier) of the example selected.
*/
static int
searchFitFourier(const QImage &original, float radiusExample)
{
  bool found = false;
  int intensity = 0;

  int intensityMin = MIN_BLUR_FOURIER, intensityMax = MAX_BLUR_FOURIER;

  while (!found && intensityMax - intensityMin >
                     2) //binary search to find intensity that we need
  {
    intensity = ((intensityMax + intensityMin) / 2);
    if (intensity % 2 == 0)
      --intensity; // to get an odd number

    float radius = getRadiusFourier(blurFilter(
      original,
      Method::GAUSSIAN,
      intensity)); // compute the radius of result image with actual intensity
    if (radius > radiusExample + INTERVAL_FOURIER) //not enough blur
      intensityMin = intensity;
    else if (radius < radiusExample - INTERVAL_FOURIER) // too blur
      intensityMax = intensity;
    else
      found = true;
  }

  return intensity;
}

QString
BlurFilterDialog::getBlurImagesPath()
{
  //return QString(Context::BackgroundContext::instance()->getPath() + "../Image/blurImages/");
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
  _intensity = ui->intensitySlider->value();
  _coeff = ui->coeffSlider->value();
  _vertical = ui->verticalSlider->value();
  _horizontal = ui->horizontalSlider->value();
  _radius = ui->radiusSlider->value();
  _method = (Method)ui->methodComboBox->currentIndex();
  _mode = (Mode)ui->modeComboBox->currentIndex();
  _area = (Area)ui->areaComboBox->currentIndex();
  _function = Function::ELLIPSE;
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

  connect(ui->intensitySlider,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(intensityChanged(int)));
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

  _intensity = searchFitFourier(_originalImg, getRadiusFourier(exampleImg));

  _method = Method::GAUSSIAN;
  ui->intensitySlider->setValue(_intensity);
  ui->intensitySlider->setSliderPosition(_intensity);
  intensityChanged(_intensity);
  ui->methodComboBox->setCurrentIndex(static_cast<int>(_method));
  updateBlurredImage();
}

bool
BlurFilterDialog::fileExists(const QString &path, const QString &name)
{
  QString filename = name;
  if (!name.contains(QStringLiteral(".png")) &&
      !name.contains(QStringLiteral(".jpg")))
    filename = name + ".png";

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

#if QT_VERSION >= 0x050200 //Qt 5.2
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
        !name.contains(QStringLiteral(".jpg")))
      savePath = savePath + ".png";

    if (fileExists(path, name)) {
      if (QMessageBox::question(
            this,
            tr("Are you sure ?"),
            tr("The file %1 already exists, do you want to overwrite it ?")
              .arg(name),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        _patternImg.save(savePath);
      else
        savePattern();
    } else
      _patternImg.save(savePath);
  } else {
    QMessageBox::critical(
      this, tr("Warning"), tr("No specified name, saving cancelled"));
  }
}

void
BlurFilterDialog::previousPatternClicked()
{
  --_currentPattern;

  if (_currentPattern < 0)
    _currentPattern = _patterns.size() - 1;

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
  _patternImg = makePattern(
    _originalImg, _function, _area, _coeff, _vertical, _horizontal, _radius);
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

  if (_mode == Mode::COMPLETE)
    _blurredImg = blurFilter(_originalImg, _method, _intensity);
  else
    _blurredImg = applyPattern(_originalImg, _patternImg, _method, _intensity);
  //_blurredImg = blurFilter(_originalImg, _method,  _intensity, _mode, _function, _area, _coeff,  _vertical, _horizontal, _radius);

  _blurredImgSmall = _blurredImg.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation);
  _blurredImgPart = takePart(_blurredImg);

  if (_mode == Mode::COMPLETE)
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

  ui->intensityLabel->setVisible(false);
  ui->intensitySlider->setVisible(false);
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
  modeChanged(static_cast<int>(Mode::COMPLETE));

  this->adjustSize();
}

void
BlurFilterDialog::showAdvancedOptions()
{
  ui->modeLabel->setVisible(true);
  ui->modeComboBox->setVisible(true);
  ui->methodLabel->setVisible(true);
  ui->methodComboBox->setVisible(true);

  ui->intensityLabel->setVisible(true);
  ui->intensitySlider->setVisible(true);

  modeChanged(
    static_cast<int>(_mode)); //to show sliders and combo box that we need

  this->updateGeometry();
}

void
BlurFilterDialog::intensityChanged(int intensity)
{
  if (intensity % 2 == 1)
    _intensity = intensity;
  else
    _intensity = intensity - 1;

  ui->intensitySlider->setValue(_intensity);

  if (ui->modeLabel->isVisible() &&
      _exampleChosen !=
        -1) //if advanced options are used and an example is chosen
  {
    const int radiusFourier = getRadiusFourier(_blurredImg);
    const QString path = QDir(getBlurExamplesPath())
                           .absoluteFilePath(_examples.at(_exampleChosen));
    const QImage exampleImg = QImage(path);
    const int radiusExample = getRadiusFourier(exampleImg);

    if (radiusFourier > radiusExample - INTERVAL_FOURIER &&
        radiusFourier < radiusExample + INTERVAL_FOURIER)
      ui->fitLabel->setVisible(true);
    else
      ui->fitLabel->setVisible(false);
  }

  if (!_originalImgSmall.isNull())
    updateBlurredImage();
}

void
BlurFilterDialog::coeffChanged(int coeff)
{
  _coeff = coeff;

  if (_function == Function::PARABOLA)
    _coeff = coeff * 0.001f;
  ; //to get a three decimal float

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
  _method = static_cast<Method>(method);

  if (!_originalImgSmall.isNull()) {
    updatePatterns();
    updateBlurredImage();
  }
}

void
BlurFilterDialog::modeChanged(int mode)
{
  _mode = static_cast<Mode>(mode);

  if (_mode == Mode::COMPLETE) {
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

  if (!_originalImgSmall.isNull())
    updateBlurredImage();
}

void
BlurFilterDialog::functionChanged(int function)
{
  _function = static_cast<Function>(function);

  _currentPattern = function + 1;
  updatePatterns();

  updateSliders();

  if (!_originalImgSmall.isNull())
    updateBlurredImage();
}

void
BlurFilterDialog::areaChanged(int area)
{
  _area = static_cast<Area>(area);

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
    case Function::LINEAR:
      _coeff = DEFAULT_COEFF_LINEAR;
      _vertical = _originalImg.height() / 5;
      ui->coeffSlider->setMinimum(MIN_COEFF_LINEAR);
      ui->coeffSlider->setMaximum(MAX_COEFF_LINEAR);
      ui->horizontalLabel->setVisible(false);
      ui->horizontalSlider->setVisible(false);
      break;

    case Function::LOG:
      _coeff = DEFAULT_COEFF_LOG;
      _vertical = _originalImg.height() / 10;
      ui->coeffSlider->setMinimum(MIN_COEFF_LOG);
      ui->coeffSlider->setMaximum(MAX_COEFF_LOG);
      ui->horizontalLabel->setVisible(false);
      ui->horizontalSlider->setVisible(false);
      break;

    case Function::PARABOLA:
      _coeff = DEFAULT_COEFF_PARABOLA;
      _vertical = ui->verticalSlider->minimum();
      ui->coeffSlider->setMinimum(MIN_COEFF_PARABOLA);
      ui->coeffSlider->setMaximum(MAX_COEFF_PARABOLA);
      _horizontal = 0;
      break;

    case Function::SINUS:
      _coeff = DEFAULT_COEFF_SINUS;
      _vertical = _originalImg.height() / 2;
      ui->coeffSlider->setMinimum(MIN_COEFF_SINUS);
      ui->coeffSlider->setMaximum(MAX_COEFF_SINUS);
      _horizontal = 0;
      break;

    case Function::ELLIPSE:
      _vertical = 0;
      _horizontal = 0;
      _coeff = DEFAULT_COEFF_ELLIPSE;
      ui->coeffSlider->setMinimum(MIN_COEFF_ELLIPSE);
      ui->coeffSlider->setMaximum(MAX_COEFF_ELLIPSE);
      break;

    case Function::HYPERBOLA:
      _vertical = 0;
      _horizontal = 0;
      _coeff = DEFAULT_COEFF_HYPERBOLA;
      ui->coeffSlider->setMinimum(MIN_COEFF_HYPERBOLA);
      ui->coeffSlider->setMaximum(MAX_COEFF_HYPERBOLA);
      break;
  }

  if (_area == Area::CENTERED) {
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
