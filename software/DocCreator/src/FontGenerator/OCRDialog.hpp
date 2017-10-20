#ifndef OCRDIALOG_H
#define OCRDIALOG_H

#include <QDialog>
#include <QString>

#include "opencv2/core/core.hpp"

namespace Ui {
class OCRDialog;
}
namespace Models {
class Font;
}

class QLabel;
class QTableView;

class OCRDialog : public QDialog
{
  Q_OBJECT

public:
  explicit OCRDialog(QWidget *parent = 0);
  ~OCRDialog();

  void setParameters(const QString &tessdataParentDir, const QString &language);

  void init(const QImage &ori, const QImage &bin);
  void setOriginalImage(const QImage &img);
  void setBinarizedImage(const QImage &img);
  const QImage getResultImage() const { return m_originalImg; }

  virtual bool eventFilter(QObject *watched, QEvent *event) override;

  QString saveFont();

  Models::Font *getFont() const;

private slots:
  void process();

  void on_tableLetters_clicked(const QModelIndex &index);
  void on_baselineSpinBox_valueChanged(int arg1);
  void on_apply_clicked();
  void on_letterLabel_textChanged();
  void on_deleteButton_clicked();
  void on_tableAlphabet_cellClicked(int row, int column);
  void on_maxSymbol_valueChanged(int arg1);
  void on_smoothed_toggled(bool checked);

  //void on_pushButton_3_clicked();
  void on_binarizationSpinBox_valueChanged(int arg1);

  void on_saveFont_clicked();

protected:
  struct FontLetter
  {
    // Binary mask of kept parts of the character
    cv::Mat mask;
    std::string label;
    float confidence = 0.f;

    // Bounding box of the character in the image
    cv::Rect rect;

    // Baseline given by Tesseract
    int baseline = 0;

    // Baseline computed by our own
    int baseline2 = 0;

    // Test
    int GT_baseline = 0;

    // Binary threshold
    double binarization_step = -1;

    // Smoothed image
    bool checked = false;
  };

protected:
  void updateView();
  void updateTable();
  void updateAlphabet();
  void computeBaselines();
  void rebinarizeCurrentLetter();
  void writeFont(const QString &filename) const;

  cv::Mat getLetterViewFromMask(const cv::Mat &original,
                                const cv::Mat &mask) const;

  QImage getQImageFromMask(
    const cv::Mat &original,
    const cv::Mat &mask,
    QRgb backgroundColor = qRgba(255, 255, 255, 0)) const;

  cv::Mat getImageFromMask(const cv::Mat &original,
                           const cv::Mat &mask,
                           int background_value = 237) const;

  QColor getConfidenceColor(float conf) const;
  std::vector<int> getSimilarLetters(const FontLetter &fl) const;

  //std::vector<FontLetter> getFinalFont() const;

private:
  Ui::OCRDialog *ui;

  QString m_tessdataParentDir;
  QString m_language;

  QTableView *m_table;
  QImage m_originalImg;
  QImage m_binarizedImg;

  std::vector<FontLetter> m_font;
  std::vector<int> m_similarList;
  std::vector<std::pair<int, int>> m_alphabet;
  std::vector<cv::Vec4i> m_baselines;
  FontLetter m_currentLetter;
  int m_currentIndex;
  int m_maxNumberOfSymbols = 5;

  std::vector<FontLetter> m_validatedFont;
};

#endif // OCRDIALOG_H
