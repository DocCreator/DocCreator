#ifndef GRADIENTDOMAINDEGRADATIONDIALOG_HPP
#define GRADIENTDOMAINDEGRADATIONDIALOG_HPP

#include "Degradations/GradientDomainDegradation.hpp"
#include <QDialog>

namespace Ui {
  class GradientDomainDegradationDialog;
}

class GradientDomainDegradationDialog : public QDialog
{
  Q_OBJECT

public:

  explicit GradientDomainDegradationDialog(QWidget *parent = nullptr);

  ~GradientDomainDegradationDialog();

  GradientDomainDegradationDialog(const GradientDomainDegradationDialog &) = delete;  
  GradientDomainDegradationDialog &operator=(const GradientDomainDegradationDialog &) = delete;  

  void setOriginalImage(const QImage &img);

  QString getStainImagesPath() const;
  size_t getNumStains() const;
  dc::GradientDomainDegradation::InsertType getInsertType() const;
  bool getDoRotations() const;

  QString getOutputFilename() const;

  static QString getStainImagesDefaultPath();
				   
public slots:
  void updateCopyright();
  void chooseImageDir();
  void chooseSaveFilename();
  void updateOkButton();

protected:
  bool isStainImagesDirValid() const;
  bool isOutputFileValid() const;
  
private:
  Ui::GradientDomainDegradationDialog *ui;

  QImage _originalImg;
};


#endif /* ! GRADIENTDOMAINDEGRADATIONDIALOG_HPP */
