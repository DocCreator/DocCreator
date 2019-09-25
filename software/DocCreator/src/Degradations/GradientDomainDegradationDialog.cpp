#include "GradientDomainDegradationDialog.hpp"

#include "ui_GradientDomainDegradationDialog.h"

#include <QFileDialog>
#include <QVariant>

#include "Utils/ImageUtils.hpp" //getWriteImageFilter()

#include "appconstants.h"
#include "core/configurationmanager.h"


QString
GradientDomainDegradationDialog::getStainImagesDefaultPath()
{
  return Core::ConfigurationManager::get(AppConfigMainGroup,
                                         AppConfigStainImagesFolderKey)
    .toString();
}

GradientDomainDegradationDialog::GradientDomainDegradationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::GradientDomainDegradationDialog)
{
  ui->setupUi(this);

  ui->comboBoxInsertType->addItem(tr("None"), QVariant(static_cast<int>(dc::GradientDomainDegradation::InsertType::INSERT_AS_IS)));
  ui->comboBoxInsertType->addItem(tr("To gray"), QVariant(static_cast<int>(dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY)));
  ui->comboBoxInsertType->addItem(tr("To gray if destination image is gray"), QVariant(static_cast<int>(dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY_IF_GRAY)));
  ui->comboBoxInsertType->setCurrentIndex(2);
      
  connect(ui->pushButtonStainsPath, SIGNAL(clicked()), this, SLOT(chooseImageDir()));
  connect(ui->pushButtonOutput, SIGNAL(clicked()), this, SLOT(chooseSaveFilename()));
  connect(ui->lineEditStainsPath, SIGNAL(textChanged(QString)), this, SLOT(updateCopyright()));
  connect(ui->lineEditOutput, SIGNAL(textChanged(QString)), this, SLOT(updateOkButton()));

  ui->lineEditStainsPath->setText(getStainImagesDefaultPath());
  updateCopyright();
}

GradientDomainDegradationDialog::~GradientDomainDegradationDialog()
{
  delete ui;
}

void
GradientDomainDegradationDialog::setOriginalImage(const QImage &img)
{
  _originalImg = img;

  updateOkButton();
}

QString GradientDomainDegradationDialog::getStainImagesPath() const
{
  return ui->lineEditStainsPath->text();
}

size_t
GradientDomainDegradationDialog::getNumStains() const
{
  return ui->spinBoxNumStains->value();
}

dc::GradientDomainDegradation::InsertType
GradientDomainDegradationDialog::getInsertType() const
{
  return static_cast<dc::GradientDomainDegradation::InsertType>( ui->comboBoxInsertType->currentData().toInt() );
}

bool
GradientDomainDegradationDialog::getDoRotations() const
{
  return ui->checkBoxRotations->isChecked();
}

QString
GradientDomainDegradationDialog::getOutputFilename() const
{
  return ui->lineEditOutput->text();
}

void
GradientDomainDegradationDialog::updateCopyright()
{
  if (ui->lineEditStainsPath->text() == getStainImagesDefaultPath()) {
    ui->labelCopyright->setText(tr("These stain images were extracted from various document images from http://www.e-codices.ch by Mathias Seuret et al."));
  }
  else {
    ui->labelCopyright->setText("");
  }
}

void
GradientDomainDegradationDialog::chooseImageDir()
{

  QString dir = ui->lineEditStainsPath->text();
  if (dir.isEmpty()) {
    dir = getStainImagesDefaultPath();
  }
  
 QString path =
    QFileDialog::getExistingDirectory(this,
                                 tr("Stain images directory"),
				      dir);

  ui->lineEditStainsPath->setText(path); 
}

void
GradientDomainDegradationDialog::chooseSaveFilename()
{
 QString path =
    QFileDialog::getSaveFileName(this,
                                 tr("Save File"),
                                 QStringLiteral("degraded_image.png"),
                                 getWriteImageFilter());
  ui->lineEditOutput->setText(path); 
}

void
GradientDomainDegradationDialog::updateOkButton()
{
  const bool imageValid = (! _originalImg.isNull());
  const bool stainsDirValid = isStainImagesDirValid();
  const bool outputFileValid = isOutputFileValid();

  QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
  assert(okButton);
  if (imageValid && stainsDirValid && outputFileValid) {
    okButton->setEnabled(true);
  }
  else {
    okButton->setEnabled(false);
  }
}

bool
GradientDomainDegradationDialog::isStainImagesDirValid() const
{
  const QString path = getStainImagesPath();
  if (path.isEmpty()) {
    return false;
  }
  return QFileInfo(path).dir().exists();
  //TODO: also check that there are images in directory
}

bool
GradientDomainDegradationDialog::isOutputFileValid() const
{
  const QString path = getOutputFilename();
  if (path.isEmpty()) {
    return false;
  }
  return QFileInfo(path).dir().exists();
}

