#define NOMINMAX //for Visual

#include "Assistant.hpp"

#include <algorithm>
#include <cassert>
#include <ctime>
#include <iostream>
#include <random>

#include <QColorDialog>
#include <QDebug>
#include <QFileDialog>
#include <QGuiApplication>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTextStream>

#include "Degradations/BleedThroughQ.hpp"
#include "Degradations/BlurFilterQ.hpp"
#include "Degradations/Distortion3DModel/src/GLWidget.hpp"
#include "Degradations/ElasticDeformationQ.hpp"
#include "Degradations/GradientDomainDegradationQ.hpp"
#include "Degradations/GrayCharacterDegradationModelQ.hpp"
#include "Degradations/HoleDegradationQ.hpp"
#include "Degradations/PhantomCharacterQ.hpp"
#include "Degradations/NoiseDegradationQ.hpp"
#include "Degradations/RotationDegradationQ.hpp"
#include "Degradations/ShadowBindingQ.hpp"
#include "Document/DocumentController.hpp"
#include "Document/DocumentToXMLExporter.hpp"
#include "RandomDocument/RandomDocumentCreator.hpp"
#include "RandomDocument/RandomDocumentExporter.hpp"
#include "RandomDocument/RandomDocumentParameters.hpp"
#include "Utils/ImageUtils.hpp"
#include "appconstants.h"
#include "context/fontcontext.h"
#include "core/configurationmanager.h"
#include "ui_Assistant.h"

//B:TODO:UGLY: REMOVE CODE DUPLICATION everywhere !!!!!!!!!!!!!!!!!!

static const int IMG_WIDTH = 240;
static const int IMG_HEIGHT = 360;

static const int BLEED_PREVIEW_WIDTH = 209;
static const int BLEED_PREVIEW_HEIGHT = 319;
static const int BLEED_X = 150;
static const int BLEED_Y = 150;

static const int CHARDEG_PREVIEW_WIDTH = 209;
static const int CHARDEG_PREVIEW_HEIGHT = 319;

static const int SHADOW_PREVIEW_WIDTH = 209;
static const int SHADOW_PREVIEW_HEIGHT = 319;

static const int PHANTOM_PREVIEW_WIDTH = 209;
static const int PHANTOM_PREVIEW_HEIGHT = 319;

static const int BLUR_PREVIEW_WIDTH = 209;
static const int BLUR_PREVIEW_HEIGHT = 319;

static const int HOLE_PREVIEW_WIDTH = 209;
static const int HOLE_PREVIEW_HEIGHT = 319;

//CODE DUPLICATION with RandomDocumentCreator
static void
saveFonts(QString &prevCurrFontName, QList<Models::Font *> &prevFonts)
{
  prevCurrFontName = Context::FontContext::instance()->getCurrentFontName();
  QStringList prevFontList = Context::FontContext::instance()->getFontNames();
  for (const QString &fontName : prevFontList) {
    prevFonts.push_back(Context::FontContext::instance()->getFont(fontName));
    Context::FontContext::instance()->removeFont(fontName);
  }
}

static void
restoreFonts(const QString &prevCurrFontName,
             const QList<Models::Font *> &prevFonts)
{
  for (Models::Font *f : prevFonts) {
    Context::FontContext::instance()->addFont(f);
  }
  if (!prevCurrFontName.isEmpty()) {
    Context::FontContext::instance()->setCurrentFont(prevCurrFontName);
  }
}

static void
saveBackgrounds(QString &prevCurrBackgroundName,
                QList<QString> &prevBackgrounds)
{
  prevCurrBackgroundName =
    Context::BackgroundContext::instance()->getCurrentBackground();
  prevBackgrounds = Context::BackgroundContext::instance()->getBackgrounds();
  Context::BackgroundContext::instance()->clear();
}

static void
restoreBackgrounds(const QString &prevCurrBackgroundName,
                   const QList<QString> &prevBackgrounds)
{
  for (const QString &b : prevBackgrounds) {
    Context::BackgroundContext::instance()->addBackground(b);
  }
  if (!prevCurrBackgroundName.isEmpty()) {
    Context::BackgroundContext::instance()->setCurrentBackground(
      prevCurrBackgroundName);
  }
}

Assistant::Assistant(DocumentController *doc, QWidget *parent)
  : QWizard(parent)
  , ui(new Ui::Assistant)
  , _progressDialog(nullptr)
  , _numGeneratedImages(0)
{
  ui->setupUi(this);

  const QImage semi(QStringLiteral(":/images/semi.png"));
  const QImage synth(QStringLiteral(":/images/synth.png"));
  ui->ImageSemi->setPixmap(QPixmap::fromImage(semi));
  ui->ImageSynth->setPixmap(QPixmap::fromImage(synth));

  _DocController = doc;

  BleedThrough_setupGUIImages();
  CharDeg_setupGUIImages();
  Noise_setupGUIImages();
  Rotation_setupGUIImages();
  Shadow_setupGUIImages();
  Phantom_setupGUIImages();
  GDD_setupGUIImages();
  Blur_setupGUIImages();
  Hole_setupGUIImages();
  ElasticDeformation_setupGUIImages();
  Dist3D_setupGUIImages();

  _BleedThrough_bleedEnable = false;
  _CharDeg_charEnable = false;
  _Noise_noiseEnable = false;
  _Rotation_rotationEnable = false;
  _Shadow_shadEnable = false;
  _Phantom_phantEnable = false;
  _GDD_gddEnable = false;
  _Blur_blurEnable = false;
  _Blur_PageEnable = false;
  _Blur_ZoneEnable = false;
  _Hole_holeEnable = false;
  _ElasticDeformation_elasticEnable = false;
  _Dist3D_dist3DEnable = false;

  //_BleedThrough_nbTirageBleed = 0;
  //_CharDeg_nbTirageChar = 0;
  _Phantom_nbPhantSelected = 0;
  _Shadow_nbShadSelected = 0;
  _Blur_nbBlurSelected = 0;
  _Hole_nbHoleSelected = 0;

  _semi = false;

  QColor Hole_defaultBackgroundColor(0, 0, 0, 255);
  Hole_setBackgroundColor(Hole_defaultBackgroundColor);

  saveFonts(_originalCurrentFont, _originalFonts);

  const QString fontPath = QDir(Core::ConfigurationManager::get(
                                  AppConfigMainGroup, AppConfigFontFolderKey)
                                  .toString())
                             .absolutePath();
  updateListFont(fontPath);

  saveBackgrounds(_originalCurrentBackgroundName, _originalBackgrounds);

  const QString backgroundPath =
    QDir(Core::ConfigurationManager::get(AppConfigMainGroup,
                                         AppConfigBackgdFolderKey)
           .toString())
      .absolutePath();
  updateListBackground(backgroundPath);

  //QObject::connect(ui->FolderText, SIGNAL(clicked()), this, SLOT(RadioButtons()));
  //QObject::connect(ui->LoremIpsum, SIGNAL(clicked()), this, SLOT(RadioButtons()));

  QObject::connect(
    ui->btnChooseTxtDir, SIGNAL(clicked()), this, SLOT(chooseTextDirectory()));
  QObject::connect(ui->listTextView,
                   SIGNAL(clicked(QModelIndex)),
                   this,
                   SLOT(textSelectionChanges()));
  QObject::connect(
    ui->btnSelectTxt, SIGNAL(clicked()), this, SLOT(textSelectAll()));
  QObject::connect(
    ui->btnDeselectTxt, SIGNAL(clicked()), this, SLOT(textDeselectAll()));

  //QObject::connect(ui->pageNumber, SIGNAL(valueChanged(int)), this, SIGNAL(completeChanged()));

  QObject::connect(
    ui->btnChooseFontDir, SIGNAL(clicked()), this, SLOT(chooseFontDirectory()));
  QObject::connect(ui->listFontView,
                   SIGNAL(clicked(QModelIndex)),
                   this,
                   SLOT(fontSelectionChanges()));
  QObject::connect(
    ui->btnSelectFont, SIGNAL(clicked()), this, SLOT(fontSelectAll()));
  QObject::connect(
    ui->btnDeselectFont, SIGNAL(clicked()), this, SLOT(fontDeselectAll()));

  QObject::connect(ui->btnChooseBackgroundDir,
                   SIGNAL(clicked()),
                   this,
                   SLOT(chooseBackgroundDirectory()));
  QObject::connect(ui->listBackgroundView,
                   SIGNAL(clicked(QModelIndex)),
                   this,
                   SLOT(backgroundSelectionChanges()));
  QObject::connect(ui->btnSelectBckgd, SIGNAL(clicked()),
		   this, SLOT(bckgdSelectAll()));
  QObject::connect(ui->btnDeselectBckgd, SIGNAL(clicked()),
		   this, SLOT(bckgdDeselectAll()));


  QObject::connect(ui->btnChooseBackgroundDirForms,
                   SIGNAL(clicked()),
                   this,
                   SLOT(chooseBackgroundDirectoryForms()));
  QObject::connect(ui->listBackgroundViewForms,
                   SIGNAL(clicked(QModelIndex)),
                   this,
                   SLOT(backgroundSelectionChangesForms()));
  QObject::connect(ui->btnSelectBckgdForms, SIGNAL(clicked()),
		   this, SLOT(bckgdSelectAllForms()));
  QObject::connect(ui->btnDeselectBckgdForms, SIGNAL(clicked()),
		   this, SLOT(bckgdDeselectAllForms()));


  //QObject::connect(ui->LoremIpsum, SIGNAL(clicked()),this, SLOT(changeLoremIpsum()));
  //QObject::connect(ui->FolderText, SIGNAL(clicked()), this, SLOT(changeLoremIpsum()));

  PageParams_connect();

  PageParamsForms_connect();

  QObject::connect(ui->FontBackgroundRandomRB,
                   SIGNAL(toggled(bool)),
                   this,
                   SLOT(updateTxtGenerationInfo()));
  QObject::connect(ui->FontBackgroundAllRB,
                   SIGNAL(toggled(bool)),
                   this,
                   SLOT(updateTxtGenerationInfo()));

  QObject::connect(ui->btnChooseOutputTxtDir,
                   SIGNAL(clicked()),
                   this,
                   SLOT(chooseOutputTxtImageDir()));
  //QObject::connect(ui->outputTxtImageDirPath, SIGNAL(editingFinished()), this, SIGNAL(completeChanged()));

  QObject::connect(ui->btnChooseInputImageDir,
                   SIGNAL(clicked()),
                   this,
                   SLOT(chooseInputImageDir()));
  //QObject::connect(ui->inputImagePath, SIGNAL(editingFinished()), this, SIGNAL(completeChanged()));
  QObject::connect(
    ui->btnChooseGTDir, SIGNAL(clicked()), this, SLOT(chooseGTDirectory()));

  //QObject::connect(ui->btnSaveFolderDeg, SIGNAL(clicked()), this, SLOT(chosePicDegDirectory()));

  QObject::connect(
    ui->CheckGT, SIGNAL(stateChanged(int)), this, SLOT(GTChecked()));

  //QObject::connect(this, SIGNAL(generationReady()), this, SLOT(generateTxtImages()));

  QObject::connect(ui->BleedMin,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(BleedThrough_changeMinBleed()));
  QObject::connect(ui->BleedMax,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(BleedThrough_changeMaxBleed()));
  QObject::connect(ui->CheckBleed,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(BleedThrough_EnableBleedOption()));
  QObject::connect(ui->BleedMin,
                   SIGNAL(sliderReleased()),
                   this,
                   SLOT(BleedThrough_nbIterationsMinChanged()));
  QObject::connect(ui->BleedMax,
                   SIGNAL(sliderReleased()),
                   this,
                   SLOT(BleedThrough_nbIterationsMaxChanged()));
  QObject::connect(
    ui->SwitchBleed, SIGNAL(clicked()), this, SLOT(BleedThrough_LoadPrevImg()));
  QObject::connect(ui->TirageBleed,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(BleedThrough_tirageBleedChanged(int)));

  QObject::connect(ui->CheckChar,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(CharDeg_EnableCharOption()));
  QObject::connect(ui->CharMin,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(CharDeg_changeMinChar()));
  QObject::connect(ui->CharMax,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(CharDeg_changeMaxChar()));
  QObject::connect(ui->CharMin,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(CharDeg_nbIterationsMinChangedChar()));
  QObject::connect(ui->CharMax,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(CharDeg_nbIterationsMaxChangedChar()));
  QObject::connect(
    ui->SwitchChar, SIGNAL(clicked()), this, SLOT(CharDeg_LoadPrevImgChar()));
  QObject::connect(ui->TirageChar,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(CharDeg_tirageCharChanged(int)));


  QObject::connect(ui->CheckNoise,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Noise_EnableNoiseOption()));
  QObject::connect(ui->GaussianNoiseCB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Noise_MethodsChanged()));
  QObject::connect(ui->SpeckleNoiseCB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Noise_MethodsChanged()));
  QObject::connect(ui->SaltAndPepperNoiseCB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Noise_MethodsChanged()));
  QObject::connect(ui->MinAverageGaussianNoiseSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Noise_changeMinAverageGaussian(double)));
  QObject::connect(ui->MaxAverageGaussianNoiseSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Noise_changeMaxAverageGaussian(double)));
  QObject::connect(ui->MinStdDevGaussianNoiseSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Noise_changeMinStdDevGaussian(double)));
  QObject::connect(ui->MaxStdDevGaussianNoiseSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Noise_changeMaxStdDevGaussian(double)));
  QObject::connect(ui->MinAverageSpeckleNoiseSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Noise_changeMinAverageSpeckle(double)));
  QObject::connect(ui->MaxAverageSpeckleNoiseSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Noise_changeMaxAverageSpeckle(double)));
  QObject::connect(ui->MinStdDevSpeckleNoiseSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Noise_changeMinStdDevSpeckle(double)));
  QObject::connect(ui->MaxStdDevSpeckleNoiseSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Noise_changeMaxStdDevSpeckle(double)));
  QObject::connect(ui->MinAmountSaltAndPepperNoiseSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Noise_changeMinAmountSaltAndPepper(double)));
  QObject::connect(ui->MaxAmountSaltAndPepperNoiseSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Noise_changeMaxAmountSaltAndPepper(double)));
  QObject::connect(ui->MinRatioSaltAndPepperNoiseSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Noise_changeMinRatioSaltAndPepper(double)));
  QObject::connect(ui->MaxRatioSaltAndPepperNoiseSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Noise_changeMaxRatioSaltAndPepper(double)));




  QObject::connect(ui->CheckRotation,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Rotation_EnableRotationOption()));
  QObject::connect(ui->RotationFillColorCB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Rotation_FillMethodChanged()));
  QObject::connect(ui->RotationFillImageCB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Rotation_FillMethodChanged()));
  QObject::connect(ui->RotationFillBorderCB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Rotation_FillMethodChanged()));
  QObject::connect(ui->RotationBorderReflectCB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Rotation_FillMethodChanged()));
  QObject::connect(ui->RotationBorderReplicateCB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Rotation_FillMethodChanged()));
  QObject::connect(ui->RotationBorderWrapCB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Rotation_FillMethodChanged()));
  QObject::connect(ui->RotationBorderReflect101CB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Rotation_FillMethodChanged()));
  QObject::connect(ui->RotationFillInpaintCB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Rotation_FillMethodChanged()));
  QObject::connect(ui->RotationInpaint1CB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Rotation_FillMethodChanged()));
  QObject::connect(ui->RotationInpaint2CB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Rotation_FillMethodChanged()));
  QObject::connect(ui->RotationInpaint3CB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Rotation_FillMethodChanged()));
  QObject::connect(ui->RotationAngleMinSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Rotation_changeMinAngle(double)));
  QObject::connect(ui->RotationAngleMaxSB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(Rotation_changeMaxAngle(double)));
  QObject::connect(ui->RotationImageRepeatsMinSB,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Rotation_changeMinRepeats(int)));
  QObject::connect(ui->RotationImageRepeatsMaxSB,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Rotation_changeMaxRepeats(int)));
  QObject::connect(ui->TirageRotation,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Rotation_tirageRotationChanged(int)));

  
  QObject::connect(ui->CheckShad,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Shadow_EnableShadOption()));
  /*
  QObject::connect(ui->RightShad, SIGNAL(clicked()), this, SLOT(Shadow_OptionChecked()));
  QObject::connect(ui->TopShad, SIGNAL(clicked()), this, SLOT(Shadow_OptionChecked()));
  QObject::connect(ui->LeftShad, SIGNAL(clicked()), this, SLOT(Shadow_OptionChecked()));
  QObject::connect(ui->BottomShad, SIGNAL(clicked()), this, SLOT(Shadow_OptionChecked()));
  QObject::connect(ui->RightShad, SIGNAL(clicked()), this, SLOT(Shadow_RightSelected()));
  QObject::connect(ui->LeftShad, SIGNAL(clicked()), this, SLOT(Shadow_LeftSelected()));
  QObject::connect(ui->TopShad, SIGNAL(clicked()), this, SLOT(Shadow_TopSelected()));
  QObject::connect(ui->BottomShad, SIGNAL(clicked()), this, SLOT(Shadow_BottomSelected()));
  */
  QObject::connect(
    ui->RightShad, SIGNAL(clicked()), this, SLOT(Shadow_selectionChanged()));
  QObject::connect(
    ui->TopShad, SIGNAL(clicked()), this, SLOT(Shadow_selectionChanged()));
  QObject::connect(
    ui->LeftShad, SIGNAL(clicked()), this, SLOT(Shadow_selectionChanged()));
  QObject::connect(
    ui->BottomShad, SIGNAL(clicked()), this, SLOT(Shadow_selectionChanged()));
  QObject::connect(
    ui->SwitchShad, SIGNAL(clicked()), this, SLOT(Shadow_LoadPrevImgShad()));
  QObject::connect(ui->shad_width,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Shadow_updatePreview()));
  QObject::connect(ui->shad_intensity,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Shadow_updatePreview()));
  QObject::connect(ui->shad_angle,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Shadow_updatePreview()));
  QObject::connect(ui->TirageShadow,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Shadow_tirageShadowChanged(int)));

  
  QObject::connect(ui->CheckPhant,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Phantom_EnablePhantOption()));

  QObject::connect(ui->Phantom_Rare,
                   SIGNAL(clicked()),
                   this,
                   SLOT(Phantom_OptionCheckedPhant()));
  QObject::connect(ui->Phantom_Frequent,
                   SIGNAL(clicked()),
                   this,
                   SLOT(Phantom_OptionCheckedPhant()));
  QObject::connect(ui->Phantom_VeryFrequent,
                   SIGNAL(clicked()),
                   this,
                   SLOT(Phantom_OptionCheckedPhant()));
  QObject::connect(
    ui->SwitchPhant, SIGNAL(clicked()), this, SLOT(Phantom_LoadPrevImgPhant()));
  QObject::connect(ui->TiragePhantom,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Phantom_tiragePhantomChanged(int)));

  QObject::connect(
    ui->CheckGDD, SIGNAL(stateChanged(int)), this, SLOT(GDD_EnableGDDOption()));
  QObject::connect(ui->GDD_btnStainImagesFolder,
                   SIGNAL(clicked()),
                   this,
                   SLOT(GDD_chooseStainImagesDirectory()));
  QObject::connect(ui->TirageGDD,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(GDD_tirageGDDChanged(int)));
  QObject::connect(ui->GDD_numStainsMin,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(GDD_changeMinNbStains(int)));
  QObject::connect(ui->GDD_numStainsMax,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(GDD_changeMaxNbStains(int)));

  //-- Blur

  QObject::connect(
    ui->CheckPattern1, SIGNAL(clicked()), this, SLOT(Blur_OptionCheckedBlur()));
  QObject::connect(
    ui->CheckPattern1, SIGNAL(clicked()), this, SLOT(Blur_CheckedPattern1()));
  QObject::connect(
    ui->CheckPattern2, SIGNAL(clicked()), this, SLOT(Blur_OptionCheckedBlur()));
  QObject::connect(
    ui->CheckPattern2, SIGNAL(clicked()), this, SLOT(Blur_CheckedPattern2()));
  QObject::connect(
    ui->CheckPattern3, SIGNAL(clicked()), this, SLOT(Blur_OptionCheckedBlur()));
  QObject::connect(
    ui->CheckPattern3, SIGNAL(clicked()), this, SLOT(Blur_CheckedPattern3()));
  QObject::connect(
    ui->CheckPattern4, SIGNAL(clicked()), this, SLOT(Blur_OptionCheckedBlur()));
  QObject::connect(
    ui->CheckPattern4, SIGNAL(clicked()), this, SLOT(Blur_CheckedPattern4()));
  QObject::connect(
    ui->CheckPattern5, SIGNAL(clicked()), this, SLOT(Blur_OptionCheckedBlur()));
  QObject::connect(
    ui->CheckPattern5, SIGNAL(clicked()), this, SLOT(Blur_CheckedPattern5()));
  QObject::connect(
    ui->CheckPattern6, SIGNAL(clicked()), this, SLOT(Blur_OptionCheckedBlur()));
  QObject::connect(
    ui->CheckPattern6, SIGNAL(clicked()), this, SLOT(Blur_CheckedPattern6()));
  QObject::connect(ui->TirageBlur,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Blur_tirageBlurChanged(int)));
  QObject::connect(ui->CheckBlur,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Blur_EnableBlurOption()));
  QObject::connect(ui->BlurMin,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Blur_changeMinBlur(int)));
  QObject::connect(ui->BlurMax,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Blur_changeMaxBlur(int)));
  QObject::connect(
    ui->BlurPage, SIGNAL(stateChanged(int)), this, SLOT(Blur_PageChanged()));
  QObject::connect(
    ui->BlurZone, SIGNAL(stateChanged(int)), this, SLOT(Blur_ByZoneChanged()));
  QObject::connect(
    ui->SwitchBlur, SIGNAL(clicked()), this, SLOT(Blur_LoadPrevImgBlur()));

  //- Hole

  QObject::connect(ui->CheckHole,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Hole_EnableHoleOption()));
  QObject::connect(ui->HoleCenterMinNb,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Hole_changeMinNbCenterHole(int)));
  QObject::connect(ui->HoleCenterMaxNb,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Hole_changeMaxNbCenterHole(int)));
  QObject::connect(ui->HoleCornerMinNb,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Hole_changeMinNbCornerHole(int)));
  QObject::connect(ui->HoleCornerMaxNb,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Hole_changeMaxNbCornerHole(int)));
  QObject::connect(ui->HoleBorderMinNb,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Hole_changeMinNbBorderHole(int)));
  QObject::connect(ui->HoleBorderMaxNb,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Hole_changeMaxNbBorderHole(int)));

  QObject::connect(ui->SmallCenter,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Hole_updatePreview()));
  QObject::connect(ui->MediumCenter,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Hole_updatePreview()));
  QObject::connect(
    ui->BigCenter, SIGNAL(stateChanged(int)), this, SLOT(Hole_updatePreview()));
  QObject::connect(ui->SmallCorner,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Hole_updatePreview()));
  QObject::connect(ui->MediumCorner,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Hole_updatePreview()));
  QObject::connect(
    ui->BigCorner, SIGNAL(stateChanged(int)), this, SLOT(Hole_updatePreview()));
  QObject::connect(ui->SmallBorder,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Hole_updatePreview()));
  QObject::connect(ui->MediumBorder,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Hole_updatePreview()));
  QObject::connect(
    ui->BigBorder, SIGNAL(stateChanged(int)), this, SLOT(Hole_updatePreview()));

  QObject::connect(
    ui->HoleColorButton, SIGNAL(clicked()), this, SLOT(Hole_chooseColor()));
  QObject::connect(
    ui->SwitchHole, SIGNAL(clicked()), this, SLOT(Hole_LoadPrevImgHole()));
  QObject::connect(ui->TirageHole,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Hole_tirageHoleChanged(int)));


  //- Elastic Deformation
  QObject::connect(ui->CheckElastic,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(ElasticDeformation_EnableElasticOption()));
  QObject::connect(ui->ElasticDeformation1CB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(ElasticDeformation_MethodsChanged()));
  QObject::connect(ui->ElasticDeformation2CB,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(ElasticDeformation_MethodsChanged()));
  QObject::connect(ui->MinAlphaElasticDeformation1SB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(ElasticDeformation_changeMinAlphaTransform1(double)));
  QObject::connect(ui->MaxAlphaElasticDeformation1SB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(ElasticDeformation_changeMaxAlphaTransform1(double)));
  QObject::connect(ui->MinSigmaElasticDeformation1SB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(ElasticDeformation_changeMinSigmaTransform1(double)));
  QObject::connect(ui->MaxSigmaElasticDeformation1SB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(ElasticDeformation_changeMaxSigmaTransform1(double)));
  QObject::connect(ui->MinAlphaElasticDeformation2SB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(ElasticDeformation_changeMinAlphaTransform2(double)));
  QObject::connect(ui->MaxAlphaElasticDeformation2SB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(ElasticDeformation_changeMaxAlphaTransform2(double)));
  QObject::connect(ui->MinSigmaElasticDeformation2SB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(ElasticDeformation_changeMinSigmaTransform2(double)));
  QObject::connect(ui->MaxSigmaElasticDeformation2SB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(ElasticDeformation_changeMaxSigmaTransform2(double)));
  QObject::connect(ui->MinAlphaAffineElasticDeformation2SB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(ElasticDeformation_changeMinAlphaAffineTransform2(double)));
  QObject::connect(ui->MaxAlphaAffineElasticDeformation2SB,
                   SIGNAL(valueChanged(double)),
                   this,
                   SLOT(ElasticDeformation_changeMaxAlphaAffineTransform2(double)));
  

  
  //- Distortion 3D

  QObject::connect(ui->CheckDist3D,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Dist3D_EnableDist3DOption()));
  QObject::connect(ui->Dist3D_btnMeshesFolder,
                   SIGNAL(clicked()),
                   this,
                   SLOT(Dist3D_chooseMeshesDirectory()));
  QObject::connect(ui->Dist3D_useBackgroundCheck,
                   SIGNAL(stateChanged(int)),
                   this,
                   SLOT(Dist3D_changeUseBackground()));
  QObject::connect(ui->Dist3D_btnBackgroundsFolder,
                   SIGNAL(clicked()),
                   this,
                   SLOT(Dist3D_chooseBackgroundsDirectory()));
  QObject::connect(ui->TirageDist3D,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(Dist3D_tirageDist3DChanged(int)));

  QObject::connect(ui->btnChooseOutputDegrImageDir,
                   SIGNAL(clicked()),
                   this,
                   SLOT(chooseOutputDegradedImageDir()));
  //QObject::connect(ui->outputDegrImagePath, SIGNAL(editingFinished()), this, SIGNAL(completeChanged()));

  Blur_LoadPattern();
  Hole_LoadHolePattern();

  //connect(this, SIGNAL(accepted()), this, SLOT(accept()));

  assert(_BleedThrough_bleedMin <= _BleedThrough_bleedMax);
  assert(_CharDeg_charMin <= _CharDeg_charMax);
}

Assistant::~Assistant()
{
  delete ui;

  restoreFonts(_originalCurrentFont, _originalFonts);
  restoreBackgrounds(_originalCurrentBackgroundName, _originalBackgrounds);
}

/**
 * gives a random value in [minV; maxV[
 *
 * @warning call srandom once before this function.
 */
static int
P_bounded_rand(int minV, int maxV)
{
  assert(minV < maxV);
  int r = rand();
  r = (r != RAND_MAX ? r : 0);
  return static_cast<int>(minV +
                          (static_cast<float>(r) / static_cast<float>(RAND_MAX) * (maxV - minV)));
}

std::random_device rd;
std::mt19937 mt(rd());

//return a random value in [rMin; rMax]
static
int
random_in_range(int rMin, int rMax)
{
  std::uniform_int_distribution<int> dist(rMin, rMax);
  return dist(mt);
}

static
float
random_in_range(float rMin, float rMax)
{
  std::uniform_real_distribution<float> dist(rMin,rMax);
  return dist(mt);
}

static
QRgb
random_color()
{
  return qRgb(random_in_range(0, 255),
	      random_in_range(0, 255),
	      random_in_range(0, 255));
}


//Take a random part of the image (trying to avoid borders)
static QImage
takePart(const QImage &img)
{
  int x = 0;
  int w = img.width();
  if (img.width() - BLEED_PREVIEW_WIDTH > 0) {
    if (img.width() > BLEED_PREVIEW_WIDTH + 2 * BLEED_X) {
      x = P_bounded_rand(BLEED_X, img.width() - BLEED_PREVIEW_WIDTH - BLEED_X);
      //to avoid to draw an area too much on the border of the image
    } else {
      x = P_bounded_rand(0, img.width() - BLEED_PREVIEW_WIDTH);
    }
    w = BLEED_PREVIEW_WIDTH;
  }

  int y = 0;
  int h = img.height();
  if (img.height() - BLEED_PREVIEW_HEIGHT > 0) {
    if (img.height() > BLEED_PREVIEW_HEIGHT + 2 * BLEED_Y) {
      y =
        P_bounded_rand(BLEED_Y, img.height() - BLEED_PREVIEW_HEIGHT - BLEED_Y);
      //to avoid to draw an area too much on the border of the image
    } else {
      y = P_bounded_rand(0, img.height() - BLEED_PREVIEW_HEIGHT);
    }
    h = BLEED_PREVIEW_HEIGHT;
  }

  assert(x < img.width());
  assert(y < img.height());
  assert(x + w <= img.width());
  assert(y + h <= img.height());

  return img.copy(QRect(x, y, w, h));
}

/*
  //B:TODO:
  //isComplete() is not a member of QWizard but of QWizardPage !

bool Assistant::isComplete() const
{
  bool result = true;
  switch(currentId()) {
  case Page_TextDirs:
    result = (! _txtList.isEmpty());
    break;
  case Page_TextRandomNb:
    result = (ui->pageNumberLabel->value() != 0);
    break;
  case Page_FontFiles:
    result = (! _fontListChoice.isEmpty());
    break;
  case Page_BackgroundFiles:
    result = (! _backgroundListChoice.isEmpty());
    break;
  case Page_PageParams:
    result = PageParams_isComplete();
    break;
  case Page_FinalText:
    result = (! _outputTxtImageDir.isEmpty());
    break;
  case Page_ImageAndGtDirs:
    result = (! _inputImageList.isEmpty());
    break;
  case Page_NbImageSetting:
    result = (! _outputDegradedImageDir.isEmpty());
    break;
  default:
    break;
  }

  return result;
}
*/

void
Assistant::initializePage(int id)
{
  QWizard::initializePage(id);

  switch (id) {
    case Page_FinalText:
      updateTxtGenerationInfo();
      break;
    default:
      break;
  }
}

/*
  Check if there are images in the given directory @a path
  and ask the user if he wants to proceed anyway.
  return true if there is no image or if the user wants to proceed, false otherwise.
 */
