#ifndef LINEDETECTIONDIALOG_HPP
#define LINEDETECTIONDIALOG_HPP

#include <QDialog>

#include "opencv2/core/core.hpp"

namespace Ui {
class LineDetectionDialog;
}

class QLabel;

class LineDetectionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit LineDetectionDialog(QWidget *parent = 0);
  ~LineDetectionDialog();
  void setOriginalImage(const QImage &img);
  QImage getResultImage() { return _houghImg; }

protected:
  void updateView();
  void process();
  int getCharacterHeight(cv::Mat img);
  void drawBaseLines(const cv::Mat &img,
                     cv::Mat &dst,
                     const std::vector<cv::Vec4i> &lines);

private:
  Ui::LineDetectionDialog *ui;
  QLabel *_thumbnail;
  QLabel *_originalLabel;

  QImage _originalImg;
  QImage _houghImg;

  int _character_height;
};

#endif // LINEDETECTIONDIALOG_HPP
