#include "LabelingComponentDialog.hpp"
#include "ui_LabelingComponentDialog.h"

LabelingComponentDialog::LabelingComponentDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::LabelingComponentDialog)
{
  _blockType = TextBlock;

  ui->setupUi(this);

  connect(
    ui->chkTextBlockType, SIGNAL(stateChanged(int)), this, SLOT(setChecked()));
  connect(
    ui->chkImageBlockType, SIGNAL(stateChanged(int)), this, SLOT(setChecked()));
  connect(
    ui->chkOtherType, SIGNAL(stateChanged(int)), this, SLOT(setChecked()));
  connect(
    ui->chkBackGround, SIGNAL(stateChanged(int)), this, SLOT(setChecked()));
}
void
LabelingComponentDialog::setChecked()
{
  if (ui->chkTextBlockType->isChecked())
    _blockType = TextBlock;
  if (ui->chkImageBlockType->isChecked())
    _blockType = ImageBlock;
  if (ui->chkBackGround->isChecked())
    _blockType = BackGround;
  if (ui->chkOtherType->isChecked())
    _blockType = Other;
}

LabelingComponentDialog::~LabelingComponentDialog()
{
  delete ui;
}