bool
Assistant::askIfProceedDespiteImageInDir(const QString &path) const
{
  QDir directory(path);
  const QStringList nameFilters = getReadImageFilterList();
  const bool hasFiles =
    (directory.entryList(nameFilters, QDir::Files | QDir::Readable).count() >
     0);
  if (hasFiles) {
    QMessageBox msgBox;
    msgBox.setText(tr("The output directory already contains image files."));
    msgBox.setInformativeText(tr("New images may overwrite existing ones.\nDo "
                                 "you want to proceed anyway?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    return (ret == QMessageBox::Yes);
  }
  return true;
}

bool
Assistant::validateCurrentPage()
{
  switch (currentId()) {
    case Page_FinalText: {
      if (_outputTxtImageDir.isEmpty())
        return false;

      if (askIfProceedDespiteImageInDir(_outputTxtImageDir)) {
        generateTxtImages();
        //B: we can not call generateTxtImages in nextId() as it is const (and called several times) !
        //So we do it here...

        return (!_inputImageList.isEmpty());
      }
      //else
      return false;

    } break;
    case Page_FinalDegradations:
      if (_outputDegradedImageDir.isEmpty()) { //B:TODO: do this in isComplete()
        QMessageBox::warning(this,
                             QStringLiteral("DocCreator"),
                             tr("No output directory specified."),
                             QMessageBox::Ok,
                             QMessageBox::Ok);
        return false;
      }
      if (nbOfDegradedImages() == 0) {
        QMessageBox::warning(
          this,
          QStringLiteral("DocCreator"),
          tr("No image will be generated as no degradation has been selected."),
          QMessageBox::Ok,
          QMessageBox::Ok);
        return false;
      }
      if (!askIfProceedDespiteImageInDir(_outputDegradedImageDir)) {
        return false;
      }

      generateDegradedImages(); //B: we add it here instead of in accept, cf REM1.

      updateResults();

      break;
    default:
      break;
  }

  return QWizard::validateCurrentPage();
}

/*
//B:REM0

A lot of checks are done in nextId() or validateCurrentPage().
One problem is that the next button, that should be disabled when these checks are invalid, always appears enabled.
According to Qt documentation, the solution would be to have one individual class inheriting from QWizardPage for each page,
and override isComplete() (and emit completeChange() when necessary).

*/

/*
  B: if the order of pages for degradations is changed in nextId()
  it is probably a good idea to also change the display order on Page_FinalDegradations
  int Assistant.ui file

 */

int
Assistant::nextId() const
{
  switch (currentId()) {
    case Page_SyntheticOrSemiChoice:
      if (ui->Synthetic->isChecked()) {
        return Page_TextDocument;
      }
      else if (ui->Semi->isChecked()) {
        return Page_ImageAndGtDirs;
      }
      return currentId();
      break;

  case Page_TextDocument:
    if (ui->ClassicTextDocument->isChecked()) {
      return Page_TextType;
    }
    else if (ui->FormTextDocument->isChecked()) {
      return Page_TextType;
    }
    return currentId();
    break;

  case Page_TextType:
      if (ui->FolderText->isChecked())
        return Page_TextDirs;
      return Page_TextRandomNb;
      break;

    case Page_TextDirs:
      if (_txtList.isEmpty())
        return currentId();
      return Page_FontFiles;
      break;

    case Page_TextRandomNb:
      if (ui->pageNumber->value() == 0)
        return currentId();
      return Page_FontFiles;
      break;

    case Page_FontFiles:
      if (_fontListChoice.isEmpty())
        return currentId();
      if (ui->ClassicTextDocument->isChecked()) {
	return Page_BackgroundFiles;
      }
      else if (ui->FormTextDocument->isChecked()) {
	return Page_BackgroundFilesForms;
      }
      break;

    case Page_BackgroundFiles:
      if (_backgroundListChoice.isEmpty())
        return currentId();
      return Page_PageParams;
      break;

    case Page_BackgroundFilesForms:
      if (_backgroundListChoice.isEmpty() && _blocks.isEmpty())
        return currentId();
      return Page_PageParamsForms;
      break;

    case Page_PageParams:
      if (!PageParams_isComplete())
        return currentId();
      return Page_FinalText;
      break;

    case Page_PageParamsForms:
      if (!PageParamsForms_isComplete())
        return currentId();
      return Page_FinalText;
      break;

    case Page_FinalText:
      if (_outputTxtImageDir.isEmpty())
        return currentId();
      return Page_ConfirmDegradations;
      break;

    case Page_ConfirmDegradations:
      return Page_CharDeg;
      break;

    case Page_ImageAndGtDirs:
      if (_inputImageList.isEmpty()) //B:TODO: do this in isComplete()
        return currentId();
      return Page_CharDeg;
      break;
    case Page_CharDeg:
      return Page_Phantom;
      break;
    case Page_Phantom:
      return Page_GDD;
      break;
    case Page_GDD:
      return Page_Bleed;
      break;
    case Page_Bleed:
      return Page_Noise;
      break;
    case Page_Noise:
      return Page_Rotation;
      break;
    case Page_Rotation:
      return Page_Blur;
      break;
    case Page_Blur:
      return Page_Shadow;
      break;
    case Page_Shadow:
      return Page_Hole;
      break;
    case Page_Hole:
      return Page_ElasticDeformation;
      break;
    case Page_ElasticDeformation:
      return Page_Dist3D;
      break;
    case Page_Dist3D:
      return Page_FinalDegradations;
      break;
    case Page_FinalDegradations:
      //return -1; //see REM1
      return Page_Results;
      break;
    case Page_Results:
      return -1;
    default:
      return -1;
      break;
  }
  return -1;
}

/*//B:REM1:
 Page_FinalDegradations is not the final page (with Finish button)
 because it seems that on the final page validateCurrentPage() is called after accept()
 (and we want to validate the page before generatingthe images)
*/

void
Assistant::RadioButtons()
{
  if (ui->FolderText->isChecked()) {
    ui->btnChooseTxtDir->setEnabled(true);
    ui->txtFilePath->setEnabled(true);
  } else {
    ui->btnChooseTxtDir->setEnabled(false);
    ui->txtFilePath->setEnabled(false);
  }
}

void
Assistant::chooseTextDirectory()
{
  _semi = false;
  const QString path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Choose text directory"),
                                      QStringLiteral("./"),
                                      QFileDialog::ShowDirsOnly);
  if (!path.isEmpty()) {
    this->updateListText(path);
  }
}

QStringList
getTextFiles(const QString &path)
{
  QStringList nameFilters;
  nameFilters << QStringLiteral("*.txt");

  QDir directory(path);
  QList<QString> fileList =
    directory.entryList(nameFilters, QDir::Files | QDir::Readable);

  return fileList;
}

void
Assistant::updateListText(const QString &path)
{
  QGuiApplication::setOverrideCursor(Qt::BusyCursor);

  ui->txtFilePath->setText(path);

  ui->listTextView->clearSelection();

  _txtDirectory = path;

  QStringList list = getTextFiles(path);

  list.sort();
  //To have files in a predictive order

  _textListModel.setStringList(list);
  ui->listTextView->setModel(&_textListModel);
  ui->listTextView->selectAll();

  textSelectionChanges();

  QGuiApplication::restoreOverrideCursor();
}

void
Assistant::textSelectionChanges()
{
  QModelIndexList listSelectionsText =
    ui->listTextView->selectionModel()->selectedIndexes();
  _txtList.clear();
  _txtList.reserve(listSelectionsText.count());
  for (const auto &t : listSelectionsText) {
    const QString selectedText =
      _textListModel.data(t, Qt::DisplayRole).toString();
    _txtList.append(selectedText);
  }

  ui->label_nbTextFiles->setText(
    tr("%n selected text(s)", "", _txtList.size()));
}

void
Assistant::textSelectAll()
{
  ui->listTextView->selectAll();
  textSelectionChanges();
}

void
Assistant::textDeselectAll()
{
  ui->listTextView->clearSelection();
  textSelectionChanges();
}

void
Assistant::chooseFontDirectory()
{
  const QString path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Choose font directory"),
                                      QStringLiteral("./"),
                                      QFileDialog::ShowDirsOnly);
  if (!path.isEmpty()) {
    this->updateListFont(path);
  }
}

void
Assistant::updateListFont(const QString &fontPath)
{
  ui->txtFilePathFont->setText(fontPath);

  //ui->listFontView->selectAll();
  ui->listFontView->clearSelection();

  _FontDirectory = fontPath;

  //B:
  //Here we modify the available fonts in the application (i.e., global FontContext).
  // Ideally we should not modify the global context, or at least only use the
  // selected fonts.
  //But for now, there is no other way than to load all fonts from fontPath in global Fontcontext
  //   to know which files are valid fonts !???
  QString fontExt =
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigFontExtKey)
      .toString();
  Context::FontContext::instance()->clear();
  Context::FontContext::instance()->initialize(fontPath, fontExt);

  QStringList list = Context::FontContext::instance()->getFontNames();

  if (!list.isEmpty()) {
    list.sort();
    Context::FontContext::instance()->setCurrentFont(list.back());
  }

  _fontListModel.setStringList(list);
  ui->listFontView->setModel(&_fontListModel);
  ui->listFontView->selectAll();

  fontSelectionChanges();
}

void
Assistant::fontSelectionChanges()
{
  QModelIndexList listSelectionsFont =
    ui->listFontView->selectionModel()->selectedIndexes();
  _fontListChoice.clear();
  _fontListChoice.reserve(listSelectionsFont.count());
  for (const auto &f : listSelectionsFont) {
    const QString selectedFont =
      _fontListModel.data(f, Qt::DisplayRole).toString();
    _fontListChoice.append(selectedFont);
  }

  ui->label_nbFonts->setText(
    tr("%n selected font(s)", "", _fontListChoice.size()));

  //emit completeChanged();
}

void
Assistant::fontSelectAll()
{
  ui->listFontView->selectAll();
  fontSelectionChanges();
}

void
Assistant::fontDeselectAll()
{
  ui->listFontView->clearSelection();
  fontSelectionChanges();
}


void
Assistant::chooseBackgroundDirectory()
{
  const QString path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Choose background images directory"),
                                      QStringLiteral("./"),
                                      QFileDialog::ShowDirsOnly);
  if (!path.isEmpty()) {
    this->updateListBackground(path);
  }
}

void
Assistant::updateListBackground(const QString &pathBack)
{
  ui->backgroundPath->setText(pathBack);

  //ui->listBackView->selectAll();
  ui->listBackgroundView->clearSelection();

  //B: here we modify global BackgroundContext
  // (see updateListFont comment)
  // We should populate global BackgroundContext only once selected list of
  // backgrounds is finalized.

  const QString &backgroundPath = pathBack;
  Context::BackgroundContext::instance()->clear();
  Context::BackgroundContext::instance()->initialize(backgroundPath);
  //B:TODO:DESIGN: why do we modify global BackgroundContext ?
  //If this function is called just to populate view (?),

  Context::BackgroundList backgroundList =
    Context::BackgroundContext::instance()->getBackgrounds();
  QStringList listBack(backgroundList);
  listBack.sort();
  _backgroundListModel.setStringList(listBack);
  ui->listBackgroundView->setModel(&_backgroundListModel);
  ui->listBackgroundView->selectAll();

  backgroundSelectionChanges(); //B: useful ?
}

void
Assistant::backgroundSelectionChanges()
{
  QModelIndexList listSelectionBack =
    ui->listBackgroundView->selectionModel()->selectedIndexes();
  _backgroundListChoice.clear();
  _backgroundListChoice.reserve(listSelectionBack.count());
  for (const auto &sb : listSelectionBack) {
    const QString selectedBack =
      _backgroundListModel.data(sb, Qt::DisplayRole).toString();
    _backgroundListChoice.append(selectedBack);
  }

  ui->label_nbBackgrounds->setText(
    tr("%n selected background image(s)", "", _backgroundListChoice.size()));

  //emit completeChanged();
}

void
Assistant::bckgdSelectAll()
{
  ui->listBackgroundView->selectAll();
  backgroundSelectionChanges();
}

void
Assistant::bckgdDeselectAll()
{
  ui->listBackgroundView->clearSelection();
  backgroundSelectionChanges();
}


void
Assistant::chooseBackgroundDirectoryForms()
{
  const QString path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Choose background images directory"),
                                      QStringLiteral("./"),
                                      QFileDialog::ShowDirsOnly);
  if (!path.isEmpty()) {
    this->updateListBackgroundForms(path);
  }
}

void
Assistant::updateListBackgroundForms(const QString &pathBack)
{
  ui->backgroundPathForms->setText(pathBack);

  //ui->listBackViewForms->selectAll();
  ui->listBackgroundViewForms->clearSelection();

  //B: here we modify global BackgroundContext
  // (see updateListFont comment)
  // We should populate global BackgroundContext only once selected list of
  // backgrounds is finalized.

  const QString &backgroundPath = pathBack;
  Context::BackgroundContext::instance()->clear();
  Context::BackgroundContext::instance()->initialize(backgroundPath);
  //B:TODO:DESIGN: why do we modify global BackgroundContext ?
  //If this function is called just to populate view (?),

  Context::BackgroundList backgroundList =
    Context::BackgroundContext::instance()->getBackgrounds();
  QStringList listBack(backgroundList);
  listBack.sort();
  _backgroundListModel.setStringList(listBack);
  ui->listBackgroundViewForms->setModel(&_backgroundListModel);
  ui->listBackgroundViewForms->selectAll();

  backgroundSelectionChangesForms(); //B: useful ?
}


static
QString
changeExtension(const QString &filename, const QString &newExtension)
{
  QString outputFilename;
  const int pos = filename.lastIndexOf('.');
  if (pos != -1) {
    outputFilename = filename;
    outputFilename.replace(pos, filename.length()-pos, newExtension);
  }
  else {
    qDebug()<<"Warning: no extension found for file: "<<filename<<"\n";
  }

  return outputFilename;
}

static
bool readBlocksCSV(const QString &filename,
	     QVector<QRect> &blocks)
{
  blocks.clear();

  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  int v[4];
  QStringList words;
  while (!file.atEnd()) {
    QString line = file.readLine();
    if (! line.startsWith("#") && !line.isEmpty() && line !="\n") {
      words = line.split(',');
      if (words.size() == 1) {
	words = line.split(';');
      }
      if (words.size() != 4) {
	qDebug()<<"ERROR: unable to parse line: "<<line<<" from csv file: "<<filename<<"\n";
	return false;
      }

      for (int i=0; i<4; ++i) {
	bool ok = false;
	v[i] = words[i].toUInt(&ok);
	if (! ok) {
	  qDebug()<<"ERROR: invalid value: "<<words[i]<<" in line: "<<line<<" from csv file: "<<filename<<"\n";
	  return false;
	}
      }
      blocks.push_back(QRect(v[0], v[1], v[2], v[3])); //x, y, w, h
    }
  }

  if (blocks.size() == 0) {
    qDebug()<<"WARNING: no block were read from csv file: "<<filename<<"\n";
  }
  return true;
}

QString
makePath(const QString &dir, const QString &file)
{
  QString res = dir;
  const QChar sep = '/';
  if (!dir.isEmpty() && dir[dir.size() - 1] != sep)
    res += sep;
  res += file;
  return res;
  //TODO: Does it work on Windows ????
}

void
Assistant::backgroundSelectionChangesForms()
{
  QModelIndexList listSelectionBack =
    ui->listBackgroundViewForms->selectionModel()->selectedIndexes();
  _backgroundListChoice.clear();
  _backgroundListChoice.reserve(listSelectionBack.count());

  _blocks.clear();

  const QString backgroundPath = ui->backgroundPathForms->text();
  const QString uniqueBlockCSVFilename = makePath(backgroundPath, "blocks.csv");
  bool useOneBlocksFile = false;
  if (QFileInfo(uniqueBlockCSVFilename).exists()) {
    QVector<QRect> blocks;
    const bool readOk = readBlocksCSV(uniqueBlockCSVFilename, blocks);
    if (readOk) {
      useOneBlocksFile = true;
      _blocks.push_back(blocks);
      assert(_blocks.size() == 1);
    }
  }
  for (const auto &sb : listSelectionBack) {
    const QString selectedBack =
      _backgroundListModel.data(sb, Qt::DisplayRole).toString();

    std::cerr<<"  "<<selectedBack.toStdString()<<"\n";
    if (! useOneBlocksFile) {
      const QString blocksCSVFilename = makePath(backgroundPath, changeExtension(selectedBack, ".csv"));
      qDebug()<<"search csv file: "<<blocksCSVFilename;
      if (QFileInfo(blocksCSVFilename).exists()) {
	QVector<QRect> blocks;
	const bool readOk = readBlocksCSV(blocksCSVFilename, blocks);
	if (readOk) {
	  qDebug()<<"read "<<blocks.size()<<" blocks from "<<blocksCSVFilename;
	  _blocks.push_back(blocks);
	  _backgroundListChoice.append(selectedBack);
	}
	else {
	  qDebug()<<"unable to read csv file: "<<blocksCSVFilename<<"\n";
	}
      }
      else {
	std::cerr<<"csv file does not exist: "<<blocksCSVFilename.toStdString()<<"\n";
      }
    }
    else {
      _backgroundListChoice.append(selectedBack);
    }
  }

  ui->label_nbBackgroundsForms->setText(
    tr("%n selected background image(s)", "", _backgroundListChoice.size())
    +tr("[with %n blocks file(s)]", "", _blocks.size())
					);

  //emit completeChanged();
}

void
Assistant::bckgdSelectAllForms()
{
  ui->listBackgroundViewForms->selectAll();
  backgroundSelectionChangesForms();
}

void
Assistant::bckgdDeselectAllForms()
{
  ui->listBackgroundViewForms->clearSelection();
  backgroundSelectionChangesForms();
}




void
Assistant::PageParams_connect()
{
  connect(ui->spinBox_marginTopMin,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMin()));
  connect(ui->spinBox_marginTopMax,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMax()));
  connect(ui->spinBox_marginBottomMin,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMin()));
  connect(ui->spinBox_marginBottomMax,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMax()));
  connect(ui->spinBox_marginLeftMin,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMin()));
  connect(ui->spinBox_marginLeftMax,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMax()));
  connect(ui->spinBox_marginRightMin,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMin()));
  connect(ui->spinBox_marginRightMax,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMax()));
  connect(ui->spinBox_blockXMin,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMin()));
  connect(ui->spinBox_blockXMax,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMax()));
  connect(ui->spinBox_blockYMin,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMin()));
  connect(ui->spinBox_blockYMax,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMax()));

  connect(ui->radioButton_lineSpacingRandom,
          SIGNAL(clicked()),
          this,
          SLOT(PageParams_updateLineSpacing()));
  connect(ui->radioButton_lineSpacingFontHeight,
          SIGNAL(clicked()),
          this,
          SLOT(PageParams_updateLineSpacing()));
  connect(ui->spinBox_lineSpacingMin,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMin()));
  connect(ui->spinBox_lineSpacingMax,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParams_updateMax()));

  connect(ui->radioButton_ImageSizeUniform,
          SIGNAL(clicked()),
          this,
          SLOT(PageParams_updateImageSize()));
  connect(ui->radioButton_ImageSizeAdaptive,
          SIGNAL(clicked()),
          this,
          SLOT(PageParams_updateImageSize()));
}

void
Assistant::PageParams_updateLineSpacing()
{
  bool enable1 = false;
  if (ui->radioButton_lineSpacingRandom->isChecked()) {
    enable1 = true;
  } else if (ui->radioButton_lineSpacingFontHeight->isChecked()) {
    enable1 = false;
  }

  ui->label_lineSpacingBetween->setEnabled(enable1);
  ui->label_lineSpacingAnd->setEnabled(enable1);
  ui->spinBox_lineSpacingMin->setEnabled(enable1);
  ui->spinBox_lineSpacingMax->setEnabled(enable1);

  //emit completeChanged();
}

void
Assistant::PageParams_updateImageSize()
{
  bool enable1 = false;
  if (ui->radioButton_ImageSizeUniform->isChecked()) {
    enable1 = true;
  } else if (ui->radioButton_ImageSizeAdaptive->isChecked()) {
    enable1 = false;
  }

  ui->label_ImageSizeBetween->setEnabled(enable1);
  ui->label_ImageSizeAnd->setEnabled(enable1);
  ui->spinBox_ImageSizeWidth->setEnabled(enable1);
  ui->spinBox_ImageSizeHeight->setEnabled(enable1);

  //emit completeChanged();
}

void
Assistant::PageParams_updateMin()
{
  //spinBox_YYYMin was updated, we change spinBox_YYYMax

  if (ui->spinBox_marginTopMin->value() > ui->spinBox_marginTopMax->value()) {
    ui->spinBox_marginTopMax->setValue(ui->spinBox_marginTopMin->value());
  }
  if (ui->spinBox_marginBottomMin->value() >
      ui->spinBox_marginBottomMax->value()) {
    ui->spinBox_marginBottomMax->setValue(ui->spinBox_marginBottomMin->value());
  }
  if (ui->spinBox_marginRightMin->value() >
      ui->spinBox_marginRightMax->value()) {
    ui->spinBox_marginRightMax->setValue(ui->spinBox_marginRightMin->value());
  }
  if (ui->spinBox_marginLeftMin->value() > ui->spinBox_marginLeftMax->value()) {
    ui->spinBox_marginLeftMax->setValue(ui->spinBox_marginLeftMin->value());
  }

  if (ui->spinBox_blockXMin->value() > ui->spinBox_blockXMax->value()) {
    ui->spinBox_blockXMax->setValue(ui->spinBox_blockXMin->value());
  }
  if (ui->spinBox_blockYMin->value() > ui->spinBox_blockYMax->value()) {
    ui->spinBox_blockYMax->setValue(ui->spinBox_blockYMin->value());
  }

  if (ui->spinBox_lineSpacingMin->value() >
      ui->spinBox_lineSpacingMax->value()) {
    ui->spinBox_lineSpacingMax->setValue(ui->spinBox_lineSpacingMin->value());
  }

  //emit completeChanged();
}

void
Assistant::PageParams_updateMax()
{
  //spinBox_YYYMax was updated, we change spinBox_YYYMin

  if (ui->spinBox_marginTopMax->value() < ui->spinBox_marginTopMin->value()) {
    ui->spinBox_marginTopMin->setValue(ui->spinBox_marginTopMax->value());
  }
  if (ui->spinBox_marginBottomMax->value() <
      ui->spinBox_marginBottomMin->value()) {
    ui->spinBox_marginBottomMin->setValue(ui->spinBox_marginBottomMax->value());
  }
  if (ui->spinBox_marginRightMax->value() <
      ui->spinBox_marginRightMin->value()) {
    ui->spinBox_marginRightMin->setValue(ui->spinBox_marginRightMax->value());
  }
  if (ui->spinBox_marginLeftMax->value() < ui->spinBox_marginLeftMin->value()) {
    ui->spinBox_marginLeftMin->setValue(ui->spinBox_marginLeftMax->value());
  }

  if (ui->spinBox_blockXMax->value() < ui->spinBox_blockXMin->value()) {
    ui->spinBox_blockXMin->setValue(ui->spinBox_blockXMax->value());
  }
  if (ui->spinBox_blockYMax->value() < ui->spinBox_blockYMin->value()) {
    ui->spinBox_blockYMin->setValue(ui->spinBox_blockYMax->value());
  }

  if (ui->spinBox_lineSpacingMax->value() <
      ui->spinBox_lineSpacingMin->value()) {
    ui->spinBox_lineSpacingMin->setValue(ui->spinBox_lineSpacingMax->value());
  }

  //emit completeChanged();
}

bool
Assistant::PageParams_isComplete() const
{
  return (
    ui->spinBox_marginTopMin->value() <= ui->spinBox_marginTopMax->value() &&
    ui->spinBox_marginBottomMin->value() <=
      ui->spinBox_marginBottomMax->value() &&
    ui->spinBox_marginLeftMin->value() <= ui->spinBox_marginLeftMax->value() &&
    ui->spinBox_marginRightMin->value() <=
      ui->spinBox_marginRightMax->value() &&
    ui->spinBox_blockXMin->value() <= ui->spinBox_blockXMax->value() &&
    ui->spinBox_blockYMin->value() <= ui->spinBox_blockYMax->value() &&
    (!ui->radioButton_lineSpacingRandom->isChecked() ||
     ui->spinBox_lineSpacingMin->value() <=
       ui->spinBox_lineSpacingMax->value()));
}

void
Assistant::PageParams_getParams(RandomDocumentParameters &params) const
{
  params.setBottomMarginMinMax(ui->spinBox_marginBottomMin->value(),
                               ui->spinBox_marginBottomMax->value());
  params.setLeftMarginMinMax(ui->spinBox_marginLeftMin->value(),
                             ui->spinBox_marginLeftMax->value());
  params.setTopMarginMinMax(ui->spinBox_marginTopMin->value(),
                            ui->spinBox_marginTopMax->value());
  params.setRightMarginMinMax(ui->spinBox_marginRightMin->value(),
                              ui->spinBox_marginRightMax->value());
  params.setNbBlocksPerColMinMax(ui->spinBox_blockXMin->value(),
                                 ui->spinBox_blockXMax->value());
  params.setNbBlocksPerRowMinMax(ui->spinBox_blockYMin->value(),
                                 ui->spinBox_blockYMax->value());
  if (ui->radioButton_lineSpacingRandom->isChecked()) {
    params.setLineSpacingType(RandomDocumentParameters::RandomLineSpacing);
    params.setLineSpacingMinMax(ui->spinBox_lineSpacingMin->value(),
                                ui->spinBox_lineSpacingMax->value());
  } else if (ui->radioButton_lineSpacingFontHeight->isChecked()) {
    params.setLineSpacingType(
      RandomDocumentParameters::FontHeightAdaptedLineSpacing);
  }
  params.setPercentOfEmptyBlocks(ui->spinBox_percentEmptyBlocks->value());

  //if (ui->FolderText->isChecked())
  //params.setNbPages(1);
  //else
  params.setNbPages(ui->pageNumber->value());

  if (ui->radioButton_ImageSizeUniform->isChecked()) {
    params.imageSizeUniform = true;
    params.imageWidth = ui->spinBox_ImageSizeWidth->value();
    params.imageHeight = ui->spinBox_ImageSizeHeight->value();
  } else if (ui->radioButton_ImageSizeAdaptive->isChecked()) {
    params.imageSizeUniform = false;
    params.imageWidth = 0;
    params.imageHeight = 0;
  }
}



void
Assistant::PageParamsForms_connect()
{
  connect(ui->radioButton_lineSpacingRandomForms,
          SIGNAL(clicked()),
          this,
          SLOT(PageParamsForms_updateLineSpacing()));
  connect(ui->radioButton_lineSpacingFontHeightForms,
          SIGNAL(clicked()),
          this,
          SLOT(PageParamsForms_updateLineSpacing()));
  connect(ui->spinBox_lineSpacingMinForms,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParamsForms_updateMin()));
  connect(ui->spinBox_lineSpacingMaxForms,
          SIGNAL(valueChanged(int)),
          this,
          SLOT(PageParamsForms_updateMax()));

  connect(ui->radioButton_ImageSizeUniformForms,
          SIGNAL(clicked()),
          this,
          SLOT(PageParamsForms_updateImageSize()));
  connect(ui->radioButton_ImageSizeAdaptiveForms,
          SIGNAL(clicked()),
          this,
          SLOT(PageParamsForms_updateImageSize()));
}

void
Assistant::PageParamsForms_updateLineSpacing()
{
  bool enable1 = false;
  if (ui->radioButton_lineSpacingRandomForms->isChecked()) {
    enable1 = true;
  } else if (ui->radioButton_lineSpacingFontHeightForms->isChecked()) {
    enable1 = false;
  }

  ui->label_lineSpacingBetweenForms->setEnabled(enable1);
  ui->label_lineSpacingAndForms->setEnabled(enable1);
  ui->spinBox_lineSpacingMinForms->setEnabled(enable1);
  ui->spinBox_lineSpacingMaxForms->setEnabled(enable1);

  //emit completeChanged();
}

void
Assistant::PageParamsForms_updateImageSize()
{
  bool enable1 = false;
  if (ui->radioButton_ImageSizeUniformForms->isChecked()) {
    enable1 = true;
  } else if (ui->radioButton_ImageSizeAdaptiveForms->isChecked()) {
    enable1 = false;
  }

  ui->label_ImageSizeBetweenForms->setEnabled(enable1);
  ui->label_ImageSizeAndForms->setEnabled(enable1);
  ui->spinBox_ImageSizeWidthForms->setEnabled(enable1);
  ui->spinBox_ImageSizeHeightForms->setEnabled(enable1);

  //emit completeChanged();
}

void
Assistant::PageParamsForms_updateMin()
{
  if (ui->spinBox_lineSpacingMinForms->value() >
      ui->spinBox_lineSpacingMaxForms->value()) {
    ui->spinBox_lineSpacingMaxForms->setValue(ui->spinBox_lineSpacingMinForms->value());
  }

  //emit completeChanged();
}

void
Assistant::PageParamsForms_updateMax()
{
  //spinBox_YYYMax was updated, we change spinBox_YYYMin

  if (ui->spinBox_lineSpacingMaxForms->value() <
      ui->spinBox_lineSpacingMinForms->value()) {
    ui->spinBox_lineSpacingMinForms->setValue(ui->spinBox_lineSpacingMaxForms->value());
  }

  //emit completeChanged();
}

bool
Assistant::PageParamsForms_isComplete() const
{
  return (
    (!ui->radioButton_lineSpacingRandomForms->isChecked() ||
     ui->spinBox_lineSpacingMinForms->value() <=
       ui->spinBox_lineSpacingMaxForms->value()));
}

void
Assistant::PageParamsForms_getParams(RandomDocumentParameters &params) const
{
  if (ui->radioButton_lineSpacingRandomForms->isChecked()) {
    params.setLineSpacingType(RandomDocumentParameters::RandomLineSpacing);
    params.setLineSpacingMinMax(ui->spinBox_lineSpacingMinForms->value(),
                                ui->spinBox_lineSpacingMaxForms->value());
  }
  else if (ui->radioButton_lineSpacingFontHeightForms->isChecked()) {
    params.setLineSpacingType(
      RandomDocumentParameters::FontHeightAdaptedLineSpacing);
  }
  params.setPercentOfEmptyBlocks(ui->spinBox_percentEmptyBlocksForms->value());

  //if (ui->FolderText->isChecked())
  //params.setNbPages(1);
  //else
  params.setNbPages(ui->pageNumber->value());

  if (ui->radioButton_ImageSizeUniformForms->isChecked()) {
    params.imageSizeUniform = true;
    params.imageWidth = ui->spinBox_ImageSizeWidthForms->value();
    params.imageHeight = ui->spinBox_ImageSizeHeightForms->value();
  }
  else if (ui->radioButton_ImageSizeAdaptiveForms->isChecked()) {
    params.imageSizeUniform = false;
    params.imageWidth = 0;
    params.imageHeight = 0;
  }

  params.useRandomBlocks = false;
  params.blocks = _blocks;
}



void
Assistant::chooseOutputTxtImageDir()
{
  const QString path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Choose output directory"),
                                      QStringLiteral("./"),
                                      QFileDialog::ShowDirsOnly);
  if (!path.isEmpty()) {
    ui->outputTxtImageDirPath->setText(path);
    _outputTxtImageDir = path;
  }

  //emit completeChanged();
}

