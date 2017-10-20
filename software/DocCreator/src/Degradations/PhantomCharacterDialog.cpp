#include "PhantomCharacterDialog.hpp"
#include "ui_PhantomCharacterDialog.h"

#include <QLabel>
#include <cassert>

#include "Utils/ImageUtils.hpp"

static const int IMG_WIDTH = 200;
static const int IMG_HEIGHT = 320;

static const int ZOOM_WIDTH = 200;
static const int ZOOM_HEIGHT = 200;
static const int ZOOM_X_INIT = 200;
static const int ZOOM_Y_INIT = 200;

PhantomCharacterDialog::PhantomCharacterDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::PhantomCharacterDialog)
  , _originalLabel(nullptr)
  , _resultLabel(nullptr)
{
  ui->setupUi(this);
  _frequency = (Frequency)ui->frequencyComboBox->currentIndex();
  _zoomX = ZOOM_X_INIT;
  _zoomY = ZOOM_Y_INIT;

  ui->xSlider->setValue(_zoomX);
  ui->ySlider->setValue(_zoomY);

  connect(ui->frequencyComboBox,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(frequencyChanged(int)));
  connect(
    ui->xSlider, SIGNAL(valueChanged(int)), this, SLOT(zoomXChanged(int)));
  connect(
    ui->ySlider, SIGNAL(valueChanged(int)), this, SLOT(zoomYChanged(int)));
}

PhantomCharacterDialog::~PhantomCharacterDialog()
{
  delete ui;
}

void
PhantomCharacterDialog::setOriginalImage(const QImage &img)
{
  _originalImg = img;
  _originalImgSmall = img.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation);

  ui->xSlider->setMaximum(img.width() - ZOOM_WIDTH);
  ui->ySlider->setMaximum(img.height() - ZOOM_HEIGHT);

  setupGUIImages();
}

void
PhantomCharacterDialog::setupGUIImages()
{
  if (!_originalImgSmall.isNull()) {
    _originalLabel = ui->originalLabel;
    _originalLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _originalLabel->setText(tr("original"));
    _originalLabel->setPixmap(QPixmap::fromImage(_originalImgSmall));

    _resultLabel = ui->resultLabel;
    _resultLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _resultLabel->setText(tr("result"));

    ui->zoomLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    ui->zoomLabel->setText(tr("zoom"));

    updateResultImage();

    this->updateGeometry();
  }
}

void
PhantomCharacterDialog::updateResultImage()
{
  _resultImg = phantomCharacter(_originalImg, _frequency);
  _resultImgSmall = _resultImg.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation);
  _resultLabel->setPixmap(QPixmap::fromImage(_resultImgSmall));

  updateZoom();
}

void
PhantomCharacterDialog::updateZoom()
{
  int x = std::max(0, std::min(_zoomX, _resultImg.width() - ZOOM_WIDTH));
  int y = std::max(0, std::min(_zoomY, _resultImg.height() - ZOOM_HEIGHT));
  int w = std::min(ZOOM_WIDTH, _resultImg.width());
  int h = std::min(ZOOM_HEIGHT, _resultImg.height());

  _resultImgPart = _resultImg.copy(QRect(x, y, w, h));

  ui->zoomLabel->setPixmap(QPixmap::fromImage(_resultImgPart));
}

void
PhantomCharacterDialog::frequencyChanged(int frequency)
{
  _frequency = (Frequency)frequency;

  if (!_originalImgSmall.isNull())
    updateResultImage();
}

void
PhantomCharacterDialog::zoomXChanged(int zoomX)
{
  _zoomX = zoomX;

  if (!_resultImg.isNull())
    updateZoom();
}

void
PhantomCharacterDialog::zoomYChanged(int zoomY)
{
  _zoomY = zoomY;

  if (!_resultImg.isNull())
    updateZoom();
}

void
PhantomCharacterDialog::changeEvent(QEvent *e)
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
