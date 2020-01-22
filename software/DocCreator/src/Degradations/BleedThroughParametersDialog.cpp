#include "BleedThroughParametersDialog.hpp"
#include "ui_BleedThroughParametersDialog.h"

#include <QFileDialog>
#include <QLabel>
#include <cassert>

#include "Degradations/BleedThroughQ.hpp"
#include "Utils/ImageUtils.hpp"

BleedThroughParametersDialog::BleedThroughParametersDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::BleedThroughParametersDialog)
  , _rectoLabel(nullptr)
  , _versoLabel(nullptr)
  , _bleedLabel(nullptr)
{
  ui->setupUi(this);

  ui->nbIterSpinBox->setValue(_params.getNbIterations());

  connect(
    ui->versoImagePathButton, SIGNAL(clicked()), this, SLOT(versoChosen()));
  //connect(ui->versoImagePathLine, SIGNAL(textChanged(QString)), &params, SLOT(setPathToVersoImage(QString)));
  connect(ui->nbIterSpinBox,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(nbIterationsChanged(int)));
}

static const int IMG_WIDTH = 240;
static const int IMG_HEIGHT = 360;

static const int BLEED_WIDTH = 220;
static const int BLEED_HEIGHT = 220;
static const int BLEED_X = 280;
static const int BLEED_Y = 320;

BleedThroughParametersDialog::~BleedThroughParametersDialog()
{
  delete ui;
}

static QImage
takePart(const QImage &img)
{
  int x = std::max(0, std::min(BLEED_X, img.width() - BLEED_WIDTH));
  int y = std::max(0, std::min(BLEED_Y, img.height() - BLEED_HEIGHT));
  int w = std::min(BLEED_WIDTH, img.width());
  int h = std::min(BLEED_HEIGHT, img.height());

  return img.copy(QRect(x, y, w, h));
}

void
BleedThroughParametersDialog::setRectoImage(const QImage &rectoImg)
{
  _rectoImg = rectoImg;
  _rectoImgSmall = toGray(rectoImg.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation));

  _rectoImgPart = takePart(rectoImg); //ROI should be interactively selectable

  //_rectoImgPart = _rectoImgPart;

  setupGUIImages();
}

void
BleedThroughParametersDialog::setVersoImage(const QImage &versoImg)
{
  _versoImg = versoImg;
  _versoImgSmall = toGray(versoImg.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation));
  _versoImgSmall = _versoImgSmall.mirrored(true, false);

  _versoImgPart = takePart(versoImg); //ROI should be interactively selectable
  _versoImgPart = _versoImgPart.mirrored(true, false);

  //_versoImgPart = _versoImgPart;

  updateVersoImage();
}

void
BleedThroughParametersDialog::updateVersoImage()
{
  if (!_rectoImgSmall.isNull() && !_versoImgSmall.isNull()) {
    assert(_versoLabel);
    _versoLabel->setPixmap(QPixmap::fromImage(_versoImgSmall));

    updateBleedImage(_params.getNbIterations(), true);
  }
}

void
BleedThroughParametersDialog::setupGUIImages()
{
  if (!_rectoImgSmall.isNull()) {

    //QBoxLayout *layout = (QBoxLayout *)ui->verticalLayout; //this->layout();

    //QHBoxLayout *hl = new QHBoxLayout(this);

    QHBoxLayout *hl = ui->layoutLabels;

    _rectoLabel = new QLabel(this);
    _rectoLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _rectoLabel->setText(tr("recto"));
    _rectoLabel->setPixmap(QPixmap::fromImage(_rectoImgSmall));

    _versoLabel = new QLabel(this);
    _versoLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _versoLabel->setText(tr("verso"));
    QImage versoTmp(
      _rectoImgSmall.width(), _rectoImgSmall.height(), QImage::Format_RGB32);
    versoTmp.fill(0xffffffff);
    _versoLabel->setPixmap(QPixmap::fromImage(versoTmp));

    _bleedLabel = new QLabel(this);
    _bleedLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _bleedLabel->setText(tr("result"));
    QImage bleedTmp(BLEED_WIDTH, BLEED_HEIGHT, QImage::Format_RGB32);
    bleedTmp.fill(0xffffffff);
    _bleedLabel->setPixmap(QPixmap::fromImage(bleedTmp));

    hl->addWidget(_rectoLabel);
    hl->addWidget(_versoLabel);
    hl->addWidget(_bleedLabel);

    this->updateGeometry();
  }
}

void
BleedThroughParametersDialog::versoChosen()
{
  QString path =
    QFileDialog::getOpenFileName(this,
                                 tr("Open File"),
                                 QStringLiteral("/DocCreator/Image"),
                                 getReadImageFilter());
  ui->versoImagePathLine->setText(path);

  _params.setPathToVersoImage(path);
  //TODO: remove !
  _versoParams.setVersoDirPath(
    QFileInfo(path).absoluteDir().absolutePath()); //B

  if (!_rectoImgSmall.isNull()) {
    QImage versoImg(path);
    setVersoImage(versoImg);
  }
}

void
BleedThroughParametersDialog::updateBleedImage(int nbIter, bool fromZero)
{
  assert(!_rectoImgSmall.isNull() && !_versoImgSmall.isNull());
  assert(!_rectoImgPart.isNull() && !_versoImgPart.isNull());
  assert(_bleedLabel);
  assert(_rectoImgPart.size() == _versoImgPart.size());

  const int prevNbIter = _params.getNbIterations();
  int lNbIter = 0;
  QImage currRectoImg;
  if (!fromZero && prevNbIter < nbIter) {
    lNbIter = nbIter - prevNbIter;
    currRectoImg = _bleedImgPart;
  } else {
    lNbIter = nbIter;
    currRectoImg = _rectoImgPart.copy();
  }

  assert(currRectoImg.size() == _rectoImgPart.size());

  _bleedImgPart =
    dc::BleedThrough::bleedThrough(_rectoImgPart, currRectoImg, _versoImgPart, lNbIter);

  _bleedLabel->setPixmap(QPixmap::fromImage(_bleedImgPart));
}

void
BleedThroughParametersDialog::nbIterationsChanged(int value)
{
  if (!_rectoImgSmall.isNull() && !_versoImgSmall.isNull()) {
    updateBleedImage(value, false);
  }

  _params.setNbIterations(value);
}

void
BleedThroughParametersDialog::changeEvent(QEvent *e)
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