int
Assistant::computeNbGeneratedTexts() const
{
  const bool oneRandomFontAndBackground =
    ui->FontBackgroundRandomRB->isChecked();

  const int nbTexts =
    (ui->FolderText->isChecked() ? _txtList.count() : ui->pageNumber->value());
  const int nbFonts = oneRandomFontAndBackground ? 1 : _fontListChoice.count();
  const int nbBackgrounds =
    oneRandomFontAndBackground ? 1 : _backgroundListChoice.count();

  const int nbGeneratedTxtImages = nbTexts * nbFonts * nbBackgrounds;

  return nbGeneratedTxtImages;
}

void
Assistant::updateTxtGenerationInfo()
{

  const bool oneRandomFontAndBackground =
    ui->FontBackgroundRandomRB->isChecked();

  const int nbTexts =
    (ui->FolderText->isChecked() ? _txtList.count() : ui->pageNumber->value());
  const int nbFontsOrig = _fontListChoice.count();
  const int nbFonts = oneRandomFontAndBackground ? 1 : nbFontsOrig;
  const int nbBackgroundsOrig = _backgroundListChoice.count();
  const int nbBackgrounds = oneRandomFontAndBackground ? 1 : nbBackgroundsOrig;

  const int nbGeneratedTxtImages = nbTexts * nbFonts * nbBackgrounds;

  assert(computeNbGeneratedTexts() == nbGeneratedTxtImages);

  if (oneRandomFontAndBackground) {
    ui->label_FinalText->setText(
      tr("With %1 text(s), 1 random font and 1 random background image:\n "
         "%2 image(s) will be generated if you click Next")
        .arg(nbTexts)
        .arg(nbGeneratedTxtImages));
  } else {
    ui->label_FinalText->setText(
      tr("With %1 text(s), %2 font(s) and %3 background image(s):\n %4 "
         "image(s) will be generated if you click Next")
        .arg(nbTexts)
        .arg(nbFonts)
        .arg(nbBackgrounds)
        .arg(nbGeneratedTxtImages));
  }

  const bool enabled = (nbFontsOrig * nbBackgroundsOrig > 1);
  ui->BackgroundFontLabel->setEnabled(enabled);
  ui->FontBackgroundRandomRB->setEnabled(enabled);
  ui->FontBackgroundAllRB->setEnabled(enabled);
}

void
Assistant::chooseInputImageDir()
{
  _semi = true;
  const QString path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Choose input image directory"),
                                      QStringLiteral("./"),
                                      QFileDialog::ShowDirsOnly);
  ui->inputImagePath->setText(path);
  _PicDirectory = path;
  if (!path.isEmpty()) {
    loadInputDegradationImageList();
  } else {
    const QString szStr = QString::number(0);
    //ui->NbPicFolder->setText(szStr);
    ui->NbInputPics->setText(szStr);
  }
}

void
Assistant::enableAccordingToInputImages()
{
  const int sz = _inputImageList.size();

  ui->label_nbInputImages->setText(tr("%n input image(s)", "", sz));

  const QString szStr = QString::number(sz);
  //ui->NbPicFolder->setText(szStr);
  ui->NbInputPics->setText(szStr);

  if (sz > 0) {
    const bool enableBleedThrough = (sz > 1);
    ui->CheckBleed->setEnabled(enableBleedThrough);

    BleedThrough_LoadPrevImg();
    CharDeg_LoadPrevImgChar();
    Shadow_LoadPrevImgShad();
    Phantom_LoadPrevImgPhant();
    Blur_LoadPrevImgBlur();
    Hole_LoadPrevImgHole();
    updateTotalPic();
  }
}

void
Assistant::loadInputDegradationImageList()
{
  QGuiApplication::setOverrideCursor(Qt::BusyCursor);

  QDir directory(_PicDirectory);

  _inputImageList.clear();

  const QStringList nameFilters = getReadImageFilterList();
  QList<QString> fileList =
    directory.entryList(nameFilters, QDir::Files | QDir::Readable);
  _inputImageList.reserve(fileList.size());
  for (const QString &filename : fileList) {
    _inputImageList.append(directory.relativeFilePath(filename));
  }

  const int sz = _inputImageList.size();

  if (sz == 0) {
    QMessageBox::critical(this,
                          QStringLiteral("No images in directory"),
                          QStringLiteral("Select a directory with images"));
  }
  enableAccordingToInputImages();

  QGuiApplication::restoreOverrideCursor();
}

void
Assistant::chooseGTDirectory()
{
  const QString path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Choose ground trough directory"),
                                      "./",
                                      QFileDialog::ShowDirsOnly);
  ui->gtFilePath->setText(path);
}

/*
void Assistant::chosePicDegDirectory()
{
  const QString path = QFileDialog::getExistingDirectory(this, tr("Choose save directory"), "./", QFileDialog::ShowDirsOnly);
  ui->outputDegrImagePathDeg->setText(path);
  _outputDegradedImageDir = path;
}
*/

void
Assistant::chooseOutputDegradedImageDir()
{
  const QString path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Choose save directory"),
                                      QStringLiteral("./"),
                                      QFileDialog::ShowDirsOnly);
  ui->outputDegrImagePath->setText(path);
  _outputDegradedImageDir = path;
}

/*
void Assistant::changeLoremIpsum()
{
  if (ui->LoremIpsum->isChecked()) {
    ui->pageNumber->show();
    ui->pageNumberLabel->show();
  }
  else {
    ui->pageNumber->hide();
    ui->pageNumberLabel->hide();
  }
}
*/

int
Assistant::nbOfDegradedImages() const
{
  int totalPic = 0;

  const int nbPic = _inputImageList.size();

  if (_BleedThrough_bleedEnable)
    totalPic += 1 * ui->TirageBleed->value() * nbPic;

  if (_CharDeg_charEnable)
    totalPic += 1 * ui->TirageChar->value() * nbPic;

  if (_Phantom_phantEnable)
    totalPic += _Phantom_nbPhantSelected * ui->TiragePhantom->value() * nbPic;

  if (_GDD_gddEnable)
    totalPic += 1 * ui->TirageGDD->value() * nbPic;

  if (_Noise_noiseEnable)
    totalPic += Noise_nbDegradations() * ui->TirageNoise->value() * nbPic;

  if (_Rotation_rotationEnable)
    totalPic += Rotation_nbDegradations() * ui->TirageRotation->value() * nbPic;
  
  if (_Shadow_shadEnable)
    totalPic += _Shadow_nbShadSelected * ui->TirageShadow->value() * nbPic;

  if (_Blur_blurEnable)
    totalPic += Blur_nbDegradations() * ui->TirageBlur->value() * nbPic;

  if (_Hole_holeEnable)
    totalPic += 1 * ui->TirageHole->value() * nbPic;

  if (_ElasticDeformation_elasticEnable)
    totalPic += ElasticDeformation_nbDegradations() * ui->TirageElasticDeformation->value() * nbPic;

  if (_Dist3D_dist3DEnable)
    totalPic += _Dist3D_meshesList.size() * ui->TirageDist3D->value() * nbPic;

  return totalPic;
}

void
Assistant::updateTotalPic()
{
  const int totalPic = nbOfDegradedImages();

  //std::cerr<<"updateTotalPic total="<<totalPic<<"\n";

  ui->TotalPic->setText(QString::number(totalPic));
}

void
Assistant::GTChecked()
{
  bool enabled = false;
  if (ui->CheckGT->isChecked()) {
    enabled = true;
  }
  ui->FolderGTTextSemi->setEnabled(enabled);
  ui->gtFilePath->setEnabled(enabled);
  ui->btnChooseGTDir->setEnabled(enabled);
}

void
Assistant::addInputImage(const QString &imageFilename)
{
  //std::cerr<<"Assistant::addInputImage "<<imageFilename.toStdString()<<"\n";
  _inputImageList.push_back(imageFilename);
}

static int
computeNumberWidth(int n)
{
  int i = 0;
  for (int m = n; m > 0; m /= 10, ++i)
    ;
  return i;
}

#define TIMING 1
#ifdef TIMING
#include <chrono> //DEBUG
#endif            //TIMING


void
Assistant::generateTxtImages()
{
  QGuiApplication::setOverrideCursor(Qt::BusyCursor);

  _inputImageList.clear();

#if 0
  RandomDocumentParameters commonParams;
  PageParams_getParams(commonParams);
  
  if (_txtList.size() != 0) {
    DocumentToXMLExporter xmlDocExporter(_outputTxtImageDir);
    RandomDocumentExporter docExporter(_DocController, _outputTxtImageDir, this);
    QObject::connect(&docExporter, SIGNAL(imageSaved(const QString &)), this, SLOT(addInputImage(const QString &)));

    for (const QString &back : _backgroundListChoice) {
      Context::BackgroundContext::instance()->setCurrentBackground(back);

      for (const QString &font : _fontListChoice) {
	for (const QString &file : _txtList) {
	  //std::cout<<"file: "<<file.toStdString()<<" font: "<<font.toStdString() <<" back: "<<back.toStdString()<<std::endl;

	  RandomDocumentParameters param;
	  commonParams.copyTo(param);
	  const QString txtPath = makePath(_txtDirectory, file);
	  QString fontPath = makePath(_FontDirectory, font);
	  if (! fontPath.endsWith(".of", Qt::CaseInsensitive))
	    fontPath += ".of";
	  param.fontList.append(fontPath);
	  param.textList.append(txtPath);
	  param.outputFolderPath = _outputTxtImageDir;

	  RandomDocumentCreator random(_DocController, param);

	  QObject::connect(&random, SIGNAL(imageReady()), &docExporter, SLOT(saveRandomDocument()));
	  QObject::connect(&random, SIGNAL(imageReady()), &xmlDocExporter, SLOT(toXML()));
	  
	  random.create();

	  qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
	}
      }

    }
  }
  else {
    DocumentToXMLExporter xmlDocExporter(_outputTxtImageDir);
    RandomDocumentExporter docExporter(_DocController, _outputTxtImageDir, this);
    QObject::connect(&docExporter, SIGNAL(imageSaved(const QString &)), this, SLOT(addInputImage(const QString &)));

    for (const QString &back : _backgroundListChoice) {
      for (const QString &font : _fontListChoice) {
	//std::cout<<"font: "<<font.toStdString() <<" back: "<<back.toStdString() <<std::endl;

	Context::BackgroundContext::instance()->setCurrentBackground(back);

	RandomDocumentParameters param;
	commonParams.copyTo(param);
	QString fontPath = makePath(_FontDirectory, font);
	if (! fontPath.endsWith(".of", Qt::CaseInsensitive))
	  fontPath += ".of";
	param.fontList.append(fontPath);
	param.outputFolderPath = _outputTxtImageDir;

	RandomDocumentCreator random(_DocController, param);

	QObject::connect(&random, SIGNAL(imageReady()), &docExporter, SLOT(saveRandomDocument()));
	QObject::connect(&random, SIGNAL(imageReady()), &xmlDocExporter, SLOT(toXML()));

	random.create();

	qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
    }
  }

#else

  DocumentToXMLExporter xmlDocExporter(_outputTxtImageDir);
  RandomDocumentExporter docExporter(_DocController, _outputTxtImageDir, this);
  QObject::connect(&docExporter,
                   SIGNAL(imageSaved(QString)),
                   this,
                   SLOT(addInputImage(QString)));

  if (ui->zeroPaddingTextCB->isChecked()) {
    const int nbGeneratedTexts = computeNbGeneratedTexts();
    const int numberWidth =
      computeNumberWidth(nbGeneratedTexts + xmlDocExporter._nb);
    xmlDocExporter.setNumberWidth(numberWidth);
    docExporter.setNumberWidth(numberWidth);
  }

  RandomDocumentParameters params;
  if (ui->ClassicTextDocument->isChecked()) {
    PageParams_getParams(params);
  }
  else {
    assert(ui->FormTextDocument->isChecked());
    PageParamsForms_getParams(params);
  }

  params.outputFolderPath = _outputTxtImageDir;

  //copy list of backgrounds/textFiles/fonts in params
  params.backgroundList = _backgroundListChoice;
  const QStringList &txtList = _txtList;
  if (ui->FolderText->isChecked()) {
    for (const QString &txtFile : txtList) {
      const QString txtPath = makePath(_txtDirectory, txtFile);
      params.textList.append(txtPath);
    }
    params.nbPages = 1;
  }
  else {
    const int nbTexts = ui->pageNumber->value();
    params.nbPages = nbTexts;
  }
  const QStringList &fontListChoice = _fontListChoice;
  for (const QString &font : fontListChoice) {
    QString fontPath = makePath(_FontDirectory, font);
    if (!fontPath.endsWith(".of", Qt::CaseInsensitive))
      fontPath += ".of";
    params.fontList.append(fontPath);
  }

  RandomDocumentCreator random(_DocController, params);

  QObject::connect(
    &random, SIGNAL(imageReady()), &docExporter, SLOT(saveRandomDocument()));
  QObject::connect(
    &random, SIGNAL(imageReady()), &xmlDocExporter, SLOT(toXML()));

#ifdef TIMING
  auto t0 = std::chrono::steady_clock::now();
#endif //TIMING

  const bool oneRandomFontAndBackground =
    ui->FontBackgroundRandomRB->isChecked();
  if (oneRandomFontAndBackground) {
    random.createAllTextsOneFontBackground();
  }
  else { //all combinations
    random.createAllTexts();
  }

#ifdef TIMING
  auto t1 = std::chrono::steady_clock::now();
  auto time1 = t1 - t0;
  std::cerr << "Assistant::generateTxtImages() time="
            << std::chrono::duration<double>(time1).count() << "s for "
            << computeNbGeneratedTexts() << " images\n";
#endif //TIMING

#endif //0

  _PicDirectory = QString();
  //std::cerr << "_inputImageList.size()=" << _inputImageList.size() << "\n";
  enableAccordingToInputImages();

  QGuiApplication::restoreOverrideCursor();
}

void
Assistant::BleedThrough_LoadPrevImg()
{
  if (!_inputImageList.empty()) {

    int nbPic = P_bounded_rand(0, _inputImageList.size());
    if (_inputImageList.size() > 1)
      while (nbPic == _BleedThrough_indexRecto)
        nbPic = P_bounded_rand(0, _inputImageList.size());

    _BleedThrough_indexRecto = nbPic;
    QString imagePath = _inputImageList[nbPic];
    imagePath = makePath(_PicDirectory, imagePath);
    _BleedThrough_rectoImg.load(imagePath);
    //std::cout << ".............  " << imagePath.toStdString() << std::endl;
    if (!_BleedThrough_rectoImg.isNull()) {

      _BleedThrough_rectoImgSmall = toGray(_BleedThrough_rectoImg.scaled(
        IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation));
      _BleedThrough_rectoImgPart = takePart(
        _BleedThrough_rectoImg); //ROI should be interactively selectable

      if (_inputImageList.size() > 1) {
        BleedThrough_setVersoImage(); //will apply current parameters
      } else {
        //just initialize pixmaps
        ui->BleedMinPreviewLabel->setPixmap(
          QPixmap::fromImage(_BleedThrough_rectoImgPart));
        ui->BleedMinPreviewLabel->setMinimumSize(
          _BleedThrough_rectoImgPart.size());
        ui->BleedMaxPreviewLabel->setPixmap(
          QPixmap::fromImage(_BleedThrough_rectoImgPart));
        ui->BleedMaxPreviewLabel->setMinimumSize(
          _BleedThrough_rectoImgPart.size());
      }

    } else {
      std::cout << "BleedThrough: recto image is null" << std::endl;
    }
  }
}

void
Assistant::BleedThrough_setVersoImage()
{
  if (_inputImageList.size() > 1) { //at least two images

    int nbPic = P_bounded_rand(0, _inputImageList.size());
    while (nbPic == _BleedThrough_indexRecto)
      nbPic = P_bounded_rand(0, _inputImageList.size());

    QString imagePath = _inputImageList[nbPic];
    imagePath = makePath(_PicDirectory, imagePath);
    _BleedThrough_versoImg.load(imagePath);
  }

  _BleedThrough_versoImgSmall = toGray(_BleedThrough_versoImg.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation));
  _BleedThrough_versoImgSmall =
    _BleedThrough_versoImgSmall.mirrored(true, false);

  _BleedThrough_versoImgPart =
    takePart(_BleedThrough_versoImg); //ROI should be interactively selectable
  _BleedThrough_versoImgPart = _BleedThrough_versoImgPart.mirrored(true, false);

  BleedThrough_updateVersoImage();
}

void
Assistant::BleedThrough_updateVersoImage()
{
  if (!_BleedThrough_rectoImgSmall.isNull() &&
      !_BleedThrough_versoImgSmall.isNull()) {
    BleedThrough_updateBleedImageMin(this->_BleedThrough_bleedMin, true);
    BleedThrough_updateBleedImageMax(this->_BleedThrough_bleedMax, true);
  }
}

void
Assistant::BleedThrough_changeMinBleed()
{
  ui->BleedMinValue->setText(QString::number(ui->BleedMin->value()));
  if (ui->BleedMax->value() < ui->BleedMin->value()) {
    ui->BleedMax->setSliderPosition(ui->BleedMin->value());
    _BleedThrough_bleedMax = ui->BleedMax->value();
  }
  _BleedThrough_bleedMin = ui->BleedMin->value();
}

void
Assistant::BleedThrough_changeMaxBleed()
{
  ui->BleedMaxValue->setText(QString::number(ui->BleedMax->value()));

  if (ui->BleedMin->value() > ui->BleedMax->value()) {
    ui->BleedMin->setSliderPosition(ui->BleedMax->value());
    _BleedThrough_bleedMin = ui->BleedMin->value();
  }
  _BleedThrough_bleedMax = ui->BleedMax->value();
}

void
Assistant::BleedThrough_updateTirageAndTotal()
{
  const int nbDegs = (_BleedThrough_bleedEnable ? 1 : 0);
  ui->NbDegBleed->setText(QString::number(nbDegs));

  const bool enabled = (nbDegs > 0);
  ui->NbDegBleed->setEnabled(enabled);
  ui->TirageBleed->setEnabled(enabled);
  ui->TotalBleed->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TirageBleed->setValue(0);

  if (
    nbDegs > 0 &&
    ui->TirageBleed->value() ==
      0) { //set TirageBleed value to 1 once we have some degradations selected
    ui->TirageBleed->setValue(1);
  }

  const int total = nbDegs * _inputImageList.size() * ui->TirageBleed->value();
  ui->TotalBleed->setText(QString::number(total));

  updateTotalPic();
}

void
Assistant::BleedThrough_EnableBleedOption()
{
  bool enabled = false;
  bool switchEnabled = false;
  if (ui->CheckBleed->isChecked()) {
    enabled = true;
    switchEnabled = (_inputImageList.size() > 1); //at least two images
    _BleedThrough_bleedEnable = enabled;
    BleedThrough_updateTirageAndTotal();
  } else {
    _BleedThrough_bleedEnable = enabled;
    ui->NbDegBleed->setEnabled(enabled);
    ui->TirageBleed->setEnabled(enabled);
    ui->TotalBleed->setEnabled(enabled);

    ui->TirageBleed->setValue(
      0); //signal/slot will call BleedThrough_tirageBleedChanged(0)
  }

  ui->OptionBleed->setEnabled(enabled);
  ui->BleedMin->setEnabled(enabled);
  ui->BleedMax->setEnabled(enabled);
  ui->BleedMinLabel->setEnabled(enabled);
  ui->BleedMaxLabel->setEnabled(enabled);
  ui->TirageBleed->setEnabled(enabled);
  ui->SwitchBleed->setEnabled(switchEnabled);
}

void
Assistant::BleedThrough_setupGUIImages()
{
  ui->SwitchBleed->setToolTip(
    tr("Change preview area of both recto and verso images"));

  ui->BleedMinPreviewLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->BleedMinPreviewLabel->setText(tr("preview"));
  ui->BleedMaxPreviewLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->BleedMaxPreviewLabel->setText(tr("preview"));
  ui->BleedMinPreviewLabel->setMinimumSize(BLEED_PREVIEW_WIDTH,
                                           BLEED_PREVIEW_HEIGHT);
  ui->BleedMaxPreviewLabel->setMinimumSize(BLEED_PREVIEW_WIDTH,
                                           BLEED_PREVIEW_HEIGHT);
}

void
Assistant::BleedThrough_updateBleedImageMin(int nbIter, bool fromZero)
{
  assert(!_BleedThrough_rectoImgSmall.isNull() &&
         !_BleedThrough_versoImgSmall.isNull());
  assert(!_BleedThrough_rectoImgPart.isNull() &&
         !_BleedThrough_versoImgPart.isNull());
  assert(ui->BleedMinPreviewLabel);
  assert(ui->BleedMaxPreviewLabel);
  assert(_BleedThrough_rectoImgPart.size() ==
         _BleedThrough_versoImgPart.size());

  const int prevNbIter = this->_BleedThrough_bleedMin;
  int lNbIter = 0;
  QImage currRectoImg;

  if (prevNbIter != 0)
    fromZero = false;

  if (!fromZero && prevNbIter < nbIter) {
    lNbIter = nbIter - prevNbIter;
    currRectoImg = _BleedThrough_bleedImgPart;
  } else {
    lNbIter = nbIter;
    currRectoImg = _BleedThrough_rectoImgPart.copy();
  }
  assert(currRectoImg.size() == _BleedThrough_rectoImgPart.size());

  _BleedThrough_bleedImgPart =
    dc::BleedThrough::bleedThrough(_BleedThrough_rectoImgPart,
                                   currRectoImg,
                                   _BleedThrough_versoImgPart,
                                   lNbIter);

  ui->BleedMinPreviewLabel->setPixmap(
    QPixmap::fromImage(_BleedThrough_bleedImgPart));
  ui->BleedMinPreviewLabel->setMinimumSize(_BleedThrough_bleedImgPart.size());
}

void
Assistant::BleedThrough_nbIterationsMinChanged()
{
  const int value = ui->BleedMin->value();
  if (!_BleedThrough_rectoImgSmall.isNull() &&
      !_BleedThrough_versoImgSmall.isNull()) {
    BleedThrough_updateBleedImageMin(value, false);

    if (ui->BleedMax->value() != _BleedThrough_bleedMax)
      BleedThrough_updateBleedImageMax(ui->BleedMax->value(), false);
  }

  this->_BleedThrough_bleedMin = value;
}

void
Assistant::BleedThrough_updateBleedImageMax(int nbIter, bool fromZero)
{
  assert(!_BleedThrough_rectoImgSmall.isNull() &&
         !_BleedThrough_versoImgSmall.isNull());
  assert(!_BleedThrough_rectoImgPart.isNull() &&
         !_BleedThrough_versoImgPart.isNull());
  assert(ui->BleedMaxPreviewLabel);
  assert(_BleedThrough_rectoImgPart.size() ==
         _BleedThrough_versoImgPart.size());

  const int prevNbIter = this->_BleedThrough_bleedMax;
  int lNbIter = 0;
  QImage currRectoImg;

  if (prevNbIter != 0)
    fromZero = false;

  if (!fromZero && prevNbIter < nbIter) {
    lNbIter = nbIter - prevNbIter;
    currRectoImg = _BleedThrough_bleedImgPart;
  } else {
    lNbIter = nbIter;
    currRectoImg = _BleedThrough_rectoImgPart.copy();
  }
  assert(currRectoImg.size() == _BleedThrough_rectoImgPart.size());

  _BleedThrough_bleedImgPart =
    dc::BleedThrough::bleedThrough(_BleedThrough_rectoImgPart,
                                   currRectoImg,
                                   _BleedThrough_versoImgPart,
                                   lNbIter);

  ui->BleedMaxPreviewLabel->setPixmap(
    QPixmap::fromImage(_BleedThrough_bleedImgPart));
  ui->BleedMaxPreviewLabel->setMinimumSize(_BleedThrough_bleedImgPart.size());
}

void
Assistant::BleedThrough_nbIterationsMaxChanged()
{
  const int value = ui->BleedMax->value();
  if (!_BleedThrough_rectoImgSmall.isNull() &&
      !_BleedThrough_versoImgSmall.isNull()) {
    BleedThrough_updateBleedImageMax(value, false);

    if (ui->BleedMin->value() != _BleedThrough_bleedMin)
      BleedThrough_updateBleedImageMin(ui->BleedMin->value(), false);
  }
  this->_BleedThrough_bleedMax = value;
}

void
Assistant::BleedThrough_tirageBleedChanged(int /*value*/)
{
  const int nbDegs = (_BleedThrough_bleedEnable ? 1 : 0);
  ui->NbDegBleed->setText(QString::number(nbDegs));
  const int total = nbDegs * _inputImageList.size() * ui->TirageBleed->value();
  ui->TotalBleed->setText(QString::number(total));
  updateTotalPic();
}

void
Assistant::CharDeg_LoadPrevImgChar()
{
  if (!_inputImageList.empty()) {

    int imgIndex = P_bounded_rand(0, _inputImageList.size());
    if (_inputImageList.size() > 1)
      while (imgIndex == _CharDeg_indexRecto)
        imgIndex = P_bounded_rand(0, _inputImageList.size());

    _CharDeg_indexRecto = imgIndex;
    assert(imgIndex < _inputImageList.size());
    QString imagePath = _inputImageList[imgIndex];
    //std::cerr<<imgIndex<<") _PicDirectory="<<_PicDirectory.toStdString()<<" imagePath="<<imagePath.toStdString()<<"\n";
    imagePath = makePath(_PicDirectory, imagePath);
    _CharDeg_rectoImgChar.load(imagePath);
    //std::cout << "............. ["<<imgIndex<<"]: "<< imagePath.toStdString() <<" isNull? "<<_CharDeg_rectoImgChar.isNull()<< std::endl;

    if (!_CharDeg_rectoImgChar.isNull()) {

      _CharDeg_rectoImgPartChar = toGray(takePart(
        _CharDeg_rectoImgChar)); //ROI should be interactively selectable

      //ui->CharMinPreviewLabel->setPixmap(QPixmap::fromImage(_CharDeg_rectoImgPartChar));
      ui->CharMinPreviewLabel->setMinimumSize(_CharDeg_rectoImgPartChar.size());

      //ui->CharMaxPreviewLabel->setPixmap(QPixmap::fromImage(_CharDeg_rectoImgPartChar));
      ui->CharMaxPreviewLabel->setMinimumSize(_CharDeg_rectoImgPartChar.size());

      //apply current parameters
      CharDeg_updateCharImageMin(_CharDeg_charMin);
      CharDeg_updateCharImageMax(_CharDeg_charMax);

    } else {
      std::cout << "CharDeg: recto image is null" << std::endl;
    }
  }
}

void
Assistant::CharDeg_setupGUIImages()
{
  ui->CharMinPreviewLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->CharMaxPreviewLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->CharMinPreviewLabel->setMinimumSize(CHARDEG_PREVIEW_WIDTH,
                                          CHARDEG_PREVIEW_HEIGHT);
  ui->CharMaxPreviewLabel->setMinimumSize(CHARDEG_PREVIEW_WIDTH,
                                          CHARDEG_PREVIEW_HEIGHT);

  ui->CharMinPreviewLabel->setText(tr("preview"));
  ui->CharMaxPreviewLabel->setText(tr("preview"));
}

void
Assistant::CharDeg_updateTirageAndTotal()
{
  const int nbDegs = (_CharDeg_charEnable ? 1 : 0);
  ui->NbDegChar->setText(QString::number(nbDegs));

  const bool enabled = (nbDegs > 0);
  ui->NbDegChar->setEnabled(enabled);
  ui->TirageChar->setEnabled(enabled);
  ui->TotalChar->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TirageChar->setValue(0);

  if (nbDegs > 0 &&
      ui->TirageChar->value() ==
        0) { //set TirageChar value to 1 once we have some degradations selected
    ui->TirageChar->setValue(1);
  }

  const int total = nbDegs * _inputImageList.size() * ui->TirageChar->value();
  ui->TotalChar->setText(QString::number(total));

  updateTotalPic();
}

void
Assistant::CharDeg_EnableCharOption()
{
  bool enabled = false;
  bool switchEnabled = false;
  if (ui->CheckChar->isChecked()) {
    enabled = true;
    switchEnabled = (_inputImageList.size() > 1);
    _CharDeg_charEnable = enabled;
    CharDeg_updateTirageAndTotal();
  } else {
    _CharDeg_charEnable = enabled;
    ui->NbDegChar->setEnabled(enabled);
    ui->TotalChar->setEnabled(enabled);
    ui->TirageChar->setEnabled(enabled);

    ui->TirageChar->setValue(
      0); //signal/slot will call CharDeg_tirageCharDegChanged(0)
  }
  ui->OptionChar->setEnabled(enabled);
  ui->CharMin->setEnabled(enabled);
  ui->CharMax->setEnabled(enabled);
  ui->CharMinLabel->setEnabled(enabled);
  ui->CharMaxLabel->setEnabled(enabled);
  ui->TirageChar->setEnabled(enabled);
  ui->SwitchChar->setEnabled(switchEnabled);
}

void
Assistant::CharDeg_changeMinChar()
{
  ui->CharMinValue->setText(QString::number(ui->CharMin->value()));
  if (ui->CharMax->value() < ui->CharMin->value()) {
    ui->CharMax->setSliderPosition(ui->CharMin->value());
    _CharDeg_charMax = ui->CharMax->value();
  }
  _CharDeg_charMin = ui->CharMin->value();
}

void
Assistant::CharDeg_changeMaxChar()
{
  ui->CharMaxValue->setText(QString::number(ui->CharMax->value()));
  if (ui->CharMin->value() > ui->CharMax->value()) {
    ui->CharMin->setSliderPosition(ui->CharMax->value());
    _CharDeg_charMin = ui->CharMin->value();
  }
  _CharDeg_charMax = ui->CharMax->value();
}

void
Assistant::CharDeg_updateCharImageMin(int level)
{
  dc::GrayscaleCharsDegradationModelQ cdgMin(_CharDeg_rectoImgPartChar);
  QImage Deg = cdgMin.degradateByLevel(level);

  ui->CharMinPreviewLabel->setPixmap(QPixmap::fromImage(Deg));
  ui->CharMinPreviewLabel->setMinimumSize(Deg.size());
}

void
Assistant::CharDeg_nbIterationsMinChangedChar()
{
  const int level = ui->CharMin->value();
  if (!_CharDeg_rectoImgPartChar.isNull())
    CharDeg_updateCharImageMin(level);
  else
    QMessageBox::information(
      this, QStringLiteral("Semi"), QStringLiteral("Image Introuvable"));
}

void
Assistant::CharDeg_updateCharImageMax(int level)
{
  dc::GrayscaleCharsDegradationModelQ cdgMax(_CharDeg_rectoImgPartChar);
  QImage Deg = cdgMax.degradateByLevel(level);

  ui->CharMaxPreviewLabel->setPixmap(QPixmap::fromImage(Deg));
  ui->CharMaxPreviewLabel->setMinimumSize(Deg.size());
}

