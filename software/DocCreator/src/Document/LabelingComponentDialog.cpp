#include "LabelingComponentDialog.hpp"
#include "ui_LabelingComponentDialog.h"

LabelingComponentDialog::LabelingComponentDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::LabelingComponentDialog)
{
  _blockType = BlockType::TextBlock;

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
    _blockType = BlockType::TextBlock;
  if (ui->chkImageBlockType->isChecked())
    _blockType = BlockType::ImageBlock;
  if (ui->chkBackGround->isChecked())
    _blockType = BlockType::BackGround;
  if (ui->chkOtherType->isChecked())
    _blockType = BlockType::Other;
}

LabelingComponentDialog::~LabelingComponentDialog()
{
  delete ui;
}
