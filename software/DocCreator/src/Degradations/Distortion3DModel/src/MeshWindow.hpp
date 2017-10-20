#ifndef WINDOW_H
#define WINDOW_H

#include "framework_global.h"
#include <QWidget>

class GLWidget;
class QDoubleSpinBox;
class QPushButton;
class QCheckBox;

class FRAMEWORK_EXPORT MeshWindow : public QWidget
{
  Q_OBJECT

public:
  explicit MeshWindow(QWidget *parent = nullptr);

  void loadMeshFile(const QString &meshFilename);
  void saveMeshFile(const QString &meshFilename);

  void computeTextureCoords(const QString &outputMeshFilename);

  void loadImageFile(const QString &imageFilename);

  void setImage(const QImage &img);

  void loadBackgroundTextureFile(const QString &imageFilename);

protected slots:

  void changeLightPos();
  void changeAmbientDiffuseSpecular();
  void changeSpecularExponent();
  void useTexture();
  void takeScreenshot();
  void takeScreenshotHiRes();

  void loadMesh();
  void saveMesh();
  void loadTexture();
  void computeTexCoords();

  void hasTexCoords(bool onoff);
  void hasImage(bool onoff);

  void changeTexMat();
  void resetTexMat();
  void applyTexMat();
  void resetViewMat();
  void applyViewMat();

  void useBackground();
  void loadBackgroundTexture();
  void changeBackgroundTexMat();

  void saveRandomImages();

protected:
  void updateTexture();

  void resetTexWidgets();

  void saveImage(const QImage &img);

private:
  GLWidget *m_glWidget;

  QDoubleSpinBox *m_lightPosX;
  QDoubleSpinBox *m_lightPosY;
  QDoubleSpinBox *m_lightPosZ;
  QDoubleSpinBox *m_lightAmbient;
  QDoubleSpinBox *m_lightDiffuse;
  QDoubleSpinBox *m_lightSpecular;
  QDoubleSpinBox *m_lightSpecularExponent;
  QCheckBox *m_useTexture;
  QPushButton *m_takeScreenshot;
  QPushButton *m_takeScreenshotHiRes;
  QPushButton *m_loadMesh;
  QPushButton *m_saveMesh;
  QPushButton *m_loadImg;
  QPushButton *m_computeTexCoords;
  QPushButton *m_resetTexMat;
  QPushButton *m_applyTexMat;
  QPushButton *m_resetViewMat;
  QPushButton *m_applyViewMat;

  QDoubleSpinBox *m_tex_tu;
  QDoubleSpinBox *m_tex_tv;
  QDoubleSpinBox *m_tex_su;
  QDoubleSpinBox *m_tex_sv;
  QDoubleSpinBox *m_tex_r;

  QCheckBox *m_useBackground;
  QPushButton *m_loadBackgroundTexture;
  QDoubleSpinBox *m_backgroundTex_tu;
  QDoubleSpinBox *m_backgroundTex_tv;
  QDoubleSpinBox *m_backgroundTex_su;
  QDoubleSpinBox *m_backgroundTex_sv;

  QPushButton *m_saveRandomImages;

  QString m_meshFilename;

  bool m_hasTexCoords;
  bool m_hasImage;
};

#endif
