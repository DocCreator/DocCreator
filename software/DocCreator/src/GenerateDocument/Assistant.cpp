#define NOMINMAX //for Visual

#include "Assistant.hpp"

#include <algorithm>
#include <cassert>
#include <ctime>
#include <iostream>

#include <QColorDialog>
#include <QDebug>
#include <QFileDialog>
#include <QGuiApplication>
#include <QMessageBox>
#include <QTextStream>

#include "Degradations/BleedThrough.hpp"
#include "Degradations/BlurFilter.hpp"
#include "Degradations/Distortion3DModel/src/GLWidget.hpp"
#include "Degradations/GrayscaleCharsDegradationModel.hpp"
#include "Degradations/HoleDegradation.hpp"
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

Assistant::Assistant(DocumentController *doc, QWidget *parent)
  : QWizard(parent)
  , ui(new Ui::Assistant)
{
  ui->setupUi(this);

  const QImage semi(QStringLiteral(":/images/semi.png"));
  const QImage synth(QStringLiteral(":/images/synth.png"));
  ui->ImageSemi->setPixmap(QPixmap::fromImage(semi));
  ui->ImageSynth->setPixmap(QPixmap::fromImage(synth));

  _DocController = doc;

  BleedThrough_setupGUIImages();
  CharDeg_setupGUIImages();
  Shadow_setupGUIImages();
  Phantom_setupGUIImages();
  Blur_setupGUIImages();
  Hole_setupGUIImages();
  Dist3D_setupGUIImages();

  _BleedThrough_bleedEnable = false;
  _CharDeg_charEnable = false;
  _Shadow_shadEnable = false;
  _Phantom_phantEnable = false;
  _Blur_blurEnable = false;
  _Blur_PageEnable = false;
  _Blur_ZoneEnable = false;
  _Hole_holeEnable = false;
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

  const QString fontPath = QDir(Core::ConfigurationManager::get(
                                  AppConfigMainGroup, AppConfigFontFolderKey)
                                  .toString())
                             .absolutePath();
  updateListFont(fontPath);
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

  //QObject::connect(ui->pageNumber, SIGNAL(valueChanged(int)), this, SIGNAL(completeChanged()));

  QObject::connect(
    ui->btnChooseFontDir, SIGNAL(clicked()), this, SLOT(chooseFontDirectory()));
  QObject::connect(ui->listFontView,
                   SIGNAL(clicked(QModelIndex)),
                   this,
                   SLOT(fontSelectionChanges()));

  QObject::connect(ui->btnChooseBackgroundDir,
                   SIGNAL(clicked()),
                   this,
                   SLOT(chooseBackgroundDirectory()));
  QObject::connect(ui->listBackgroundView,
                   SIGNAL(clicked(QModelIndex)),
                   this,
                   SLOT(backgroundSelectionChanges()));
  //QObject::connect(ui->LoremIpsum, SIGNAL(clicked()),this, SLOT(changeLoremIpsum()));
  //QObject::connect(ui->FolderText, SIGNAL(clicked()), this, SLOT(changeLoremIpsum()));

  PageParams_connect();

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
                   SLOT(Shadow_TirageShadowChanged(int)));

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
                   SLOT(Phantom_TiragePhantomChanged(int)));

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
                   SLOT(Blur_TirageBlurChanged(int)));
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
                   SLOT(Hole_TirageHoleChanged(int)));

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
                   SLOT(Dist3D_TirageDist3DChanged(int)));

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
}

static QImage
takePart(const QImage &img)
{
  int x = std::max(0, std::min(BLEED_X, img.width() - BLEED_PREVIEW_WIDTH));
  int y = std::max(0, std::min(BLEED_Y, img.height() - BLEED_PREVIEW_HEIGHT));
  int w = std::min(BLEED_PREVIEW_WIDTH, img.width());
  int h = std::min(BLEED_PREVIEW_HEIGHT, img.height());

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
    msgBox.setInformativeText(tr("Do you want to proceed anyway?"));
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
        generateTxtImages(); //B: we can not call generateTxtImages in nextId() as it is const (and called several times) ! So we do it here...
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

      generateDegradedImages(); //B: we od it here instead of in accept, cf REM1.

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
According to Qt documentation, the solution would be to have one individual class inheriting from QWizardPage for each page, and override isComplete() (and emit completeChange() when necessary).
But as we have all the GUI defined in Assistant.ui, it probably entails that we have to do one .ui per page !?

*/

/*
  B: if the order of pages is changed in nextId()
  it is probably a good idea to also change the display order on Page_FinalDegradations
  int Assistant.ui file

 */

int
Assistant::nextId() const
{
  switch (currentId()) {
    case Page_SyntheticOrSemiChoice:
      if (ui->Synthetic->isChecked()) {
        return Page_TextType;
      } else if (ui->Semi->isChecked()) {
        return Page_ImageAndGtDirs;
      }
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
      return Page_BackgroundFiles;
      break;

    case Page_BackgroundFiles:
      if (_backgroundListChoice.isEmpty())
        return currentId();
      return Page_PageParams;
      break;

    case Page_PageParams:
      if (!PageParams_isComplete())
        return currentId();
      return Page_FinalText;
      break;

    case Page_FinalText:
      if (_outputTxtImageDir.isEmpty())
        return currentId();
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
      return Page_Bleed;
      break;
    case Page_Bleed:
      return Page_Blur;
      break;
    case Page_Blur:
      return Page_Shadow;
      break;
    case Page_Shadow:
      return Page_Hole;
      break;
    case Page_Hole:
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
    ui->txtFilePath->setText(path);
    _txtDirectory = path;

    this->populateTxtList();
  }
}

void
Assistant::populateTxtList()
{
  QGuiApplication::setOverrideCursor(Qt::BusyCursor);

  _txtList.clear();

  if (!_txtDirectory.isEmpty()) {
    QStringList nameFilters;
    nameFilters << QStringLiteral("*.txt");

    QDir directory(_txtDirectory);
    QList<QString> fileList =
      directory.entryList(nameFilters, QDir::Files | QDir::Readable);
    _txtList.reserve(fileList.size());
    for (const QString &file : fileList) {
      _txtList.append(directory.relativeFilePath(file));
    }
  }
  ui->label_nbTextFiles->setText(tr("%n text file(s)", "", _txtList.count()));

  QGuiApplication::restoreOverrideCursor();

  //emit completeChanged();
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

  const QStringList list = Context::FontContext::instance()->getFontNames();

  _fontList.setStringList(list);
  ui->listFontView->setModel(&_fontList);
  //ui->listFontView->selectAll();

  fontSelectionChanges(); //B: useful ?
}

void
Assistant::fontSelectionChanges()
{
  QModelIndexList listSelectionsFont =
    ui->listFontView->selectionModel()->selectedIndexes();
  _fontListChoice.clear();
  _fontListChoice.reserve(listSelectionsFont.count());
  for (const auto &f : listSelectionsFont) {
    const QString selectedFont = _fontList.data(f, Qt::DisplayRole).toString();
    _fontListChoice.append(selectedFont);
  }

  ui->label_nbFonts->setText(
    tr("%n selected font(s)", "", _fontListChoice.size()));

  //emit completeChanged();
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
    ui->backgroundPath->setText(path);
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
  _backgroundList.setStringList(listBack);
  ui->listBackgroundView->setModel(&_backgroundList);
  //ui->listBackgroundView->selectAll();

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
      _backgroundList.data(sb, Qt::DisplayRole).toString();
    _backgroundListChoice.append(selectedBack);
  }

  ui->label_nbBackgrounds->setText(
    tr("%n selected background image(s)", "", _backgroundListChoice.size()));

  //emit completeChanged();
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

  if (ui->spinBox_marginTopMin->value() > ui->spinBox_marginTopMax->value())
    ui->spinBox_marginTopMax->setValue(ui->spinBox_marginTopMin->value());
  if (ui->spinBox_marginBottomMin->value() >
      ui->spinBox_marginBottomMax->value())
    ui->spinBox_marginBottomMax->setValue(ui->spinBox_marginBottomMin->value());
  if (ui->spinBox_marginRightMin->value() > ui->spinBox_marginRightMax->value())
    ui->spinBox_marginRightMax->setValue(ui->spinBox_marginRightMin->value());
  if (ui->spinBox_marginLeftMin->value() > ui->spinBox_marginLeftMax->value())
    ui->spinBox_marginLeftMax->setValue(ui->spinBox_marginLeftMin->value());

  if (ui->spinBox_blockXMin->value() > ui->spinBox_blockXMax->value())
    ui->spinBox_blockXMax->setValue(ui->spinBox_blockXMin->value());
  if (ui->spinBox_blockYMin->value() > ui->spinBox_blockYMax->value())
    ui->spinBox_blockYMax->setValue(ui->spinBox_blockYMin->value());

  if (ui->spinBox_lineSpacingMin->value() > ui->spinBox_lineSpacingMax->value())
    ui->spinBox_lineSpacingMax->setValue(ui->spinBox_lineSpacingMin->value());

  //emit completeChanged();
}

