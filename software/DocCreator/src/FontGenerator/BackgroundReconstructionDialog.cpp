#include "BackgroundReconstructionDialog.hpp"
#include "ui_BackgroundReconstructionDialog.h"

#include <opencv2/imgproc/imgproc.hpp> //cvtColor

#include "TextInpainting.hpp"
#include <Utils/convertor.h>

static const int IMG_WIDTH = 500;
static const int IMG_HEIGHT = 680;

BackgroundReconstructionDialog::BackgroundReconstructionDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::BackgroundReconstructionDialog)
{
  ui->setupUi(this);
}

BackgroundReconstructionDialog::~BackgroundReconstructionDialog()
{
  delete ui;
}

void
BackgroundReconstructionDialog::setOriginalImage(const QImage &img)
{

  QImage small = img.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation);
  _smallOriginalImg = Convertor::getCvMat(small);

  ui->originalLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->originalLabel->setText(tr("original image"));
  ui->originalLabel->setPixmap(QPixmap::fromImage(small));

  _originalImg = Convertor::getCvMat(img);
}

void
BackgroundReconstructionDialog::setBinarizedImage(const QImage &img)
{

  QImage small = img.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation);

  cv::cvtColor(Convertor::getCvMat(small), _smallBinarizedImg, cv::COLOR_BGR2GRAY);
  cv::cvtColor(Convertor::getCvMat(img), _binarizedImg, cv::COLOR_BGR2GRAY);

  process(true);
}

void
BackgroundReconstructionDialog::process(bool preview)
{

  cv::Mat background;
  if (preview) { // We work only with the thumbnail
    background = TextInpainting::getBackground(_smallOriginalImg,
                                               _smallBinarizedImg,
                                               getMaxTextArea(),
                                               getMaxTextWidth(),
                                               getMaxTextHeight());
    _background = Convertor::getQImage(background);

    ui->background->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    ui->background->setText(tr("estimated background"));
    ui->background->setPixmap(QPixmap::fromImage(_background.scaled(
      IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation)));

  } else { // With the whole image
    background = TextInpainting::getBackground(_originalImg,
                                               _binarizedImg,
                                               getMaxTextArea(),
                                               getMaxTextWidth(),
                                               getMaxTextHeight());
    _background = Convertor::getQImage(background);
  }
}

void
BackgroundReconstructionDialog::on_slider_valueChanged(int value)
{
  max_text_area = value;
  process(true);
}
