#define _USE_MATH_DEFINES // for Visual

#include "MeshWindow.hpp"

#include <cmath> //M_PI

#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDesktopWidget>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

#include "GLWidget.hpp"
#include "RandomScreenshotsParamsDialog.hpp"
#include "Utils/ImageUtils.hpp"

#include <iostream> //DEBUG

static const float DEFAULT_LIGHT_X = 0.1f;
static const float DEFAULT_LIGHT_Y = 0.1f;
static const float DEFAULT_LIGHT_Z = 5.0f;

static const float DEFAULT_AMBIENT = 0.01f;
static const float DEFAULT_DIFFUSE = 0.81f;
static const float DEFAULT_SPECULAR = 0.16f;
static const float DEFAULT_SPECULAR_EXPONENT = 6.0f;

MeshWindow::MeshWindow(QWidget *parent)
  : QWidget(parent)
  , m_hasTexCoords(false)
  , m_hasImage(false)
{
  this->setAttribute(Qt::WA_DeleteOnClose);

  auto mainLayout = new QGridLayout;

  m_glWidget = new GLWidget(this);

  m_glWidget->setMinimumSize(800, 600);

  QDesktopWidget desktop;
  const int h = desktop.geometry().height();
  const int w = desktop.geometry().width();
  resize(w, h);

  mainLayout->addWidget(m_glWidget);

  QLabel *lightPosLabel = new QLabel(tr("Light position: "), this);
  m_lightPosX = new QDoubleSpinBox(this);
  m_lightPosY = new QDoubleSpinBox(this);
  m_lightPosZ = new QDoubleSpinBox(this);
  m_lightPosX->setValue(DEFAULT_LIGHT_X);
  m_lightPosY->setValue(DEFAULT_LIGHT_Y);
  m_lightPosZ->setValue(DEFAULT_LIGHT_Z);

  m_lightPosX->setMinimum(-5.0);
  m_lightPosX->setMaximum(5.0);
  m_lightPosY->setMinimum(-5.0);
  m_lightPosY->setMaximum(5.0);
  m_lightPosZ->setMinimum(-5.0);
  m_lightPosZ->setMaximum(5.0);
  m_lightPosX->setSingleStep(0.04);
  m_lightPosY->setSingleStep(0.04);
  m_lightPosZ->setSingleStep(0.04);

  QLabel *lightLabel = new QLabel(tr("Light/Material: "), this);
  QLabel *lightAmbientLabel = new QLabel(tr("Ambient: "), this);
  QLabel *lightDiffuseLabel = new QLabel(tr("Diffuse: "), this);
  QLabel *lightSpecularLabel = new QLabel(tr("Specular: "), this);
  QLabel *lightSpecularExponentLabel =
    new QLabel(tr("Specular exponent: "), this);
  m_lightAmbient = new QDoubleSpinBox(this);
  m_lightDiffuse = new QDoubleSpinBox(this);
  m_lightSpecular = new QDoubleSpinBox(this);

  m_lightAmbient->setValue(DEFAULT_AMBIENT);
  m_lightDiffuse->setValue(DEFAULT_DIFFUSE);
  m_lightSpecular->setValue(DEFAULT_SPECULAR);

  m_lightAmbient->setMinimum(0.0);
  m_lightAmbient->setMaximum(1.0);
  m_lightAmbient->setSingleStep(0.01);
  m_lightDiffuse->setMinimum(0.0);
  m_lightDiffuse->setMaximum(1.0);
  m_lightDiffuse->setSingleStep(0.02);
  m_lightSpecular->setMinimum(0.0);
  m_lightSpecular->setMaximum(1.0);
  m_lightSpecular->setSingleStep(0.02);

  m_lightSpecularExponent = new QDoubleSpinBox(this);
  m_lightSpecularExponent->setMinimum(0.0);
  m_lightSpecularExponent->setMaximum(256.0);
  m_lightSpecularExponent->setSingleStep(1.0);
  m_lightSpecularExponent->setValue(DEFAULT_SPECULAR_EXPONENT);

  m_loadMesh = new QPushButton(tr("Load mesh..."), this);
  m_saveMesh = new QPushButton(tr("Save mesh..."), this);
  m_loadImg = new QPushButton(tr("Load texture..."), this);
  m_computeTexCoords = new QPushButton(tr("Compute texture coords"), this);

  m_takeScreenshot = new QPushButton(tr("Take screenshot..."), this);
  m_takeScreenshotHiRes =
    new QPushButton(tr("Take high resolution screenshot..."), this);

  m_useTexture = new QCheckBox(tr("Use texture"), this);
  m_useTexture->setCheckState(Qt::Unchecked);

  m_resetTexMat = new QPushButton(tr("Reset texture"), this);
  m_applyTexMat = new QPushButton(tr("Apply texture transform"), this);
  m_resetViewMat = new QPushButton(tr("Reset view"), this);
  m_applyViewMat = new QPushButton(tr("Apply view transform"), this);

  QLabel *texTranslationLabel = new QLabel(tr("translation: "), this);
  QLabel *texScaleLabel = new QLabel(tr("scale: "), this);
  QLabel *texRotationLabel = new QLabel(tr("rotation: "), this);
  m_tex_tu = new QDoubleSpinBox(this);
  m_tex_tu->setMinimum(-2.0);
  m_tex_tu->setMaximum(2.0);
  m_tex_tu->setSingleStep(0.01);
  m_tex_tu->setValue(0);

  m_tex_tv = new QDoubleSpinBox(this);
  m_tex_tv->setMinimum(-2.0);
  m_tex_tv->setMaximum(2.0);
  m_tex_tv->setSingleStep(0.01);
  m_tex_tv->setValue(0);

  m_tex_su = new QDoubleSpinBox(this);
  m_tex_su->setMinimum(-2.0);
  m_tex_su->setMaximum(2.0);
  m_tex_su->setSingleStep(0.01);
  m_tex_su->setValue(1);

  m_tex_sv = new QDoubleSpinBox(this);
  m_tex_sv->setMinimum(-2.0);
  m_tex_sv->setMaximum(2.0);
  m_tex_sv->setSingleStep(0.01);
  m_tex_sv->setValue(1);

  m_tex_r = new QDoubleSpinBox(this);
  m_tex_r->setMinimum(0.0);
  m_tex_r->setMaximum(360.0);
  m_tex_r->setSingleStep(0.1);
  m_tex_r->setValue(0);

  //background
  m_useBackground = new QCheckBox(tr("Use background"), this);
  m_loadBackgroundTexture = new QPushButton(tr("Load background..."), this);
  QLabel *backgroundTexTranslationLabel = new QLabel(tr("translation: "), this);
  QLabel *backgroundTexScaleLabel = new QLabel(tr("scale: "), this);
  m_backgroundTex_tu = new QDoubleSpinBox(this);
  m_backgroundTex_tu->setMinimum(-10.0);
  m_backgroundTex_tu->setMaximum(10.0);
  m_backgroundTex_tu->setSingleStep(0.01);
  m_backgroundTex_tu->setValue(0.0);

  m_backgroundTex_tv = new QDoubleSpinBox(this);
  m_backgroundTex_tv->setMinimum(-10.0);
  m_backgroundTex_tv->setMaximum(10.0);
  m_backgroundTex_tv->setSingleStep(0.01);
  m_backgroundTex_tv->setValue(0.0);

  m_backgroundTex_su = new QDoubleSpinBox(this);
  m_backgroundTex_su->setMinimum(-10.0);
  m_backgroundTex_su->setMaximum(10.0);
  m_backgroundTex_su->setSingleStep(0.01);
  m_backgroundTex_su->setValue(1);

  m_backgroundTex_sv = new QDoubleSpinBox(this);
  m_backgroundTex_sv->setMinimum(-10.0);
  m_backgroundTex_sv->setMaximum(10.0);
  m_backgroundTex_sv->setSingleStep(0.01);
  m_backgroundTex_sv->setValue(1);

  m_saveRandomImages = new QPushButton(tr("Save random images..."), this);

  connect(
    m_lightPosX, SIGNAL(valueChanged(double)), this, SLOT(changeLightPos()));
  connect(
    m_lightPosY, SIGNAL(valueChanged(double)), this, SLOT(changeLightPos()));
  connect(
    m_lightPosZ, SIGNAL(valueChanged(double)), this, SLOT(changeLightPos()));
  connect(m_lightAmbient,
          SIGNAL(valueChanged(double)),
          this,
          SLOT(changeAmbientDiffuseSpecular()));
  connect(m_lightDiffuse,
          SIGNAL(valueChanged(double)),
          this,
          SLOT(changeAmbientDiffuseSpecular()));
  connect(m_lightSpecular,
          SIGNAL(valueChanged(double)),
          this,
          SLOT(changeAmbientDiffuseSpecular()));
  connect(m_lightSpecularExponent,
          SIGNAL(valueChanged(double)),
          this,
          SLOT(changeSpecularExponent()));
  connect(m_takeScreenshot, SIGNAL(clicked()), this, SLOT(takeScreenshot()));
  connect(m_takeScreenshotHiRes,
          SIGNAL(clicked()),
          this,
          SLOT(takeScreenshotHiRes()));
  connect(m_useTexture, SIGNAL(stateChanged(int)), this, SLOT(useTexture()));

  connect(m_loadMesh, SIGNAL(clicked()), this, SLOT(loadMesh()));
  connect(m_saveMesh, SIGNAL(clicked()), this, SLOT(saveMesh()));
  connect(m_loadImg, SIGNAL(clicked()), this, SLOT(loadTexture()));
  connect(
    m_computeTexCoords, SIGNAL(clicked()), this, SLOT(computeTexCoords()));

  connect(
    m_glWidget, SIGNAL(hasTexCoords(bool)), this, SLOT(hasTexCoords(bool)));
  connect(m_glWidget, SIGNAL(hasImage(bool)), this, SLOT(hasImage(bool)));

  connect(m_tex_tu, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  connect(m_tex_tv, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  connect(m_tex_su, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  connect(m_tex_sv, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  connect(m_tex_r, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  connect(m_resetTexMat, SIGNAL(clicked()), this, SLOT(resetTexMat()));
  connect(m_applyTexMat, SIGNAL(clicked()), this, SLOT(applyTexMat()));
  connect(m_resetViewMat, SIGNAL(clicked()), this, SLOT(resetViewMat()));
  connect(m_applyViewMat, SIGNAL(clicked()), this, SLOT(applyViewMat()));

  connect(
    m_useBackground, SIGNAL(stateChanged(int)), this, SLOT(useBackground()));
  connect(m_loadBackgroundTexture,
          SIGNAL(clicked()),
          this,
          SLOT(loadBackgroundTexture()));
  connect(m_backgroundTex_tu,
          SIGNAL(valueChanged(double)),
          this,
          SLOT(changeBackgroundTexMat()));
  connect(m_backgroundTex_tv,
          SIGNAL(valueChanged(double)),
          this,
          SLOT(changeBackgroundTexMat()));
  connect(m_backgroundTex_su,
          SIGNAL(valueChanged(double)),
          this,
          SLOT(changeBackgroundTexMat()));
  connect(m_backgroundTex_sv,
          SIGNAL(valueChanged(double)),
          this,
          SLOT(changeBackgroundTexMat()));

  connect(
    m_saveRandomImages, SIGNAL(clicked()), this, SLOT(saveRandomImages()));

  auto vLayout = new QVBoxLayout;

  auto hLayout1 = new QHBoxLayout;
  hLayout1->addWidget(lightPosLabel);
  hLayout1->addWidget(m_lightPosX);
  hLayout1->addWidget(m_lightPosY);
  hLayout1->addWidget(m_lightPosZ);

  auto hLayoutLA = new QHBoxLayout;
  hLayoutLA->addWidget(lightAmbientLabel);
  hLayoutLA->addWidget(m_lightAmbient);
  auto hLayoutLD = new QHBoxLayout;
  hLayoutLD->addWidget(lightDiffuseLabel);
  hLayoutLD->addWidget(m_lightDiffuse);
  auto hLayoutLS = new QHBoxLayout;
  hLayoutLS->addWidget(lightSpecularLabel);
  hLayoutLS->addWidget(m_lightSpecular);
  auto hLayoutLSE = new QHBoxLayout;
  hLayoutLSE->addWidget(lightSpecularExponentLabel);
  hLayoutLSE->addWidget(m_lightSpecularExponent);

  auto hLayout2 = new QHBoxLayout;
  hLayout2->addWidget(m_loadMesh);
  hLayout2->addWidget(m_loadImg);

  auto hLayoutTexT = new QHBoxLayout;
  hLayoutTexT->addWidget(texTranslationLabel);
  hLayoutTexT->addWidget(m_tex_tu);
  hLayoutTexT->addWidget(m_tex_tv);
  auto hLayoutTexS = new QHBoxLayout;
  hLayoutTexS->addWidget(texScaleLabel);
  hLayoutTexS->addWidget(m_tex_su);
  hLayoutTexS->addWidget(m_tex_sv);
  auto hLayoutTexR = new QHBoxLayout;
  hLayoutTexR->addWidget(texRotationLabel);
  hLayoutTexR->addWidget(m_tex_r);

  auto hLayoutTexM = new QHBoxLayout;
  hLayoutTexM->addWidget(m_resetTexMat);
  hLayoutTexM->addWidget(m_applyTexMat);

  auto hLayoutViewM = new QHBoxLayout;
  hLayoutViewM->addWidget(m_resetViewMat);
  hLayoutViewM->addWidget(m_applyViewMat);

  auto backgroundGB = new QGroupBox(tr("Background"), this);
  auto backgroundVLayout = new QVBoxLayout;
  auto backgroundHLayout0 = new QHBoxLayout;
  backgroundHLayout0->addWidget(m_useBackground);
  backgroundHLayout0->addWidget(m_loadBackgroundTexture);
  auto backgroundHLayout1 = new QHBoxLayout;
  backgroundHLayout1->addWidget(backgroundTexTranslationLabel);
  backgroundHLayout1->addWidget(m_backgroundTex_tu);
  backgroundHLayout1->addWidget(m_backgroundTex_tv);
  auto backgroundHLayout2 = new QHBoxLayout;
  backgroundHLayout2->addWidget(backgroundTexScaleLabel);
  backgroundHLayout2->addWidget(m_backgroundTex_su);
  backgroundHLayout2->addWidget(m_backgroundTex_sv);

  backgroundVLayout->addLayout(backgroundHLayout0);
  backgroundVLayout->addLayout(backgroundHLayout1);
  backgroundVLayout->addLayout(backgroundHLayout2);
  backgroundGB->setLayout(backgroundVLayout);

  vLayout->addLayout(hLayout2);
  vLayout->addLayout(hLayout1);
  vLayout->addWidget(lightLabel);
  vLayout->addLayout(hLayoutLA);
  vLayout->addLayout(hLayoutLD);
  vLayout->addLayout(hLayoutLS);
  vLayout->addLayout(hLayoutLSE);
  vLayout->addWidget(m_useTexture);
  vLayout->addWidget(m_computeTexCoords);

  vLayout->addLayout(hLayoutTexT);
  vLayout->addLayout(hLayoutTexS);
  vLayout->addLayout(hLayoutTexR);
  vLayout->addLayout(hLayoutTexM);

  vLayout->addLayout(hLayoutViewM);

  vLayout->addWidget(m_saveMesh);

  vLayout->addWidget(backgroundGB);

  vLayout->addWidget(m_takeScreenshot);
  vLayout->addWidget(m_takeScreenshotHiRes);
  vLayout->addWidget(m_saveRandomImages);

  vLayout->addStretch();

  mainLayout->addLayout(vLayout, 0, 1);

  setLayout(mainLayout);
}

void
MeshWindow::changeLightPos()
{
  const float x = m_lightPosX->value();
  const float y = m_lightPosY->value();
  const float z = m_lightPosZ->value();
  m_glWidget->setLightPos(x, y, z);
}

void
MeshWindow::changeAmbientDiffuseSpecular()
{
  const float a = m_lightAmbient->value();
  const float d = m_lightDiffuse->value();
  const float s = m_lightSpecular->value();

  Eigen::Vector3f av = Eigen::Vector3f::Constant(a);
  Eigen::Vector3f dv = Eigen::Vector3f::Constant(d);
  Eigen::Vector3f sv = Eigen::Vector3f::Constant(s);

  m_glWidget->setAmbientDiffuseSpecular(av, dv, sv);
}

void
MeshWindow::changeSpecularExponent()
{
  const float specularExponent = m_lightSpecularExponent->value();

  m_glWidget->setSpecularExponent(specularExponent);
}

void
MeshWindow::changeTexMat()
{
  //Get texture matrix
  const float tu = m_tex_tu->value();
  const float tv = m_tex_tv->value();
  const float su = m_tex_su->value();
  const float sv = m_tex_sv->value();
  const float r = m_tex_r->value();

  const float alpha = r * M_PI / 180.f;
  const float cos_alpha = std::cos(alpha);
  const float sin_alpha = std::sin(alpha);

  Eigen::Matrix3f texMat = Eigen::Matrix3f::Identity();
  texMat(0, 0) = su * cos_alpha;
  texMat(0, 1) = sin_alpha;
  texMat(0, 2) = tu;
  texMat(1, 0) = -sin_alpha;
  texMat(1, 1) = sv * cos_alpha;
  texMat(1, 2) = tv;

  //std::cerr<<"r="<<r<<" alpha="<<alpha<<" cos="<<cos_alpha<<" sin="<<sin_alpha<<"\n";
  //std::cerr<<"su="<<su<<" sv="<<sv<<" tu="<<tu<<" tv="<<tv<<"\n";
  //std::cerr<<"texMat:\n"<<texMat<<"\n";

  m_glWidget->setTextureMatrix(texMat);
}

void
MeshWindow::resetTexWidgets()
{
  disconnect(
    m_tex_tu, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  disconnect(
    m_tex_tv, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  disconnect(
    m_tex_su, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  disconnect(
    m_tex_sv, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  disconnect(m_tex_r, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));

  m_tex_tu->setValue(0);
  m_tex_tv->setValue(0);
  m_tex_su->setValue(1);
  m_tex_sv->setValue(1);
  m_tex_r->setValue(0);

  connect(m_tex_tu, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  connect(m_tex_tv, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  connect(m_tex_su, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  connect(m_tex_sv, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
  connect(m_tex_r, SIGNAL(valueChanged(double)), this, SLOT(changeTexMat()));
}

void
MeshWindow::resetViewMat()
{
  m_glWidget->resetViewMatrix();
}

void
MeshWindow::resetTexMat()
{
  Eigen::Matrix3f texMat = Eigen::Matrix3f::Identity();

  m_glWidget->setTextureMatrix(texMat);

  resetTexWidgets();
}

void
MeshWindow::applyTexMat()
{
  m_glWidget->applyTextureMatrix();

  resetTexWidgets();
}

void
MeshWindow::applyViewMat()
{
  m_glWidget->applyViewMatrix();
}

void
MeshWindow::saveImage(const QImage &img)
{
  qApp->processEvents(
    QEventLoop::ExcludeUserInputEvents); //qApp->processEvents();

  if (!img.isNull()) {
    QString filename = QFileDialog::getSaveFileName(
      nullptr,
      tr("Save File"),
      QString(),
      tr("Images (*.png *.jpg)")); //TODO: get supported images formats !!!
    if (!filename.isEmpty()) {
      const bool saveOk = img.save(filename);
      if (!saveOk) {
        QMessageBox::warning(
          nullptr, tr("Error"), tr("Image %1 was not saved").arg(filename));
      }
    }
  } else {
    QMessageBox::warning(nullptr, tr("Error"), tr("Unable to capture image"));
  }
}

void
MeshWindow::takeScreenshot()
{
  const QImage img = m_glWidget->takeScreenshot();

  saveImage(img);
}

void
MeshWindow::takeScreenshotHiRes()
{
  const QImage img = m_glWidget->takeScreenshotHiRes();

  saveImage(img);
}

void
MeshWindow::useTexture()
{
  m_glWidget->setUseTexture(m_useTexture->checkState() == Qt::Checked);
}

void
MeshWindow::loadMesh()
{
  QString fileName = QFileDialog::getOpenFileName(
    this, tr("Open Mesh File"), m_meshFilename, tr("Meshes (*.obj *.brs)"));
  if (!fileName.isEmpty()) {
    loadMeshFile(fileName);
  }
}

void
MeshWindow::saveMesh()
{
  QString fileName = QFileDialog::getSaveFileName(
    this, tr("Save File"), QString(), tr("Meshes (*.obj *.brs)"));
  if (!fileName.isEmpty()) {
    saveMeshFile(fileName);
  }
}

void
MeshWindow::loadTexture()
{
  QString fileName = QFileDialog::getOpenFileName(
    this, tr("Open Image File"), QString(), getReadImageFilter());
  if (!fileName.isEmpty()) {
    loadImageFile(fileName);
  }
}

void
MeshWindow::computeTexCoords()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  qApp->processEvents(
    QEventLoop::ExcludeUserInputEvents); //qApp->processEvents();

  m_glWidget->computeTextureCoords();

  QApplication::restoreOverrideCursor();
}

void
MeshWindow::saveMeshFile(const QString &meshFilename)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  qApp->processEvents(
    QEventLoop::ExcludeUserInputEvents); //qApp->processEvents();

  const bool saveOk = m_glWidget->saveMesh(meshFilename);

  QApplication::restoreOverrideCursor();

  if (!saveOk) {
    QMessageBox::warning(
      nullptr, tr("Error"), tr("Mesh %1 was not saved").arg(meshFilename));
  }
}

void
MeshWindow::loadMeshFile(const QString &meshFilename)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  qApp->processEvents(
    QEventLoop::ExcludeUserInputEvents); //qApp->processEvents();

  const bool loadOk = m_glWidget->loadMesh(meshFilename);

  QApplication::restoreOverrideCursor();

  if (!loadOk) {
    QMessageBox::warning(
      nullptr, tr("Error"), tr("Mesh %1 was not loaded").arg(meshFilename));
  } else {
    //TODO? we should also add application name ?
    const QString title = QFileInfo(meshFilename).fileName() + "[*]";
    setWindowTitle(title);

    m_meshFilename = meshFilename;
  }
}

void
MeshWindow::computeTextureCoords(const QString &outputMeshFilename)
{
  computeTexCoords();

  QApplication::setOverrideCursor(Qt::WaitCursor);
  qApp->processEvents(
    QEventLoop::ExcludeUserInputEvents); //qApp->processEvents();

  m_glWidget->saveMesh(outputMeshFilename);

  QApplication::restoreOverrideCursor();
}

void
MeshWindow::setImage(const QImage &img)
{
  if (!img.isNull()) {
    m_glWidget->setTexture(img);
  } else {
    QMessageBox::warning(nullptr, tr("Error"), tr("Invalid image"));
  }
}

void
MeshWindow::loadImageFile(const QString &imageFilename)
{
  QImage img(imageFilename);
  if (!img.isNull()) {
    m_glWidget->setTexture(img);
  } else {
    QMessageBox::warning(
      nullptr, tr("Error"), tr("Image %1 was not loaded").arg(imageFilename));
  }
}

void
MeshWindow::hasTexCoords(bool onoff)
{
  m_hasTexCoords = onoff;

  updateTexture();
}

void
MeshWindow::hasImage(bool onoff)
{
  m_hasImage = onoff;

  updateTexture();
}

void
MeshWindow::updateTexture()
{
  if (m_hasTexCoords && m_hasImage) {
    m_useTexture->setCheckState(Qt::Checked);
  } else {
    m_useTexture->setCheckState(Qt::Unchecked);
  }
}

void
MeshWindow::useBackground()
{
  m_glWidget->setUseBackgroundTexture(m_useBackground->checkState() ==
                                      Qt::Checked);
}

void
MeshWindow::loadBackgroundTexture()
{
  QString fileName = QFileDialog::getOpenFileName(
    this, tr("Open File"), QString(), getReadImageFilter());
  if (!fileName.isEmpty()) {
    loadBackgroundTextureFile(fileName);
  }
}

void
MeshWindow::loadBackgroundTextureFile(const QString &imageFilename)
{
  QImage img(imageFilename);
  if (!img.isNull()) {
    m_glWidget->setBackgroundTexture(img);
  } else {
    QMessageBox::warning(
      nullptr, tr("Error"), tr("Image %1 was not loaded").arg(imageFilename));
  }
}

void
MeshWindow::changeBackgroundTexMat()
{
  //Get texture matrix
  const float tu = m_backgroundTex_tu->value();
  const float tv = m_backgroundTex_tv->value();
  const float su = m_backgroundTex_su->value();
  const float sv = m_backgroundTex_sv->value();

  Eigen::Matrix3f texMat = Eigen::Matrix3f::Identity();
  texMat(0, 0) = su;
  texMat(0, 1) = 0;
  texMat(0, 2) = tu;
  texMat(1, 0) = 0;
  texMat(1, 1) = sv;
  texMat(1, 2) = tv;

  //std::cerr<<"su="<<su<<" sv="<<sv<<" tu="<<tu<<" tv="<<tv<<"\n";
  //std::cerr<<"texMat:\n"<<texMat<<"\n";

  m_glWidget->setBackgroundTextureMatrix(texMat);
}

void
MeshWindow::saveRandomImages()
{
  RandomScreenshotsParamsDialog dialog;
  if (dialog.exec()) {

    QString dir = dialog.getDstDirectory();
    QString filePrefix = dialog.getDstPrefix();
    QString extension = "." + dialog.getDstExtension();
    size_t numScreenshots = dialog.getNumScreenshots();

    QString prefix = dir + "/" + filePrefix; //TODO: NON PORTABLE !!!

    float minPhy = dialog.getMinPhy();
    float maxPhy = dialog.getMaxPhy();
    float minTheta = dialog.getMinTheta();
    float maxTheta = dialog.getMaxTheta();

    m_glWidget->takeRandomScreenshots(
      prefix, extension, numScreenshots, minPhy, maxPhy, minTheta, maxTheta);
  }
}