void
Assistant::PageParams_updateMax()
{
  //spinBox_YYYMax was updated, we change spinBox_YYYMin

  if (ui->spinBox_marginTopMax->value() < ui->spinBox_marginTopMin->value())
    ui->spinBox_marginTopMin->setValue(ui->spinBox_marginTopMax->value());
  if (ui->spinBox_marginBottomMax->value() <
      ui->spinBox_marginBottomMin->value())
    ui->spinBox_marginBottomMin->setValue(ui->spinBox_marginBottomMax->value());
  if (ui->spinBox_marginRightMax->value() < ui->spinBox_marginRightMin->value())
    ui->spinBox_marginRightMin->setValue(ui->spinBox_marginRightMax->value());
  if (ui->spinBox_marginLeftMax->value() < ui->spinBox_marginLeftMin->value())
    ui->spinBox_marginLeftMin->setValue(ui->spinBox_marginLeftMax->value());

  if (ui->spinBox_blockXMax->value() < ui->spinBox_blockXMin->value())
    ui->spinBox_blockXMin->setValue(ui->spinBox_blockXMax->value());
  if (ui->spinBox_blockYMax->value() < ui->spinBox_blockYMin->value())
    ui->spinBox_blockYMin->setValue(ui->spinBox_blockYMax->value());

  if (ui->spinBox_lineSpacingMax->value() < ui->spinBox_lineSpacingMin->value())
    ui->spinBox_lineSpacingMin->setValue(ui->spinBox_lineSpacingMax->value());

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
      tr("With %1 text(s), 1 random font and 1 one random background image:\n "
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

  if (_Shadow_shadEnable)
    totalPic += _Shadow_nbShadSelected * ui->TirageShadow->value() * nbPic;

  totalPic += Blur_nbDegradations() * ui->TirageBlur->value() *
              nbPic; //_Blur_nbTirageBlur * nbPic;

  if (_Hole_holeEnable)
    totalPic += 1 * ui->TirageHole->value() * nbPic;

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
  std::cerr << "Assistant::addInputImage " << imageFilename.toStdString()
            << "\n";
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
	  const QString txtPath = _txtDirectory + "/" + file;
	  const QString fontPath = _FontDirectory + "/" + font + ".of";
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
	const QString fontPath = _FontDirectory + "/" + font + ".of";
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
                   SIGNAL(imageSaved(const QString &)),
                   this,
                   SLOT(addInputImage(const QString &)));

  if (ui->zeroPaddingTextCB->isChecked()) {
    const int nbGeneratedTexts = computeNbGeneratedTexts();
    const int numberWidth =
      computeNumberWidth(nbGeneratedTexts + xmlDocExporter._nb);
    xmlDocExporter.setNumberWidth(numberWidth);
    docExporter.setNumberWidth(numberWidth);
  }

  RandomDocumentParameters params;
  PageParams_getParams(params);

  params.outputFolderPath = _outputTxtImageDir;

  //copy list of backgrounds/textFiles/fonts in params
  params.backgroundList = _backgroundListChoice;
  const QStringList &txtList = _txtList;
  for (const QString &txtFile : txtList) {
    const QString txtPath = _txtDirectory + "/" + txtFile;
    params.textList.append(txtPath);
  }
  const QStringList &fontListChoice = _fontListChoice;
  for (const QString &font : fontListChoice) {
    const QString fontPath = _FontDirectory + "/" + font + ".of";
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
  } else { //all combinations
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
  std::cerr << "_inputImageList.size()=" << _inputImageList.size() << "\n";
  enableAccordingToInputImages();

  QGuiApplication::restoreOverrideCursor();
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
                          (static_cast<float>(r) / RAND_MAX * (maxV - minV)));
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
    QString chemin = _inputImageList[nbPic];
    chemin = _PicDirectory + "/" + chemin;
    _BleedThrough_rectoImg.load(chemin);
    //std::cout << ".............  " << chemin.toStdString() << std::endl;
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

    QString chemin = _inputImageList[nbPic];
    chemin = _PicDirectory + "/" + chemin;
    _BleedThrough_versoImg.load(chemin);
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
  ui->SwitchBleed->setToolTip(tr("Switch both recto and verso images"));

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

  _BleedThrough_bleedImgPart = bleedThrough(_BleedThrough_rectoImgPart,
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

  _BleedThrough_bleedImgPart = bleedThrough(_BleedThrough_rectoImgPart,
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

    int nbPic = P_bounded_rand(0, _inputImageList.size());
    if (_inputImageList.size() > 1)
      while (nbPic == _CharDeg_indexRecto)
        nbPic = P_bounded_rand(0, _inputImageList.size());

    _CharDeg_indexRecto = nbPic;
    assert(nbPic < _inputImageList.size());
    QString chemin = _inputImageList[nbPic];
    chemin = _PicDirectory + "/" + chemin;
    _CharDeg_rectoImgChar.load(chemin);
    //std::cout << ".............  " << chemin.toStdString() << std::endl;

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
  GrayscaleCharsDegradationModel cdgMin(_CharDeg_rectoImgPartChar);
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
  GrayscaleCharsDegradationModel cdgMax(_CharDeg_rectoImgPartChar);
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
Assistant::Shadow_LoadPrevImgShad()
{
  if (!_inputImageList.empty()) {

    int nbPic = P_bounded_rand(0, _inputImageList.size());
    if (_inputImageList.size() > 1)
      while (nbPic == _Shadow_indexRecto)
        nbPic = P_bounded_rand(0, _inputImageList.size());
    assert(nbPic < _inputImageList.size());

    _Shadow_indexRecto = nbPic;
    QString path = _inputImageList[nbPic];
    path = _PicDirectory + "/" + path;
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
Assistant::Shadow_setPreview(ShadowBorder border)
{
  const float distanceRatio =
    ui->shad_width->value() / (float)ui->shad_width->maximum();
  const float intensity =
    1.f - ui->shad_intensity->value() / (float)ui->shad_intensity->maximum();
  const float angle = ui->shad_angle->value();

  QImage deg = shadowBinding(
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
  ShadowBorder border;
  if (cb == ui->RightShad)
    border = ShadowBorder::RIGHT;
  else if (cb == ui->TopShad)
    border = ShadowBorder::TOP;
  else if (cb == ui->LeftShad)
    border = ShadowBorder::LEFT;
  else //if (cb == ui->BottomShad)
    border = ShadowBorder::BOTTOM;

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

  //std::cerr<<"DEBUG Shadow_updateTirageAndTotal() nbDegs="<<nbDegs<<" total="<<total<<"\n";

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
      0); //signal/slot will call Shadow_TirageShadowChanged(0)
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
Assistant::Shadow_TirageShadowChanged(int /*value*/)
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

    int nbPic = P_bounded_rand(0, _inputImageList.size());
    if (_inputImageList.size() > 1)
      while (_Phantom_indexRecto == nbPic)
        nbPic = P_bounded_rand(0, _inputImageList.size());

    _Phantom_indexRecto = nbPic;

    QString chemin = _inputImageList[nbPic];
    chemin = _PicDirectory + "/" + chemin;
    _Phantom_rectoImgPhant.load(chemin);

    if (!_Phantom_rectoImgPhant.isNull()) {

      _Phantom_rectoImgPartPhant = takePart(_Phantom_rectoImgPhant);

      //ui->PhantomPreviewLabel->setPixmap(QPixmap::fromImage(_Phantom_rectoImgPartPhant));
      //ui->PhantomPreviewLabel->setMinimumSize(_Phantom_rectoImgPartPhant.size());

      //apply current parameters
      if (ui->Phantom_VeryFrequent->isChecked()) {
        Phantom_apply(Frequency::VERY_FREQUENT);
      } else if (ui->Phantom_Frequent->isChecked()) {
        Phantom_apply(Frequency::FREQUENT);
      } else if (ui->Phantom_Rare->isChecked()) {
        Phantom_apply(Frequency::RARE);
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

  if (
    nbDegs > 0 &&
    ui->TiragePhantom->value() ==
      0) { //set TiragePhantom value to 1 once we have some degradations selected
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
  } else {
    _Phantom_phantEnable = enabled;
    ui->NbDegPhantom->setEnabled(enabled);
    ui->TiragePhantom->setEnabled(enabled);
    ui->TotalPhantom->setEnabled(enabled);

    ui->TiragePhantom->setValue(
      0); //signal/slot will call Phantom_TiragePhantomChanged(0)
  }

  ui->OptionPhant->setEnabled(enabled);
  ui->Phantom_Frequent->setEnabled(enabled);
  ui->Phantom_Rare->setEnabled(enabled);
  ui->Phantom_VeryFrequent->setEnabled(enabled);
  ui->SwitchPhant->setEnabled(switchEnabled);
}

void
Assistant::Phantom_TiragePhantomChanged(int /*value*/)
{
  const int nbDegs = _Phantom_nbPhantSelected;
  ui->NbDegPhantom->setText(QString::number(nbDegs));
  const int total =
    nbDegs * _inputImageList.size() * ui->TiragePhantom->value();
  ui->TotalPhantom->setText(QString::number(total));

  //std::cerr<<"Phantom_TiragePhantomChanged(v) nbDegs="<<nbDegs<<" total="<<total<<"\n";

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
Assistant::Phantom_apply(Frequency frequency)
{
  PhantomCharacter phant(
    _Phantom_rectoImgPartPhant,
    frequency); //B? should we apply on big image and takePart afterwards ?
  QImage Deg = phant.apply();
  ui->PhantomPreviewLabel->setPixmap(QPixmap::fromImage(Deg));
  ui->PhantomPreviewLabel->setMinimumSize(Deg.size());
}

void
Assistant::Phantom_updatePreview()
{
  //we apply the most frequent checked one to be more visible

  if (ui->Phantom_VeryFrequent->isChecked()) {
    Phantom_apply(Frequency::VERY_FREQUENT);
  } else if (ui->Phantom_Frequent->isChecked()) {
    Phantom_apply(Frequency::FREQUENT);
  } else if (ui->Phantom_Rare->isChecked()) {
    Phantom_apply(Frequency::RARE);
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
    ui->TirageBlur->setValue(
      1); //will call Blur_TirageBlurChanged via signal/slot
  } else {
    const int total = nbDegs * _inputImageList.size() *
                      ui->TirageBlur->value(); //_Blur_nbTirageBlur;
    ui->TotalBlur->setText(QString::number(total));
    updateTotalPic();
  }
}

void
Assistant::Blur_TirageBlurChanged(int /*value*/)
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
  const QString chemin = Context::BackgroundContext::instance()->getPath() +
                         "../Image/blurImages/blurPatterns/";

  QImage pattern1;
  pattern1.load(chemin + "patternArea1.png");
  QImage pattern2;
  pattern2.load(chemin + "patternArea2.png");
  QImage pattern3;
  pattern3.load(chemin + "patternArea3.png");
  QImage pattern4;
  pattern4.load(chemin + "patternArea4.png");
  QImage pattern5;
  pattern5.load(chemin + "patternArea5.png");
  QImage pattern6;
  pattern6.load(chemin + "patternArea6.png");

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
      0); //signal/slot will call Blur_TirageBlurChanged(0)
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
    QImage blurTmp =
      blurFilter(_Blur_rectoImgBlur, Method::GAUSSIAN, intensity);
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
    int nbPic = P_bounded_rand(0, _inputImageList.size());
    if (!_inputImageList.empty())
      while (_Blur_indexRecto == nbPic)
        nbPic = P_bounded_rand(0, _inputImageList.size());
    assert(nbPic < _inputImageList.size());
    _Blur_indexRecto = nbPic;
    QString chemin = _inputImageList[nbPic];
    chemin = _PicDirectory + "/" + chemin;
    _Blur_rectoImgBlur.load(chemin);
    //std::cout << ".............  " << chemin.toStdString() << std::endl;
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
      makePattern(_Blur_rectoImgBlur, Function::LINEAR, Area::UP, 1, 50, 50);
    QImage Deg =
      applyPattern(_Blur_rectoImgBlur, pattern, Method::GAUSSIAN, intensity);
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
    QImage pattern =
      makePattern(_Blur_rectoImgBlur, Function::LOG, Area::UP, 1, 50, 50);
    QImage Deg =
      applyPattern(_Blur_rectoImgBlur, pattern, Method::GAUSSIAN, intensity);
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
      makePattern(_Blur_rectoImgBlur, Function::PARABOLA, Area::UP, 1, 50, 50);
    QImage Deg =
      applyPattern(_Blur_rectoImgBlur, pattern, Method::GAUSSIAN, intensity);
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
      makePattern(_Blur_rectoImgBlur, Function::SINUS, Area::UP, 1, 50, 50);
    QImage Deg =
      applyPattern(_Blur_rectoImgBlur, pattern, Method::GAUSSIAN, intensity);
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
      makePattern(_Blur_rectoImgBlur, Function::ELLIPSE, Area::UP, 1, 50, 50);
    QImage Deg =
      applyPattern(_Blur_rectoImgBlur, pattern, Method::GAUSSIAN, intensity);
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
      makePattern(_Blur_rectoImgBlur, Function::HYPERBOLA, Area::UP, 1, 50, 50);
    QImage Deg =
      applyPattern(_Blur_rectoImgBlur, pattern, Method::GAUSSIAN, intensity);
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
      0); //signal/slot will call Hole_TirageHoleChanged(0);
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
  const int nbDegs = (_Hole_nbHoleSelected ? 1 : 0);

  ui->NbDegHole->setText(QString::number(nbDegs));

  const bool enabled = (nbDegs > 0);
  ui->NbDegHole->setEnabled(enabled);
  ui->TirageHole->setEnabled(enabled);
  ui->TotalHole->setEnabled(enabled);
  if (nbDegs == 0)
    ui->TirageHole->setValue(0);

  if (nbDegs > 0 && ui->TirageHole->value() == 0) {
    ui->TirageHole->setValue(
      1); //will call Hole_TirageHoleChanged via signal/slot
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
Assistant::Hole_TirageHoleChanged(int /*value*/)
{
  const int nbDegs = (_Hole_nbHoleSelected ? 1 : 0);
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
void Assistant::Hole_TirageCornerChanged(int value)
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

void Assistant::Hole_TirageCenterChanged(int value)
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

    int nbPic = P_bounded_rand(0, _inputImageList.size());
    if (_inputImageList.size() > 1)
      while (nbPic == _Hole_indexRecto)
        nbPic = P_bounded_rand(0, _inputImageList.size());

    _Hole_indexRecto = nbPic;

    QString chemin = _inputImageList[nbPic];
    chemin = _PicDirectory + "/" + chemin;
    _Hole_rectoImgHole.load(chemin);
    //std::cout << ".............  " << chemin.toStdString() << std::endl;
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
  const std::vector<size_t> &m_areas;

  explicit ImageAreaSorter(const std::vector<size_t> &areas)
    : m_areas(areas)
  {}

  bool operator()(int i, int j) const
  {
    assert(i >= 0 && i < (int)m_areas.size());
    assert(j >= 0 && j < (int)m_areas.size());

    return m_areas[i] < m_areas[j];
  }
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
      area = image.width() * (size_t)image.height();
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
  HoleType type;
  int side;
  QColor color;

  explicit HoleData(const QString &pattern = QString(),
                    int pX = 0,
                    int pY = 0,
                    int pSize = 0,
                    HoleType pType = HoleType::CENTER,
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
                          HoleType type,
                          const QColor &color,
                          int side = 0)
{
  HoleData result;

  assert(ind < holePatterns.size());
  const QString filename = holePatterns[ind];
  QImage holePattern(filename);

  if (!holePattern.isNull()) {

    std::cerr << "   filename=" << filename.toStdString() << "\n";

    if (scaleW != 1.f && scaleH != 1.f) {
      const int holeWidth = static_cast<int>(holePattern.width() * scaleW);
      const int holeHeight = static_cast<int>(holePattern.height() * scaleH);
      holePattern = holePattern.scaled(
        holeWidth, holeHeight, Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    assert(!holePattern.isNull());

    //B: Some of these checks should be in lower-level function !
    int startX = 0;
    int startY = 0;
    int endX = degImg.width();
    int endY = degImg.height();

    //B: Here we move the corner/border pattern
    //   according to its 'side'.
    //top left corner of pattern x coord will be in [startX; endX]
    // and y coord will be in [startY; endY]

    const float ratio = 0.3f; //must be in [0; 1.0]
    // 0.6 means 60% of pattern can be outside
    // With ratio=1 we often have pattern completely outside

    const int holeW = holePattern.width();
    const int holeH = holePattern.height();
    const int imgW = degImg.width();
    const int imgH = degImg.height();

    const int rw = std::max(static_cast<int>(holeW * ratio), 1);
    const int rh = std::max(static_cast<int>(holeH * ratio), 1);

    if (type == HoleType::CORNER) {

      //holePattern is oriented to be top-left corner. But pattern geometry does not change for other corners.
      switch (static_cast<Corner>(side)) {
        case Corner::TOPLEFT:
          startX = -rw;
          endX = 1;
          startY = -rh;
          endY = 1;
          break;
        case Corner::TOPRIGHT:
          startX = imgW - holeW;
          endX = imgW + rw - holeW;
          startY = -rh;
          endY = 1;
          break;
        case Corner::BOTTOMRIGHT:
          startX = imgW - holeW;
          endX = imgW + rw - holeW;
          startY = imgH - holeH;
          endY = imgH + rh - holeH;
          break;
        case Corner::BOTTOMLEFT:
        default:
          startX = -rw;
          endX = 1;
          startY = imgH - holeH;
          endY = imgH + rh - holeH;
          break;
      }

      //std::cerr<<"   corner side="<<side<<" startX="<<startX<<" endX="<<endX<<" startY="<<startY<<" endY="<<endY<<"\n";
    } else if (type == HoleType::BORDER) {
      //Warning: hole pattern is oriented for top border. Its geometry changes for borders LEFT & RIGHT
      switch (static_cast<Border>(side)) {
        case Border::TOP:
          startX = -rw;
          endX = imgW + rw - holeW;
          startY = -rh; //0
          endY = 1;     //1
          break;
        case Border::RIGHT:
          startX = imgW - holeH;    //imgW-holeH;
          endX = imgW - holeH + rh; //imgW-holeH+1;
          startY = -rw;
          endY = imgH + rw - holeW;
          break;
        case Border::BOTTOM:
          startX = -rw;
          endX = imgW + rw - holeW;
          startY = imgH - holeH;
          endY = imgH + rh - holeH; //imgH-holeH+1;
          break;
        case Border::LEFT:
          startX = -rh; //0;
          endX = 1;
          startY = -rw;
          endY = imgH + holeW - rw;
          break;
      }
    }

    int posX = 0;
    if (randPosX)
      posX = P_bounded_rand(startX, endX);
    int posY = 0;
    if (randPosY)
      posY = P_bounded_rand(startY, endY);

    //std::cerr<<"    posX="<<posX<<" posY="<<posY<<"  img w="<<imgW<<" h="<<imgH<<" hole w="<<holeW<<" h="<<holeH<<"\n";

    const int size = 0;

    result = HoleData(filename, posX, posY, size, type, side, color);

    degImg =
      holeDegradation(degImg, holePattern, posX, posY, size, type, side, color);

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
    const HoleType type = HoleType::CORNER;

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
    const HoleType type = HoleType::BORDER;

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
    const HoleType type = HoleType::CENTER;

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
  } else {
    _Dist3D_dist3DEnable = enabled;
    ui->NbDegDist3D->setEnabled(enabled);
    ui->TirageDist3D->setEnabled(enabled);
    ui->TotalDist3D->setEnabled(enabled);

    ui->TirageDist3D->setValue(
      0); //signal/slot will call Dist3D_TirageDist3DChanged(0)
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
Assistant::Dist3D_TirageDist3DChanged(int /*value*/)
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
Assistant::shadEnable() const
{
  return _Shadow_shadEnable;
}

bool
Assistant::phantEnable() const
{
  return _Phantom_phantEnable; //ui->Phantom_Rare->isChecked();
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
              ShadowBorder border,
              float intensity,
              float angle)
{
  QString saveXml = QStringLiteral("<Degradation>\n");
  saveXml += "\t<PictureName>" + filename + "</PictureName>\n";
  saveXml += QLatin1String("\t<DegradationName>Shadow</DegradationName>\n");
  saveXml += QLatin1String("\t<Parameters>\n");
  saveXml += "\t\t<DistanceRatio>" + QString::number(distanceRatio) +
             "</DistanceRatio>\n";
  saveXml +=
    "\t\t<Border>" + QString::number(static_cast<int>(border)) + "</Border>\n";
  saveXml += "\t\t<Intensity>" + QString::number(intensity) + "</Intensity>\n";
  saveXml += "\t\t<Angle>" + QString::number(angle) + "</Angle>\n";
  saveXml += QLatin1String("\t</Parameters>\n");
  saveXml += QLatin1String("</Degradation>");
  return saveXml;
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
  QFile file(absoluteFilenameXML);
  const bool openOk = file.open(QIODevice::WriteOnly | QIODevice::Text);
  if (!openOk) {
    std::cerr << "ERROR: unable to write xml file: "
              << absoluteFilenameXML.toStdString() << "\n";
    return false;
  }
  QTextStream out(&file);
  out << xml;
  file.close();

  return true;
}

static QString
Shadow_getBorderStr(ShadowBorder border)
{
  switch (border) {
    case ShadowBorder::TOP:
      return QStringLiteral("Top");
      break;
    case ShadowBorder::RIGHT:
      return QStringLiteral("Right");
      break;
    case ShadowBorder::BOTTOM:
      return QStringLiteral("Bottom");
      break;
    case ShadowBorder::LEFT:
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
                 ShadowBorder border,
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
  QString saveXml = "<Degradation>\n";
  saveXml += "\t<PictureName>" + filename +  "</PictureName>\n";
  saveXml += "\t<DegradationName>Hole</DegradationName>\n";
  saveXml += "\t\t<Number>"+ QString::number(1) +"</Number>\n";
  saveXml += "\t<Parameters>\n";
  saveXml += "\t\t<Hole>" + patternFilename + "</Hole>\n";
  saveXml += "\t\t<HoleType>" + QString::number(static_cast<int>(type)) + "</HoleType>\n";
  saveXml += "\t\t<Side>" + QString::number(side) + "</Position>\n";
  saveXml += "\t\t<X>" + QString::number(posX) + "</X>\n";
  saveXml += "\t\t<Y>" + QString::number(posY) + "</Y>\n";
  saveXml += "\t\t<ColorBehind>" + color.name() + "</ColorBehind>\n";
  saveXml += "\t</Parameters>\n";
  saveXml += "</Degradation>";
  return saveXml;
}
*/

static QString
Hole_getXML(const QString &filename, std::vector<HoleData> &holes)
{
  QString saveXml = QStringLiteral("<Degradation>\n");
  saveXml += "\t<PictureName>" + filename + "</PictureName>\n";
  saveXml += QLatin1String("\t<DegradationName>Hole</DegradationName>\n");
  const unsigned int nbHole = holes.size();
  saveXml += "\t\t<Number>" + QString::number(nbHole) + "</Number>\n";
  saveXml += QLatin1String("\t<Parameters>\n");

  for (unsigned int i = 0; i < nbHole; ++i) {
    const QString &patternFilename = holes[i].patternFilename;
    const QString holeTypeStr =
      QString::number(static_cast<int>(holes[i].type));
    const QString holeSideStr = QString::number(holes[i].side);

    saveXml += "\t\t<Hole>" + patternFilename + "</Hole>\n";
    saveXml += "\t\t<Size>" + QString::number(holes[i].size) + "</Size>\n";
    saveXml += "\t\t<HoleType>" + holeTypeStr + "</HoleType>\n";
    saveXml += "\t\t<Side>" + holeSideStr + "</Side>\n";
    saveXml += "\t\t<X>" + QString::number(holes[i].posX) + "</X>\n";
    saveXml += "\t\t<Y>" + QString::number(holes[i].posY) + "</Y>\n";
    saveXml += "\t\t<ColorBehind>" + holes[i].color.name() + "</ColorBehind>\n";
  }
  saveXml += QLatin1String("\t</Parameters>\n");
  saveXml += QLatin1String("</Degradation>");
  return saveXml;
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
  QFile file(absoluteFilenameXML);
  const bool openOk = file.open(QIODevice::WriteOnly | QIODevice::Text);
  if (!openOk) {
    std::cerr << "ERROR: unable to write xml file: "
              << absoluteFilenameXML.toStdString() << "\n";
    return false;
  }
  QTextStream out(&file);
  out << xml;
  file.close();

  return true;
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
  const int nbTirage = this->nbTirageBleed();

  for (int i = 0; i < nbTirage; ++i) {
    const int nbOcc = P_bounded_rand(bleedMin, bleedMax + 1);

    const QImage imageBleed = bleedThrough(recto, verso, nbOcc);
    const QString prefixFilename =
      imageBasename + "Bleed_" + QString::number(i);
    const QString filename = prefixFilename + ".png";
    imageBleed.save(outputImageDir + filename);

    QString saveXml = QStringLiteral("<Degradation>\n");
    saveXml += "\t<PictureName>" + filename + "</PictureName>\n";
    saveXml +=
      QLatin1String("\t<DegradationName>Bleed-Through</DegradationName>\n");
    saveXml += QLatin1String("\t<Parameters>\n");
    saveXml += "\t\t<Verso>" + versoPath + "</Verso>\n";
    saveXml +=
      "\t\t<BleedIntensity>" + QString::number(nbOcc) + "</BleedIntensity>\n";
    saveXml += QLatin1String("\t</Parameters>\n");
    saveXml += QLatin1String("</Degradation>");

    QFile file(outputImageDir + prefixFilename + ".xml");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << saveXml;
    file.close();

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  }
}

void
Assistant::do_charDeg(const QString &imageBasename,
                      const QImage &recto,
                      const QString &outputImageDir) const
{
  const int charMin = this->charMin();
  const int charMax = this->charMax();
  const int nbTirages = this->nbTirageChar();

  for (int i = 0; i < nbTirages; ++i) {

    const int level = P_bounded_rand(charMin, charMax + 1);

    QImage imgCharTmp;
    imgCharTmp = toGray(recto);
    GrayscaleCharsDegradationModel deg(imgCharTmp);
    imgCharTmp = deg.degradateByLevel(level);

    const QString prefixFilename =
      imageBasename + "CharDeg_" + QString::number(i);
    const QString filename = prefixFilename + ".png";
    imgCharTmp.save(outputImageDir + filename);

    QString saveXml = QStringLiteral("<Degradation>\n");
    saveXml += "\t<PictureName>" + filename + "</PictureName>\n";
    saveXml += "\t<DegradationName>Characters Degradation</DegradationName>\n";
    saveXml += QLatin1String("\t<Parameters>\n");
    saveXml += "\t\t<DegradationIntensity>" + QString::number(level) +
               "</DegradationIntensity>\n";
    saveXml += QLatin1String("\t</Parameters>\n");
    saveXml += QLatin1String("</Degradation>");

    QFile file(outputImageDir + prefixFilename + ".xml");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << saveXml;
    file.close();

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  }
}

void
Assistant::do_shadow(const QString &imageBasename,
                     const QImage &recto,
                     const QString &outputImageDir) const
{
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

  for (int i = 0; i < _Shadow_nbShadSelected; ++i) {
    const ShadowBorder border = _Shadow_borderStack[i];
    picTmpShad = shadowBinding(recto, distanceRatio, border, intensity, angle);
    Shadow_saveImage(picTmpShad,
                     path,
                     prefix,
                     imgExt,
                     distanceRatio,
                     border,
                     intensity,
                     angle);

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  }
}

static QString
Phantom_getFrequencyStr(Frequency frequency)
{
  switch (frequency) {
    case Frequency::RARE:
      return QStringLiteral("RARE");
      break;
    case Frequency::FREQUENT:
      return QStringLiteral("FREQUENT");
      break;
    case Frequency::VERY_FREQUENT:
    default:
      return QStringLiteral("VERY_FREQUENT");
      break;
  }
}

static void
Phantom_applyAndSave(const QImage &recto,
                     Frequency frequency,
                     int i,
                     const QString &imageBasename,
                     const QString &outputImageDir)
{
  QImage imgTmpPhant = phantomCharacter(recto, frequency);

  const QString freqStr = Phantom_getFrequencyStr(frequency);
  const QString prefixFilename =
    imageBasename + "Phantom_" + freqStr + "_" + QString::number(i);
  const QString filename = prefixFilename + ".png";

  imgTmpPhant.save(outputImageDir + filename);

  QString saveXml = QStringLiteral("<Degradation>\n");
  saveXml += "\t<PictureName>" + filename + "</PictureName>\n";
  saveXml += "\t<DegradationName>Phantom Characters</DegradationName>\n";
  saveXml += QLatin1String("\t<Parameters>\n");
  saveXml += "\t\t<Frequency>" + freqStr + "</Frequency>\n";
  saveXml += QLatin1String("\t</Parameters>\n");
  saveXml += QLatin1String("</Degradation>");

  QFile file(outputImageDir + prefixFilename + ".xml");
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);
  out << saveXml;
  file.close();
}

void
Assistant::do_phantom(const QString &imageBasename,
                      const QImage &recto,
                      const QString &outputImageDir) const
{
  //B:TODO: plusieurs tirages !!!!!!!

  const int nbTirage = ui->TiragePhantom->value();

  for (int i = 0; i < nbTirage; ++i) {

    //QImage imgTmpPhant;

    if (ui->Phantom_Rare->isChecked()) {
      Phantom_applyAndSave(
        recto, Frequency::RARE, i, imageBasename, outputImageDir);

      qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    if (ui->Phantom_Frequent->isChecked()) {
      Phantom_applyAndSave(
        recto, Frequency::FREQUENT, i, imageBasename, outputImageDir);

      qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    if (ui->Phantom_VeryFrequent->isChecked()) {

      Phantom_applyAndSave(
        recto, Frequency::VERY_FREQUENT, i, imageBasename, outputImageDir);

      qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
  }
}

static QString
Blur_getFunctionStr(Function f)
{
  switch (f) {
    case Function::LINEAR:
      return QStringLiteral("LINEAR");
      break;
    case Function::LOG:
      return QStringLiteral("LOG");
      break;
    case Function::PARABOLA:
      return QStringLiteral("PARABOLA");
      break;
    case Function::SINUS:
      return QStringLiteral("SINUS");
      break;
    case Function::ELLIPSE:
      return QStringLiteral("ELLIPSE");
      break;
    case Function::HYPERBOLA:
    default:
      return QStringLiteral("HYPERBOLA");
      break;
  }
}

static QString
Blur_getMethodStr(Method m)
{
  switch (m) {
    case Method::GAUSSIAN:
      return QStringLiteral("GAUSSIAN");
      break;
    case Method::MEDIAN:
      return QStringLiteral("MEDIAN");
      break;
    case Method::NORMAL:
    default:
      return QStringLiteral("NORMAL");
      break;
  }
}

static QString
Blur_getAreaStr(Area a)
{
  switch (a) {
    case Area::UP:
      return QStringLiteral("UP");
      break;
    case Area::DOWN:
      return QStringLiteral("DOWN");
      break;
    case Area::CENTERED:
    default:
      return QStringLiteral("CENTERED");
      break;
  }
}

static QString
Blur_getXML_area(const QString &filename,
                 Function function,
                 Area area,
                 float coeff1,
                 int vert,
                 int horiz,
                 int radius,
                 Method method,
                 int intensity)
{
  QString saveXml = QStringLiteral("<Degradation>\n");
  saveXml += "\t<PictureName>" + filename + "</PictureName>\n";
  saveXml += QLatin1String("\t<DegradationName>BlurZone</DegradationName>\n");
  saveXml += QLatin1String("\t<Parameters>\n");
  saveXml += "\t\t<Function>" + Blur_getFunctionStr(function) + "</Function>\n";
  saveXml += "\t\t<Area>" + Blur_getAreaStr(area) + "</Area>\n";
  saveXml += "\t\t<coeff1>" + QString::number(coeff1) + "</coeff1>\n";
  saveXml += "\t\t<Vertical>" + QString::number(vert) + "</Vertical>\n";
  saveXml += "\t\t<Horizontal>" + QString::number(horiz) + "</Horizontal>\n";
  saveXml += "\t\t<Radius>" + QString::number(radius) + "</Radius>\n";
  saveXml += "\t\t<Method>" + Blur_getMethodStr(method) + "</Method>\n";
  saveXml += "\t\t<Intensity>" + QString::number(intensity) + "</Intensity>\n";
  saveXml += QLatin1String("\t</Parameters>\n");
  saveXml += QLatin1String("</Degradation>");
  return saveXml;
}

static QString
Blur_getXML_page(const QString &filename, Method method, int intensity)
{
  QString saveXml = QStringLiteral("<Degradation>\n");
  saveXml += "\t<PictureName>" + filename + "</PictureName>\n";
  saveXml +=
    QLatin1String("\t<DegradationName>BlurComplete</DegradationName>\n");
  saveXml += QLatin1String("\t<Parameters>\n");
  saveXml += "\t\t<Method>" + Blur_getMethodStr(method) + "</Method>\n";
  saveXml += "\t\t<Intensity>" + QString::number(intensity) + "</Intensity>\n";
  saveXml += QLatin1String("\t</Parameters>\n");
  saveXml += QLatin1String("</Degradation>");
  return saveXml;
}

/**
apply blur on a specific area

@param[in] recto input image
*/
static QImage
Blur_applyArea(const QImage &recto,
               Function function,
               Area area,
               float coeff1,
               int vert,
               int horiz,
               int radius,
               Method method,
               int intensity)
{
  //B:TODO: is it equivalent ???
  //QImage pattern = makePattern(recto, function, area, coeff1, vert, horiz, radius);
  //return applyPattern(recto, pattern, method, intensity);
  return blurFilter(
    recto, method, intensity, function, area, coeff1, vert, horiz, radius);
}

static void
Blur_applyAreaAndSave(const QImage &recto,
                      int index,
                      Function f,
                      Area a,
                      float coeff1,
                      int vert,
                      int horiz,
                      int radius,
                      Method m,
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

  const QString saveXml =
    Blur_getXML_area(filename, f, a, coeff1, vert, horiz, radius, m, intensity);

  QFile file(outputImageDir + prefixFilename + ".xml");
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);
  out << saveXml;
  file.close();
}

static void
Blur_applyPageAndSave(const QImage &recto,
                      int index,
                      Method m,
                      int intensity,
                      const QString &imageBasename,
                      const QString &outputImageDir)
{
  QImage blurTmp = blurFilter(recto, m, intensity);

  const QString prefixFilename =
    imageBasename + "Blur_Complete_" + QString::number(index);
  const QString filename = prefixFilename + ".png";

  blurTmp.save(outputImageDir + filename);

  QString saveXml = Blur_getXML_page(filename, m, intensity);

  QFile file(outputImageDir + prefixFilename + ".xml");
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);
  out << saveXml;
  file.close();
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

  if (this->getBlur_ZoneEnable()) {

    const Area a = Area::UP;
    const float coeff1 = 1.f;
    const int vert = 50;
    const int horiz = 50;
    const int radius = 10;
    const Method m = Method::GAUSSIAN;

    if (this->pattern1()) {
      for (int i = 0; i < this->nbTirageBlur(); ++i) {
        Blur_applyAreaAndSave(recto,
                              i,
                              Function::LINEAR,
                              a,
                              coeff1,
                              vert,
                              horiz,
                              radius,
                              m,
                              Blur_getIntensity(blurMin, blurMax),
                              imageBasename,
                              outputImageDir);

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
    }

    if (this->pattern2()) {
      for (int i = 0; i < this->nbTirageBlur(); ++i) {
        Blur_applyAreaAndSave(recto,
                              i,
                              Function::LOG,
                              a,
                              coeff1,
                              vert,
                              horiz,
                              radius,
                              m,
                              Blur_getIntensity(blurMin, blurMax),
                              imageBasename,
                              outputImageDir);

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
    }

    if (this->pattern3()) {
      for (int i = 0; i < this->nbTirageBlur(); ++i) {
        Blur_applyAreaAndSave(recto,
                              i,
                              Function::PARABOLA,
                              a,
                              coeff1,
                              vert,
                              horiz,
                              radius,
                              m,
                              Blur_getIntensity(blurMin, blurMax),
                              imageBasename,
                              outputImageDir);

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
    }

    if (this->pattern4()) {
      for (int i = 0; i < this->nbTirageBlur(); ++i) {
        Blur_applyAreaAndSave(recto,
                              i,
                              Function::SINUS,
                              a,
                              coeff1,
                              vert,
                              horiz,
                              radius,
                              m,
                              Blur_getIntensity(blurMin, blurMax),
                              imageBasename,
                              outputImageDir);

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
    }

    if (this->pattern5()) {
      for (int i = 0; i < this->nbTirageBlur(); ++i) {
        Blur_applyAreaAndSave(recto,
                              i,
                              Function::ELLIPSE,
                              a,
                              coeff1,
                              vert,
                              horiz,
                              radius,
                              m,
                              Blur_getIntensity(blurMin, blurMax),
                              imageBasename,
                              outputImageDir);

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
    }

    if (this->pattern6()) {
      for (int i = 0; i < this->nbTirageBlur(); ++i) {
        Blur_applyAreaAndSave(recto,
                              i,
                              Function::HYPERBOLA,
                              a,
                              coeff1,
                              vert,
                              horiz,
                              radius,
                              m,
                              Blur_getIntensity(blurMin, blurMax),
                              imageBasename,
                              outputImageDir);

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
    }
  }

  //blur on whole page
  if (this->getBlur_PageEnable()) {

    const Method m = Method::GAUSSIAN;

    for (int i = 0; i < this->nbTirageBlur(); ++i) {

      Blur_applyPageAndSave(recto,
                            i,
                            m,
                            Blur_getIntensity(blurMin, blurMax),
                            imageBasename,
                            outputImageDir);

      qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
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

  const int nbTirages = ui->TirageHole->value();
  for (int i = 0; i < nbTirages; ++i) {

    //std::cerr << "----------------------- hole " << i <<"/"<<nbTirages<< "\n";

    rectoC = recto.copy();

    holes.clear();
    holes.reserve(20); //arbitrary

    holes = Hole_doHoles(rectoC, scaleW, scaleH, color, randPosX, randPosY);

    const QString lPrefix = prefix + QString::number(i);
    Hole_saveImage(rectoC, path, lPrefix, imgExt, holes);
    //TODO: handle error !

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
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
      imageBasename + "Distortion3D_" + meshBasename;
    const QString filename = prefixFilename + ".png";

    img.save(outputImageDir + filename);

    QString saveXml = QStringLiteral("<Degradation>\n");
    saveXml += "\t<PictureName>" + filename + "</PictureName>\n";
    saveXml +=
      QLatin1String("\t<DegradationName>Distortion 3D</DegradationName>\n");
    saveXml += QLatin1String("\t<Parameters>\n");
    saveXml += "\t\t<Mesh>" + fiMesh.fileName() + "</Mesh>\n";
    if (useBackground) {
      saveXml += "\t\t<Background>" + QFileInfo(backgroundFilename).fileName() +
                 "</Background>\n";
    }
    //TODO: save other parameters ???
    saveXml += QLatin1String("\t</Parameters>\n");
    saveXml += QLatin1String("</Degradation>");

    QFile file(outputImageDir + prefixFilename + ".xml");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << saveXml;
    file.close();

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  }

  delete w;
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
  for (int picIndex = 0; picIndex < numPics; ++picIndex) {
    const QString &pic = picsList[picIndex];

    //Load recto

    const QString imageBasename =
      QFileInfo(pic).completeBaseName(); //remove path & last extension

    std::cerr << "----------------------- " << picIndex << "/" << numPics
              << "\n";

    const QString rectoPath = inputImageDir + pic;
    QImage recto(rectoPath);

    if (recto.isNull()) {
      std::cerr << "ERROR: unable to load image: " << rectoPath.toStdString()
                << "\n";
    } else {
      assert(!recto.isNull());

      //Apply bleedThrough if enabled
      if (this->bleedEnable()) {
        do_bleed(imageBasename, recto, picIndex, inputImageDir, savePath);
      }

      //Apply CharacterDegradation if enabled
      if (this->charEnable()) {
        do_charDeg(imageBasename, recto, savePath);
      }

      //Apply Shadow Binding if enabled
      if (this->shadEnable()) {
        do_shadow(imageBasename, recto, savePath);
      }

      //Apply phantom characters if enabled
      if (this->phantEnable()) {
        do_phantom(imageBasename, recto, savePath);
      }

      //Apply blur degradations if enabled
      if (this->blurEnable()) {
        do_blur(imageBasename, recto, savePath);
      }

      //Apply holes if enabled
      if (this->holeEnable()) {
        do_hole(imageBasename, recto, savePath);
      }

      //Apply 3D if enabled
      if (this->dist3DEnable()) {
        do_3D(imageBasename, recto, savePath);
      }
    }
  }

  QGuiApplication::restoreOverrideCursor();
}

void
Assistant::updateResults()
{
  ui->label_results->setText(
    tr("%n image(s) have been written into directory:\n%1",
       "",
       ui->TotalPic->text().toUInt())
      .arg(_outputDegradedImageDir));
}

void
Assistant::accept()
{
  //std::cerr<<"accept() !\n";

  QWizard::accept();
}
