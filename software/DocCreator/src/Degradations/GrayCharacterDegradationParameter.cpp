#include "GrayCharacterDegradationParameter.hpp"

#include "ui_GrayCharacterDegradationParameter.h"
#include <Degradations/GrayscaleCharsDegradationModel.hpp>
#include <QFileDialog>

#include "Document/DocumentController.hpp"
#include "Utils/ImageUtils.hpp" //getWriteImageFilter()
#include "Utils/convertor.h"

GrayCharacterDegradationParameter::GrayCharacterDegradationParameter(
  DocumentController *docController,
  QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::GrayCharacterDegradationParameter)
{
  ui->setupUi(this);

  //
  _docController = docController;
  // open browser to find a saving folder
  connect(ui->btnSaveto, SIGNAL(clicked()), this, SLOT(chooseSaveDirectory()));
  //
}

GrayCharacterDegradationParameter::~GrayCharacterDegradationParameter()
{
  delete ui;
}

void
GrayCharacterDegradationParameter::chooseSaveDirectory()
{
  QString path =
    QFileDialog::getSaveFileName(this,
                                 tr("Save File"),
                                 QStringLiteral("degraded_image.png"),
                                 getWriteImageFilter());
  ui->txtFilePath->setText(path);
}

void
GrayCharacterDegradationParameter::degrade()
{

  const int level = ui->hsSlider->value(); // get degradation level

  //B: CODE DUPLICATION ? there is almost the same code in GrayCharacterDegradationDialog.cpp

  // get image to degrade
  //assert(_docController);
  QImage img = _docController->toQImage(WithTextBlocks | WithImageBlocks);
  if (img.isNull())
    return;

  // contructor for the degradation model
  GrayscaleCharsDegradationModel cdg(img);

  QImage dst = cdg.degradateByLevel(level);

  QString path = ui->txtFilePath->text();

  if (!path.isEmpty())
    dst.save(path);
}
