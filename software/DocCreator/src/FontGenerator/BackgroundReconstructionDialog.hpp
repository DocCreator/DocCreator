#ifndef BACKGROUNDRECONSTRUCTIONDIALOG_HPP
#define BACKGROUNDRECONSTRUCTIONDIALOG_HPP

#include <QDialog>

#include <opencv2/core/core.hpp>

class QLabel;

namespace Ui {
class BackgroundReconstructionDialog;
}

class BackgroundReconstructionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit BackgroundReconstructionDialog(QWidget *parent = 0);
  ~BackgroundReconstructionDialog();
  void setOriginalImage(const QImage &img);
  void setBinarizedImage(const QImage &img);
  QImage getResultImage() const { return _background; }

  void process(bool preview = false);

  float getMaxTextArea() const { return max_text_area * 0.01f; }
  float getMaxTextWidth() const { return max_text_area * 0.005f; }
  float getMaxTextHeight() const { return max_text_area * 0.005f; }

private slots:
  void on_slider_valueChanged(int value);

private:
  int max_text_area = 20;

  Ui::BackgroundReconstructionDialog *ui;

  cv::Mat _originalImg;
  cv::Mat _binarizedImg;

  cv::Mat _smallOriginalImg;
  cv::Mat _smallBinarizedImg;
  QImage _background;
  QImage _structure;
};

#endif // BACKGROUNDRECONSTRUCTIONDIALOG_HPP
