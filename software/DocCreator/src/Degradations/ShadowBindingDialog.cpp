#include "ShadowBindingDialog.hpp"
#include "ui_ShadowBindingDialog.h"
#include "ShadowBindingQ.hpp"

#include <cassert>

#include <QRadioButton>
#include <QSlider>

static const int IMG_WIDTH = 200;
static const int IMG_HEIGHT = 320;

ShadowBindingDialog::ShadowBindingDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::ShadowBindingDialog)
  , _originalLabel(nullptr)
  , _resultLabel(nullptr)
{
  ui->setupUi(this);
  _distance = getDistanceAux(ui->distanceSlider->value());
  _intensity = getIntensityAux(ui->intensitySlider->value());
  _angle = ui->angleSlider->value();

  _border = dc::ShadowBinding::Border::LEFT;

  connect(ui->intensitySlider,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(intensityChanged(int)));
  connect(ui->distanceSlider,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(distanceChanged(int)));
  connect(
    ui->angleSlider, SIGNAL(valueChanged(int)), this, SLOT(angleChanged(int)));
  connect(ui->leftButton, SIGNAL(clicked()), this, SLOT(borderChanged()));
  connect(ui->rightButton, SIGNAL(clicked()), this, SLOT(borderChanged()));
  connect(ui->topButton, SIGNAL(clicked()), this, SLOT(borderChanged()));
  connect(ui->bottomButton, SIGNAL(clicked()), this, SLOT(borderChanged()));
}

ShadowBindingDialog::~ShadowBindingDialog()
{
  delete ui;
}

void
ShadowBindingDialog::setOriginalImage(const QImage &img)
{
  _originalImg = img;
  _originalImgSmall = img.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation);

  //ui->distanceSlider->setMaximum(img.width()*20/100);
  setupGUIImages();
}

void
ShadowBindingDialog::setupGUIImages()
{

  if (!_originalImgSmall.isNull()) {

    _originalLabel = ui->originalLabel;
    _originalLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _originalLabel->setText(tr("original"));
    _originalLabel->setPixmap(QPixmap::fromImage(_originalImgSmall));
    _originalLabel->setMinimumSize(_originalImgSmall.size());
    _originalLabel->setMaximumSize(IMG_WIDTH, IMG_HEIGHT);

    _resultLabel = ui->resultLabel;
    _resultLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _resultLabel->setText(tr("result"));
    _resultLabel->setMaximumSize(IMG_WIDTH, IMG_HEIGHT);
    updateResultImage();

    this->updateGeometry();
  }
}

void
ShadowBindingDialog::updateResultImage()
{
  _resultImg =
    dc::ShadowBinding::shadowBinding(_originalImg, _border, _distance, _intensity, _angle);
  _resultImgSmall = _resultImg.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation);
  _resultLabel->setPixmap(QPixmap::fromImage(_resultImgSmall));
  _resultLabel->setMinimumSize(_resultImgSmall.size());
}

float
ShadowBindingDialog::getIntensityAux(int intensity) const
{
  assert(ui->intensitySlider->maximum() != 0);
  return 1.0f -
         intensity /
           (float)ui->intensitySlider
             ->maximum(); //"1.0f-int." to have the effec that increase when sldier is increased.
}

void
ShadowBindingDialog::intensityChanged(int intensity)
{
  _intensity = getIntensityAux(intensity);

  if (!_originalImgSmall.isNull())
    updateResultImage();
}

int
ShadowBindingDialog::getDistanceAux(int distance) const
{
  assert(ui->distanceSlider->maximum() != 0);
  //we cap the maximum of shadowed area to 40% of minimum size (w or h) of image
  int w = 800;
  if (!_originalImgSmall.isNull())
    w = std::min(_originalImg.width(), _originalImg.height());
  w = 0.4 * w;

  const int d =
    static_cast<int>(w * distance / (float)ui->distanceSlider->maximum() + 0.5);
  //std::cerr<<"*** distance="<<distance<<" w="<<w<<" maxD="<<ui->distanceSlider->maximum()<<" => d="<<d<<"\n";
  return d;
}

void
ShadowBindingDialog::distanceChanged(int distance)
{
  _distance = getDistanceAux(distance);

  if (!_originalImgSmall.isNull())
    updateResultImage();
}

void
ShadowBindingDialog::angleChanged(int angle)
{
  _angle = angle;

  if (!_originalImgSmall.isNull())
    updateResultImage();
}

void
ShadowBindingDialog::borderChanged()
{
  if (ui->leftButton->isChecked())
    _border = dc::ShadowBinding::Border::LEFT;
  else if (ui->topButton->isChecked())
    _border = dc::ShadowBinding::Border::TOP;
  else if (ui->rightButton->isChecked())
    _border = dc::ShadowBinding::Border::RIGHT;
  else
    _border = dc::ShadowBinding::Border::BOTTOM;

  if (!_originalImgSmall.isNull())
    updateResultImage();
}

void
ShadowBindingDialog::changeEvent(QEvent *e)
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
