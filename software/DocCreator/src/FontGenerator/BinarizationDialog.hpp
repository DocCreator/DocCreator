#ifndef BINARIZATIONDIALOG_H
#define BINARIZATIONDIALOG_H

#include <QDialog>
#include <QMouseEvent>
#include <QStringList>

#include "Binarization.hpp"
#include <Utils/convertor.h>

class QLabel;

namespace Ui {
class BinarizationDialog;
}

class BinarizationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit BinarizationDialog(QWidget *parent = 0);
  ~BinarizationDialog();

  void setOriginalImage(const QImage &img);
  QImage getResultImage();
  bool eventFilter(QObject *watched, QEvent *event);

protected:
  void updateView();
  QImage takePart(const QImage &img);
  void process();

private slots:
  void on_backgroundSilder_valueChanged(int value);
  void on_comboBox_currentIndexChanged(int index);
  void on_enhancementSlider_valueChanged(int value);
  void on_checkBox_background_toggled(bool checked);
  void on_checkBox_enhancement_toggled(bool checked);
  void on_checkBox_advanced_toggled(bool checked);
  void on_windowSlider_valueChanged(int value);
  void on_kernelSlider_valueChanged(int value);

private:
  Ui::BinarizationDialog *_ui;
  QLabel *_originalLabel;
  QLabel *_binarizedLabel;
  QLabel *_originalLabelPart;

  cv::Mat _originalImg;
  QImage _binarizedImg;
  QImage _originalQImage;

  QImage _thumbnail;
  QImage _cropImg;

  int _binarization_method = 0;
  int _background_erosion = 12;
  double _post_process_v = 0.7;
  int _window_size = 40;
  double _kernel_size = 0.34;
  double _crop_x = 0.5;
  double _crop_y = 0.5;

  bool _background_checked = true;
  bool _enhancement_checked = true;
  bool _advanced_checked = false;
};

#endif // BINARIZATIONDIALOG_H
