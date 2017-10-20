#include "RandomDocumentParametersDialog.hpp"
#include "ui_RandomDocumentParametersDialog.h"

#include <QFileDialog>

RandomDocumentParametersDialog::RandomDocumentParametersDialog(QWidget *parent)
  : QDialog(parent)
  , _ui(new Ui::RandomDocumentParametersDialog)
  , _params(this)
{
  _ui->setupUi(this);

  _ui->nbBlocksPerRowSpinBoxMin->setValue(_params.nbBlocksPerRowMin);
  _ui->nbBlocksPerRowSpinBoxMax->setValue(_params.nbBlocksPerRowMax);
  _ui->nbBlocksPerColSpinBoxMin->setValue(_params.nbBlocksPerColMin);
  _ui->nbBlocksPerColSpinBoxMax->setValue(_params.nbBlocksPerColMax);

  _ui->bottomMarginSpinBox->setValue(_params.bottomMarginMax);
  _ui->rightMarginSpinBox->setValue(_params.rightMarginMax);
  _ui->leftMarginSpinBox->setValue(_params.leftMarginMax);
  _ui->topMarginSpinBox->setValue(_params.topMarginMax);

  _ui->lineSpacingMaxSpinBox->setValue(_params.lineSpacingMax);
  _ui->lineSpacingMinSpinBox->setValue(_params.lineSpacingMin);

  //_ui->nbDocsSpinBox->setValue(_params.nbDocs);
  _ui->nbPagesSpinBox->setValue(_params.nbPages);
  _ui->percentOfEmptyBlocksSpinBox->setValue(_params.percentOfEmptyBlocks);

  connect(_ui->outPutFolderButton,
          SIGNAL(clicked()),
          this,
          SLOT(chooseOutputFolderSlot()));

  connect(
    _ui->fontListButton, SIGNAL(clicked()), this, SLOT(chooseFontsSlot()));

  connect(_ui->textButton, SIGNAL(clicked()), this, SLOT(chooseTextsSlot()));

  connect(_ui->lineSpacingMinSpinBox,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(udateLineSpacingMin()));
  connect(_ui->lineSpacingMaxSpinBox,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(udateLineSpacingMax()));

  connect(_ui->bottomMarginSpinBox,
          SIGNAL(valueChanged(int)),
          &_params,
          SLOT(setBottomMargin(int)));
  connect(_ui->leftMarginSpinBox,
          SIGNAL(valueChanged(int)),
          &_params,
          SLOT(setLeftMargin(int)));
  connect(_ui->rightMarginSpinBox,
          SIGNAL(valueChanged(int)),
          &_params,
          SLOT(setRightMargin(int)));
  connect(_ui->topMarginSpinBox,
          SIGNAL(valueChanged(int)),
          &_params,
          SLOT(setTopMargin(int)));

  connect(_ui->nbBlocksPerColSpinBoxMin,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(updateNbBlocksPerColMin()));
  connect(_ui->nbBlocksPerColSpinBoxMax,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(updateNbBlocksPerColMax()));
  connect(_ui->nbBlocksPerRowSpinBoxMin,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(updateNbBlocksPerRowsMin()));
  connect(_ui->nbBlocksPerRowSpinBoxMax,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(updateNbBlocksPerRowsMax()));

  //connect(_ui->nbDocsSpinBox, SIGNAL(valueChanged(int)), &_params, SLOT(setNbDocs(int)));
  connect(_ui->nbPagesSpinBox,
          SIGNAL(valueChanged(int)),
          &_params,
          SLOT(setNbPages(int)));
  connect(_ui->percentOfEmptyBlocksSpinBox,
          SIGNAL(valueChanged(int)),
          &_params,
          SLOT(setPercentOfEmptyBlocks(int)));

  //B:TODO: we should be able to validate only if !_params.outputFolderPath.isEmpty() && !_params.fontList.isEmpty()
}

RandomDocumentParametersDialog::~RandomDocumentParametersDialog()
{
  delete _ui;
}

