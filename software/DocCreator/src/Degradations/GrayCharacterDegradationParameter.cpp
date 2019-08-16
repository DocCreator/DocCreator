#include "GrayCharacterDegradationParameter.hpp"

#include "ui_GrayCharacterDegradationParameter.h"

#include <QFileDialog>

#include "Utils/ImageUtils.hpp" //getWriteImageFilter()


GrayCharacterDegradationParameter::GrayCharacterDegradationParameter(
  QWidget *parent) :
  QDialog(parent),
  ui(new Ui::GrayCharacterDegradationParameter)
{
  ui->setupUi(this);

  connect(ui->btnSaveto, SIGNAL(clicked()), this, SLOT(chooseSaveFilename()));
  connect(ui->txtFilePath, SIGNAL(textChanged(QString)), this, SLOT(updateOkButton()));
}

GrayCharacterDegradationParameter::~GrayCharacterDegradationParameter()
{
  delete ui;
}

void
GrayCharacterDegradationParameter::setOriginalImage(const QImage &img)
{
  _originalImg = img;

  updateOkButton();
}


void
GrayCharacterDegradationParameter::chooseSaveFilename()
{
  QString path =
    QFileDialog::getSaveFileName(this,
                                 tr("Save File"),
                                 QStringLiteral("degraded_image.png"),
                                 getWriteImageFilter());
  ui->txtFilePath->setText(path);
}

bool
GrayCharacterDegradationParameter::isOutputFileValid() const
{
  const QString path = getOutputFilename();
  if (path.isEmpty())
    return false;
  return QFileInfo(path).dir().exists();
}

void
GrayCharacterDegradationParameter::updateOkButton()
{
  const bool imageValid = (! _originalImg.isNull());
  const bool outputFileValid = isOutputFileValid();

  QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
  assert(okButton);
  if (imageValid && outputFileValid)
    okButton->setEnabled(true);
  else
    okButton->setEnabled(false);
}

int
GrayCharacterDegradationParameter::getLevel() const
{
  return ui->hsSlider->value(); // get degradation level
}

QString
GrayCharacterDegradationParameter::getOutputFilename() const
{
  return ui->txtFilePath->text();
}

/*
void
GrayCharacterDegradationParameter::degrade()
{


  //B: CODE DUPLICATION ? there is almost the same code in GrayCharacterDegradationDialog.cpp

  // get image to degrade
  //assert(_docController);
  QImage img = _docController->toQImage(WithTextBlocks | WithImageBlocks);
  if (img.isNull())
    return;

  // contructor for the degradation model
  dc::GrayscaleCharsDegradationModel cdg(img);

  QImage dst = cdg.degradateByLevel(level);

  const QString path = ui->txtFilePath->text();

  if (!path.isEmpty())
    dst.save(path);
}

*/
