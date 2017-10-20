#define _USE_MATH_DEFINES // for Visual

#include "RandomScreenshotsParamsDialog.hpp"

#include <cmath> //M_PI

#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QImageWriter>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <cmath>

static const int DEFAULT_NUM_SCREENSHOTS = 10;

static const double DEFAULT_MIN_PHI = -0.8;
static const double DEFAULT_MAX_PHI = 0.8;
static const double DEFAULT_MIN_THETA = -0.8;
static const double DEFAULT_MAX_THETA = 0.8;

RandomScreenshotsParamsDialog::RandomScreenshotsParamsDialog(QWidget *parent)
  : QDialog(parent)
{

  QLabel *dstDirectoryL = new QLabel(tr("Destination directory: "), this);
  m_dstDirectory = new QLineEdit(this);
  QPushButton *dstDirectoryB = new QPushButton(tr("..."), this);

  QLabel *dstPrefixL = new QLabel(tr("Output filename prefix: "), this);
  m_dstPrefix = new QLineEdit(this);
  m_dstPrefix->setText(tr("screenshot"));

  QLabel *extL = new QLabel(tr("Extension: "), this);
  m_extensions = new QComboBox(this);

  QList<QByteArray> list = QImageWriter::supportedImageFormats();
  const int sz = list.size();
  for (int i = 0; i < sz; ++i) {
    const QString s(list[i]);
    m_extensions->addItem(s);
  }
  int ind = m_extensions->findText(QStringLiteral("png"), Qt::MatchFixedString);
  if (ind != -1) {
    m_extensions->setCurrentIndex(ind);
  }

  QLabel *numScreenshotsL = new QLabel(tr("Number of images: "), this);
  m_numScreenshots = new QSpinBox(this);
  m_numScreenshots->setMinimum(1);
  m_numScreenshots->setMaximum(10000);
  m_numScreenshots->setValue(DEFAULT_NUM_SCREENSHOTS);

  m_advancedButton = new QPushButton(tr("&Advanced options"));
  m_advancedButton->setCheckable(true);

  m_extension = new QWidget; //without parent on purpose
  m_minPhy = new QDoubleSpinBox;
  m_minPhy->setDecimals(2);
  m_minPhy->setSingleStep(0.01);
  m_minPhy->setMinimum(-M_PI);
  m_minPhy->setMaximum(M_PI);
  m_minPhy->setValue(DEFAULT_MIN_PHI);
  m_maxPhy = new QDoubleSpinBox;
  m_maxPhy->setDecimals(2);
  m_maxPhy->setSingleStep(0.01);
  m_maxPhy->setMinimum(-M_PI);
  m_maxPhy->setMaximum(M_PI);
  m_maxPhy->setValue(DEFAULT_MAX_PHI);
  m_minTheta = new QDoubleSpinBox;
  m_minTheta->setDecimals(2);
  m_minTheta->setSingleStep(0.01);
  m_minTheta->setMinimum(-M_PI);
  m_minTheta->setMaximum(M_PI);
  m_minTheta->setValue(DEFAULT_MIN_THETA);
  m_maxTheta = new QDoubleSpinBox;
  m_maxTheta->setDecimals(2);
  m_maxTheta->setSingleStep(0.01);
  m_maxTheta->setMinimum(-M_PI);
  m_maxTheta->setMaximum(M_PI);
  m_maxTheta->setValue(DEFAULT_MAX_THETA);

  auto phyLabel = new QLabel(tr("Phy: "));
  auto phyMinLabel = new QLabel(tr("min"));
  auto phyMaxLabel = new QLabel(tr("max"));
  auto thetaLabel = new QLabel(tr("Theta: "));
  auto thetaMinLabel = new QLabel(tr("min"));
  auto thetaMaxLabel = new QLabel(tr("max"));
  auto hPhyMin = new QHBoxLayout;
  hPhyMin->addWidget(phyMinLabel);
  hPhyMin->addWidget(m_minPhy);
  hPhyMin->addStretch();
  auto hPhyMax = new QHBoxLayout;
  hPhyMax->addWidget(phyMaxLabel);
  hPhyMax->addWidget(m_maxPhy);
  hPhyMax->addStretch();
  auto hThetaMin = new QHBoxLayout;
  hThetaMin->addWidget(thetaMinLabel);
  hThetaMin->addWidget(m_minTheta);
  hThetaMin->addStretch();
  auto hThetaMax = new QHBoxLayout;
  hThetaMax->addWidget(thetaMaxLabel);
  hThetaMax->addWidget(m_maxTheta);
  hThetaMin->addStretch();

  auto gLayout = new QGridLayout;
  gLayout->addWidget(phyLabel, 0, 0);
  gLayout->addLayout(hPhyMin, 0, 1);
  gLayout->addLayout(hPhyMax, 0, 2);
  gLayout->addWidget(thetaLabel, 1, 0);
  gLayout->addLayout(hThetaMin, 1, 1);
  gLayout->addLayout(hThetaMax, 1, 2);
  m_extension->setLayout(gLayout);

  auto advancedL = new QHBoxLayout;
  advancedL->addWidget(m_advancedButton);
  advancedL->addStretch();

  connect(m_advancedButton,
          SIGNAL(toggled(bool)),
          m_extension,
          SLOT(setVisible(bool)));

  m_extension->hide();

  auto buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  auto vLayout = new QVBoxLayout;
  auto hLayout1 = new QHBoxLayout;
  hLayout1->addWidget(dstDirectoryL);
  hLayout1->addWidget(m_dstDirectory);
  hLayout1->addWidget(dstDirectoryB);

  auto hLayout2 = new QHBoxLayout;
  hLayout2->addWidget(dstPrefixL);
  hLayout2->addWidget(m_dstPrefix);

  auto hLayout3 = new QHBoxLayout;
  hLayout3->addWidget(extL);
  hLayout3->addWidget(m_extensions);

  auto hLayout4 = new QHBoxLayout;
  hLayout4->addWidget(numScreenshotsL);
  hLayout4->addWidget(m_numScreenshots);

  vLayout->addLayout(hLayout1);
  vLayout->addLayout(hLayout2);
  vLayout->addLayout(hLayout3);
  vLayout->addLayout(hLayout4);
  vLayout->addLayout(advancedL);
  vLayout->addWidget(m_extension);
  vLayout->addWidget(buttonBox);
  vLayout->addStretch();

  setLayout(vLayout);

  connect(dstDirectoryB, SIGNAL(clicked()), this, SLOT(chooseDstDirectory()));
}

