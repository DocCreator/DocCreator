#include "GrayCharacterDegradationDialog.hpp"

#include "ui_GrayCharacterDegradationDialog.h"
#include <Degradations/GrayCharacterDegradationModelQ.hpp>
#include <QFileDialog>

#include "Document/DocumentController.hpp"
#include "Utils/ImageUtils.hpp" //getWriteImageFilter()
#include "Utils/convertor.h"

GrayCharacterDegradationDialog::GrayCharacterDegradationDialog(
  DocumentController *docController,
  QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::GrayCharacterDegradationDialog)
{
  ui->setupUi(this);

  _docController = docController;
  connect(ui->btnBrowser, SIGNAL(clicked()), this, SLOT(chooseSaveDirectory()));

  connect(ui->cboIndependent,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(changeIndependentPercent()));
  connect(ui->cboOverlapping,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(changeOverlappingPercent()));
  connect(ui->cboDisconnection,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(changeDisconnectionPercent()));
}

GrayCharacterDegradationDialog::~GrayCharacterDegradationDialog()
{
  delete ui;
}

void
GrayCharacterDegradationDialog::chooseSaveDirectory()
{
  QString path =
    QFileDialog::getSaveFileName(this,
                                 tr("Save File"),
                                 QStringLiteral("degraded_image.png"),
                                 getWriteImageFilter());
  ui->txtSaveTo->setText(path);
}

void
GrayCharacterDegradationDialog::changeIndependentPercent()
{

  int I = ui->cboIndependent->value();
  int O = ui->cboOverlapping->value();
  int D = ui->cboDisconnection->value();

  int delta = 0;
  if ((I + O + D) > 100) {
    delta = I + O + D - 100;
  }

  ui->cboOverlapping->setValue(O - delta / 2);
  ui->cboDisconnection->setValue(D - delta + delta / 2);

  ui->cboOverlapping->update();
  ui->cboDisconnection->update();
}

void
GrayCharacterDegradationDialog::changeOverlappingPercent()
{

  int I = ui->cboIndependent->value();
  int O = ui->cboOverlapping->value();
  int D = ui->cboDisconnection->value();

  int delta = 0;
  if ((I + O + D) > 100) {
    delta = I + O + D - 100;
  }

  ui->cboIndependent->setValue(I - delta / 2);
  ui->cboDisconnection->setValue(D - delta + delta / 2);

  ui->cboIndependent->update();
  ui->cboDisconnection->update();
}

void
GrayCharacterDegradationDialog::changeDisconnectionPercent()
{
  int I = ui->cboIndependent->value();
  int O = ui->cboOverlapping->value();
  int D = ui->cboDisconnection->value();

  int delta = 0;
  if ((I + O + D) > 100) {
    delta = I + O + D - 100;
  }

  ui->cboIndependent->setValue(I - delta / 2);
  ui->cboOverlapping->setValue(O - delta + delta / 2);

  ui->cboIndependent->update();
  ui->cboOverlapping->update();
}

void
GrayCharacterDegradationDialog::degrade()
{

  int I = ui->cboIndependent->value();
  int O = ui->cboOverlapping->value();
  int D = ui->cboDisconnection->value();
  int levelOfNoise = ui->cboLevelOfNoise->value();

  //B: CODE DUPLICATION ? there is almost the same code in GrayCharacterDegradationParameter.cpp

  //B
  //assert(_docController);
  QImage img = _docController->toQImage(WithTextBlocks | WithImageBlocks);
  if (img.isNull()) {
    return;
  }

  dc::GrayscaleCharsDegradationModelQ cdg(img);

  const QImage dst = cdg.degradate(levelOfNoise, I, O, D);

  //B: It would be better to be able to visualize the produced image
  // and not save it on disk

  const QString path = ui->txtSaveTo->text();
  if (!path.isEmpty()) {
    dst.save(path);
  }
}
