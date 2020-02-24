#include "OCRSettingsDialog.hpp"

#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <cassert>

OCRSettingsDialog::OCRSettingsDialog(QWidget *parent,
                                     const QString &tessdataParentDir)
  : QDialog(parent)
  , m_tessdataParentDir2(tessdataParentDir)
{
  buildGUI();

  getEnv();
  if (m_dirLE->text().isEmpty()) {
    m_dirLE->setText(tessdataParentDir);
  }
}

void
OCRSettingsDialog::buildGUI()
{
  auto dirL = new QLabel(tr("tessdata parent directory"), this);
  dirL->setToolTip(tr("Parent directory of your \"tessdata\" directory."));
  m_dirLE = new QLineEdit(this);
  m_dirLE->setReadOnly(true);
  connect(m_dirLE,
          SIGNAL(textChanged(QString)),
          this,
          SLOT(updateLanguages()));

  auto openDirB = new QPushButton(tr("..."), this);
  openDirB->setToolTip(tr("Search for parent directory of tessdata directory"));
  auto getEnvB = new QPushButton(tr("Get env."), this);
  getEnvB->setToolTip(tr("Set directory to TESSDATA_PREFIX environment variable"));

  connect(openDirB, SIGNAL(clicked()), this, SLOT(openDir()));
  connect(getEnvB, SIGNAL(clicked()), this, SLOT(getEnv()));

  auto langL = new QLabel(tr("Language: "), this);
  langL->setToolTip(tr("Choose language to recognize in your document"));
  m_langCB = new QComboBox(this);

  auto buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  m_okButton = buttonBox->button(QDialogButtonBox::Ok);

  auto hLayout = new QHBoxLayout;
  hLayout->addWidget(dirL);
  hLayout->addWidget(m_dirLE);
  hLayout->addWidget(openDirB);
  hLayout->addWidget(getEnvB);
  auto hLayout2 = new QHBoxLayout;
  hLayout2->addWidget(langL);
  hLayout2->addWidget(m_langCB);

  auto vLayout = new QVBoxLayout;
  vLayout->addLayout(hLayout);
  vLayout->addLayout(hLayout2);
  vLayout->addWidget(buttonBox);

  setLayout(vLayout);

  setWindowTitle(tr("Tesseract OCR language settings"));
}

QString
OCRSettingsDialog::tessdataParentDir() const
{
  assert(m_dirLE);
  //B:Warning: tessdataParentDir must end with "/"
  QString path = m_dirLE->text();
  if (!path.isEmpty() && !path.endsWith('/')) {
    path.push_back('/');
  }
  return path;
}

QString
OCRSettingsDialog::language() const
{
  assert(m_langCB);
  return m_langCB->currentText();
}

void
OCRSettingsDialog::openDir()
{
  const QString path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Choose tessdata parent directory"),
                                      QStringLiteral("./"),
                                      QFileDialog::ShowDirsOnly);
  m_dirLE->setText(path);
}

void
OCRSettingsDialog::getEnv()
{
  QString envVar = qgetenv("TESSDATA_PREFIX");
  m_dirLE->setText(envVar);
}

void
OCRSettingsDialog::updateLanguages()
{
  //populate m_langCB
  m_langCB->clear();

  QDir dir(m_dirLE->text());
  bool ok = dir.cd(QStringLiteral("tessdata"));
  if (ok) {
    dir.setFilter(QDir::Files | QDir::Readable);
    QStringList filters;
    filters << QStringLiteral("*.traineddata");
    dir.setNameFilters(filters);
    QStringList files = dir.entryList();
    for (const auto &file : files) {
      QString lang = file;
      lang.truncate(file.lastIndexOf(QStringLiteral(".traineddata")));
      m_langCB->addItem(lang);
    }
  }

  //enable/disable "Ok" button
  const bool enabled = (m_langCB->count() > 0);
  m_okButton->setEnabled(enabled);
}