void
Assistant::CharDeg_nbIterationsMaxChangedChar()
{
  const int level = ui->CharMax->value();
  if (!_CharDeg_rectoImgPartChar.isNull())
    CharDeg_updateCharImageMax(level);
  else
    QMessageBox::information(
      this, QStringLiteral("Semi"), QStringLiteral("Image Introuvable"));
}

void
Assistant::CharDeg_tirageCharChanged(int /*value*/)
{
  const int nbDegs = (_CharDeg_charEnable ? 1 : 0);
  ui->NbDegChar->setText(QString::number(nbDegs));
  const int total = nbDegs * _inputImageList.size() * ui->TirageChar->value();
  ui->TotalChar->setText(QString::number(total));

  //std::cerr<<"Char_tirageCharChanged(v) nbDegs="<<nbDegs<<" total="<<total<<"\n";

  updateTotalPic();
}


void
Assistant::Noise_setupGUIImages()
{
  if (ui->GaussianNoiseAddTypeCB->count() == 0) {
    ui->GaussianNoiseAddTypeCB->addItem(tr("None"),
					QVariant(static_cast<int>(dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS)));
    ui->GaussianNoiseAddTypeCB->addItem(tr("To gray"),
					QVariant(static_cast<int>(dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY)));
    ui->GaussianNoiseAddTypeCB->addItem(tr("To gray if image is gray"),
					QVariant(static_cast<int>(dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY_IF_GRAY)));
    assert(ui->GaussianNoiseAddTypeCB->count() == 3);
    ui->GaussianNoiseAddTypeCB->setCurrentIndex(2);
  }

  if (ui->SpeckleNoiseAddTypeCB->count() == 0) {
    ui->SpeckleNoiseAddTypeCB->addItem(tr("None"),
					QVariant(static_cast<int>(dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS)));
    ui->SpeckleNoiseAddTypeCB->addItem(tr("To gray"),
					QVariant(static_cast<int>(dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY)));
    ui->SpeckleNoiseAddTypeCB->addItem(tr("To gray if image is gray"),
					QVariant(static_cast<int>(dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY_IF_GRAY)));
    assert(ui->SpeckleNoiseAddTypeCB->count() == 3);
    ui->SpeckleNoiseAddTypeCB->setCurrentIndex(2);
  }

}

int
Assistant::Noise_nbDegradations() const
{
  int nbDegs = 0;
  if (ui->GaussianNoiseCB->isChecked()) {
    nbDegs += 1;
  }
  if (ui->SpeckleNoiseCB->isChecked()) {
    nbDegs += 1;
  }
  if (ui->SaltAndPepperNoiseCB->isChecked()) {
    nbDegs += 1;
  }
  return nbDegs;
}

void
Assistant::Noise_updateTirageAndTotal()
{
  const int nbDegs = Noise_nbDegradations();
  ui->NbDegNoise->setText(QString::number(nbDegs));

  const bool enabled = (nbDegs > 0);
  ui->NbDegNoise->setEnabled(enabled);
  ui->TirageNoise->setEnabled(enabled);
  ui->TotalNoise->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TirageNoise->setValue(0);

  if (nbDegs > 0 &&
      ui->TirageNoise->value() == 0) {
    //set TirageNoise value to 1 once we have some degradations selected
    ui->TirageNoise->setValue(1);
  }

  const int total =
    nbDegs * _inputImageList.size() * ui->TirageNoise->value();
  ui->TotalNoise->setText(QString::number(total));

  updateTotalPic();
}

void
Assistant::Noise_EnableNoiseOption()
{
  bool enabled = false;
  if (ui->CheckNoise->isChecked()) {
    enabled = true;
    _Noise_noiseEnable = enabled;
    Noise_updateTirageAndTotal();
  }
  else {
    _Noise_noiseEnable = enabled;
    ui->NbDegNoise->setEnabled(enabled);
    ui->TirageNoise->setEnabled(enabled);
    ui->TotalNoise->setEnabled(enabled);

    ui->TirageNoise->setValue(0);
    //signal/slot will call Noise_tirageNoiseChanged(0)
  }

  ui->OptionNoise->setEnabled(enabled);
  ui->GaussianNoiseCB->setEnabled(enabled);
  ui->SpeckleNoiseCB->setEnabled(enabled);
  ui->SaltAndPepperNoiseCB->setEnabled(enabled);

  ui->MinAverageGaussianNoiseSB->setEnabled(enabled);
  ui->MaxAverageGaussianNoiseSB->setEnabled(enabled);
  ui->MinStdDevGaussianNoiseSB->setEnabled(enabled);
  ui->MaxStdDevGaussianNoiseSB->setEnabled(enabled);
  ui->MinAverageSpeckleNoiseSB->setEnabled(enabled);
  ui->MaxAverageSpeckleNoiseSB->setEnabled(enabled);
  ui->MinStdDevSpeckleNoiseSB->setEnabled(enabled);
  ui->MaxStdDevSpeckleNoiseSB->setEnabled(enabled);
  ui->MinAmountSaltAndPepperNoiseSB->setEnabled(enabled);
  ui->MaxAmountSaltAndPepperNoiseSB->setEnabled(enabled);
  ui->MinRatioSaltAndPepperNoiseSB->setEnabled(enabled);
  ui->MaxRatioSaltAndPepperNoiseSB->setEnabled(enabled);
}

void
Assistant::Noise_MethodsChanged()
{
  Noise_updateTirageAndTotal();
}

void
Assistant::Noise_tirageNoiseChanged(int /*nbTirage*/)
{
  const int nbDegs = Noise_nbDegradations();
  ui->NbDegNoise->setText(QString::number(nbDegs));
  const int total = nbDegs * _inputImageList.size() *
                    ui->TirageNoise->value(); //_Noise_nbTirageNoise;
  ui->TotalNoise->setText(QString::number(total));
  updateTotalPic();
}

void
Assistant::Noise_changeMinAverageGaussian(double value)
{
  if (ui->MaxAverageGaussianNoiseSB->value() < value)
    ui->MaxAverageGaussianNoiseSB->setValue(value);
}

void
Assistant::Noise_changeMaxAverageGaussian(double value)
{
  if (ui->MinAverageGaussianNoiseSB->value() > value)
    ui->MinAverageGaussianNoiseSB->setValue(value);
}

void
Assistant::Noise_changeMinStdDevGaussian(double value)
{
  if (ui->MaxStdDevGaussianNoiseSB->value() < value)
    ui->MaxStdDevGaussianNoiseSB->setValue(value);
}

void
Assistant::Noise_changeMaxStdDevGaussian(double value)
{
  if (ui->MinStdDevGaussianNoiseSB->value() > value)
    ui->MinStdDevGaussianNoiseSB->setValue(value);
}

void
Assistant::Noise_changeMinAverageSpeckle(double value)
{
  if (ui->MaxAverageSpeckleNoiseSB->value() < value)
    ui->MaxAverageSpeckleNoiseSB->setValue(value);
}

void
Assistant::Noise_changeMaxAverageSpeckle(double value)
{
  if (ui->MinAverageSpeckleNoiseSB->value() > value)
    ui->MinAverageSpeckleNoiseSB->setValue(value);
}

void
Assistant::Noise_changeMinStdDevSpeckle(double value)
{
  if (ui->MaxStdDevSpeckleNoiseSB->value() < value)
    ui->MaxStdDevSpeckleNoiseSB->setValue(value);
}

void
Assistant::Noise_changeMaxStdDevSpeckle(double value)
{
  if (ui->MinStdDevSpeckleNoiseSB->value() > value)
    ui->MinStdDevSpeckleNoiseSB->setValue(value);
}

void
Assistant::Noise_changeMinAmountSaltAndPepper(double value)
{
  if (ui->MaxAmountSaltAndPepperNoiseSB->value() < value)
    ui->MaxAmountSaltAndPepperNoiseSB->setValue(value);
}

void
Assistant::Noise_changeMaxAmountSaltAndPepper(double value)
{
  if (ui->MinAmountSaltAndPepperNoiseSB->value() > value)
    ui->MinAmountSaltAndPepperNoiseSB->setValue(value);
}

void
Assistant::Noise_changeMinRatioSaltAndPepper(double value)
{
  if (ui->MaxRatioSaltAndPepperNoiseSB->value() < value)
    ui->MaxRatioSaltAndPepperNoiseSB->setValue(value);
}

void
Assistant::Noise_changeMaxRatioSaltAndPepper(double value)
{
  if (ui->MinRatioSaltAndPepperNoiseSB->value() > value)
    ui->MinRatioSaltAndPepperNoiseSB->setValue(value);
}



void
Assistant::Rotation_setupGUIImages()
{
  ui->RotationColorSelectionPB->setColor(qRgb(0,0,0));
}

int
Assistant::Rotation_nbDegradations() const
{
  int nbDegs = 0;
  if (ui->RotationFillColorCB->isChecked()) {
    nbDegs += 1;
  }
  if (ui->RotationFillImageCB->isChecked()) {
    nbDegs += 1;
  }
  if (ui->RotationFillBorderCB->isChecked()) {
    if (ui->RotationBorderReplicateCB->isChecked()) {
      nbDegs += 1;
    }
    if (ui->RotationBorderReflectCB->isChecked()) {
      nbDegs += 1;
    }
    if (ui->RotationBorderWrapCB->isChecked()) {
      nbDegs += 1;
    }
    if (ui->RotationBorderReflect101CB->isChecked()) {
      nbDegs += 1;
    }
  }
  if (ui->RotationFillInpaintCB->isChecked()) {
    if (ui->RotationInpaint1CB->isChecked()) {
      nbDegs += 1;
    }
    if (ui->RotationInpaint2CB->isChecked()) {
      nbDegs += 1;
    }
    if (ui->RotationInpaint3CB->isChecked()) {
      nbDegs += 1;
    }
  }
  return nbDegs;
}


void
Assistant::Rotation_updateTirageAndTotal()
{
  const int nbDegs = Rotation_nbDegradations();
  ui->NbDegRotation->setText(QString::number(nbDegs));

  const bool enabled = (nbDegs > 0);
  ui->NbDegRotation->setEnabled(enabled);
  ui->TirageRotation->setEnabled(enabled);
  ui->TotalRotation->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TirageRotation->setValue(0);

  if (nbDegs > 0 &&
      ui->TirageRotation->value() == 0) {
    //set TirageRotation value to 1 once we have some degradations selected
    ui->TirageRotation->setValue(1);
  }

  const int total =
    nbDegs * _inputImageList.size() * ui->TirageRotation->value();
  ui->TotalRotation->setText(QString::number(total));

  updateTotalPic();  
}

void
Assistant::Rotation_EnableRotationOption()
{
  bool enabled = false;
  if (ui->CheckRotation->isChecked()) {
    enabled = true;
    _Rotation_rotationEnable = enabled;
    Rotation_updateTirageAndTotal();
  }
  else {
    _Rotation_rotationEnable = enabled;
    ui->NbDegRotation->setEnabled(enabled);
    ui->TirageRotation->setEnabled(enabled);
    ui->TotalRotation->setEnabled(enabled);
    
    ui->TirageRotation->setValue(0);
    //signal/slot will call Rotation_tirageRotationChanged(0)
  }

  ui->OptionRotation->setEnabled(enabled);
  ui->RotationAngleMinSB->setEnabled(enabled);
  ui->RotationAngleMinSB->setEnabled(enabled);
  ui->RotationAngleMaxSB->setEnabled(enabled);
  ui->RotationFillColorCB->setEnabled(enabled);
  ui->RotationSpecificColorRB->setEnabled(enabled);
  ui->RotationColorSelectionPB->setEnabled(enabled);
  ui->RotationRandomColorRB->setEnabled(enabled);

  ui->RotationFillImageCB->setEnabled(enabled);

  ui->RotationImageRepeatsMinSB->setEnabled(enabled);
  ui->RotationImageRepeatsMaxSB->setEnabled(enabled);
  ui->RotationFillBorderCB->setEnabled(enabled);
  ui->RotationBorderReplicateCB->setEnabled(enabled);
  ui->RotationBorderReflectCB->setEnabled(enabled);
  ui->RotationBorderWrapCB->setEnabled(enabled);
  ui->RotationBorderReflect101CB->setEnabled(enabled);
  ui->RotationFillInpaintCB->setEnabled(enabled);
  ui->RotationInpaint1CB->setEnabled(enabled);
  ui->RotationInpaint2CB->setEnabled(enabled);
  ui->RotationInpaint3CB->setEnabled(enabled);


  if (enabled == true) {
    if (ui->RotationFillBorderCB->isChecked()) {
      if (! ui->RotationBorderReplicateCB->isChecked() &&
	  ! ui->RotationBorderReflectCB->isChecked() &&
	  ! ui->RotationBorderWrapCB->isChecked() &&
	  ! ui->RotationBorderReflect101CB->isChecked()) {
	ui->RotationBorderReplicateCB->setCheckState(Qt::Checked);
      }
    }
    
    if (ui->RotationFillInpaintCB->isChecked()) {
      if (! ui->RotationInpaint1CB->isChecked() &&
	  ! ui->RotationInpaint2CB->isChecked() &&
	  ! ui->RotationInpaint3CB->isChecked()) {
	ui->RotationInpaint1CB->setCheckState(Qt::Checked);
      }
    }
    
  }
  
}

void
Assistant::Rotation_FillMethodChanged()
{
  Rotation_updateTirageAndTotal();
}

void
Assistant::Rotation_tirageRotationChanged(int /*nbTirage*/)
{
  const int nbDegs = Rotation_nbDegradations();
  ui->NbDegRotation->setText(QString::number(nbDegs));
  const int total = nbDegs * _inputImageList.size() *
                    ui->TirageRotation->value(); //_Rotation_nbTirageRotation;
  ui->TotalRotation->setText(QString::number(total));
  updateTotalPic();
}

void
Assistant::Rotation_changeMinAngle(double value)
{
  if (ui->RotationAngleMaxSB->value() < value)
    ui->RotationAngleMaxSB->setValue(value);
}

void
Assistant::Rotation_changeMaxAngle(double value)
{
  if (ui->RotationAngleMinSB->value() > value)
    ui->RotationAngleMinSB->setValue(value);
}

void
Assistant::Rotation_changeMinRepeats(int value)
{
  if (ui->RotationImageRepeatsMaxSB->value() < value)
    ui->RotationImageRepeatsMaxSB->setValue(value);
}

void
Assistant::Rotation_changeMaxRepeats(int value)
{
  if (ui->RotationImageRepeatsMinSB->value() > value)
    ui->RotationImageRepeatsMinSB->setValue(value);

}




void
Assistant::Shadow_LoadPrevImgShad()
{
  if (!_inputImageList.empty()) {

    int imgIndex = P_bounded_rand(0, _inputImageList.size());
    if (_inputImageList.size() > 1)
      while (imgIndex == _Shadow_indexRecto)
        imgIndex = P_bounded_rand(0, _inputImageList.size());
    assert(imgIndex < _inputImageList.size());

    _Shadow_indexRecto = imgIndex;
    QString path = _inputImageList[imgIndex];
    path = makePath(_PicDirectory, path);
    _Shadow_rectoImgShad.load(path);

    if (!_Shadow_rectoImgShad.isNull()) {

      _Shadow_rectoImgShad =
        _Shadow_rectoImgShad.scaled(SHADOW_PREVIEW_WIDTH,
                                    SHADOW_PREVIEW_HEIGHT,
                                    Qt::KeepAspectRatio,
                                    Qt::FastTransformation);

      //apply current parameters
      Shadow_updatePreviewAll();
    } else {
      std::cout << "Shadow: recto image is null: " << path.toStdString()
                << std::endl;
    }
  }
}

void
Assistant::Shadow_setupGUIImages()
{
  ui->ShadPreviewLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->ShadPreviewLabel->setText(tr("preview"));
  ui->ShadPreviewLabel->setMinimumSize(SHADOW_PREVIEW_WIDTH,
                                       SHADOW_PREVIEW_HEIGHT);
}

void
Assistant::Shadow_setPreview(dc::ShadowBinding::Border border)
{
  const float distanceRatio =
    ui->shad_width->value() / (float)ui->shad_width->maximum();
  const float intensity =
    1.f - ui->shad_intensity->value() / (float)ui->shad_intensity->maximum();
  const float angle = ui->shad_angle->value();

  QImage deg = dc::ShadowBinding::shadowBinding(
    _Shadow_rectoImgShad, distanceRatio, border, intensity, angle);
  ui->ShadPreviewLabel->setPixmap(QPixmap::fromImage(deg));
  ui->ShadPreviewLabel->setMinimumSize(deg.size());
}

//called when a slider is changed
void
Assistant::Shadow_updatePreview()
{
  if (_Shadow_nbShadSelected > 0) {
    Shadow_setPreview(_Shadow_borderStack[_Shadow_nbShadSelected - 1]);
  }
}

//check if a border is selected or reset original preview otherwise.
void
Assistant::Shadow_updatePreviewAll()
{
  if (_Shadow_nbShadSelected > 0) {
    Shadow_setPreview(_Shadow_borderStack[_Shadow_nbShadSelected - 1]);
  } else {
    ui->ShadPreviewLabel->setPixmap(QPixmap::fromImage(_Shadow_rectoImgShad));
    ui->ShadPreviewLabel->setMinimumSize(_Shadow_rectoImgShad.size());
  }
}

void
Assistant::Shadow_selectionChanged()
{
  QCheckBox *cb = qobject_cast<QCheckBox *>(sender());
  assert(cb);
  const bool checked = cb->isChecked();
  dc::ShadowBinding::Border border;
  if (cb == ui->RightShad)
    border = dc::ShadowBinding::Border::RIGHT;
  else if (cb == ui->TopShad)
    border = dc::ShadowBinding::Border::TOP;
  else if (cb == ui->LeftShad)
    border = dc::ShadowBinding::Border::LEFT;
  else //if (cb == ui->BottomShad)
    border = dc::ShadowBinding::Border::BOTTOM;

  if (checked) {
#ifndef NDEBUG
    //check border is not already in stack
    for (int i = 0; i < _Shadow_nbShadSelected; ++i)
      if (_Shadow_borderStack[i] == border)
        assert(false);
#endif //NDEBUG
    //add new border to stack
    assert(_Shadow_nbShadSelected < 4);
    _Shadow_borderStack[_Shadow_nbShadSelected] = border;
    ++_Shadow_nbShadSelected;
    assert(_Shadow_nbShadSelected <= 4);
  } else {
    //remove border from stack
    assert(_Shadow_nbShadSelected > 0);
    int j = 0;
    for (int i = 0; i < _Shadow_nbShadSelected; ++i, ++j)
      if (_Shadow_borderStack[i] == border)
        break;
    assert(0 <= j && j < _Shadow_nbShadSelected);
    for (int i = j + 1; i < _Shadow_nbShadSelected; ++i)
      _Shadow_borderStack[i - 1] = _Shadow_borderStack[i];
    --_Shadow_nbShadSelected;
    assert(_Shadow_nbShadSelected >= 0);
  }

  //update preview
  Shadow_updatePreviewAll();

  //update nb of images to generate
  Shadow_updateTirageAndTotal(); //B: we should do this only when Tab is validated, or when TotalPic is shown !!
}

void
Assistant::Shadow_updateTirageAndTotal()
{
  const int nbDegs = _Shadow_nbShadSelected;
  ui->NbDegShadow->setText(QString::number(nbDegs));

  const bool enabled = (nbDegs > 0);
  ui->NbDegShadow->setEnabled(enabled);
  ui->TirageShadow->setEnabled(enabled);
  ui->TotalShadow->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TirageShadow->setValue(0);

  if (
    nbDegs > 0 &&
    ui->TirageShadow->value() ==
      0) { //set TirageShadow value to 1 once we have some degradations selected
    ui->TirageShadow->setValue(1);
  }

  const int total = nbDegs * _inputImageList.size() * ui->TirageShadow->value();
  ui->TotalShadow->setText(QString::number(total));

  updateTotalPic();
}

void
Assistant::Shadow_EnableShadOption()
{
  bool enabled = false;
  bool switchEnabled = false;
  if (ui->CheckShad->isChecked()) {
    enabled = true;
    switchEnabled = (_inputImageList.size() > 1);
    _Shadow_shadEnable = enabled;
    Shadow_updateTirageAndTotal();
  } else {
    _Shadow_shadEnable = enabled;
    ui->NbDegShadow->setEnabled(enabled);
    ui->TirageShadow->setEnabled(enabled);
    ui->TotalShadow->setEnabled(enabled);

    ui->TirageShadow->setValue(
      0); //signal/slot will call Shadow_tirageShadowChanged(0)
  }

  ui->OptionShad->setEnabled(enabled);
  ui->TopShad->setEnabled(enabled);
  ui->LeftShad->setEnabled(enabled);
  ui->BottomShad->setEnabled(enabled);
  ui->RightShad->setEnabled(enabled);
  ui->SwitchShad->setEnabled(switchEnabled);

  ui->shad_width->setEnabled(enabled);
  ui->shad_intensity->setEnabled(enabled);
  ui->shad_angle->setEnabled(enabled);
}

void
Assistant::Shadow_tirageShadowChanged(int /*value*/)
{
  const int nbDegs = _Shadow_nbShadSelected;
  ui->NbDegShadow->setText(QString::number(nbDegs));
  const int total = nbDegs * _inputImageList.size() * ui->TirageShadow->value();
  ui->TotalShadow->setText(QString::number(total));
  updateTotalPic();
}

void
Assistant::Phantom_LoadPrevImgPhant()
{
  if (!_inputImageList.empty()) {

    int imgIndex = P_bounded_rand(0, _inputImageList.size());
    if (_inputImageList.size() > 1)
      while (_Phantom_indexRecto == imgIndex)
        imgIndex = P_bounded_rand(0, _inputImageList.size());

    _Phantom_indexRecto = imgIndex;

    QString imagePath = _inputImageList[imgIndex];
    imagePath = makePath(_PicDirectory, imagePath);
    _Phantom_rectoImgPhant.load(imagePath);

    if (!_Phantom_rectoImgPhant.isNull()) {

      _Phantom_rectoImgPartPhant = takePart(_Phantom_rectoImgPhant);

      //ui->PhantomPreviewLabel->setPixmap(QPixmap::fromImage(_Phantom_rectoImgPartPhant));
      //ui->PhantomPreviewLabel->setMinimumSize(_Phantom_rectoImgPartPhant.size());

      //apply current parameters
      if (ui->Phantom_VeryFrequent->isChecked()) {
        Phantom_apply(dc::PhantomCharacter::Frequency::VERY_FREQUENT);
      } else if (ui->Phantom_Frequent->isChecked()) {
        Phantom_apply(dc::PhantomCharacter::Frequency::FREQUENT);
      } else if (ui->Phantom_Rare->isChecked()) {
        Phantom_apply(dc::PhantomCharacter::Frequency::RARE);
      } else {
        ui->PhantomPreviewLabel->setPixmap(
          QPixmap::fromImage(_Phantom_rectoImgPartPhant));
        ui->PhantomPreviewLabel->setMinimumSize(
          _Phantom_rectoImgPartPhant.size());
      }

    } else {
      std::cout << "Phantom recto image is null" << std::endl;
    }
  }
}

void
Assistant::Phantom_setupGUIImages()
{
  _PhantomPatternsPath =
    Core::ConfigurationManager::get(AppConfigMainGroup,
                                    AppConfigPhantomPatternsFolderKey)
      .toString();

  ui->PhantomPreviewLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->PhantomPreviewLabel->setText(QStringLiteral("result"));
  ui->PhantomPreviewLabel->setMinimumSize(PHANTOM_PREVIEW_WIDTH,
                                          PHANTOM_PREVIEW_HEIGHT);
}

void
Assistant::Phantom_updateTirageAndTotal()
{
  const int nbDegs = _Phantom_nbPhantSelected;
  ui->NbDegPhantom->setText(QString::number(nbDegs));

  const bool enabled = (nbDegs > 0);
  ui->NbDegPhantom->setEnabled(enabled);
  ui->TiragePhantom->setEnabled(enabled);
  ui->TotalPhantom->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TiragePhantom->setValue(0);

  if (nbDegs > 0 &&
      ui->TiragePhantom->value() == 0) {
    //set TiragePhantom value to 1 once we have some degradations selected
    ui->TiragePhantom->setValue(1);
  }

  const int total =
    nbDegs * _inputImageList.size() * ui->TiragePhantom->value();
  ui->TotalPhantom->setText(QString::number(total));

  updateTotalPic();
}

void
Assistant::Phantom_EnablePhantOption()
{
  bool enabled = false;
  bool switchEnabled = false;
  if (ui->CheckPhant->isChecked()) {
    enabled = true;
    switchEnabled = (_inputImageList.size() > 1);
    _Phantom_phantEnable = enabled;
    Phantom_updateTirageAndTotal();
  }
  else {
    _Phantom_phantEnable = enabled;
    ui->NbDegPhantom->setEnabled(enabled);
    ui->TiragePhantom->setEnabled(enabled);
    ui->TotalPhantom->setEnabled(enabled);

    ui->TiragePhantom->setValue(0);
    //signal/slot will call Phantom_tiragePhantomChanged(0)
  }

  ui->OptionPhant->setEnabled(enabled);
  ui->Phantom_Frequent->setEnabled(enabled);
  ui->Phantom_Rare->setEnabled(enabled);
  ui->Phantom_VeryFrequent->setEnabled(enabled);
  ui->SwitchPhant->setEnabled(switchEnabled);
}

void
Assistant::Phantom_tiragePhantomChanged(int /*value*/)
{
  const int nbDegs = _Phantom_nbPhantSelected;
  ui->NbDegPhantom->setText(QString::number(nbDegs));
  const int total =
    nbDegs * _inputImageList.size() * ui->TiragePhantom->value();
  ui->TotalPhantom->setText(QString::number(total));

  //std::cerr<<"Phantom_tiragePhantomChanged(v) nbDegs="<<nbDegs<<" total="<<total<<"\n";

  updateTotalPic();
}

void
Assistant::Phantom_OptionCheckedPhant()
{
  _Phantom_nbPhantSelected = 0;

  if (ui->Phantom_Rare->isChecked()) {
    ++_Phantom_nbPhantSelected;
  }

  if (ui->Phantom_Frequent->isChecked()) {
    ++_Phantom_nbPhantSelected;
  }

  if (ui->Phantom_VeryFrequent->isChecked()) {
    ++_Phantom_nbPhantSelected;
  }

  Phantom_updatePreview();

  Phantom_updateTirageAndTotal();
}

void
Assistant::Phantom_apply(dc::PhantomCharacter::Frequency frequency)
{
  dc::PhantomCharacterQ phant(
    _Phantom_rectoImgPartPhant,
    frequency,
    _PhantomPatternsPath); //B? should we apply on big image and takePart afterwards ?
  QImage Deg = phant.apply();
  ui->PhantomPreviewLabel->setPixmap(QPixmap::fromImage(Deg));
  ui->PhantomPreviewLabel->setMinimumSize(Deg.size());
}

void
Assistant::Phantom_updatePreview()
{
  //we apply the most frequent checked one to be more visible

  if (ui->Phantom_VeryFrequent->isChecked()) {
    Phantom_apply(dc::PhantomCharacter::Frequency::VERY_FREQUENT);
  } else if (ui->Phantom_Frequent->isChecked()) {
    Phantom_apply(dc::PhantomCharacter::Frequency::FREQUENT);
  } else if (ui->Phantom_Rare->isChecked()) {
    Phantom_apply(dc::PhantomCharacter::Frequency::RARE);
  } else {
    ui->PhantomPreviewLabel->setPixmap(
      QPixmap::fromImage(_Phantom_rectoImgPartPhant));
  }
}

void
Assistant::Blur_OptionCheckedBlur()
{
  _Blur_nbBlurSelected = 0;

  if (ui->CheckPattern1->isChecked()) {
    _Blur_pattern1 = true;
    ++_Blur_nbBlurSelected;
  } else
    _Blur_pattern1 = false;

  if (ui->CheckPattern2->isChecked()) {
    _Blur_pattern2 = true;
    ++_Blur_nbBlurSelected;
  } else
    _Blur_pattern2 = false;

  if (ui->CheckPattern3->isChecked()) {
    _Blur_pattern3 = true;
    ++_Blur_nbBlurSelected;
  } else
    _Blur_pattern3 = false;

  if (ui->CheckPattern4->isChecked()) {
    _Blur_pattern4 = true;
    ++_Blur_nbBlurSelected;
  } else
    _Blur_pattern4 = false;

  if (ui->CheckPattern5->isChecked()) {
    _Blur_pattern5 = true;
    ++_Blur_nbBlurSelected;
  } else
    _Blur_pattern5 = false;

  if (ui->CheckPattern6->isChecked()) {
    _Blur_pattern6 = true;
    ++_Blur_nbBlurSelected;
  } else
    _Blur_pattern6 = false;

  Blur_updateTirageAndTotal();
}

int
Assistant::Blur_nbDegradations() const
{
  int nbDegs = 0;
  if (_Blur_ZoneEnable)
    nbDegs += _Blur_nbBlurSelected;
  if (_Blur_PageEnable)
    nbDegs += 1;
  return nbDegs;
}

void
Assistant::Blur_updateTirageAndTotal()
{
  const int nbDegs = Blur_nbDegradations();
  ui->NbDegBlur->setText(QString::number(nbDegs));

  const bool enabled = (nbDegs > 0);
  ui->NbDegBlur->setEnabled(enabled);
  ui->TirageBlur->setEnabled(enabled);
  ui->TotalBlur->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TirageBlur->setValue(0);

  if (nbDegs > 0 && ui->TirageBlur->value() == 0) {
    ui->TirageBlur->setValue(1);
    //will call Blur_tirageBlurChanged via signal/slot
  } else {
    const int total = nbDegs * _inputImageList.size() *
                      ui->TirageBlur->value(); //_Blur_nbTirageBlur;
    ui->TotalBlur->setText(QString::number(total));
    updateTotalPic();
  }
}

void
Assistant::Blur_tirageBlurChanged(int /*value*/)
{
  //_Blur_nbTirageBlur = value;

  const int nbDegs = Blur_nbDegradations();
  ui->NbDegBlur->setText(QString::number(nbDegs));
  const int total = nbDegs * _inputImageList.size() *
                    ui->TirageBlur->value(); //_Blur_nbTirageBlur;
  ui->TotalBlur->setText(QString::number(total));
  updateTotalPic();
}

static const int BLUR_PATTERN_WIDTH = 101;
static const int BLUR_PATTERN_HEIGHT = 121;