void
RandomScreenshotsParamsDialog::setDstDirectory(const QString &dir)
{
  m_dstDirectory->setText(dir);
}

QString
RandomScreenshotsParamsDialog::getDstDirectory() const
{
  return m_dstDirectory->text();
}

void
RandomScreenshotsParamsDialog::setDstPrefix(const QString &prefix)
{
  m_dstPrefix->setText(prefix);
}

QString
RandomScreenshotsParamsDialog::getDstPrefix() const
{
  return m_dstPrefix->text();
}

void
RandomScreenshotsParamsDialog::setDstExtension(const QString &text)
{
  int ind = m_extensions->findText(text);
  if (ind != -1) {
    m_extensions->setCurrentIndex(ind);
  }
}

QString
RandomScreenshotsParamsDialog::getDstExtension() const
{
  return m_extensions->currentText();
}

void
RandomScreenshotsParamsDialog::setNumScreenshots(size_t num)
{
  m_numScreenshots->setValue(num);
}

size_t
RandomScreenshotsParamsDialog::getNumScreenshots() const
{
  return m_numScreenshots->value();
}

void
RandomScreenshotsParamsDialog::chooseDstDirectory()
{
  QString dir = QFileDialog::getExistingDirectory(
    this,
    tr("Choose destination directory"),
    QString(),
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  m_dstDirectory->setText(dir);
}

void
RandomScreenshotsParamsDialog::setMinPhy(float v)
{
  m_minPhy->setValue(v);
}

float
RandomScreenshotsParamsDialog::getMinPhy() const
{
  return m_minPhy->value();
}

void
RandomScreenshotsParamsDialog::setMaxPhy(float v)
{
  m_maxPhy->setValue(v);
}

float
RandomScreenshotsParamsDialog::getMaxPhy() const
{
  return m_maxPhy->value();
}

void
RandomScreenshotsParamsDialog::setMinTheta(float v)
{
  m_minTheta->setValue(v);
}

float
RandomScreenshotsParamsDialog::getMinTheta() const
{
  return m_minTheta->value();
}

void
RandomScreenshotsParamsDialog::setMaxTheta(float v)
{
  m_maxTheta->setValue(v);
}

float
RandomScreenshotsParamsDialog::getMaxTheta() const
{
  return m_maxTheta->value();
}
