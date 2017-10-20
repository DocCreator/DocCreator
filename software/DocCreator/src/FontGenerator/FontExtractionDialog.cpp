#include "FontExtractionDialog.hpp"
#include "ui_FontExtractionDialog.h"

FontExtractionDialog::FontExtractionDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::FontExtractionDialog)
  , m_criteria(0)
{
  ui->setupUi(this);
}

FontExtractionDialog::~FontExtractionDialog()
{
  delete ui;
}

void
FontExtractionDialog::on_fontName_textChanged(const QString &arg1)
{
  m_fontName = arg1;
}

void
FontExtractionDialog::on_criteriaBox_currentIndexChanged(int index)
{
  m_criteria = index;
}

void
FontExtractionDialog::init()
{
  m_criteria = 0;
}

QString
FontExtractionDialog::getFontName()
{
  return m_fontName;
}

int
FontExtractionDialog::getCriteria()
{
  return m_criteria;
}