void
Assistant::Blur_LoadPattern()
{
  const QString blurPatternsPath =
    Context::BackgroundContext::instance()->getPath() +
    "../Image/blurImages/blurPatterns/";

  QImage pattern1;
  pattern1.load(blurPatternsPath + "patternArea1.png");
  QImage pattern2;
  pattern2.load(blurPatternsPath + "patternArea2.png");
  QImage pattern3;
  pattern3.load(blurPatternsPath + "patternArea3.png");
  QImage pattern4;
  pattern4.load(blurPatternsPath + "patternArea4.png");
  QImage pattern5;
  pattern5.load(blurPatternsPath + "patternArea5.png");
  QImage pattern6;
  pattern6.load(blurPatternsPath + "patternArea6.png");

  QHBoxLayout *pat1 = ui->PrevPattern1;
  QHBoxLayout *pat2 = ui->PrevPattern2;
  QHBoxLayout *pat3 = ui->PrevPattern3;
  QHBoxLayout *pat4 = ui->PrevPattern4;
  QHBoxLayout *pat5 = ui->PrevPattern5;
  QHBoxLayout *pat6 = ui->PrevPattern6;

  QLabel *patL1 = new QLabel(this);
  QLabel *patL2 = new QLabel(this);
  QLabel *patL3 = new QLabel(this);
  QLabel *patL4 = new QLabel(this);
  QLabel *patL5 = new QLabel(this);
  QLabel *patL6 = new QLabel(this);

  patL1->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  patL2->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  patL3->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  patL4->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  patL5->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  patL6->setFrameStyle(QFrame::Panel | QFrame::Sunken);

  pattern1 = pattern1.scaled(BLUR_PATTERN_WIDTH,
                             BLUR_PATTERN_HEIGHT,
                             Qt::IgnoreAspectRatio,
                             Qt::FastTransformation);
  patL1->setPixmap(QPixmap::fromImage(pattern1));
  pat1->addWidget(patL1);

  pattern2 = pattern2.scaled(BLUR_PATTERN_WIDTH,
                             BLUR_PATTERN_HEIGHT,
                             Qt::IgnoreAspectRatio,
                             Qt::FastTransformation);
  patL2->setPixmap(QPixmap::fromImage(pattern2));
  pat2->addWidget(patL2);

  pattern3 = pattern3.scaled(BLUR_PATTERN_WIDTH,
                             BLUR_PATTERN_HEIGHT,
                             Qt::IgnoreAspectRatio,
                             Qt::FastTransformation);
  patL3->setPixmap(QPixmap::fromImage(pattern3));
  pat3->addWidget(patL3);

  pattern4 = pattern4.scaled(BLUR_PATTERN_WIDTH,
                             BLUR_PATTERN_HEIGHT,
                             Qt::IgnoreAspectRatio,
                             Qt::FastTransformation);
  patL4->setPixmap(QPixmap::fromImage(pattern4));
  pat4->addWidget(patL4);

  pattern5 = pattern5.scaled(BLUR_PATTERN_WIDTH,
                             BLUR_PATTERN_HEIGHT,
                             Qt::IgnoreAspectRatio,
                             Qt::FastTransformation);
  patL5->setPixmap(QPixmap::fromImage(pattern5));
  pat5->addWidget(patL5);

  pattern6 = pattern6.scaled(BLUR_PATTERN_WIDTH,
                             BLUR_PATTERN_HEIGHT,
                             Qt::IgnoreAspectRatio,
                             Qt::FastTransformation);
  patL6->setPixmap(QPixmap::fromImage(pattern6));
  pat6->addWidget(patL6);

  this->updateGeometry();
}

void
Assistant::Blur_EnableBlurOption()
{
  bool enabled = false;
  bool switchEnabled = false;
  if (ui->CheckBlur->isChecked()) {
    enabled = true;
    switchEnabled = (_inputImageList.size() > 1);
    _Blur_blurEnable = enabled;
    Blur_updateTirageAndTotal();
  } else {
    _Blur_blurEnable = enabled;
    ui->NbDegBlur->setEnabled(enabled);
    ui->TirageBlur->setEnabled(enabled);
    ui->TotalBlur->setEnabled(enabled);

    ui->TirageBlur->setValue(
      0); //signal/slot will call Blur_tirageBlurChanged(0)
  }

  ui->BlurZone->setEnabled(enabled);
  ui->BlurPage->setEnabled(enabled);
  ui->BlurMin->setEnabled(enabled);
  ui->BlurMax->setEnabled(enabled);
  ui->SwitchBlur->setEnabled(switchEnabled);
}

void
Assistant::Blur_ByZoneChanged()
{
  bool enabled = false;
  if (ui->BlurZone->isChecked()) {
    enabled = true;
  }
  _Blur_ZoneEnable = enabled;
  ui->CheckPattern1->setEnabled(enabled);
  ui->CheckPattern2->setEnabled(enabled);
  ui->CheckPattern3->setEnabled(enabled);
  ui->CheckPattern4->setEnabled(enabled);
  ui->CheckPattern5->setEnabled(enabled);
  ui->CheckPattern6->setEnabled(enabled);
}

static int
Blur_getIntensityMean(int blurMin, int blurMax)
{
  int intensity = (blurMin + blurMax) / 2;
  while (intensity % 2 == 0)
    ++intensity;
  return intensity;
}

void
Assistant::Blur_PageChanged()
{
  if (ui->BlurPage->isChecked()) {
    int intensity = Blur_getIntensityMean(blurMin(), blurMax());
    std::cout << "Blur: intensity=" << intensity << " in [" << blurMin() << "; "
              << blurMax() << "]" << std::endl;
    QImage blurTmp = dc::BlurFilter::blur(
      _Blur_rectoImgBlur, dc::BlurFilter::Method::GAUSSIAN, intensity);
    ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(blurTmp));
    ui->BlurPreviewLabel->setMinimumSize(blurTmp.size());
    _Blur_PageEnable = true;
  } else {
    Blur_CheckedPattern1();
    _Blur_PageEnable = false;
  }

  Blur_updateTirageAndTotal();
}

void
Assistant::Blur_changeMinBlur(int value)
{
  if (ui->BlurMax->value() < value)
    ui->BlurMax->setSliderPosition(value);

  Blur_PageChanged();
}

void
Assistant::Blur_changeMaxBlur(int value)
{
  if (ui->BlurMin->value() > value)
    ui->BlurMin->setSliderPosition(value);

  Blur_PageChanged();
}

void
Assistant::Blur_LoadPrevImgBlur()
{
  if (!_inputImageList.empty()) {
    int imgIndex = P_bounded_rand(0, _inputImageList.size());
    if (!_inputImageList.empty())
      while (_Blur_indexRecto == imgIndex)
        imgIndex = P_bounded_rand(0, _inputImageList.size());
    assert(imgIndex < _inputImageList.size());
    _Blur_indexRecto = imgIndex;
    QString imagePath = _inputImageList[imgIndex];
    imagePath = makePath(_PicDirectory, imagePath);
    _Blur_rectoImgBlur.load(imagePath);
    //std::cout << ".............  " << imagePath.toStdString() << std::endl;
    if (!_Blur_rectoImgBlur.isNull()) {
      _Blur_rectoImgBlurDeg = _Blur_rectoImgBlur;

      _Blur_rectoImgBlur = _Blur_rectoImgBlur.scaled(BLUR_PREVIEW_WIDTH,
                                                     BLUR_PREVIEW_HEIGHT,
                                                     Qt::KeepAspectRatio,
                                                     Qt::FastTransformation);
      //ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(_Blur_rectoImgBlur));
      //ui->BlurPreviewLabel->setMinimumSize(_Blur_rectoImgBlur.size());

      //apply current parameters
      Blur_PageChanged();

    } else {
      std::cout << "Blur: recto image is null" << std::endl;
    }
  }
}

void
Assistant::Blur_setupGUIImages()
{
  ui->BlurPreviewLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->BlurPreviewLabel->setText(tr("preview"));
  ui->BlurPreviewLabel->setMinimumSize(BLUR_PREVIEW_WIDTH, BLUR_PREVIEW_HEIGHT);
}

void
Assistant::Blur_CheckedPattern1()
{
  if (ui->CheckPattern1->isChecked()) {
    const int intensity = Blur_getIntensityMean(blurMin(), blurMax());
    QImage pattern =
      dc::BlurFilter::makePattern(_Blur_rectoImgBlur,
                                  dc::BlurFilter::Function::LINEAR,
                                  dc::BlurFilter::Area::UP,
                                  1,
                                  50,
                                  50);
    QImage Deg = dc::BlurFilter::applyPattern(
      _Blur_rectoImgBlur, pattern, dc::BlurFilter::Method::GAUSSIAN, intensity);
    ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(Deg));
    ui->BlurPreviewLabel->setMinimumSize(Deg.size());
  } else {
    if (ui->CheckPattern2->isChecked())
      Blur_CheckedPattern2();
    if (ui->CheckPattern3->isChecked())
      Blur_CheckedPattern3();
    if (ui->CheckPattern4->isChecked())
      Blur_CheckedPattern4();
    if (ui->CheckPattern5->isChecked())
      Blur_CheckedPattern5();
    if (ui->CheckPattern6->isChecked())
      Blur_CheckedPattern6();
    else {
      ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(_Blur_rectoImgBlur));
      ui->BlurPreviewLabel->setMinimumSize(_Blur_rectoImgBlur.size());
    }
  }
}

void
Assistant::Blur_CheckedPattern2()
{
  if (ui->CheckPattern2->isChecked()) {
    const int intensity = Blur_getIntensityMean(blurMin(), blurMax());
    QImage pattern = dc::BlurFilter::makePattern(_Blur_rectoImgBlur,
                                                 dc::BlurFilter::Function::LOG,
                                                 dc::BlurFilter::Area::UP,
                                                 1,
                                                 50,
                                                 50);
    QImage Deg = dc::BlurFilter::applyPattern(
      _Blur_rectoImgBlur, pattern, dc::BlurFilter::Method::GAUSSIAN, intensity);
    ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(Deg));
    ui->BlurPreviewLabel->setMinimumSize(Deg.size());
  } else {
    if (ui->CheckPattern1->isChecked())
      Blur_CheckedPattern1();
    if (ui->CheckPattern3->isChecked())
      Blur_CheckedPattern3();
    if (ui->CheckPattern4->isChecked())
      Blur_CheckedPattern4();
    if (ui->CheckPattern5->isChecked())
      Blur_CheckedPattern5();
    if (ui->CheckPattern6->isChecked())
      Blur_CheckedPattern6();
    else {
      ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(_Blur_rectoImgBlur));
      ui->BlurPreviewLabel->setMinimumSize(_Blur_rectoImgBlur.size());
    }
  }
}

void
Assistant::Blur_CheckedPattern3()
{
  if (ui->CheckPattern3->isChecked()) {
    const int intensity = Blur_getIntensityMean(blurMin(), blurMax());
    QImage pattern =
      dc::BlurFilter::makePattern(_Blur_rectoImgBlur,
                                  dc::BlurFilter::Function::PARABOLA,
                                  dc::BlurFilter::Area::UP,
                                  1,
                                  50,
                                  50);
    QImage Deg = dc::BlurFilter::applyPattern(
      _Blur_rectoImgBlur, pattern, dc::BlurFilter::Method::GAUSSIAN, intensity);
    ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(Deg));
    ui->BlurPreviewLabel->setMinimumSize(Deg.size());
  } else {
    if (ui->CheckPattern2->isChecked())
      Blur_CheckedPattern2();
    if (ui->CheckPattern1->isChecked())
      Blur_CheckedPattern1();
    if (ui->CheckPattern4->isChecked())
      Blur_CheckedPattern4();
    if (ui->CheckPattern5->isChecked())
      Blur_CheckedPattern5();
    if (ui->CheckPattern6->isChecked())
      Blur_CheckedPattern6();
    else {
      ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(_Blur_rectoImgBlur));
      ui->BlurPreviewLabel->setMinimumSize(_Blur_rectoImgBlur.size());
    }
  }
}

void
Assistant::Blur_CheckedPattern4()
{
  if (ui->CheckPattern4->isChecked()) {
    const int intensity = Blur_getIntensityMean(blurMin(), blurMax());
    QImage pattern =
      dc::BlurFilter::makePattern(_Blur_rectoImgBlur,
                                  dc::BlurFilter::Function::SINUS,
                                  dc::BlurFilter::Area::UP,
                                  1,
                                  50,
                                  50);
    QImage Deg = dc::BlurFilter::applyPattern(
      _Blur_rectoImgBlur, pattern, dc::BlurFilter::Method::GAUSSIAN, intensity);
    ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(Deg));
    ui->BlurPreviewLabel->setMinimumSize(Deg.size());

  } else {
    if (ui->CheckPattern2->isChecked())
      Blur_CheckedPattern2();
    if (ui->CheckPattern3->isChecked())
      Blur_CheckedPattern3();
    if (ui->CheckPattern1->isChecked())
      Blur_CheckedPattern1();
    if (ui->CheckPattern5->isChecked())
      Blur_CheckedPattern5();
    if (ui->CheckPattern6->isChecked())
      Blur_CheckedPattern6();
    else {
      ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(_Blur_rectoImgBlur));
      ui->BlurPreviewLabel->setMinimumSize(_Blur_rectoImgBlur.size());
    }
  }
}

void
Assistant::Blur_CheckedPattern5()
{
  if (ui->CheckPattern5->isChecked()) {
    const int intensity = Blur_getIntensityMean(blurMin(), blurMax());
    QImage pattern =
      dc::BlurFilter::makePattern(_Blur_rectoImgBlur,
                                  dc::BlurFilter::Function::ELLIPSE,
                                  dc::BlurFilter::Area::UP,
                                  1,
                                  50,
                                  50);
    QImage Deg = dc::BlurFilter::applyPattern(
      _Blur_rectoImgBlur, pattern, dc::BlurFilter::Method::GAUSSIAN, intensity);
    ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(Deg));
    ui->BlurPreviewLabel->setMinimumSize(Deg.size());
  } else {
    if (ui->CheckPattern2->isChecked())
      Blur_CheckedPattern2();
    if (ui->CheckPattern3->isChecked())
      Blur_CheckedPattern3();
    if (ui->CheckPattern4->isChecked())
      Blur_CheckedPattern4();
    if (ui->CheckPattern1->isChecked())
      Blur_CheckedPattern1();
    if (ui->CheckPattern6->isChecked())
      Blur_CheckedPattern6();
    else {
      ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(_Blur_rectoImgBlur));
      ui->BlurPreviewLabel->setMinimumSize(_Blur_rectoImgBlur.size());
    }
  }
}

void
Assistant::Blur_CheckedPattern6()
{
  if (ui->CheckPattern6->isChecked()) {
    const int intensity = Blur_getIntensityMean(blurMin(), blurMax());
    QImage pattern =
      dc::BlurFilter::makePattern(_Blur_rectoImgBlur,
                                  dc::BlurFilter::Function::HYPERBOLA,
                                  dc::BlurFilter::Area::UP,
                                  1,
                                  50,
                                  50);
    QImage Deg = dc::BlurFilter::applyPattern(
      _Blur_rectoImgBlur, pattern, dc::BlurFilter::Method::GAUSSIAN, intensity);
    ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(Deg));
    ui->BlurPreviewLabel->setMinimumSize(Deg.size());
  } else {
    if (ui->CheckPattern2->isChecked())
      Blur_CheckedPattern2();
    if (ui->CheckPattern3->isChecked())
      Blur_CheckedPattern3();
    if (ui->CheckPattern4->isChecked())
      Blur_CheckedPattern4();
    if (ui->CheckPattern5->isChecked())
      Blur_CheckedPattern5();
    if (ui->CheckPattern1->isChecked())
      Blur_CheckedPattern1();
    else {
      ui->BlurPreviewLabel->setPixmap(QPixmap::fromImage(_Blur_rectoImgBlur));
      ui->BlurPreviewLabel->setMinimumSize(_Blur_rectoImgBlur.size());
    }
  }
}

void
Assistant::Hole_EnableHoleOption()
{
  bool enabled = false;
  bool switchEnabled = false;
  if (ui->CheckHole->isChecked()) {
    enabled = true;
    switchEnabled = (_inputImageList.size() > 1);
    _Hole_holeEnable = enabled;
    Hole_updateTirageAndTotal();
  } else {
    _Hole_holeEnable = enabled;
    ui->NbDegHole->setEnabled(enabled);
    ui->TirageHole->setEnabled(enabled);
    ui->TotalHole->setEnabled(enabled);

    ui->TirageHole->setValue(
      0); //signal/slot will call Hole_tirageHoleChanged(0);
  }

  ui->SmallBorder->setEnabled(enabled);
  ui->MediumBorder->setEnabled(enabled);
  ui->BigBorder->setEnabled(enabled);
  ui->SmallCorner->setEnabled(enabled);
  ui->MediumCorner->setEnabled(enabled);
  ui->BigCorner->setEnabled(enabled);
  ui->SmallCenter->setEnabled(enabled);
  ui->MediumCenter->setEnabled(enabled);
  ui->BigCenter->setEnabled(enabled);
  ui->HoleCenterMinNb->setEnabled(enabled);
  ui->HoleCenterMaxNb->setEnabled(enabled);
  ui->HoleCornerMinNb->setEnabled(enabled);
  ui->HoleCornerMaxNb->setEnabled(enabled);
  ui->HoleBorderMinNb->setEnabled(enabled);
  ui->HoleBorderMaxNb->setEnabled(enabled);
  ui->HoleColorButton->setEnabled(enabled);
  ui->SwitchHole->setEnabled(switchEnabled);
}

void
Assistant::Hole_changeMinNbCenterHole(int value)
{
  if (ui->HoleCenterMaxNb->value() < value)
    ui->HoleCenterMaxNb->setSliderPosition(value);
  Hole_updatePreview();
}

void
Assistant::Hole_changeMaxNbCenterHole(int value)
{
  if (ui->HoleCenterMinNb->value() > value)
    ui->HoleCenterMinNb->setSliderPosition(value);
  Hole_updatePreview();
}

void
Assistant::Hole_changeMinNbCornerHole(int value)
{
  if (ui->HoleCornerMaxNb->value() < value)
    ui->HoleCornerMaxNb->setValue(value);
  Hole_updatePreview();
}

void
Assistant::Hole_changeMaxNbCornerHole(int value)
{
  if (ui->HoleCornerMinNb->value() > value)
    ui->HoleCornerMinNb->setValue(value);
  Hole_updatePreview();
}

void
Assistant::Hole_changeMinNbBorderHole(int value)
{
  if (ui->HoleBorderMaxNb->value() < value)
    ui->HoleBorderMaxNb->setValue(value);
  Hole_updatePreview();
}

void
Assistant::Hole_changeMaxNbBorderHole(int value)
{
  if (ui->HoleBorderMinNb->value() > value)
    ui->HoleBorderMinNb->setValue(value);
  Hole_updatePreview();
}

void
Assistant::Hole_OptionCheckedHole()
{
  _Hole_nbHoleBorderSelected = 0;
  _Hole_nbHoleCenterSelected = 0;
  _Hole_nbHoleCornerSelected = 0;

  if (ui->SmallBorder->isChecked()) {
    _Hole_smallHoleBorder = true;
    ++_Hole_nbHoleBorderSelected;
  } else
    _Hole_smallHoleBorder = false;

  if (ui->MediumBorder->isChecked()) {
    _Hole_mediumHoleBorder = true;
    ++_Hole_nbHoleBorderSelected;
  } else
    _Hole_mediumHoleBorder = false;

  if (ui->BigBorder->isChecked()) {
    _Hole_bigHoleBorder = true;
    ++_Hole_nbHoleBorderSelected;
  } else
    _Hole_bigHoleBorder = false;

  if (ui->SmallCorner->isChecked()) {
    _Hole_smallHoleCorner = true;
    ++_Hole_nbHoleCornerSelected;
  } else
    _Hole_smallHoleCorner = false;

  if (ui->MediumCorner->isChecked()) {
    _Hole_mediumHoleCorner = true;
    ++_Hole_nbHoleCornerSelected;
  } else
    _Hole_mediumHoleCorner = false;

  if (ui->BigCorner->isChecked()) {
    _Hole_bigHoleCorner = true;
    ++_Hole_nbHoleCornerSelected;
  } else
    _Hole_bigHoleCorner = false;

  if (ui->SmallCenter->isChecked()) {
    _Hole_smallHoleCenter = true;
    ++_Hole_nbHoleCenterSelected;
  } else
    _Hole_smallHoleCenter = false;

  if (ui->MediumCenter->isChecked()) {
    _Hole_mediumHoleCenter = true;
    ++_Hole_nbHoleCenterSelected;
  } else
    _Hole_mediumHoleCenter = false;

  if (ui->BigCenter->isChecked()) {
    _Hole_bigHoleCenter = true;
    ++_Hole_nbHoleCenterSelected;
  } else
    _Hole_bigHoleCenter = false;

  _Hole_nbHoleSelected = _Hole_nbHoleBorderSelected +
                         _Hole_nbHoleCenterSelected +
                         _Hole_nbHoleCornerSelected;

  /*
  int majBilanHole = 0;
  const int nbPic = _inputImageList.size();

  majBilanHole += ui->TirageCorner->value() * _Hole_nbHoleCornerSelected * nbPic;
  majBilanHole += ui->TirageBorder->value() * _Hole_nbHoleBorderSelected * nbPic;
  majBilanHole += ui->TirageHole->value() * _Hole_nbHoleCenterSelected * nbPic;

  ui->TotalHole->setText(QString::number(majBilanHole));
  updateTotalPic();
  */
  Hole_updateTirageAndTotal();
}

void
Assistant::Hole_updateTirageAndTotal()
{
  const int nbDegs = (_Hole_nbHoleSelected != 0 ? 1 : 0);

  ui->NbDegHole->setText(QString::number(nbDegs));

  const bool enabled = (nbDegs > 0);
  ui->NbDegHole->setEnabled(enabled);
  ui->TirageHole->setEnabled(enabled);
  ui->TotalHole->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TirageHole->setValue(0);

  if (nbDegs > 0 && ui->TirageHole->value() == 0) {
    ui->TirageHole->setValue(1);
    //will call Hole_tirageHoleChanged via signal/slot
  } else {
    const int total = nbDegs * _inputImageList.size() * ui->TirageHole->value();
    ui->TotalHole->setText(QString::number(total));
    updateTotalPic();
  }
}

void
Assistant::Hole_setBackgroundColor(const QColor &color)
{
  assert(ui);
  assert(ui->colorLabel);

  ui->colorLabel->setAutoFillBackground(true);
  QString values = QString::number(color.red()) + ", " +
                   QString::number(color.green()) + ", " +
                   QString::number(color.blue()) + ", " +
                   QString::number(color.alpha());

  ui->colorLabel->setStyleSheet("QLabel { background-color: rgba(" + values +
                                "); }");

  bool visible = true;
  if (color.alpha() == 0)
    visible = false;
  ui->colorLabel->setVisible(visible);

  _Hole_colorBehind = color;
}

void
Assistant::Hole_chooseColor()
{
  QColor color = QColorDialog::getColor(Qt::black,
                                        this,
                                        "Choose the color of background",
                                        QColorDialog::ShowAlphaChannel);

  Hole_setBackgroundColor(color);

  Hole_updatePreview();
}

void
Assistant::Hole_tirageHoleChanged(int /*value*/)
{
  const int nbDegs = (_Hole_nbHoleSelected != 0 ? 1 : 0);
  ui->NbDegHole->setText(QString::number(nbDegs));

  const bool enabled = (nbDegs > 0);
  ui->NbDegHole->setEnabled(enabled);
  ui->TirageHole->setEnabled(enabled);
  ui->TotalHole->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TirageHole->setValue(0);

  if (nbDegs > 0 &&
      ui->TirageHole->value() ==
        0) { //set TirageHole value to 1 once we have some degradations selected
    ui->TirageHole->setValue(1);
  }

  const int total = nbDegs * _inputImageList.size() * ui->TirageHole->value();
  ui->TotalHole->setText(QString::number(total));

  updateTotalPic();
}

/*
void Assistant::Hole_tirageCornerChanged(int value)
{
  _Hole_nbHoleCorner = value;
  int majBilanHole = 0;
  int nbPic = _inputImageList.size();

  majBilanHole += ui->TirageCorner->value() * _Hole_nbHoleCornerSelected * nbPic;
  majBilanHole += ui->TirageBorder->value() * _Hole_nbHoleBorderSelected * nbPic;
  majBilanHole += ui->TirageHole->value() * _Hole_nbHoleCenterSelected * nbPic;

  ui->TotalHole->setText(QString::number(majBilanHole));
  updateTotalPic();
}

void Assistant::Hole_tirageCenterChanged(int value)
{
  _Hole_nbHoleCenter = value;
  int majBilanHole = 0;
  int nbPic = _inputImageList.size();

  majBilanHole += ui->TirageCorner->value() * _Hole_nbHoleCornerSelected * nbPic;
  majBilanHole += ui->TirageBorder->value() * _Hole_nbHoleBorderSelected * nbPic;
  majBilanHole += ui->TirageHole->value() * _Hole_nbHoleCenterSelected * nbPic;

  ui->TotalHole->setText(QString::number(majBilanHole));
  updateTotalPic();
}
*/

void
Assistant::Hole_LoadPrevImgHole()
{
  if (!_inputImageList.empty()) {

    int imgIndex = P_bounded_rand(0, _inputImageList.size());
    if (_inputImageList.size() > 1)
      while (imgIndex == _Hole_indexRecto)
        imgIndex = P_bounded_rand(0, _inputImageList.size());

    _Hole_indexRecto = imgIndex;

    QString imagePath = _inputImageList[imgIndex];
    imagePath = makePath(_PicDirectory, imagePath);
    _Hole_rectoImgHole.load(imagePath);
    //std::cout << ".............  " << imagePath.toStdString() << std::endl;
    if (!_Hole_rectoImgHole.isNull()) {
      _Hole_ImgHoleOriginal = _Hole_rectoImgHole;
      _Hole_rectoImgHole = _Hole_rectoImgHole.scaled(HOLE_PREVIEW_WIDTH,
                                                     HOLE_PREVIEW_HEIGHT,
                                                     Qt::KeepAspectRatio,
                                                     Qt::FastTransformation);

      //apply current parameters
      Hole_updatePreview();

    } else {
      std::cout << "Hole: recto image is null" << std::endl;
    }
  }
}

void
Assistant::Hole_setupGUIImages()
{
  ui->HolePreviewLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->HolePreviewLabel->setText(tr("preview"));
  ui->HolePreviewLabel->setMinimumSize(HOLE_PREVIEW_WIDTH, HOLE_PREVIEW_HEIGHT);

  ui->HoleColorButton->setToolTip(tr("color of background"));
}

namespace {

struct ImageAreaSorter
{
  explicit ImageAreaSorter(const std::vector<size_t> &areas)
    : m_areas(areas)
  {}

  bool operator()(int i, int j) const
  {
    assert(i >= 0 && i < (int)m_areas.size());
    assert(j >= 0 && j < (int)m_areas.size());

    return m_areas[i] < m_areas[j];
  }

private:
  const std::vector<size_t> &m_areas;
};

} //end anonymous namespace

/*
  sort @a filenames in increasing area order.
*/
static void
sortImageFilenamesByImageArea(QStringList &filenames)
{
  const size_t INVALID_AREA =
    -1; //must be the greatest possible area to be last when sorted
  const int sz = filenames.count();
  std::vector<int> indices(sz);
  for (int i = 0; i < sz; ++i) {
    indices[i] = i;
  }
  std::vector<size_t> areas(sz);
  for (int i = 0; i < sz; ++i) {
    const QString &filename = filenames[i];
    const QImage image(filename);
    size_t area = INVALID_AREA;
    if (!image.isNull())
      area = image.width() * static_cast<size_t>(image.height());
    areas[i] = area;
  }

  assert(areas.size() == indices.size());

  std::sort(indices.begin(), indices.end(), ImageAreaSorter(areas));

  int nbRemove = 0;
  QStringList tmp = filenames;
  for (int i = 0; i < sz; ++i) {
    if (areas[i] == INVALID_AREA)
      ++nbRemove;
    assert(indices[i] < tmp.size());
    filenames[i] = tmp[indices[i]];
  }

  while (nbRemove > 0) {
    assert(!filenames.isEmpty());
    filenames.removeLast();
    --nbRemove;
  }
}

void
Assistant::Hole_LoadHolePattern()
{
  //B:TODO:UGLY: paths hard-coded !!!

  const QString path = Context::BackgroundContext::instance()->getPath() +
                       "../Image/holePatterns/";
  const QString centerHolesPath = path + "centerHoles";
  const QString borderHolesPath = path + "borderHoles";
  const QString cornerHolesPath = path + "cornerHoles";

  QDir directoryCenter(centerHolesPath);
  QDir directoryBorder(borderHolesPath);
  QDir directoryCorner(cornerHolesPath);

  const QStringList nameFilters = getReadImageFilterList();

  QStringList centerPatterns =
    directoryCenter.entryList(nameFilters, QDir::Files | QDir::Readable);
  _Hole_CenterHoles.reserve(centerPatterns.size());
  for (const QString &filename : centerPatterns)
    _Hole_CenterHoles.append(directoryCenter.absoluteFilePath(filename));
  sortImageFilenamesByImageArea(_Hole_CenterHoles);

  QStringList cornerPatterns =
    directoryCorner.entryList(nameFilters, QDir::Files | QDir::Readable);
  _Hole_CornerHoles.reserve(cornerPatterns.size());
  for (const QString &filename : cornerPatterns)
    _Hole_CornerHoles.append(directoryCorner.absoluteFilePath(filename));
  sortImageFilenamesByImageArea(_Hole_CornerHoles);

  QStringList borderPatterns =
    directoryBorder.entryList(nameFilters, QDir::Files | QDir::Readable);
  _Hole_BorderHoles.reserve(borderPatterns.size());
  for (const QString &filename : borderPatterns)
    _Hole_BorderHoles.append(directoryBorder.absoluteFilePath(filename));
  sortImageFilenamesByImageArea(_Hole_BorderHoles);

  /*
#ifndef NDEBUG
    qDebug()<<"my sorted list:";
    for (const QString &file : _Hole_BorderHoles) {
    QImage img(file);
    qDebug()<<file<<" "<<img.width()*img.height();
    }
#endif //NDEBUG
  */
}

struct HoleData
{
  QString patternFilename;
  int posX;
  int posY;
  int size;
  dc::HoleDegradation::HoleType type;
  int side;
  QColor color;

  explicit HoleData(
    const QString &pattern = QString(),
    int pX = 0,
    int pY = 0,
    int pSize = 0,
    dc::HoleDegradation::HoleType pType = dc::HoleDegradation::HoleType::CENTER,
    int pSide = 0,
    QColor pColor = QColor())
    : patternFilename(pattern)
    , posX(pX)
    , posY(pY)
    , size(pSize)
    , type(pType)
    , side(pSide)
    , color(pColor)
  {}
};

//const int ind = P_bounded_rand(minIndex, maxIndex);

