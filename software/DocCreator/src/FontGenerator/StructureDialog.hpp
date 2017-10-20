#ifndef STRUCTUREDIALOG_HPP
#define STRUCTUREDIALOG_HPP

#include <QDialog>

#include <opencv2/core/core.hpp> //cv::Rect

class QLabel;
class DocumentController;
namespace Ui {
class StructureDialog;
}

class StructureDialog : public QDialog
{
  Q_OBJECT

public:
  explicit StructureDialog(DocumentController *ctrl, QWidget *parent = 0);
  ~StructureDialog();
  QImage getResultImage();
  void setOriginalImage(const QImage &img);
  void setBinaryImage(const QImage &img);
  void setBackground(const QImage &img);
  void loremIpsum();

  void init(const QImage &ori, const QImage &bin, const QImage &bkg);
  void updateView();
  void process();
  std::vector<cv::Rect> getBlocks();

private slots:
  void on_horizontalSlider_valueChanged(int value);
  void on_comboBox_currentIndexChanged(int index);

private:
  Ui::StructureDialog *ui;

  cv::Mat _originalImg;
  cv::Mat _binaryImg;
  cv::Mat _background;
  cv::Mat _structure;

  cv::Mat _distanceMap;
  int _characterHeight = 15;

  std::vector<cv::Rect> _blocks;

  DocumentController *_ctrl;

  float _dilation = 1.5;
  int _model = 0; // 0 - structure extracted from image. 1 - single text block.
                  // 2 - Double column
};

#endif // STRUCTUREDIALOG_HPP
