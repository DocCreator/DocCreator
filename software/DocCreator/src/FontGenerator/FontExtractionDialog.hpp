#ifndef FONTEXTRACTIONDIALOG_HPP
#define FONTEXTRACTIONDIALOG_HPP

#include <QDialog>

namespace Ui {
class FontExtractionDialog;
}

class FontExtractionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FontExtractionDialog(QWidget *parent = 0);
  ~FontExtractionDialog();
  void init();
  QString getFontName();
  int getCriteria();

private slots:
  void on_fontName_textChanged(const QString &arg1);

  void on_criteriaBox_currentIndexChanged(int index);

private:
  Ui::FontExtractionDialog *ui;
  QString m_fontName;
  int m_criteria;
};

#endif // FONTEXTRACTIONDIALOG_HPP
