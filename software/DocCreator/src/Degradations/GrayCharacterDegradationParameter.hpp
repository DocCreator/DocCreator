#ifndef GRAYCHARACTERDEGRADATIONPARAMETER_HPP
#define GRAYCHARACTERDEGRADATIONPARAMETER_HPP

#include <QDialog>
class DocumentController;

namespace Ui {
class GrayCharacterDegradationParameter;
}

class GrayCharacterDegradationParameter : public QDialog
{
  Q_OBJECT

public:
  explicit GrayCharacterDegradationParameter(QWidget *parent = 0);
  ~GrayCharacterDegradationParameter();

  void setOriginalImage(const QImage &img);

  int getLevel() const;
  QString getOutputFilename() const;

public slots:
  void chooseSaveFilename();
  void updateOkButton();

protected:
  bool isOutputFileValid() const;

private:
  Ui::GrayCharacterDegradationParameter *ui;

  QImage _originalImg;
};

#endif // GRAYCHARACTERDEGRADATIONPARAMETER_HPP