HoleData
Hole_degradateImageRandom(QImage &degImg,
                          const QStringList &holePatterns,
                          int ind,
                          float scaleW,
                          float scaleH,
                          bool randPosX,
                          bool randPosY,
                          dc::HoleDegradation::HoleType type,
                          const QColor &color,
                          int side = 0)
{
  HoleData result;

  assert(ind < holePatterns.size());
  const QString filename = holePatterns[ind];
  QImage holePattern(filename);

  if (!holePattern.isNull()) {

    if (scaleW != 1.f && scaleH != 1.f) {
      const int holeWidth = static_cast<int>(holePattern.width() * scaleW);
      const int holeHeight = static_cast<int>(holePattern.height() * scaleH);
      holePattern = holePattern.scaled(
        holeWidth, holeHeight, Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    assert(!holePattern.isNull());

    const float ratioOutside = 0.3f; //must be in [0; 1.0]
    // 0.6 means 60% of pattern can be outside
    // With ratio=1 we often have pattern completely outside

    const int size = 0;

    const cv::Point pos = dc::HoleDegradation::getRandomPosition(
      cv::Size(degImg.width(), degImg.height()),
      cv::Size(holePattern.width(), holePattern.height()),
      type,
      ratioOutside,
      side);
    int posX = 0;
    if (randPosX)
      posX = pos.x;
    int posY = 0;
    if (randPosY)
      posY = pos.y;

    result = HoleData(filename, posX, posY, size, type, side, color);

    degImg = dc::HoleDegradation::holeDegradation(
      degImg, holePattern, posX, posY, size, type, side, color);

  } else {
    qDebug() << "ERROR: unable to load hole file: " << filename;
    return HoleData();
  }

  return result;
}

/*
static
int
Hole_getRandomNbOfHoles(int holeMin, int holeMax)
{
  int nbHole = holeMax;
  if (holeMax != holeMin)
    nbHole = P_bounded_rand(holeMin, holeMax);
  return nbHole;
}
*/

// get @a N random values in [@a minV; @a maxV[
// There may be duplicates
static std::vector<int>
getNRandom(int N, int minV, int maxV)
{
  assert(minV < maxV);
  assert(N >= 0);

  std::vector<int> res;
  res.reserve(N);
  while (res.size() < (size_t)N) {
    int r = P_bounded_rand(minV, maxV);
    res.push_back(r);
  }
  return res;
}

// get @a N different/uniq random values in [@a minV; @a maxV[
// Suppose that range is large enough !
// output indices are NOT sorted !!
static std::vector<int>
getNRandomUniq(int N, int minV, int maxV)
{
  assert(minV < maxV);
  assert(N <= maxV - minV);
  assert(N >= 0);

  std::vector<int> res;
  res.reserve(N);
  while (res.size() < (size_t)N) {
    int r = P_bounded_rand(minV, maxV);
    bool found = false;
    const size_t sz = res.size();
    for (size_t j = 0; j < sz; ++j)
      if (res[j] == r) {
        found = true;
        break;
      }
    if (!found)
      res.push_back(r);
  }
  return res;
}

// get @a N different random values in [@a minV; @a maxV[ if possible
// If range is not large enough, we will have duplicates, but we try to minimize
// them.
static std::vector<int>
getNRandomUniqIfPossible(int N, int minV, int maxV)
{
  assert(minV < maxV);
  assert(N >= 0);

  if (N <= maxV - minV) {
    return getNRandomUniq(N, minV, maxV);
  }

  //N > maxV-minV

  const int d = N / (maxV - minV);
  const int r = N - d * (maxV - minV);

  //each value in [minV, maxV[ will be present at least d times
  // and r more values will be present one more time

  std::vector<int> res;
  res.reserve(N);
  for (int i = 0; i < d; ++i) {
    for (int j = minV; j < maxV; ++j)
      res.push_back(j);
  }
  const std::vector<int> resTmp = getNRandomUniq(r, minV, maxV);
  const size_t sz = resTmp.size();
  for (size_t i = 0; i < sz; ++i)
    res.push_back(resTmp[i]);

  return res;
}

/*
  Get a vector with number of holes by hole size, such as sum of number of holes is equal to @a nbHoles.
 */
std::vector<int>
getNumberOfHolesBySize(int nbHoles,
                       bool withSmall,
                       bool withMedium,
                       bool withBig)
{
  std::vector<int> res(3, 0);

  int nbSelected =
    (withSmall ? 1 : 0) + (withMedium ? 1 : 0) + (withBig ? 1 : 0);
  if (nbSelected == 0) {
    //do nothing
    //prevent to get in the last else
  }
  if (nbSelected == 1) {
    if (withSmall)
      res[0] = nbHoles;
    else if (withMedium)
      res[1] = nbHoles;
    else
      res[2] = nbHoles;
  } else if (nbSelected == 2) {
    const int firstHalf = nbHoles / 2;
    const int secondHalf = nbHoles - firstHalf;
    if (withSmall) {
      res[0] = firstHalf;
      if (withMedium)
        res[1] = secondHalf;
      else
        res[2] = secondHalf;
    } else if (withMedium) {
      res[1] = nbHoles / 2;
      assert(withBig);
      res[2] = secondHalf;
    }
  } else {
    assert(nbSelected == 3);
    const int third = nbHoles / 3;
    const int rest = nbHoles - third - third;
    res[0] = third;
    res[1] = third;
    res[2] = rest;
  }

#ifndef NDEBUG
  int sum = 0;
  for (int i = 0; i < 3; ++i)
    sum += res[i];
  assert(sum == nbHoles);
#endif //NDEBUG

  return res;
}

std::vector<HoleData>
Assistant::Hole_doHoles(QImage &recto,
                        float scaleW,
                        float scaleH,
                        const QColor &color,
                        bool randPosX,
                        bool randPosY) const
{
  std::vector<HoleData> holes;
  holes.reserve(20); //arbitrary

  //do corner holes
  if (this->nbHoleCornerSelected() > 0) {

    const QStringList &patterns = _Hole_CornerHoles;
    const dc::HoleDegradation::HoleType type =
      dc::HoleDegradation::HoleType::CORNER;

    const int nbPatterns = patterns.size();
    const int minNb = ui->HoleCornerMinNb->value();
    const int maxNb = std::max(ui->HoleCornerMaxNb->value(), minNb + 1);

    const int nbHoles = P_bounded_rand(minNb, maxNb);
    std::vector<int> sides =
      getNRandomUniq(nbHoles, 0, 4); //At most we can draw 4 corners.
    assert(sides.size() == (size_t)nbHoles);

    //std::cerr << "nb corner holes=" << sides.size() << "\n";

    std::vector<int> nbHolesBySize = getNumberOfHolesBySize(
      nbHoles, smallHoleCorner(), mediumHoleCorner(), bigHoleCorner());
    assert(nbHolesBySize.size() == 3);
    size_t k = 0;
    for (int i = 0; i < 3; ++i) {

      if (nbHolesBySize[i] > 0) {
        int minIndex = 0;
        int maxIndex = nbPatterns;
        switch (i) {
          case 0: //small
            minIndex = 0;
            maxIndex = nbPatterns / 3;
            break;
          case 1: //medium
            minIndex = nbPatterns / 3;
            maxIndex = 2 * nbPatterns / 3;
            break;
          case 2: //big
          default:
            minIndex = 2 * nbPatterns / 3;
            maxIndex = nbPatterns;
            break;
        }

        std::vector<int> indices =
          getNRandomUniqIfPossible(nbHolesBySize[i], minIndex, maxIndex);

        for (int ind : indices) {
          assert(k < sides.size());

          //std::cerr << "size " << i << " side=" << sides[k] << "\n";

          HoleData hole = Hole_degradateImageRandom(recto,
                                                    patterns,
                                                    ind,
                                                    scaleW,
                                                    scaleH,
                                                    randPosX,
                                                    randPosY,
                                                    type,
                                                    color,
                                                    sides[k]);
          holes.push_back(hole);
          ++k;
        }
      }
    }

    //qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  }

  //do border holes
  if (this->nbHoleBorderSelected() > 0) {

    const QStringList &patterns = _Hole_BorderHoles;
    const dc::HoleDegradation::HoleType type =
      dc::HoleDegradation::HoleType::BORDER;

    const int nbPatterns = patterns.size();
    const int minNb = ui->HoleBorderMinNb->value();
    const int maxNb = std::max(ui->HoleBorderMaxNb->value(), minNb + 1);

    const int nbHoles = P_bounded_rand(minNb, maxNb);
    std::vector<int> sides = getNRandom(
      nbHoles,
      0,
      4); //we allow duplicates (we can have several defects on same border)
    assert((size_t)nbHoles == sides.size());

    //std::cerr << "nbHoles borders=" << nbHoles << "\n";

    std::vector<int> nbHolesBySize = getNumberOfHolesBySize(
      nbHoles, smallHoleBorder(), mediumHoleBorder(), bigHoleBorder());
    assert(nbHolesBySize.size() == 3);
    size_t k = 0;
    for (int i = 0; i < 3; ++i) {

      if (nbHolesBySize[i] > 0) {

        int minIndex = 0;
        int maxIndex = nbPatterns;

        switch (i) {
          case 0: //small
            minIndex = 0;
            maxIndex = nbPatterns / 3;
            break;
          case 1: //medium
            minIndex = nbPatterns / 3;
            maxIndex = 2 * nbPatterns / 3;
            break;
          case 2: //big
          default:
            minIndex = 2 * nbPatterns / 3;
            maxIndex = nbPatterns;
            break;
        }

        std::vector<int> indices =
          getNRandomUniqIfPossible(nbHolesBySize[i], minIndex, maxIndex);

        for (int ind : indices) {
          assert(k < sides.size());
          //std::cerr << " border side=" << sides[k] << "\n";
          HoleData hole = Hole_degradateImageRandom(recto,
                                                    patterns,
                                                    ind,
                                                    scaleW,
                                                    scaleH,
                                                    randPosX,
                                                    randPosY,
                                                    type,
                                                    color,
                                                    sides[k]);
          holes.push_back(hole);
          ++k;
        }
      }
    }

    //qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  }

  //do center holes
  if (this->nbHoleCenterSelected() > 0) {

    const QStringList &patterns = _Hole_CenterHoles;
    const dc::HoleDegradation::HoleType type =
      dc::HoleDegradation::HoleType::CENTER;

    const int nbPatterns = patterns.size();
    const int minNb = this->holeMin();
    const int maxNb = std::max(this->holeMax(), minNb + 1);

    const int nbHoles = P_bounded_rand(minNb, maxNb);

    std::vector<int> nbHolesBySize = getNumberOfHolesBySize(
      nbHoles, smallHoleCenter(), mediumHoleCenter(), bigHoleCenter());
    assert(nbHolesBySize.size() == 3);
    for (int i = 0; i < 3; ++i) {

      if (nbHolesBySize[i] > 0) {

        int minIndex = 0;
        int maxIndex = nbPatterns;

        switch (i) {
          case 0: //small
            minIndex = 0;
            maxIndex = nbPatterns / 3;
            break;
          case 1: //medium
            minIndex = nbPatterns / 3;
            maxIndex = 2 * nbPatterns / 3;
            break;
          case 2: //big
          default:
            minIndex = 2 * nbPatterns / 3;
            maxIndex = nbPatterns;
            break;
        }

        std::vector<int> indices =
          getNRandomUniqIfPossible(nbHolesBySize[i], minIndex, maxIndex);

        for (int ind : indices) {
          HoleData hole = Hole_degradateImageRandom(recto,
                                                    patterns,
                                                    ind,
                                                    scaleW,
                                                    scaleH,
                                                    randPosX,
                                                    randPosY,
                                                    type,
                                                    color);
          holes.push_back(hole);
        }
      }
    }

    //qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  }

  return holes;
}

void
Assistant::Hole_updatePreview()
{
  Hole_OptionCheckedHole();

  QImage imgDeg = _Hole_rectoImgHole.copy();

  const float scaleW =
    HOLE_PREVIEW_WIDTH / static_cast<float>(_Hole_ImgHoleOriginal.width());
  const float scaleH =
    HOLE_PREVIEW_HEIGHT / static_cast<float>(_Hole_ImgHoleOriginal.height());

  const QColor color = _Hole_colorBehind;

  const bool randPosX = true;
  const bool randPosY = true;

  Hole_doHoles(imgDeg, scaleW, scaleH, color, randPosX, randPosY);

  ui->HolePreviewLabel->setPixmap(QPixmap::fromImage(imgDeg));
  ui->HolePreviewLabel->setMinimumSize(imgDeg.size());
}



void
Assistant::ElasticDeformation_setupGUIImages()
{
  if (ui->BorderElasticDeformation1CB->count() == 0) {
    ui->BorderElasticDeformation1CB->addItem(tr("Constant"),
					   QVariant(static_cast<int>(cv::BORDER_CONSTANT)));
    ui->BorderElasticDeformation1CB->addItem(tr("Replicate"),
					   QVariant(static_cast<int>(cv::BORDER_REPLICATE)));
    ui->BorderElasticDeformation1CB->addItem(tr("Reflect"),
					   QVariant(static_cast<int>(cv::BORDER_REFLECT)));
    ui->BorderElasticDeformation1CB->addItem(tr("Wrap"),
					   QVariant(static_cast<int>(cv::BORDER_WRAP)));
    ui->BorderElasticDeformation1CB->addItem(tr("Reflect101"),
					   QVariant(static_cast<int>(cv::BORDER_REFLECT_101)));
    ui->BorderElasticDeformation1CB->setCurrentIndex(0);
  }

  if (ui->InterpolationElasticDeformation1CB->count() == 0) {
    ui->InterpolationElasticDeformation1CB->addItem(tr("Nearest"),
						  QVariant(static_cast<int>(cv::INTER_NEAREST)));
    ui->InterpolationElasticDeformation1CB->addItem(tr("Linear"),
						  QVariant(static_cast<int>(cv::INTER_LINEAR)));
    ui->InterpolationElasticDeformation1CB->addItem(tr("Area"),
					   QVariant(static_cast<int>(cv::INTER_AREA)));
    ui->InterpolationElasticDeformation1CB->addItem(tr("Cubic"),
					   QVariant(static_cast<int>(cv::INTER_CUBIC)));
    ui->InterpolationElasticDeformation1CB->addItem(tr("Lanczos4"),
					   QVariant(static_cast<int>(cv::INTER_LANCZOS4)));
    ui->InterpolationElasticDeformation1CB->setCurrentIndex(1);
  }

  if (ui->BorderElasticDeformation2CB->count() == 0) {
    ui->BorderElasticDeformation2CB->addItem(tr("Constant"),
					   QVariant(static_cast<int>(cv::BORDER_CONSTANT)));
    ui->BorderElasticDeformation2CB->addItem(tr("Replicate"),
					   QVariant(static_cast<int>(cv::BORDER_REPLICATE)));
    ui->BorderElasticDeformation2CB->addItem(tr("Reflect"),
					   QVariant(static_cast<int>(cv::BORDER_REFLECT)));
    ui->BorderElasticDeformation2CB->addItem(tr("Wrap"),
					   QVariant(static_cast<int>(cv::BORDER_WRAP)));
    ui->BorderElasticDeformation2CB->addItem(tr("Reflect101"),
					   QVariant(static_cast<int>(cv::BORDER_REFLECT_101)));
    ui->BorderElasticDeformation2CB->setCurrentIndex(0);
  }

  if (ui->InterpolationElasticDeformation2CB->count() == 0) {
    ui->InterpolationElasticDeformation2CB->addItem(tr("Nearest"),
						  QVariant(static_cast<int>(cv::INTER_NEAREST)));
    ui->InterpolationElasticDeformation2CB->addItem(tr("Linear"),
						  QVariant(static_cast<int>(cv::INTER_LINEAR)));
    ui->InterpolationElasticDeformation2CB->addItem(tr("Area"),
					   QVariant(static_cast<int>(cv::INTER_AREA)));
    ui->InterpolationElasticDeformation2CB->addItem(tr("Cubic"),
					   QVariant(static_cast<int>(cv::INTER_CUBIC)));
    ui->InterpolationElasticDeformation2CB->addItem(tr("Lanczos4"),
					   QVariant(static_cast<int>(cv::INTER_LANCZOS4)));
    ui->InterpolationElasticDeformation2CB->setCurrentIndex(1);
  }

}

int
Assistant::ElasticDeformation_nbDegradations() const
{
  int nbDegs = 0;
  if (ui->ElasticDeformation1CB->isChecked()) {
    nbDegs += 1;
  }
  if (ui->ElasticDeformation2CB->isChecked()) {
    nbDegs += 1;
  }
  return nbDegs;
}

void
Assistant::ElasticDeformation_updateTirageAndTotal()
{
  const int nbDegs = ElasticDeformation_nbDegradations();
  ui->NbDegElasticDeformation->setText(QString::number(nbDegs));

  const bool enabled = (nbDegs > 0);
  ui->NbDegElasticDeformation->setEnabled(enabled);
  ui->TirageElasticDeformation->setEnabled(enabled);
  ui->TotalElasticDeformation->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TirageElasticDeformation->setValue(0);

  if (nbDegs > 0 &&
      ui->TirageElasticDeformation->value() == 0) {
    //set TirageElasticDeformation value to 1 once we have some degradations selected
    ui->TirageElasticDeformation->setValue(1);
  }

  const int total =
    nbDegs * _inputImageList.size() * ui->TirageElasticDeformation->value();
  ui->TotalElasticDeformation->setText(QString::number(total));

  updateTotalPic();
}

void
Assistant::ElasticDeformation_EnableElasticOption()
{
  bool enabled = false;
  if (ui->CheckElastic->isChecked()) {
    enabled = true;
    _ElasticDeformation_elasticEnable = enabled;
    ElasticDeformation_updateTirageAndTotal();
  }
  else {
    _ElasticDeformation_elasticEnable = enabled;
    ui->NbDegElasticDeformation->setEnabled(enabled);
    ui->TirageElasticDeformation->setEnabled(enabled);
    ui->TotalElasticDeformation->setEnabled(enabled);

    ui->TirageElasticDeformation->setValue(0);
    //signal/slot will call ElasticDeformation_tirageElasticDeformationChanged(0)
  }

  ui->OptionElastic->setEnabled(enabled);
  ui->ElasticDeformation1CB->setEnabled(enabled);
  ui->ElasticDeformation2CB->setEnabled(enabled);

  ui->MinAlphaElasticDeformation1SB->setEnabled(enabled);
  ui->MaxAlphaElasticDeformation1SB->setEnabled(enabled);
  ui->MinSigmaElasticDeformation1SB->setEnabled(enabled);
  ui->MaxSigmaElasticDeformation1SB->setEnabled(enabled);
  ui->MinAlphaElasticDeformation2SB->setEnabled(enabled);
  ui->MaxAlphaElasticDeformation2SB->setEnabled(enabled);
  ui->MinSigmaElasticDeformation2SB->setEnabled(enabled);
  ui->MaxSigmaElasticDeformation2SB->setEnabled(enabled);  
  ui->MinAlphaAffineElasticDeformation2SB->setEnabled(enabled);
  ui->MaxAlphaAffineElasticDeformation2SB->setEnabled(enabled);
}

void
Assistant::ElasticDeformation_MethodsChanged()
{
  ElasticDeformation_updateTirageAndTotal();
}

void
Assistant::ElasticDeformation_tirageElasticChanged(int /*nbTirage*/)
{
  const int nbDegs = ElasticDeformation_nbDegradations();
  ui->NbDegElasticDeformation->setText(QString::number(nbDegs));
  const int total = nbDegs * _inputImageList.size() *
                    ui->TirageElasticDeformation->value(); //_ElasticDeformation_nbTirageElasticDeformation;
  ui->TotalElasticDeformation->setText(QString::number(total));
  updateTotalPic();
}

void
Assistant::ElasticDeformation_changeMinAlphaTransform1(double value)
{
  if (ui->MaxAlphaElasticDeformation1SB->value() < value)
    ui->MaxAlphaElasticDeformation1SB->setValue(value);
}

void
Assistant::ElasticDeformation_changeMaxAlphaTransform1(double value)
{
  if (ui->MinAlphaElasticDeformation1SB->value() > value)
    ui->MinAlphaElasticDeformation1SB->setValue(value);
}

void
Assistant::ElasticDeformation_changeMinSigmaTransform1(double value)
{
  if (ui->MaxSigmaElasticDeformation1SB->value() < value)
    ui->MaxSigmaElasticDeformation1SB->setValue(value);
}

  void
Assistant::ElasticDeformation_changeMaxSigmaTransform1(double value)
{
  if (ui->MinSigmaElasticDeformation1SB->value() > value)
    ui->MinSigmaElasticDeformation1SB->setValue(value);
}

void
Assistant::ElasticDeformation_changeMinAlphaTransform2(double value)
{
  if (ui->MaxAlphaElasticDeformation2SB->value() < value)
    ui->MaxAlphaElasticDeformation2SB->setValue(value);
}

void
Assistant::ElasticDeformation_changeMaxAlphaTransform2(double value)
{
  if (ui->MinAlphaElasticDeformation2SB->value() > value)
    ui->MinAlphaElasticDeformation2SB->setValue(value);
}

void
Assistant::ElasticDeformation_changeMinSigmaTransform2(double value)
{
  if (ui->MaxSigmaElasticDeformation2SB->value() < value)
    ui->MaxSigmaElasticDeformation2SB->setValue(value);
}

void
Assistant::ElasticDeformation_changeMaxSigmaTransform2(double value)
{
  if (ui->MinSigmaElasticDeformation2SB->value() > value)
    ui->MinSigmaElasticDeformation2SB->setValue(value);
}

void
Assistant::ElasticDeformation_changeMinAlphaAffineTransform2(double value)
{
  if (ui->MaxAlphaAffineElasticDeformation2SB->value() < value)
    ui->MaxAlphaAffineElasticDeformation2SB->setValue(value);
}

void
Assistant::ElasticDeformation_changeMaxAlphaAffineTransform2(double value)
{
  if (ui->MinAlphaAffineElasticDeformation2SB->value() > value)
    ui->MinAlphaAffineElasticDeformation2SB->setValue(value);
}




void
Assistant::Dist3D_updateTirageAndTotal()
{
  const int nbDegs = _Dist3D_meshesList.size();
  ui->NbDegDist3D->setText(QString::number(nbDegs));

  const bool enabled = (nbDegs > 0);
  ui->NbDegDist3D->setEnabled(enabled);
  ui->TirageDist3D->setEnabled(enabled);
  ui->TotalDist3D->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TirageDist3D->setValue(0);

  if (
    nbDegs > 0 &&
    ui->TirageDist3D->value() ==
      0) { //set TirageDist3D value to 1 once we have some degradations selected
    ui->TirageDist3D->setValue(1);
  }

  const int total = nbDegs * _inputImageList.size() * ui->TirageDist3D->value();
  ui->TotalDist3D->setText(QString::number(total));

  updateTotalPic();
}

void
Assistant::Dist3D_EnableDist3DOption()
{
  bool enabled = false;
  if (ui->CheckDist3D->isChecked()) {
    enabled = true;
    _Dist3D_dist3DEnable = enabled;
    Dist3D_updateTirageAndTotal();
  }
  else {
    _Dist3D_dist3DEnable = enabled;
    ui->NbDegDist3D->setEnabled(enabled);
    ui->TirageDist3D->setEnabled(enabled);
    ui->TotalDist3D->setEnabled(enabled);

    ui->TirageDist3D->setValue(0);
    //signal/slot will call Dist3D_tirageDist3DChanged(0)
  }

  ui->Dist3D_btnMeshesFolder->setEnabled(enabled);
  ui->Dist3D_meshesFolderLabel->setEnabled(enabled);
  ui->Dist3D_meshesPath->setEnabled(enabled);
  ui->Dist3D_nbMeshes->setEnabled(enabled);
  ui->Dist3D_nbMeshesLabel->setEnabled(enabled);
  ui->Dist3D_useBackgroundCheck->setEnabled(enabled);
  ui->Dist3D_backgroundsPath->setEnabled(enabled);
  ui->Dist3D_btnBackgroundsFolder->setEnabled(enabled);
  ui->Dist3D_backgroundsFolderLabel->setEnabled(enabled);
}

void
Assistant::Dist3D_tirageDist3DChanged(int /*value*/)
{
  const int nbDegs = _Dist3D_meshesList.size();
  ui->NbDegDist3D->setText(QString::number(nbDegs));
  const int total = nbDegs * _inputImageList.size() * ui->TirageDist3D->value();
  ui->TotalDist3D->setText(QString::number(total));
  updateTotalPic();
}

void
Assistant::Dist3D_setupGUIImages()
{
  const QString meshPath =
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigMeshFolderKey)
      .toString();

  ui->Dist3D_meshesPath->setText(meshPath);
  _Dist3D_meshDirectory = meshPath;
  Dist3D_loadListMeshes();

  Dist3D_EnableDist3DOption();

  ui->Dist3D_useBackgroundCheck->setChecked(false);
  Dist3D_changeUseBackground();
}

void
Assistant::Dist3D_chooseMeshesDirectory()
{
  const QString path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Choose meshes directory"),
                                      ui->Dist3D_meshesPath->text(),
                                      QFileDialog::ShowDirsOnly);

  ui->Dist3D_meshesPath->setText(path);
  _Dist3D_meshDirectory = path;

  if (!path.isEmpty()) {
    Dist3D_loadListMeshes();
  } else {
    const QString szStr = QString::number(0);
    ui->Dist3D_nbMeshes->setText(szStr);
    //ui->TotalDist3D_numMeshes->setText(szStr);
    ui->NbDegDist3D->setText(szStr);
    ui->TotalDist3D->setText(szStr);
  }
}

void
Assistant::Dist3D_loadListMeshes()
{
  QDir directory(_Dist3D_meshDirectory);

  _Dist3D_meshesList.clear();

  QStringList nameFilters;
  nameFilters << QStringLiteral("*.brs")
              << QStringLiteral("*.obj"); //B:TODO:UGLY: make a function
  QList<QString> fileList =
    directory.entryList(nameFilters, QDir::Files | QDir::Readable);
  _Dist3D_meshesList.reserve(fileList.size());
  for (const QString &filename : fileList) {

    //std::cerr<<"mesh: filename="<<filename.toStdString()<<" abs="<<directory.absoluteFilePath(filename).toStdString()<<"\n";

    _Dist3D_meshesList.append(directory.absoluteFilePath(filename));
  }

  const int sz = _Dist3D_meshesList.size();

  const QString szStr = QString::number(sz);
  ui->Dist3D_nbMeshes->setText(szStr);
  //ui->TotalDist3D_numMeshes->setText(szStr);
  ui->NbDegDist3D->setText(szStr);

  const QString str = QString::number(sz * _inputImageList.size());
  ui->TotalDist3D->setText(str);

  if (sz == 0) {
    QMessageBox::critical(this,
                          QStringLiteral("No meshes in folder"),
                          QStringLiteral("Select a folder with meshes"));
  }
}

void
Assistant::Dist3D_changeUseBackground()
{
  const bool enabled = ui->Dist3D_useBackgroundCheck->isChecked();
  ui->Dist3D_backgroundsPath->setEnabled(enabled);
  ui->Dist3D_btnBackgroundsFolder->setEnabled(enabled);
  ui->Dist3D_backgroundsFolderLabel->setEnabled(enabled);

  if (enabled) {
    if (ui->Dist3D_backgroundsPath->text().isEmpty()) {
      const QString meshBackgroundsPath =
        Core::ConfigurationManager::get(AppConfigMainGroup,
                                        AppConfigMeshFolderKey)
          .toString() +
        "backgrounds"; //TODO:UGLY !
      ui->Dist3D_backgroundsPath->setText(meshBackgroundsPath);
      _Dist3D_meshDirectory = meshBackgroundsPath;
      Dist3D_loadMeshBackgrounds();
    }
  }
}

void
Assistant::Dist3D_chooseBackgroundsDirectory()
{
  const QString path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Choose meshes directory"),
                                      ui->Dist3D_backgroundsPath->text(),
                                      QFileDialog::ShowDirsOnly);

  ui->Dist3D_backgroundsPath->setText(path);
  _Dist3D_meshBackgroundsDirectory = path;

  if (!path.isEmpty()) {
    Dist3D_loadMeshBackgrounds();
  }
}

void
Assistant::Dist3D_loadMeshBackgrounds()
{
  QDir directory(_Dist3D_meshDirectory);

  _Dist3D_meshBackgroundsList.clear();

  const QStringList nameFilters = getReadImageFilterList();
  QList<QString> fileList =
    directory.entryList(nameFilters, QDir::Files | QDir::Readable);
  _Dist3D_meshBackgroundsList.reserve(fileList.size());
  for (const QString &filename : fileList) {
    _Dist3D_meshBackgroundsList.append(directory.absoluteFilePath(filename));
  }

  const int sz = _Dist3D_meshBackgroundsList.size();

  if (sz == 0) {
    QMessageBox::critical(this,
                          QStringLiteral("No pics in folder"),
                          QStringLiteral("Select a folder with pictures"));
  }
}

void
Assistant::GDD_updateTirageAndTotal()
{
  const int nbDegs = (_GDD_gddEnable ? 1 : 0);
  ui->NbDegGDD->setText(QString::number(nbDegs));

  //TODO:here??? : check that stainImagesDir is valid and contains images

  const bool enabled = (nbDegs > 0);
  ui->NbDegGDD->setEnabled(enabled);
  ui->TirageGDD->setEnabled(enabled);
  ui->TotalGDD->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TirageGDD->setValue(0);

  if (nbDegs > 0 &&
      ui->TirageGDD->value() ==
        0) { //set TirageGDD value to 1 once we have some degradations selected
    ui->TirageGDD->setValue(1);
  }

  const int total = nbDegs * _inputImageList.size() * ui->TirageGDD->value();
  ui->TotalGDD->setText(QString::number(total));

  updateTotalPic();
}

void
Assistant::GDD_EnableGDDOption()
{
  bool enabled = false;
  if (ui->CheckGDD->isChecked()) {
    enabled = true;
    _GDD_gddEnable = enabled;
    GDD_updateTirageAndTotal();
  } else {
    _GDD_gddEnable = enabled;
    ui->NbDegGDD->setEnabled(enabled);
    ui->TirageGDD->setEnabled(enabled);
    ui->TotalGDD->setEnabled(enabled);

    ui->TirageGDD->setValue(0); //signal/slot will call GDD_tirageGDDChanged(0)
  }

  ui->GDD_btnStainImagesFolder->setEnabled(enabled);
  ui->GDD_stainImagesFolderLabel->setEnabled(enabled);
  ui->GDD_stainImagesPath->setEnabled(enabled);
  ui->GDD_numStainsLabel->setEnabled(enabled);
  ui->GDD_numStainsMinLabel->setEnabled(enabled);
  ui->GDD_numStainsMin->setEnabled(enabled);
  ui->GDD_numStainsMaxLabel->setEnabled(enabled);
  ui->GDD_numStainsMax->setEnabled(enabled);
  ui->GDD_insertTypeLabel->setEnabled(enabled);
  ui->GDD_insertTypeCB->setEnabled(enabled);
  ui->GDD_doRotations->setEnabled(enabled);
}

void
Assistant::GDD_tirageGDDChanged(int /*value*/)
{
  const int nbDegs = (_GDD_gddEnable ? 1 : 0);
  ui->NbDegGDD->setText(QString::number(nbDegs));
  const int total = nbDegs * _inputImageList.size() * ui->TirageGDD->value();
  ui->TotalGDD->setText(QString::number(total));
  updateTotalPic();
}

void
Assistant::GDD_setupGUIImages()
{
  const QString stainImagesPath =
    Core::ConfigurationManager::get(AppConfigMainGroup,
                                    AppConfigStainImagesFolderKey)
      .toString();

  ui->GDD_stainImagesPath->setText(stainImagesPath);
  _GDD_stainImagesDirectory = stainImagesPath;

  GDD_EnableGDDOption();

  if (ui->GDD_insertTypeCB->count() == 0) {
    ui->GDD_insertTypeCB->addItem(
      tr("None"),
      QVariant(static_cast<int>(
        dc::GradientDomainDegradation::InsertType::INSERT_AS_IS)));
    ui->GDD_insertTypeCB->addItem(
      tr("To gray"),
      QVariant(static_cast<int>(
        dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY)));
    ui->GDD_insertTypeCB->addItem(
      tr("To gray if destination image is gray"),
      QVariant(static_cast<int>(
        dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY_IF_GRAY)));
  }
  assert(ui->GDD_insertTypeCB->count() == 3);
  ui->GDD_insertTypeCB->setCurrentIndex(2);

  ui->GDD_doRotations->setChecked(true);
}

void
Assistant::GDD_chooseStainImagesDirectory()
{
  const QString path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Choose stain images directory"),
                                      ui->GDD_stainImagesPath->text(),
                                      QFileDialog::ShowDirsOnly);

  ui->GDD_stainImagesPath->setText(path);
  _GDD_stainImagesDirectory = path;

  if (path.isEmpty()) {
    //TODO: also check that there are images !?

    const QString szStr = QString::number(0);
    ui->NbDegGDD->setText(szStr);
    ui->TotalGDD->setText(szStr);
  }
}

void
Assistant::GDD_changeMinNbStains(int value)
{
  if (ui->GDD_numStainsMax->value() < value)
    ui->GDD_numStainsMax->setValue(value);
}

void
Assistant::GDD_changeMaxNbStains(int value)
{
  if (ui->GDD_numStainsMin->value() > value)
    ui->GDD_numStainsMin->setValue(value);
}

void
Assistant::setDocController(DocumentController *DocController)
{
  _DocController = DocController;
}

QStringList
Assistant::picsList() const
{
  return _inputImageList;
}

QString
Assistant::saveDirectory() const
{
  return _outputDegradedImageDir;
}

QString
Assistant::PicDirectory() const
{
  return _PicDirectory;
}

bool
Assistant::semi() const
{
  return _semi;
}

void
Assistant::setSemi(bool semi)
{
  _semi = semi;
}

bool
Assistant::bleedEnable() const
{
  return _BleedThrough_bleedEnable;
}

int
Assistant::nbTirageBleed() const
{
  return ui->TirageBleed->value();
}

int
Assistant::bleedMin() const
{
  return _BleedThrough_bleedMin;
}

int
Assistant::bleedMax() const
{
  return _BleedThrough_bleedMax;
}

bool
Assistant::charEnable() const
{
  return _CharDeg_charEnable;
}

int
Assistant::nbTirageChar() const
{
  return ui->TirageChar->value();
}

int
Assistant::charMax() const
{
  return _CharDeg_charMax;
}

int
Assistant::charMin() const
{
  return _CharDeg_charMin;
}

bool
Assistant::noiseEnable() const
{
  return _Noise_noiseEnable;
}

bool
Assistant::rotationEnable() const
{
  return _Rotation_rotationEnable;
}

bool
Assistant::shadEnable() const
{
  return _Shadow_shadEnable;
}

bool
Assistant::phantEnable() const
{
  return _Phantom_phantEnable; //ui->Phantom_Rare->isChecked();
}

bool
Assistant::gddEnable() const
{
  return _GDD_gddEnable;
}

bool
Assistant::elasticDeformationEnable() const
{
  return _ElasticDeformation_elasticEnable;
}

int
Assistant::blurMax() const
{
  return ui->BlurMax->value();
}

int
Assistant::blurMin() const
{
  return ui->BlurMin->value();
}

int
Assistant::nbTirageBlur() const
{
  return ui->TirageBlur->value(); //_Blur_nbTirageBlur;
}

bool
Assistant::blurEnable() const
{
  return _Blur_blurEnable;
}

bool
Assistant::pattern6() const
{
  return _Blur_pattern6;
}

bool
Assistant::pattern5() const
{
  return _Blur_pattern5;
}

bool
Assistant::pattern4() const
{
  return _Blur_pattern4;
}

bool
Assistant::pattern3() const
{
  return _Blur_pattern3;
}

bool
Assistant::pattern2() const
{
  return _Blur_pattern2;
}

bool
Assistant::pattern1() const
{
  return _Blur_pattern1;
}

bool
Assistant::getBlur_PageEnable() const
{
  return _Blur_PageEnable;
}

bool
Assistant::getBlur_ZoneEnable() const
{
  return _Blur_ZoneEnable;
}

bool
Assistant::bigHoleCenter() const
{
  return _Hole_bigHoleCenter;
}

bool
Assistant::mediumHoleCenter() const
{
  return _Hole_mediumHoleCenter;
}

bool
Assistant::smallHoleCenter() const
{
  return _Hole_smallHoleCenter;
}

bool
Assistant::bigHoleBorder() const
{
  return _Hole_bigHoleBorder;
}

bool
Assistant::mediumHoleBorder() const
{
  return _Hole_mediumHoleBorder;
}

bool
Assistant::smallHoleBorder() const
{
  return _Hole_smallHoleBorder;
}

bool
Assistant::bigHoleCorner() const
{
  return _Hole_bigHoleCorner;
}

bool
Assistant::mediumHoleCorner() const
{
  return _Hole_mediumHoleCorner;
}

bool
Assistant::smallHoleCorner() const
{
  return _Hole_smallHoleCorner;
}

int
Assistant::nbHoleCorner() const
{
  return _Hole_nbHoleCorner;
}

int
Assistant::nbHoleCenter() const
{
  return _Hole_nbHoleCenter;
}

int
Assistant::getNbHoleBorder() const
{
  return _Hole_nbHoleBorder;
}

int
Assistant::holeMax() const
{
  return ui->HoleCenterMaxNb->value();
}

int
Assistant::holeMin() const
{
  return ui->HoleCenterMinNb->value();
}

bool
Assistant::holeEnable() const
{
  return _Hole_holeEnable;
}

int
Assistant::nbHoleCornerSelected() const
{
  return _Hole_nbHoleCornerSelected;
}

int
Assistant::nbHoleBorderSelected() const
{
  return _Hole_nbHoleBorderSelected;
}

int
Assistant::nbHoleCenterSelected() const
{
  return _Hole_nbHoleCenterSelected;
}

QColor
Assistant::getColorBehind() const
{
  return _Hole_colorBehind;
}

bool
Assistant::dist3DEnable() const
{
  //B:TODO:
  //useless to have _*_*Enable members & *Enable() functions !?!
  //We should directly access the ui->Check*  member !?

  return _Dist3D_dist3DEnable;
}

static QString
Shadow_getXML(const QString &filename,
              float distanceRatio,
              dc::ShadowBinding::Border border,
              float intensity,
              float angle)
{
  QString xml = QStringLiteral("<Degradation>\n");
  xml += "\t<PictureName>" + filename + "</PictureName>\n";
  xml += QLatin1String("\t<DegradationName>Shadow</DegradationName>\n");
  xml += QLatin1String("\t<Parameters>\n");
  xml += "\t\t<DistanceRatio>" + QString::number(distanceRatio) +
             "</DistanceRatio>\n";
  xml +=
    "\t\t<Border>" + QString::number(static_cast<int>(border)) + "</Border>\n";
  xml += "\t\t<Intensity>" + QString::number(intensity) + "</Intensity>\n";
  xml += "\t\t<Angle>" + QString::number(angle) + "</Angle>\n";
  xml += QLatin1String("\t</Parameters>\n");
  xml += QLatin1String("</Degradation>");
  return xml;
}

static
bool
saveXml(const QString &saveXml,
	const QString &outputXmlFilename)
{
  QFile file(outputXmlFilename);
  const bool openOk = file.open(QIODevice::WriteOnly | QIODevice::Text);
  if (!openOk) {
    std::cerr << "ERROR: unable to write xml file: "
              << outputXmlFilename.toStdString() << "\n";
    return false;
  }
  QTextStream out(&file);
  out << saveXml;
  file.close();
  return true; 
}

static bool
Shadow_saveImage(const QImage &img,
                 const QString &path,
                 const QString &prefix,
                 const QString &ext,
                 const QString &xml)
{

  const QString filename = prefix + ext;
  const QString absoluteFilename = path + filename;

  const bool saveOk = img.save(absoluteFilename);
  if (!saveOk) {
    std::cerr << "ERROR: unable to save image: "
              << absoluteFilename.toStdString() << "\n";
    return false;
  }

  const QString absoluteFilenameXML = path + prefix + ".xml";
  return saveXml(xml, absoluteFilenameXML);
}

static QString
Shadow_getBorderStr(dc::ShadowBinding::Border border)
{
  switch (border) {
    case dc::ShadowBinding::Border::TOP:
      return QStringLiteral("Top");
      break;
    case dc::ShadowBinding::Border::RIGHT:
      return QStringLiteral("Right");
      break;
    case dc::ShadowBinding::Border::BOTTOM:
      return QStringLiteral("Bottom");
      break;
    case dc::ShadowBinding::Border::LEFT:
    default:
      return QStringLiteral("Left");
      break;
  }
}

static bool
Shadow_saveImage(const QImage &img,
                 const QString &path,
                 const QString &prefixStr,
                 const QString &ext,
                 float distanceRatio,
                 dc::ShadowBinding::Border border,
                 float intensity,
                 float angle)
{
  const QString borderStr = Shadow_getBorderStr(border);

  const QString prefix = prefixStr + borderStr;
  const QString filename = prefix + ext;

  const QString xml =
    Shadow_getXML(filename, distanceRatio, border, intensity, angle);

  return Shadow_saveImage(img, path, prefix, ext, xml);
}

/*
static
QString
Hole_getXML(const QString &filename,
            const QString &patternFilename,
            int posX, int posY, int size, HoleType type, int side, QColor color)
{
  QString xml = "<Degradation>\n";
  xml += "\t<PictureName>" + filename +  "</PictureName>\n";
  xml += "\t<DegradationName>Hole</DegradationName>\n";
  xml += "\t\t<Number>"+ QString::number(1) +"</Number>\n";
  xml += "\t<Parameters>\n";
  xml += "\t\t<Hole>" + patternFilename + "</Hole>\n";
  xml += "\t\t<HoleType>" + QString::number(static_cast<int>(type)) + "</HoleType>\n";
  xml += "\t\t<Side>" + QString::number(side) + "</Position>\n";
  xml += "\t\t<X>" + QString::number(posX) + "</X>\n";
  xml += "\t\t<Y>" + QString::number(posY) + "</Y>\n";
  xml += "\t\t<ColorBehind>" + color.name() + "</ColorBehind>\n";
  xml += "\t</Parameters>\n";
  xml += "</Degradation>";
  return xml;
}
*/

static QString
Hole_getXML(const QString &filename, std::vector<HoleData> &holes)
{
  QString xml = QStringLiteral("<Degradation>\n");
  xml += "\t<PictureName>" + filename + "</PictureName>\n";
  xml += QLatin1String("\t<DegradationName>Hole</DegradationName>\n");
  const unsigned int nbHole = holes.size();
  xml += "\t\t<Number>" + QString::number(nbHole) + "</Number>\n";
  xml += QLatin1String("\t<Parameters>\n");

  for (unsigned int i = 0; i < nbHole; ++i) {
    const QString &patternFilename = holes[i].patternFilename;
    const QString holeTypeStr =
      QString::number(static_cast<int>(holes[i].type));
    const QString holeSideStr = QString::number(holes[i].side);

    xml += "\t\t<Hole>" + patternFilename + "</Hole>\n";
    xml += "\t\t<Size>" + QString::number(holes[i].size) + "</Size>\n";
    xml += "\t\t<HoleType>" + holeTypeStr + "</HoleType>\n";
    xml += "\t\t<Side>" + holeSideStr + "</Side>\n";
    xml += "\t\t<X>" + QString::number(holes[i].posX) + "</X>\n";
    xml += "\t\t<Y>" + QString::number(holes[i].posY) + "</Y>\n";
    xml += "\t\t<ColorBehind>" + holes[i].color.name() + "</ColorBehind>\n";
  }
  xml += QLatin1String("\t</Parameters>\n");
  xml += QLatin1String("</Degradation>");
  return xml;
}

static bool
Hole_saveImage(const QImage &img,
               const QString &path,
               const QString &prefixStr,
               const QString &ext,
               const QString &xml)
{
  const QString filename = prefixStr + ext;
  const QString absoluteFilename = path + filename;
  const bool saveOk = img.save(absoluteFilename);
  if (!saveOk) {
    std::cerr << "ERROR: unable to save image: "
              << absoluteFilename.toStdString() << "\n";
    return false;
  }

  const QString absoluteFilenameXML = path + prefixStr + ".xml";
  return saveXml(xml, absoluteFilenameXML);
}

/*
static
QString getHoleTypeStr(HoleType type)
{
  QString holeTypeStr;
  switch (type) {
  case HoleType::CENTER:
    holeTypeStr = "Center";
    break;
  case HoleType::BORDER:
    holeTypeStr = "Border";
    break;
  case HoleType::CORNER:
    holeTypeStr = "Corner";
    break;
  default:
    break;
  }
  return holeTypeStr;
}
*/
/*
static
QString getHoleSideStr(HoleType type, int side)
{
  QString sideStr;
  if (type == HoleType::BORDER) {
    switch (static_cast<Border>(side)) {
    case Border::TOP:
      sideStr = "Top";
      break;
    case Border::RIGHT:
      sideStr = "Right";
      break;
    case Border::BOTTOM:
      sideStr = "Bottom";
      break;
    case Border::LEFT:
      sideStr = "Left";
      break;
    default:
      break;
    }
  }
  else if (type == HoleType::CORNER) {
    switch (static_cast<Corner>(side)) {
    case Corner::TOPLEFT:
      sideStr = "TopLeft";
      break;
    case Corner::TOPRIGHT:
      sideStr = "TopRight";
      break;
    case Corner::BOTTOMRIGHT:
      sideStr = "BottomRight";
      break;
    case Corner::BOTTOMLEFT:
      sideStr = "BottomLeft";
      break;
    default:
      break;
    }
  }
  return sideStr;
}
*/
/*
static
bool
Hole_saveImage(const QImage &img,
               const QString &path,
               const QString &prefixStr,
               const QString &ext,
               const QString &patternFilename,
               int posX, int posY, int size, HoleType type, int side, QColor color)
{
  const QString holeTypeStr = getHoleTypeStr(type);
  const QString sideStr = getHoleSideStr(type, side);

  const QString prefix = prefixStr+holeTypeStr+"_"+sideStr;
  const QString filename = prefix + ext;

  const QString xml = Hole_getXML(filename,
                                  patternFilename,
                                  posX, posY, size, type, side, color);

  return Hole_saveImage(img, path, prefixStr, holeTypeStr, sideStr, ext, xml);

}
*/

/*
static
bool
Hole_saveImage(const QImage &img,
               const QString &path,
               const QString &prefixStr,
               const QString &ext,
               std::vector<HoleData> holes,
               int size, HoleType type, int side, QColor color)
{
  const QString holeTypeStr = getHoleTypeStr(type);
  const QString sideStr = getHoleSideStr(type, side);

  const QString prefix = prefixStr+holeTypeStr+"_"+sideStr;
  const QString filename = prefix + ext;

  const QString xml = Hole_getXML(filename,
                                  holes,
                                  size, type, side, color);

  return Hole_saveImage(img, path, prefixStr, holeTypeStr, sideStr, ext, xml);

}
*/

static bool
Hole_saveImage(const QImage &img,
               const QString &path,
               const QString &prefixStr,
               const QString &ext,
               std::vector<HoleData> holes)
{
  const QString filename = prefixStr + ext;

  const QString xml = Hole_getXML(filename, holes);

  return Hole_saveImage(img, path, prefixStr, ext, xml);
}

  


void
Assistant::do_bleed(const QString &imageBasename,
                    const QImage &recto,
                    int picIndex,
                    const QString &inputImageDir,
                    const QString &outputImageDir) const
{
  assert(picIndex < this->picsList().size());

  //We choose a random verso (different from recto if possible)
  const QStringList &picsList = this->picsList();
  const int numPics = picsList.size();
  int randomNb = P_bounded_rand(0, numPics);
  if (numPics > 1)
    while (picIndex == randomNb)
      randomNb = P_bounded_rand(0, numPics);
  assert(randomNb < numPics);
  QString versoPath = inputImageDir + picsList[randomNb];
  QImage verso(versoPath);
  verso = verso.mirrored(true, false).scaled(recto.width(), recto.height());

  const int bleedMin = this->bleedMin();
  const int bleedMax = this->bleedMax();

  const int numDraws = this->nbTirageBleed();
  for (int i = 0; i < numDraws; ++i) {
    const int nbOcc = P_bounded_rand(bleedMin, bleedMax + 1);

    const QImage imageBleed =
      dc::BleedThrough::bleedThrough(recto, verso, nbOcc);
    const QString prefixFilename =
      imageBasename + "Bleed_" + QString::number(i);
    const QString filename = prefixFilename + ".png";
    imageBleed.save(outputImageDir + filename);

    QString xml = QStringLiteral("<Degradation>\n");
    xml += "\t<PictureName>" + filename + "</PictureName>\n";
    xml +=
      QLatin1String("\t<DegradationName>Bleed-Through</DegradationName>\n");
    xml += QLatin1String("\t<Parameters>\n");
    xml += "\t\t<Verso>" + versoPath + "</Verso>\n";
    xml +=
      "\t\t<BleedIntensity>" + QString::number(nbOcc) + "</BleedIntensity>\n";
    xml += QLatin1String("\t</Parameters>\n");
    xml += QLatin1String("</Degradation>");

    saveXml(xml, outputImageDir + prefixFilename + ".xml");

    updateProgress();
  }
}

void
Assistant::do_charDeg(const QString &imageBasename,
                      const QImage &recto,
                      const QString &outputImageDir) const
{
  const int charMin = this->charMin();
  const int charMax = this->charMax();

  const int numDraws = this->nbTirageChar();
  for (int i = 0; i < numDraws; ++i) {

    const int level = P_bounded_rand(charMin, charMax + 1);

    QImage imgCharTmp;
    imgCharTmp = toGray(recto);
    dc::GrayscaleCharsDegradationModelQ deg(imgCharTmp);
    imgCharTmp = deg.degradateByLevel(level);

    const QString prefixFilename =
      imageBasename + "CharDeg_" + QString::number(i);
    const QString filename = prefixFilename + ".png";
    imgCharTmp.save(outputImageDir + filename);

    QString xml = QStringLiteral("<Degradation>\n");
    xml += "\t<PictureName>" + filename + "</PictureName>\n";
    xml += "\t<DegradationName>Characters Degradation</DegradationName>\n";
    xml += QLatin1String("\t<Parameters>\n");
    xml += "\t\t<DegradationIntensity>" + QString::number(level) +
               "</DegradationIntensity>\n";
    xml += QLatin1String("\t</Parameters>\n");
    xml += QLatin1String("</Degradation>");

    saveXml(xml, outputImageDir + prefixFilename + ".xml");

    updateProgress();
  }
}

void
Assistant::do_GDD(const QString &imageBasename,
                  const QImage &recto,
                  const QString &outputImageDir) const
{
  const QString stainImagesPath = ui->GDD_stainImagesPath->text();
  const int minNumStains = ui->GDD_numStainsMin->value();
  const int maxNumStains =
    std::max(ui->GDD_numStainsMin->value(), minNumStains + 1);
  const dc::GradientDomainDegradation::InsertType insertType =
    static_cast<dc::GradientDomainDegradation::InsertType>(
      ui->GDD_insertTypeCB->currentData().toInt());
  const bool doRotations = ui->GDD_doRotations->isChecked();

  const int numDraws = ui->TirageGDD->value();
  for (int i = 0; i < numDraws; ++i) {

    const int numStains = P_bounded_rand(minNumStains, maxNumStains);

    //TODO:OPTIM: for each image, we re-list the stain images directory

    QImage imgOut = dc::GradientDomainDegradation::degradation(
      recto, stainImagesPath, numStains, insertType, doRotations);

    const QString prefixFilename =
      imageBasename + "GDDeg_" + QString::number(i);
    const QString filename = prefixFilename + ".png";
    imgOut.save(outputImageDir + filename);

    QString xml = QStringLiteral("<Degradation>\n");
    xml += "\t<PictureName>" + filename + "</PictureName>\n";
    xml +=
      "\t<DegradationName>Gradient Domain Degradation</DegradationName>\n";
    xml += QLatin1String("\t<Parameters>\n");
    xml +=
      "\t\t<StainImagesPath>" + stainImagesPath + "</StainImagePath>\n";
    xml +=
      "\t\t<NumStains>" + QString::number(numStains) + "</NumStains>\n";
    xml += "\t\t<InsertType>" +
               QString::number(static_cast<int>(insertType)) +
               "</InsertType>\n";
    xml += "\t\t<DoRotations>" +
               QString::number(static_cast<int>(doRotations)) +
               "</DoRotations>\n";
    xml += QLatin1String("\t</Parameters>\n");
    xml += QLatin1String("</Degradation>");

    saveXml(xml, outputImageDir + prefixFilename + ".xml");

    updateProgress();
  }
}

static
QString
Rotation_makePrefixXml(const QString &imageFilename,
		       const QString &fillMethod,
		       float angle)
{
  QString xml = QStringLiteral("<Degradation>\n");
  xml += "\t<PictureName>" + imageFilename + "</PictureName>\n";
  xml += "\t<DegradationName>Rotation</DegradationName>\n";
  xml += QLatin1String("\t<Parameters>\n");
  xml += "\t\t<Angle>" + QString::number(angle) + "</Angle>\n";
  xml += "\t\t<Fill>" + fillMethod + "</Fill>\n";
  return xml;
}

static
QString
Rotation_makeSuffixeXml()
{
  QString xml;
  xml += QLatin1String("\t</Parameters>\n");
  xml += QLatin1String("</Degradation>");
  return xml;
}

static
bool
Rotation_saveFillColorXml(const QString &outputXmlFilename,
			  const QString &imageFilename,
			  float angle, QColor color)
{
  QString xml = Rotation_makePrefixXml(imageFilename, "Color", angle);  
  xml += "\t\t<Color>" + color.name() + "</Color>\n";
  xml += Rotation_makeSuffixeXml();
  return saveXml(xml, outputXmlFilename);
}

//contrary to bleed, here we do not check if the chosen image is different than the current image...
static
QString
getRandomImagePath(const QString &inputImageDir,
	       const QStringList &picsList)
{
  const int numPics = picsList.size();
  int randomNb = 0;
  if (numPics > 1)
    randomNb = P_bounded_rand(0, numPics);
  assert(randomNb < numPics);
  return (inputImageDir + picsList[randomNb]);
}

static
bool
Rotation_saveFillImageXml(const QString &outputXmlFilename,
			  const QString &imageFilename,
			  float angle,
			  const QString &backgroundImgPath)
{
  QString xml = Rotation_makePrefixXml(imageFilename, "Image", angle);  
  xml += "\t\t<Background>" + backgroundImgPath + "</Background>\n";
  xml += Rotation_makeSuffixeXml();
  return saveXml(xml, outputXmlFilename);
}

static
bool
Rotation_saveFillBorderXml(const QString &outputXmlFilename,
			   const QString &imageFilename,
			   float angle,
			   const QString &borderModeStr)
{
  QString xml = Rotation_makePrefixXml(imageFilename, "Border", angle);  
  xml += "\t\t<Replication>" + borderModeStr + "</Replication>\n";
  xml += Rotation_makeSuffixeXml();
  return saveXml(xml, outputXmlFilename);
}

static
bool
Rotation_saveFillBorder(const QImage &recto, float angle,
			dc::RotationDegradation::BorderReplication mode,
			const QString &imageBasename, int i,
			const QString &outputImageDir)
{
  QImage imgRot = dc::RotationDegradation::rotateFillBorder(recto, angle, mode);

  QString modeStr;
  switch (mode) {
  case dc::RotationDegradation::BorderReplication::REPLICATE:
    modeStr = "REPLICATE";
    break;
  case dc::RotationDegradation::BorderReplication::REFLECT:
    modeStr = "REFLECT";
    break;
  case dc::RotationDegradation::BorderReplication::WRAP:
    modeStr = "WRAP";
    break;
  case dc::RotationDegradation::BorderReplication::REFLECT101:
    modeStr = "REFLECT101";
    break;
  }
  
  const QString prefixFilename = imageBasename + "Rot_" + modeStr + "_" + QString::number(i);
  const QString filename = prefixFilename + ".png";
  imgRot.save(outputImageDir + filename);
  return Rotation_saveFillBorderXml(outputImageDir + prefixFilename + ".xml",
				    filename, angle, modeStr);
}

static
bool
Rotation_saveFillInpaintXml(const QString &outputXmlFilename,
			    const QString &imageFilename, float angle,
			    const QString &methodStr)
{
  QString xml = Rotation_makePrefixXml(imageFilename, "Inpaint", angle);  
  xml += "\t\t<Method>" + methodStr + "</Method>\n";
  xml += Rotation_makeSuffixeXml();
  return saveXml(xml, outputXmlFilename);
}

static
bool
Rotation_saveFillInpaint(const QImage &recto, float angle,
			 int method,
			 const QString &imageBasename, int i,
			 const QString &outputImageDir)
{
  QImage imgRot;
  QString methodStr;
  if (method == 1) {
    imgRot = dc::RotationDegradation::rotateFillInpaint1(recto, angle); //use default inpaintingRatio
    methodStr = "1";
  }
  else if (method == 2) {
    imgRot = dc::RotationDegradation::rotateFillInpaint2(recto, angle); 
    methodStr = "2";
  }
  else if (method == 3) {
    imgRot = dc::RotationDegradation::rotateFillInpaint3(recto, angle); 
    methodStr = "3";
  }

  const QString prefixFilename = imageBasename + "Rot_" + "Inpaint"+ methodStr + "_" + QString::number(i);
  const QString filename = prefixFilename + ".png";
  imgRot.save(outputImageDir + filename);
  return Rotation_saveFillInpaintXml(outputImageDir + prefixFilename + ".xml",
				    filename, angle, methodStr);
}

void
Assistant::do_Rotation(const QString &imageBasename,
		       const QImage &recto,
		       const QString &inputImageDir,
		       const QString &outputImageDir) const
{
  const int numDraws = ui->TirageRotation->value();
  const float angleMin = ui->RotationAngleMinSB->value();
  const float angleMax = ui->RotationAngleMaxSB->value();
  assert(angleMin<=angleMax);
  
  for (int i = 0; i < numDraws; ++i) {

    const float angle = random_in_range(angleMin, angleMax);

    if (ui->RotationFillColorCB->isChecked()) {
      QImage imgRot;
      QColor color;
      if (ui->RotationSpecificColorRB->isChecked()) {
	color = ui->RotationColorSelectionPB->getColor();
      }
      else {
	assert(ui->RotationRandomColorRB->isChecked());
	color = random_color();
      }
      imgRot = dc::RotationDegradation::rotateFillColor(recto, angle, color);

      const QString prefixFilename = imageBasename + "Rot_color_" + QString::number(i);
      const QString filename = prefixFilename + ".png";
      imgRot.save(outputImageDir + filename);
      Rotation_saveFillColorXml(outputImageDir + prefixFilename + ".xml",
				filename, angle, color);
      updateProgress();
    }

    if (ui->RotationFillImageCB->isChecked()) {
      const QString backgroundImgPath = getRandomImagePath(inputImageDir, this->picsList());
      const QImage background(backgroundImgPath);
      const int repeats = random_in_range(ui->RotationImageRepeatsMinSB->value(),
					  ui->RotationImageRepeatsMaxSB->value());
      QImage imgRot = dc::RotationDegradation::rotateFillImage(recto, angle, background, repeats);

      const QString prefixFilename = imageBasename + "Rot_bckgrd_" + QString::number(i);
      const QString filename = prefixFilename + ".png";
      imgRot.save(outputImageDir + filename);
      Rotation_saveFillImageXml(outputImageDir + prefixFilename + ".xml",
				filename, angle, backgroundImgPath);
      updateProgress();
    }

    if (ui->RotationFillBorderCB->isChecked()) {
      if (ui->RotationBorderReplicateCB->isChecked()) {
	Rotation_saveFillBorder(recto, angle, dc::RotationDegradation::BorderReplication::REPLICATE,
				imageBasename, i, outputImageDir);
	updateProgress();
      }
      if (ui->RotationBorderReflectCB->isChecked()) {
	Rotation_saveFillBorder(recto, angle, dc::RotationDegradation::BorderReplication::REFLECT,
				imageBasename, i, outputImageDir);
	updateProgress();
      }
      if (ui->RotationBorderWrapCB->isChecked()) {
	Rotation_saveFillBorder(recto, angle, dc::RotationDegradation::BorderReplication::WRAP,
				imageBasename, i, outputImageDir);
	updateProgress();
      }
      if (ui->RotationBorderReflect101CB->isChecked()) {
	Rotation_saveFillBorder(recto, angle, dc::RotationDegradation::BorderReplication::REFLECT101,
				imageBasename, i, outputImageDir);
	updateProgress();
      }
    }

    if (ui->RotationFillInpaintCB->isChecked()) {
      if (ui->RotationInpaint1CB->isChecked()) {
	Rotation_saveFillInpaint(recto, angle, 1,
				imageBasename, i, outputImageDir);
	updateProgress();
      }
      if (ui->RotationInpaint2CB->isChecked()) {
	Rotation_saveFillInpaint(recto, angle, 2,
				imageBasename, i, outputImageDir);

	updateProgress();
      }
       if (ui->RotationInpaint3CB->isChecked()) {
	Rotation_saveFillInpaint(recto, angle, 3,
				imageBasename, i, outputImageDir);
	updateProgress();
      }
     
    }
    
  }
}


static
QString
Noise_makePrefixXml(const QString &imageFilename,
		    const QString &noiseMethod)
{
  QString xml = QStringLiteral("<Degradation>\n");
  xml += "\t<PictureName>" + imageFilename + "</PictureName>\n";
  xml += "\t<DegradationName>Noise</DegradationName>\n";
  xml += QLatin1String("\t<Parameters>\n");
  xml += "\t\t<Type>" + noiseMethod + "</Type>\n";
  return xml;
}

static
QString
Noise_makeSuffixeXml()
{
  QString xml;
  xml += QLatin1String("\t</Parameters>\n");
  xml += QLatin1String("</Degradation>");
  return xml;
}

static
bool
Noise_saveNoiseXml(const QString &outputXmlFilename,
		   const QString &imageFilename,
		   const QString &method,
		   float average,
		   float stddev,
		   dc::NoiseDegradation::AddNoiseType addType)
{
  QString xml = Noise_makePrefixXml(imageFilename, method);
  xml += "\t\t<Average>" + QString::number(average) + "</Average>\n";
  xml += "\t\t<StdDev>" + QString::number(stddev) + "</StdDev>\n";
  switch (addType) {
  case dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS:
    xml += "\t\t<Add>ADD_NOISE_AS_IS</Add>\n";
    break;
  case dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY:
    xml += "\t\t<Add>ADD_NOISE_AS_GRAY</Add>\n";
    break;
  case dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY_IF_GRAY:
    xml += "\t\t<Add>ADD_NOISE_AS_GRAY_IF_GRAY</Add>\n";
    break;
  }
  xml += Noise_makeSuffixeXml();
  return saveXml(xml, outputXmlFilename);
}

static
bool
Noise_saveGaussianNoiseXml(const QString &outputXmlFilename,
			   const QString &imageFilename,
			   float average,
			   float stddev,
			   dc::NoiseDegradation::AddNoiseType addType)
{
  return Noise_saveNoiseXml(outputXmlFilename,
			    imageFilename,
			    "Gaussian",
			    average, stddev, addType);
}

static
bool
Noise_saveSpeckleNoiseXml(const QString &outputXmlFilename,
			  const QString &imageFilename,
			  float average,
			  float stddev,
			  dc::NoiseDegradation::AddNoiseType addType)
{
  return Noise_saveNoiseXml(outputXmlFilename,
			    imageFilename,
			    "Speckle",
			    average, stddev, addType);
}

static
bool
Noise_saveSaltAndPepperNoiseXml(const QString &outputXmlFilename,
				const QString &imageFilename,
				float amount, float ratio)
{
  QString xml = Noise_makePrefixXml(imageFilename, "SaltAndPepper");
  xml += "\t\t<Amount>" + QString::number(amount) + "</Amount>\n";
  xml += "\t\t<Ratio>" + QString::number(ratio) + "</Ratio>\n";
  xml += Noise_makeSuffixeXml();
  return saveXml(xml, outputXmlFilename);
}

void
Assistant::do_Noise(const QString &imageBasename,
		    const QImage &recto,
		    const QString &outputImageDir) const
{
  const int numDraws = ui->TirageNoise->value();
  for (int i = 0; i < numDraws; ++i) {

    if (ui->GaussianNoiseCB->isChecked()) {

      const float averageMin = ui->MinAverageGaussianNoiseSB->value();
      const float averageMax = ui->MaxAverageGaussianNoiseSB->value();
      const float average = random_in_range(averageMin, averageMax);

      const float stdDevMin = ui->MinStdDevGaussianNoiseSB->value();
      const float stdDevMax = ui->MaxStdDevGaussianNoiseSB->value();
      const float stdDev = random_in_range(stdDevMin, stdDevMax);

      dc::NoiseDegradation::AddNoiseType addType =
	static_cast<dc::NoiseDegradation::AddNoiseType>(ui->GaussianNoiseAddTypeCB->currentData().toInt());

      QImage imgN;
      imgN = dc::NoiseDegradation::addGaussianNoise(recto, average, stdDev, addType);

      const QString prefixFilename = imageBasename + "Noise_Gaussian_" + QString::number(i);
      const QString filename = prefixFilename + ".png";
      imgN.save(outputImageDir + filename);
      Noise_saveGaussianNoiseXml(outputImageDir + prefixFilename + ".xml",
				 filename, average, stdDev, addType);
      updateProgress();
    }

    if (ui->SpeckleNoiseCB->isChecked()) {

      const float averageMin = ui->MinAverageSpeckleNoiseSB->value();
      const float averageMax = ui->MaxAverageSpeckleNoiseSB->value();
      const float average = random_in_range(averageMin, averageMax);

      const float stdDevMin = ui->MinStdDevSpeckleNoiseSB->value();
      const float stdDevMax = ui->MaxStdDevSpeckleNoiseSB->value();
      const float stdDev = random_in_range(stdDevMin, stdDevMax);

      dc::NoiseDegradation::AddNoiseType addType =
	static_cast<dc::NoiseDegradation::AddNoiseType>(ui->SpeckleNoiseAddTypeCB->currentData().toInt());

      QImage imgN;
      imgN = dc::NoiseDegradation::addSpeckleNoise(recto, average, stdDev, addType);

      const QString prefixFilename = imageBasename + "Noise_Speckle_" + QString::number(i);
      const QString filename = prefixFilename + ".png";
      imgN.save(outputImageDir + filename);
      Noise_saveSpeckleNoiseXml(outputImageDir + prefixFilename + ".xml",
				filename, average, stdDev, addType);
      updateProgress();
    }

    if (ui->SaltAndPepperNoiseCB->isChecked()) {

      const float amountMin = ui->MinAmountSaltAndPepperNoiseSB->value();
      const float amountMax = ui->MaxAmountSaltAndPepperNoiseSB->value();
      const float amount = random_in_range(amountMin, amountMax);

      const float ratioMin = ui->MinRatioSaltAndPepperNoiseSB->value();
      const float ratioMax = ui->MaxRatioSaltAndPepperNoiseSB->value();
      const float ratio = random_in_range(ratioMin, ratioMax);

      QImage imgN;
      imgN = dc::NoiseDegradation::addSaltAndPepperNoise(recto, amount, ratio);

      const QString prefixFilename = imageBasename + "Noise_SaltAndPepper_" + QString::number(i);
      const QString filename = prefixFilename + ".png";
      imgN.save(outputImageDir + filename);
      Noise_saveSaltAndPepperNoiseXml(outputImageDir + prefixFilename + ".xml",
				      filename, amount, ratio);
      updateProgress();
    }

  }
}




void
Assistant::do_shadow(const QString &imageBasename,
                     const QImage &recto,
                     const QString &outputImageDir) const
{

  const int numDraws = ui->TirageShadow->value();
  for (int i = 0; i < numDraws; ++i) {
    QImage picTmpShad;

    //some code duplication with Shadow_setPreview()
    const float distanceRatio =
      ui->shad_width->value() / (float)ui->shad_width->maximum();
    const float intensity =
      1.f - ui->shad_intensity->value() / (float)ui->shad_intensity->maximum();
    const float angle = ui->shad_angle->value();
    
    const QString &path = outputImageDir;
    const QString prefix = imageBasename + "Shadow_";
    const QString imgExt = QStringLiteral(".png");
    
    for (int j = 0; j < _Shadow_nbShadSelected; ++j) {
      const dc::ShadowBinding::Border border = _Shadow_borderStack[j];
      picTmpShad = dc::ShadowBinding::shadowBinding(recto, distanceRatio, border,
						    intensity, angle);
      Shadow_saveImage(picTmpShad,
		       path,
		       prefix,
		       imgExt,
		       distanceRatio,
		       border,
		       intensity,
		       angle);

      updateProgress();
    }
  }
}

static QString
Phantom_getFrequencyStr(dc::PhantomCharacter::Frequency frequency)
{
  switch (frequency) {
    case dc::PhantomCharacter::Frequency::RARE:
      return QStringLiteral("RARE");
      break;
    case dc::PhantomCharacter::Frequency::FREQUENT:
      return QStringLiteral("FREQUENT");
      break;
    case dc::PhantomCharacter::Frequency::VERY_FREQUENT:
    default:
      return QStringLiteral("VERY_FREQUENT");
      break;
  }
}

static void
Phantom_applyAndSave(const QImage &recto,
                     dc::PhantomCharacter::Frequency frequency,
                     const QString &phantomPatternsPath,
                     int i,
                     const QString &imageBasename,
                     const QString &outputImageDir)
{
  QImage imgTmpPhant = dc::PhantomCharacter::phantomCharacter(
    recto, frequency, phantomPatternsPath);

  const QString freqStr = Phantom_getFrequencyStr(frequency);
  const QString prefixFilename =
    imageBasename + "Phantom_" + freqStr + "_" + QString::number(i);
  const QString filename = prefixFilename + ".png";

  imgTmpPhant.save(outputImageDir + filename);

  QString xml = QStringLiteral("<Degradation>\n");
  xml += "\t<PictureName>" + filename + "</PictureName>\n";
  xml += "\t<DegradationName>Phantom Characters</DegradationName>\n";
  xml += QLatin1String("\t<Parameters>\n");
  xml += "\t\t<Frequency>" + freqStr + "</Frequency>\n";
  xml += QLatin1String("\t</Parameters>\n");
  xml += QLatin1String("</Degradation>");

  saveXml(xml, outputImageDir + prefixFilename + ".xml");
}

void
Assistant::do_phantom(const QString &imageBasename,
                      const QImage &recto,
                      const QString &outputImageDir) const
{
  const int numDraws = ui->TiragePhantom->value();
  for (int i = 0; i < numDraws; ++i) {

    //QImage imgTmpPhant;

    if (ui->Phantom_Rare->isChecked()) {
      Phantom_applyAndSave(recto,
                           dc::PhantomCharacter::Frequency::RARE,
                           _PhantomPatternsPath,
                           i,
                           imageBasename,
                           outputImageDir);

      updateProgress();
    }

    if (ui->Phantom_Frequent->isChecked()) {
      Phantom_applyAndSave(recto,
                           dc::PhantomCharacter::Frequency::FREQUENT,
                           _PhantomPatternsPath,
                           i,
                           imageBasename,
                           outputImageDir);

      updateProgress();
    }

    if (ui->Phantom_VeryFrequent->isChecked()) {

      Phantom_applyAndSave(recto,
                           dc::PhantomCharacter::Frequency::VERY_FREQUENT,
                           _PhantomPatternsPath,
                           i,
                           imageBasename,
                           outputImageDir);

      updateProgress();
    }
  }
}

static QString
Blur_getFunctionStr(dc::BlurFilter::Function f)
{
  switch (f) {
    case dc::BlurFilter::Function::LINEAR:
      return QStringLiteral("LINEAR");
      break;
    case dc::BlurFilter::Function::LOG:
      return QStringLiteral("LOG");
      break;
    case dc::BlurFilter::Function::PARABOLA:
      return QStringLiteral("PARABOLA");
      break;
    case dc::BlurFilter::Function::SINUS:
      return QStringLiteral("SINUS");
      break;
    case dc::BlurFilter::Function::ELLIPSE:
      return QStringLiteral("ELLIPSE");
      break;
    case dc::BlurFilter::Function::HYPERBOLA:
    default:
      return QStringLiteral("HYPERBOLA");
      break;
  }
}

static QString
Blur_getMethodStr(dc::BlurFilter::Method m)
{
  switch (m) {
    case dc::BlurFilter::Method::GAUSSIAN:
      return QStringLiteral("GAUSSIAN");
      break;
    case dc::BlurFilter::Method::MEDIAN:
      return QStringLiteral("MEDIAN");
      break;
    case dc::BlurFilter::Method::NORMAL:
    default:
      return QStringLiteral("NORMAL");
      break;
  }
}

static QString
Blur_getAreaStr(dc::BlurFilter::Area a)
{
  switch (a) {
    case dc::BlurFilter::Area::UP:
      return QStringLiteral("UP");
      break;
    case dc::BlurFilter::Area::DOWN:
      return QStringLiteral("DOWN");
      break;
    case dc::BlurFilter::Area::CENTERED:
    default:
      return QStringLiteral("CENTERED");
      break;
  }
}

static QString
Blur_getXML_area(const QString &filename,
                 dc::BlurFilter::Function function,
                 dc::BlurFilter::Area area,
                 float coeff1,
                 int vert,
                 int horiz,
                 int radius,
                 dc::BlurFilter::Method method,
                 int intensity)
{
  QString xml = QStringLiteral("<Degradation>\n");
  xml += "\t<PictureName>" + filename + "</PictureName>\n";
  xml += QLatin1String("\t<DegradationName>BlurZone</DegradationName>\n");
  xml += QLatin1String("\t<Parameters>\n");
  xml += "\t\t<Function>" + Blur_getFunctionStr(function) + "</Function>\n";
  xml += "\t\t<Area>" + Blur_getAreaStr(area) + "</Area>\n";
  xml += "\t\t<coeff1>" + QString::number(coeff1) + "</coeff1>\n";
  xml += "\t\t<Vertical>" + QString::number(vert) + "</Vertical>\n";
  xml += "\t\t<Horizontal>" + QString::number(horiz) + "</Horizontal>\n";
  xml += "\t\t<Radius>" + QString::number(radius) + "</Radius>\n";
  xml += "\t\t<Method>" + Blur_getMethodStr(method) + "</Method>\n";
  xml += "\t\t<Intensity>" + QString::number(intensity) + "</Intensity>\n";
  xml += QLatin1String("\t</Parameters>\n");
  xml += QLatin1String("</Degradation>");
  return xml;
}

static QString
Blur_getXML_page(const QString &filename,
                 dc::BlurFilter::Method method,
                 int intensity)
{
  QString xml = QStringLiteral("<Degradation>\n");
  xml += "\t<PictureName>" + filename + "</PictureName>\n";
  xml +=
    QLatin1String("\t<DegradationName>BlurComplete</DegradationName>\n");
  xml += QLatin1String("\t<Parameters>\n");
  xml += "\t\t<Method>" + Blur_getMethodStr(method) + "</Method>\n";
  xml += "\t\t<Intensity>" + QString::number(intensity) + "</Intensity>\n";
  xml += QLatin1String("\t</Parameters>\n");
  xml += QLatin1String("</Degradation>");
  return xml;
}

/**
apply blur on a specific area

@param[in] recto input image
*/
static QImage
Blur_applyArea(const QImage &recto,
               dc::BlurFilter::Function function,
               dc::BlurFilter::Area area,
               float coeff1,
               int vert,
               int horiz,
               int radius,
               dc::BlurFilter::Method method,
               int intensity)
{
  //B:TODO: is it equivalent ???
  //QImage pattern = makePattern(recto, function, area, coeff1, vert, horiz, radius);
  //return applyPattern(recto, pattern, method, intensity);
  return dc::BlurFilter::blur(
    recto, method, intensity, function, area, coeff1, vert, horiz, radius);
}

static void
Blur_applyAreaAndSave(const QImage &recto,
                      int index,
                      dc::BlurFilter::Function f,
                      dc::BlurFilter::Area a,
                      float coeff1,
                      int vert,
                      int horiz,
                      int radius,
                      dc::BlurFilter::Method m,
                      int intensity,
                      const QString &imageBasename,
                      const QString &outputImageDir)
{
  QImage blurTmp =
    Blur_applyArea(recto, f, a, coeff1, vert, horiz, radius, m, intensity);

  const QString prefixFilename = imageBasename + "Blur_" +
                                 Blur_getFunctionStr(f) + "_" +
                                 QString::number(index);
  const QString filename = prefixFilename + ".png";

  blurTmp.save(outputImageDir + filename);

  const QString xml =
    Blur_getXML_area(filename, f, a, coeff1, vert, horiz, radius, m, intensity);

  saveXml(xml, outputImageDir + prefixFilename + ".xml");
}

static void
Blur_applyPageAndSave(const QImage &recto,
                      int index,
                      dc::BlurFilter::Method m,
                      int intensity,
                      const QString &imageBasename,
                      const QString &outputImageDir)
{
  QImage blurTmp = dc::BlurFilter::blur(recto, m, intensity);

  const QString prefixFilename =
    imageBasename + "Blur_Complete_" + QString::number(index);
  const QString filename = prefixFilename + ".png";

  blurTmp.save(outputImageDir + filename);

  QString xml = Blur_getXML_page(filename, m, intensity);

  saveXml(xml, outputImageDir + prefixFilename + ".xml");
}

static int
Blur_getIntensity(int blurMin, int blurMax)
{
  int intensity = P_bounded_rand(blurMin, blurMax + 1);
  if (intensity % 2 == 0) //ensure odd intensity
    ++intensity;
  return intensity;
}

void
Assistant::do_blur(const QString &imageBasename,
                   const QImage &recto,
                   const QString &outputImageDir) const
{
  const int blurMax = this->blurMax();
  const int blurMin = this->blurMin();

  const int numDraws = this->nbTirageBlur();
  
  if (this->getBlur_ZoneEnable()) {

    const dc::BlurFilter::Area a = dc::BlurFilter::Area::UP;
    const float coeff1 = 1.f;
    const int vert = 50;
    const int horiz = 50;
    const int radius = 10;
    const dc::BlurFilter::Method m = dc::BlurFilter::Method::GAUSSIAN;
    
    if (this->pattern1()) {
      for (int i = 0; i < numDraws; ++i) {
	Blur_applyAreaAndSave(recto,
			      i,
			      dc::BlurFilter::Function::LINEAR,
			      a,
			      coeff1,
			      vert,
			      horiz,
			      radius,
			      m,
			      Blur_getIntensity(blurMin, blurMax),
			      imageBasename,
			      outputImageDir);

	updateProgress();
      }
    }

    if (this->pattern2()) {
      for (int i = 0; i < numDraws; ++i) {
	Blur_applyAreaAndSave(recto,
			      i,
			      dc::BlurFilter::Function::LOG,
			      a,
			      coeff1,
			      vert,
			      horiz,
			      radius,
			      m,
			      Blur_getIntensity(blurMin, blurMax),
			      imageBasename,
			      outputImageDir);

	updateProgress();
      }
    }

    if (this->pattern3()) {
      for (int i = 0; i < numDraws; ++i) {
	Blur_applyAreaAndSave(recto,
			      i,
			      dc::BlurFilter::Function::PARABOLA,
			      a,
			      coeff1,
			      vert,
			      horiz,
			      radius,
			      m,
			      Blur_getIntensity(blurMin, blurMax),
			      imageBasename,
			      outputImageDir);

	updateProgress();
      }
    }

    if (this->pattern4()) {
      for (int i = 0; i < numDraws; ++i) {
	Blur_applyAreaAndSave(recto,
			      i,
			      dc::BlurFilter::Function::SINUS,
			      a,
			      coeff1,
			      vert,
			      horiz,
			      radius,
			      m,
			      Blur_getIntensity(blurMin, blurMax),
			      imageBasename,
			      outputImageDir);

	updateProgress();
      }
    }

    if (this->pattern5()) {
      for (int i = 0; i < numDraws; ++i) {
	Blur_applyAreaAndSave(recto,
			      i,
			      dc::BlurFilter::Function::ELLIPSE,
			      a,
			      coeff1,
			      vert,
			      horiz,
			      radius,
			      m,
			      Blur_getIntensity(blurMin, blurMax),
			      imageBasename,
			      outputImageDir);

	updateProgress();
      }
    }

    if (this->pattern6()) {
      for (int i = 0; i < numDraws; ++i) {
	Blur_applyAreaAndSave(recto,
			      i,
			      dc::BlurFilter::Function::HYPERBOLA,
			      a,
			      coeff1,
			      vert,
			      horiz,
			      radius,
			      m,
			      Blur_getIntensity(blurMin, blurMax),
			      imageBasename,
			      outputImageDir);

	updateProgress();
      }
    }
  }

  //blur on whole page
  if (this->getBlur_PageEnable()) {

    const dc::BlurFilter::Method m = dc::BlurFilter::Method::GAUSSIAN;

    for (int i = 0; i < numDraws; ++i) {

      Blur_applyPageAndSave(recto,
			    i,
			    m,
			    Blur_getIntensity(blurMin, blurMax),
			    imageBasename,
			    outputImageDir);

      updateProgress();
    }
    
  }
}

void
Assistant::do_hole(const QString &imageBasename,
                   const QImage &recto,
                   const QString &outputImageDir) const
{
  //B:TODO: clean code

  const QString &path = outputImageDir;
  const QString prefix = imageBasename + "Hole_";
  const QString imgExt = QStringLiteral(".png");

  const float scaleW = 1.f;
  const float scaleH = 1.f;

  const QColor color = _Hole_colorBehind;

  const bool randPosX = true;
  const bool randPosY = true;

  std::vector<HoleData> holes;
  QImage rectoC;

  const int numDraws = ui->TirageHole->value();
  for (int i = 0; i < numDraws; ++i) {

    //std::cerr << "----------------------- hole " << i <<"/"<<nbTirages<< "\n";

    rectoC = recto.copy();

    holes.clear();
    holes.reserve(20); //arbitrary

    holes = Hole_doHoles(rectoC, scaleW, scaleH, color, randPosX, randPosY);

    const QString lPrefix = prefix + QString::number(i);
    Hole_saveImage(rectoC, path, lPrefix, imgExt, holes);
    //TODO: handle error !

    updateProgress();
  }
}



static
QString
ElasticDeformation_makePrefixXml(const QString &imageFilename,
				 const QString &method)
{
  QString xml = QStringLiteral("<Degradation>\n");
  xml += "\t<PictureName>" + imageFilename + "</PictureName>\n";
  xml += "\t<DegradationName>ElasticDeformation</DegradationName>\n";
  xml += QLatin1String("\t<Parameters>\n");
  xml += "\t\t<Type>" + method + "</Type>\n";
  return xml;
}

static
QString
ElasticDeformation_makeSuffixeXml()
{
  QString xml;
  xml += QLatin1String("\t</Parameters>\n");
  xml += QLatin1String("</Degradation>");
  return xml;
}

static
bool
ElasticDeformation_saveXml(const QString &outputXmlFilename,
			   const QString &imageFilename,
			   int method,
			   float alpha,
			   float sigma,
			   float alpha_affine,
			   int borderMode,
			   int interpolation)
{
  QString xml = ElasticDeformation_makePrefixXml(imageFilename, QString::number(method));
  xml += "\t\t<Alpha>" + QString::number(alpha) + "</Alpha>\n";
  xml += "\t\t<Sigma>" + QString::number(sigma) + "</Sigma>\n";
  if (method == 2) {
    xml += "\t\t<AlphaAffine>" + QString::number(alpha_affine) + "</AlphaAffine>\n";
  }
  xml += "\t\t<BorderMode>" + QString::number(borderMode) + "</BorderMode>\n";
  xml += "\t\t<Interpolation>" + QString::number(interpolation) + "</Interpolation>\n";
  xml += ElasticDeformation_makeSuffixeXml();
  return saveXml(xml, outputXmlFilename);
}

void
Assistant::do_ElasticDeformation(const QString &imageBasename,
				 const QImage &recto,
				 const QString &outputImageDir) const
{
  const int numDraws = ui->TirageElasticDeformation->value();
  for (int i = 0; i < numDraws; ++i) {

    if (ui->ElasticDeformation1CB->isChecked()) {

      const float alphaMin = ui->MinAlphaElasticDeformation1SB->value();
      const float alphaMax = ui->MaxAlphaElasticDeformation1SB->value();
      const float alpha = random_in_range(alphaMin, alphaMax);

      const float sigmaMin = ui->MinSigmaElasticDeformation1SB->value();
      const float sigmaMax = ui->MaxSigmaElasticDeformation1SB->value();
      const float sigma = random_in_range(sigmaMin, sigmaMax);

      const int borderMode = ui->BorderElasticDeformation1CB->currentData().toInt();
      const int interpolation = ui->InterpolationElasticDeformation1CB->currentData().toInt();

      QImage imgN;
      imgN = dc::ElasticDeformation::transform(recto, alpha, sigma, borderMode, interpolation);

      const QString prefixFilename = imageBasename + "ElasticDeformation_1_" + QString::number(i);
      const QString filename = prefixFilename + ".png";
      imgN.save(outputImageDir + filename);
      ElasticDeformation_saveXml(outputImageDir + prefixFilename + ".xml",
				 filename,
				 1,
				 alpha, sigma, 0, borderMode, interpolation);
      updateProgress();
    }

    if (ui->ElasticDeformation2CB->isChecked()) {

      const float alphaMin = ui->MinAlphaElasticDeformation2SB->value();
      const float alphaMax = ui->MaxAlphaElasticDeformation2SB->value();
      const float alpha = random_in_range(alphaMin, alphaMax);

      const float sigmaMin = ui->MinSigmaElasticDeformation2SB->value();
      const float sigmaMax = ui->MaxSigmaElasticDeformation2SB->value();
      const float sigma = random_in_range(sigmaMin, sigmaMax);

      const float alphaAffineMin = ui->MinAlphaAffineElasticDeformation2SB->value();
      const float alphaAffineMax = ui->MaxAlphaAffineElasticDeformation2SB->value();
      const float alphaAffine = random_in_range(alphaAffineMin, alphaAffineMax);

      
      const int borderMode = ui->BorderElasticDeformation2CB->currentData().toInt();
      const int interpolation = ui->InterpolationElasticDeformation2CB->currentData().toInt();

      QImage imgN;
      imgN = dc::ElasticDeformation::transform(recto, alpha, sigma, borderMode, interpolation);

      const QString prefixFilename = imageBasename + "ElasticDeformation_2_" + QString::number(i);
      const QString filename = prefixFilename + ".png";
      imgN.save(outputImageDir + filename);
      ElasticDeformation_saveXml(outputImageDir + prefixFilename + ".xml",
				 filename,
				 2,
				 alpha, sigma, alphaAffine, borderMode, interpolation);
      updateProgress();
    }
    
  }
}




void
Assistant::do_3D(const QString &imageBasename,
                 const QImage &recto,
                 const QString &outputImageDir) const
{
  const bool useBackground = ui->Dist3D_useBackgroundCheck->isChecked() &&
                             !_Dist3D_meshBackgroundsList.isEmpty();

  auto w = new GLWidget();
  w->show();
  w->setTexture(
    recto); //must be called after show() [to have an initialized GL context] !
  w->setUseTexture(true);

  if (useBackground) {
    w->setUseBackgroundTexture(true);
  }

  const int numDraws = ui->TirageDist3D->value();
  for (int i=0; i<numDraws; ++i) {

    for (const QString &meshFilename : _Dist3D_meshesList) {

      //std::cerr<<"meshFilename="<<meshFilename.toStdString()<<"\n";

      w->loadMesh(meshFilename);

      QString backgroundFilename;
      if (useBackground) {

	const int index = P_bounded_rand(0, _Dist3D_meshBackgroundsList.size());
	backgroundFilename = _Dist3D_meshBackgroundsList[index];
	QImage backgroundImg(backgroundFilename);

	w->setBackgroundTexture(backgroundImg);
      }

      QImage img = w->takeScreenshotHiRes();

      QFileInfo fiMesh(meshFilename);

      const QString meshBasename = fiMesh.baseName();

      const QString prefixFilename =
	imageBasename + "Distortion3D_"  + QString::number(i) +"_"+ meshBasename;
      const QString filename = prefixFilename + ".png";

      img.save(outputImageDir + filename);

      QString xml = QStringLiteral("<Degradation>\n");
      xml += "\t<PictureName>" + filename + "</PictureName>\n";
      xml +=
	QLatin1String("\t<DegradationName>Distortion 3D</DegradationName>\n");
      xml += QLatin1String("\t<Parameters>\n");
      xml += "\t\t<Mesh>" + fiMesh.fileName() + "</Mesh>\n";
      if (useBackground) {
	xml += "\t\t<Background>" + QFileInfo(backgroundFilename).fileName() +
	  "</Background>\n";
      }
      //TODO: save other parameters ???
      xml += QLatin1String("\t</Parameters>\n");
      xml += QLatin1String("</Degradation>");

      saveXml(xml, outputImageDir + prefixFilename + ".xml");

      updateProgress();
    }
  }

  delete w;
}

//To call when one more image has been saved
void
Assistant::updateProgress() const
{
  assert(_progressDialog);

  _progressDialog->setValue(_progressDialog->value() + 1);
  _numGeneratedImages += 1;
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void
Assistant::generateDegradedImages() const
{
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

  QGuiApplication::setOverrideCursor(Qt::BusyCursor);

  QString savePath = this->saveDirectory();
  QString inputImageDir = this->PicDirectory();
  QStringList picsList = this->picsList();

  if (!savePath.isEmpty() && savePath[savePath.length() - 1] != '/')
    savePath += '/';
  if (!inputImageDir.isEmpty() &&
      inputImageDir[inputImageDir.length() - 1] != '/')
    inputImageDir += '/';

  const int numPics = picsList.size();
  const size_t numImagesToGenerate = ui->TotalPic->text().toUInt();
  
  if (numPics > 0)
  {
    _numGeneratedImages = 0;
    _progressDialog =
      new QProgressDialog("Generating images...", "Cancel", 0, numImagesToGenerate);
    _progressDialog->setWindowModality(Qt::WindowModal);
    _progressDialog->setMinimumDuration(10); //time in ms before it appears
    _progressDialog->show();
  }

  for (int picIndex = 0; picIndex < numPics; ++picIndex) {
    const QString &pic = picsList[picIndex];

    //Load recto

    const QString imageBasename =
      QFileInfo(pic).completeBaseName() +
      "_"; //remove path & last extension, and add "_"

    const QString rectoPath = inputImageDir + pic;
    QImage recto(rectoPath);

    if (recto.isNull()) {
      std::cerr << "ERROR: unable to load image: " << rectoPath.toStdString()
                << "\n";
      //B:TODO: _progressDialog will not be updated correctly in this case.
    } 
    else {
      assert(!recto.isNull());

      //Apply bleedThrough if enabled
      if (this->bleedEnable()) {
        do_bleed(imageBasename, recto, picIndex, inputImageDir, savePath);

        if (_progressDialog->wasCanceled()) {
          break;
        }
      }

      //Apply CharacterDegradation if enabled
      if (this->charEnable()) {
        do_charDeg(imageBasename, recto, savePath);
        if (_progressDialog->wasCanceled()) {
          break;
        }
      }

      //Apply Shadow Binding if enabled
      if (this->shadEnable()) {
        do_shadow(imageBasename, recto, savePath);
        if (_progressDialog->wasCanceled()) {
          break;
        }
      }

      //Apply phantom characters if enabled
      if (this->phantEnable()) {
        do_phantom(imageBasename, recto, savePath);
        if (_progressDialog->wasCanceled()) {
          break;
        }
      }

      //Apply blur degradations if enabled
      if (this->blurEnable()) {
        do_blur(imageBasename, recto, savePath);
        if (_progressDialog->wasCanceled()) {
          break;
        }
      }

      //Apply holes if enabled
      if (this->holeEnable()) {
        do_hole(imageBasename, recto, savePath);
      }

      //Apply 3D if enabled
      if (this->dist3DEnable()) {
        do_3D(imageBasename, recto, savePath);
        if (_progressDialog->wasCanceled()) {
          break;
        }
      }

      //Apply Gradient Domain Degradation if enabled
      if (this->gddEnable()) {
        do_GDD(imageBasename, recto, savePath);
        if (_progressDialog->wasCanceled()) {
          break;
        }
      }

      //Apply NoiseDegradation if enabled
      if (this->noiseEnable()) {
        do_Noise(imageBasename, recto, savePath);
        if (_progressDialog->wasCanceled()) {
          break;
        }
      }

      //Apply RotationDegradation if enabled
      if (this->rotationEnable()) {
        do_Rotation(imageBasename, recto, inputImageDir, savePath);
        if (_progressDialog->wasCanceled()) {
          break;
        }
      }

    }
  }

  delete _progressDialog;
  _progressDialog = nullptr;

  QGuiApplication::restoreOverrideCursor();
}

void
Assistant::updateResults()
{
  const size_t numImagesToGenerate = ui->TotalPic->text().toUInt();

  ui->label_results->setText(
    tr("%1/%n image(s) have been written into directory:\n%3",
       "",
       numImagesToGenerate)
      .arg(_numGeneratedImages)
      .arg(_outputDegradedImageDir));
}

void
Assistant::accept()
{
  //std::cerr<<"accept() !\n";

  QWizard::accept();
}