void
RandomDocumentParametersDialog::updateNbBlocksPerColMin()
{
  if (_ui->nbBlocksPerColSpinBoxMin->value() >
      _ui->nbBlocksPerColSpinBoxMax->value())
    _ui->nbBlocksPerColSpinBoxMax->setValue(
      _ui->nbBlocksPerColSpinBoxMin->value());

  _params.setNbBlocksPerColMinMax(_ui->nbBlocksPerColSpinBoxMin->value(),
                                  _ui->nbBlocksPerColSpinBoxMax->value());
}
void
RandomDocumentParametersDialog::updateNbBlocksPerColMax()
{
  if (_ui->nbBlocksPerColSpinBoxMax->value() <
      _ui->nbBlocksPerColSpinBoxMin->value())
    _ui->nbBlocksPerColSpinBoxMin->setValue(
      _ui->nbBlocksPerColSpinBoxMax->value());

  _params.setNbBlocksPerColMinMax(_ui->nbBlocksPerColSpinBoxMin->value(),
                                  _ui->nbBlocksPerColSpinBoxMax->value());
}

void
RandomDocumentParametersDialog::updateNbBlocksPerRowMin()
{
  if (_ui->nbBlocksPerRowSpinBoxMin->value() >
      _ui->nbBlocksPerRowSpinBoxMax->value())
    _ui->nbBlocksPerRowSpinBoxMax->setValue(
      _ui->nbBlocksPerRowSpinBoxMin->value());

  _params.setNbBlocksPerRowMinMax(_ui->nbBlocksPerRowSpinBoxMin->value(),
                                  _ui->nbBlocksPerRowSpinBoxMax->value());
}
void
RandomDocumentParametersDialog::updateNbBlocksPerRowMax()
{
  if (_ui->nbBlocksPerRowSpinBoxMax->value() <
      _ui->nbBlocksPerRowSpinBoxMin->value())
    _ui->nbBlocksPerRowSpinBoxMin->setValue(
      _ui->nbBlocksPerRowSpinBoxMax->value());

  _params.setNbBlocksPerRowMinMax(_ui->nbBlocksPerRowSpinBoxMin->value(),
                                  _ui->nbBlocksPerRowSpinBoxMax->value());
}

void
RandomDocumentParametersDialog::updateLineSpacingMin()
{
  if (_ui->lineSpacingMinSpinBox->value() > _ui->lineSpacingMaxSpinBox->value())
    _ui->lineSpacingMaxSpinBox->setValue(_ui->lineSpacingMinSpinBox->value());

  _params.setLineSpacingMinMax(_ui->lineSpacingMinSpinBox->value(),
                               _ui->lineSpacingMaxSpinBox->value());
}
void
RandomDocumentParametersDialog::updateLineSpacingMax()
{
  if (_ui->lineSpacingMaxSpinBox->value() < _ui->lineSpacingMinSpinBox->value())
    _ui->lineSpacingMinSpinBox->setValue(_ui->lineSpacingMaxSpinBox->value());

  _params.setLineSpacingMinMax(_ui->lineSpacingMinSpinBox->value(),
                               _ui->lineSpacingMaxSpinBox->value());
}

void
RandomDocumentParametersDialog::changeEvent(QEvent *e)
{
  QDialog::changeEvent(e);
  switch (e->type()) {
    case QEvent::LanguageChange:
      _ui->retranslateUi(this);
      break;
    default:
      break;
  }
}

void
RandomDocumentParametersDialog::chooseOutputFolderSlot()
{
  QString folderPath =
    QFileDialog::getExistingDirectory(this, tr("OutPut Folder"));
  _ui->outPutFolderLine->setText(folderPath);
  _params.outputFolderPath = folderPath;
}

void
RandomDocumentParametersDialog::chooseFontsSlot()
{
  QString dir;
  QStringList fontList =
    QFileDialog::getOpenFileNames(this,
                                  tr("Choose font files..."),
                                  dir,
                                  QStringLiteral("Xml Old font (*.of *.xml)"));
  _ui->fontListLine->setText(fontList.join(QStringLiteral(",")));
  _params.fontList = fontList;
}

void
RandomDocumentParametersDialog::chooseTextsSlot()
{
  QString dir;
  QStringList textList =
    QFileDialog::getOpenFileNames(this,
                                  tr("Choose text files..."),
                                  dir,
                                  QStringLiteral("Text files (*.txt)"));
  _ui->textLineEdit->setText(textList.join(QStringLiteral(",")));
  _params.textList = textList;
}

//B:TODO: If _ui->fontListLine & _ui->textListLine is filled manually with filenames separated by commas, we do not get them !
