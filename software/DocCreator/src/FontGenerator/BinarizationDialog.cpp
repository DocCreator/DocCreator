#include "BinarizationDialog.hpp"
#include "ui_BinarizationDialog.h"

#include <QPainter>
#include <QRectF>

static const int IMG_WIDTH = 350;
static const int IMG_HEIGHT = 450;
static const int CROP_WIDTH = 350;
static const int CROP_HEIGHT = 120;

BinarizationDialog::BinarizationDialog(QWidget *parent)
  : QDialog(parent)
  , _ui(new Ui::BinarizationDialog)
  , _originalLabel(nullptr)
  , _binarizedLabel(nullptr)
  , _originalLabelPart(nullptr)
{
  _ui->setupUi(this);
  _ui->originalLabel->installEventFilter(this);
}

BinarizationDialog::~BinarizationDialog()
{
  delete _ui;
}

QImage
BinarizationDialog::takePart(const QImage &img)
{
  // Crop the image
  const int x = std::max(
    0, std::min((int)(_crop_x * img.width()), img.width() - CROP_WIDTH));
  const int y = std::max(
    0, std::min((int)(_crop_y * img.height()), img.height() - CROP_HEIGHT));
  const int w = std::min(CROP_WIDTH, img.width());
  const int h = std::min(CROP_HEIGHT, img.height());

  return img.copy(QRect(x, y, w, h));
}

bool
BinarizationDialog::eventFilter(QObject *watched, QEvent *event)
{
  // Add an event to detect the click on the image to update the preview area
  QLabel *label = qobject_cast<QLabel *>(watched);
  if (label != nullptr && event->type() == QEvent::MouseButtonPress) {
    QMouseEvent *k = (QMouseEvent *)event;

    const double val_w = (double)(std::min(CROP_WIDTH, _originalImg.cols)) /
                         _originalImg.cols; //.width();
    const double val_h = (double)(std::min(CROP_HEIGHT, _originalImg.rows)) /
                         _originalImg.rows; //height();

    _crop_x = (double)k->pos().x() / _thumbnail.width() - val_w / 2;
    _crop_y = (double)k->pos().y() / _thumbnail.height() - val_h / 2;

    updateView();
  }

  return QObject::eventFilter(watched, event);
}

void
BinarizationDialog::setOriginalImage(const QImage &img)
{
  _originalImg = Convertor::getCvMat(img);
  _originalQImage = img;
  _ui->advanced_widget->setEnabled(false);

  process();
}

void
BinarizationDialog::process()
{

  cv::Mat mat;
  if (_background_checked)
    Binarization::preProcess(_originalImg, mat, _background_erosion);
  else
    _originalImg.copyTo(mat);

  const int winx = _window_size < mat.cols ? _window_size : mat.cols - 1;
  const int winy = _window_size < mat.rows ? _window_size : mat.rows - 1;

  cv::Mat img_bin(mat.rows, mat.cols, CV_8U);
  switch (_binarization_method) {
    case 0: // Wolf & jolion
      Binarization::NiblackSauvolaWolfJolion(
        mat, img_bin, Binarization::WOLFJOLION, 128, winx, winy, _kernel_size);
      break;
    case 1: // Sauvola
      Binarization::NiblackSauvolaWolfJolion(
        mat, img_bin, Binarization::SAUVOLA, 128, winx, winy, _kernel_size);
      break;
    case 2: // Otsu
    default:
      Binarization::binarize(mat, img_bin);
      break;
  }

  if (_enhancement_checked)
    Binarization::postProcess(img_bin, img_bin, 0.9, (int)10 * _post_process_v);

  _binarizedImg = Convertor::getQImage(img_bin);
  updateView();
}

void
BinarizationDialog::updateView()
{

  _thumbnail = _binarizedImg.scaled(IMG_WIDTH, IMG_HEIGHT);

  _originalLabel = _ui->originalLabel;
  _originalLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  _originalLabel->setText(tr("thumbnail"));

  // Draw a red rectangle on the thumbnail
  QPainter p(&_thumbnail);
  p.setPen(Qt::red);

  const double val_w =
    (double)(std::min(CROP_WIDTH, _originalImg.cols)) / _originalImg.cols;
  const double val_h =
    (double)(std::min(CROP_HEIGHT, _originalImg.rows)) / _originalImg.rows;

  // Rectangle coordinates
  const int rect_x = (int)(_crop_x * IMG_WIDTH);
  const int rect_y = (int)(_crop_y * IMG_HEIGHT);
  const int rect_w = (int)(val_w * IMG_WIDTH);
  const int rect_h = (int)(val_h * IMG_HEIGHT);

  QRectF rect(rect_x, rect_y, rect_w, rect_h);
  p.drawRect(rect);
  p.end();

  _originalLabel->setPixmap(QPixmap::fromImage(_thumbnail));

  _originalLabelPart = _ui->originalLabelPart;
  _originalLabelPart->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  _originalLabelPart->setText(tr("original label part"));
  _originalLabelPart->setPixmap(QPixmap::fromImage(takePart(_originalQImage)));

  _binarizedLabel = _ui->binarizedLabel;
  _binarizedLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  _binarizedLabel->setText(tr("binarized"));
  _binarizedLabel->setPixmap(QPixmap::fromImage(takePart(_binarizedImg)));
}

QImage
BinarizationDialog::getResultImage()
{
  return _binarizedImg;
}

void
BinarizationDialog::on_backgroundSilder_valueChanged(int value)
{
  _background_erosion = 60 - value;
  process();
}

void
BinarizationDialog::on_comboBox_currentIndexChanged(int index)
{
  _binarization_method = index;
  switch (_binarization_method) {
    case 0: // Wolf & Jolion
      _ui->local_widget->setVisible(true);
      break;
    case 1: // Sauvola
      _ui->local_widget->setVisible(true);
      break;
    case 2: // Otsu
      _ui->local_widget->setVisible(false);
      break;
  }
  process();
}

void
BinarizationDialog::on_enhancementSlider_valueChanged(int value)
{
  _post_process_v = value / 100.0;
  process();
}

void
BinarizationDialog::on_checkBox_background_toggled(bool checked)
{
  _background_checked = checked;
  _ui->backgroundSilder->setEnabled(checked);
  process();
}

void
BinarizationDialog::on_checkBox_enhancement_toggled(bool checked)
{
  _enhancement_checked = checked;
  _ui->enhancementSlider->setEnabled(checked);
  process();
}

void
BinarizationDialog::on_checkBox_advanced_toggled(bool checked)
{
  _advanced_checked = checked;
  _ui->advanced_widget->setEnabled(checked);
}

void
BinarizationDialog::on_windowSlider_valueChanged(int value)
{
  _window_size = value;
  process();
}

void
BinarizationDialog::on_kernelSlider_valueChanged(int value)
{
  _kernel_size = value / 100.0;
  process();
}
