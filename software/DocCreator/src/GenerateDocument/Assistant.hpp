#ifndef ASSISTANT_H
#define ASSISTANT_H

#include <QStringList>
#include <QStringListModel>
#include <QWizard>
#include <QWizardPage>

#include "context.h"
#include "Degradations/PhantomCharacter.hpp" //Frequency
#include "Degradations/ShadowBinding.hpp"    //ShadowBorder

class DocumentController;
class RandomDocumentParameters;

namespace Ui {
class Assistant;
}
class QProgressDialog;

struct HoleData;

class Assistant : public QWizard
{
  Q_OBJECT

public:
  explicit Assistant(DocumentController *doc, QWidget *parent = 0);

  ~Assistant();

protected:
  //enum{Choix_Methode};
  virtual int nextId() const override;
  //virtual bool isComplete() const override;
  virtual void initializePage(int id) override;
  virtual bool validateCurrentPage() override;

  void updateListText(const QString &textPath);
  void updateListFont(const QString &fontPath);
  void updateListBackground(const QString &pathBack);
  void loadInputDegradationImageList();

  bool semi() const;
  QString PicDirectory() const;
  QString saveDirectory() const;
  QStringList picsList() const;

  void setSemi(bool semi);

  //Bleed Through
  void BleedThrough_updateBleedImageMin(int nbIter, bool fromZero);
  void BleedThrough_updateBleedImageMax(int nbIter, bool fromZero);
  void BleedThrough_updateVersoImage();
  void BleedThrough_setupGUIImages();

  //Char Deg
  void CharDeg_setupGUIImages();
  void CharDeg_updateCharImageMin(int level);
  void CharDeg_updateCharImageMax(int level);

  //Shadow
  void Shadow_setupGUIImages();

  //Phantom Functions
  void Phantom_setupGUIImages();

  //GDD
  void GDD_setupGUIImages();
  
  //Blur Functions
  void Blur_setupGUIImages();
  void Blur_LoadPattern();

  //hole functions
  void Hole_LoadHolePattern();
  void Hole_setupGUIImages();

  int charMin() const;
  int charMax() const;
  int nbTirageChar() const;
  bool charEnable() const;
  bool shadEnable() const;
  bool gddEnable() const;
  bool phantEnable() const;
  bool pattern1() const;
  bool pattern2() const;
  bool pattern3() const;
  bool pattern4() const;
  bool pattern5() const;
  bool pattern6() const;
  bool blurEnable() const;
  int nbTirageBlur() const;
  int blurMin() const;
  int blurMax() const;
  int nbHoleCenterSelected() const;
  int nbHoleBorderSelected() const;
  int nbHoleCornerSelected() const;
  bool holeEnable() const;
  int holeMin() const;
  int holeMax() const;
  int nbHoleCenter() const;
  int nbHoleCorner() const;
  bool smallHoleCorner() const;
  bool mediumHoleCorner() const;
  bool bigHoleCorner() const;
  bool smallHoleBorder() const;
  bool mediumHoleBorder() const;
  bool bigHoleBorder() const;
  bool smallHoleCenter() const;
  bool mediumHoleCenter() const;
  bool bigHoleCenter() const;
  QColor getColorBehind() const;
  int getNbHoleBorder() const;
  int bleedMax() const;
  int bleedMin() const;
  int nbTirageBleed() const;
  bool bleedEnable() const;

  bool getBlur_ZoneEnable() const;
  bool getBlur_PageEnable() const;

  //distortion3D functions
  void Dist3D_setupGUIImages();
  bool dist3DEnable() const;

  void setDocController(DocumentController *DocController);

signals:
  void generationReady();

protected slots:

  virtual void accept() override;

  void updateTxtGenerationInfo();

  void RadioButtons();
  void chooseTextDirectory();
  void chooseBackgroundDirectory();
  void chooseFontDirectory();
  void chooseInputImageDir();
  void chooseGTDirectory();
  void chooseOutputDegradedImageDir();
  //void changeLoremIpsum();
  void textSelectionChanges();
  void fontSelectionChanges();
  void backgroundSelectionChanges();
  void PageParams_updateMin();
  void PageParams_updateMax();
  void PageParams_updateLineSpacing();
  void PageParams_updateImageSize();

  void addInputImage(const QString &imageFilename);

  void updateTotalPic();
  void GTChecked();
  void generateTxtImages();
  void chooseOutputTxtImageDir();
  //void chosePicDegDirectory();

  //Bleed Slots
  void BleedThrough_changeMinBleed();
  void BleedThrough_changeMaxBleed();
  void BleedThrough_EnableBleedOption();
  void BleedThrough_nbIterationsMinChanged();
  void BleedThrough_nbIterationsMaxChanged();
  void BleedThrough_LoadPrevImg();
  void BleedThrough_setVersoImage();
  void BleedThrough_tirageBleedChanged(int nbTirage);

  //Character Degradation Slots
  void CharDeg_LoadPrevImgChar();
  void CharDeg_EnableCharOption();
  void CharDeg_changeMinChar();
  void CharDeg_changeMaxChar();
  void CharDeg_nbIterationsMinChangedChar();
  void CharDeg_nbIterationsMaxChangedChar();
  void CharDeg_tirageCharChanged(int nbTirage);

  //ShadowBinding slots
  void Shadow_LoadPrevImgShad();
  void Shadow_EnableShadOption();
  void Shadow_selectionChanged();
  void Shadow_updatePreview();
  void Shadow_tirageShadowChanged(int nbTirage);
  /*
    void Shadow_OptionChecked();
    void Shadow_RightSelected();
    void Shadow_LeftSelected();
    void Shadow_TopSelected();
    void Shadow_BottomSelected();
  */

  //Phantom slots
  void Phantom_OptionCheckedPhant();
  void Phantom_EnablePhantOption();
  void Phantom_LoadPrevImgPhant();
  void Phantom_tiragePhantomChanged(int value);

  //Blur slots
  void Blur_OptionCheckedBlur();
  void Blur_EnableBlurOption();
  void Blur_changeMinBlur(int value);
  void Blur_changeMaxBlur(int value);
  void Blur_ByZoneChanged();
  void Blur_PageChanged();
  void Blur_CheckedPattern1();
  void Blur_CheckedPattern2();
  void Blur_CheckedPattern3();
  void Blur_CheckedPattern4();
  void Blur_CheckedPattern5();
  void Blur_CheckedPattern6();
  void Blur_LoadPrevImgBlur();
  void Blur_tirageBlurChanged(int value);

  //Hole slots
  void Hole_EnableHoleOption();
  void Hole_changeMinNbCenterHole(int value);
  void Hole_changeMaxNbCenterHole(int value);
  void Hole_changeMinNbCornerHole(int value);
  void Hole_changeMaxNbCornerHole(int value);
  void Hole_changeMinNbBorderHole(int value);
  void Hole_changeMaxNbBorderHole(int value);
  void Hole_OptionCheckedHole();
  void Hole_chooseColor();
  void Hole_tirageHoleChanged(int value);

  void Hole_updatePreview();

  void Hole_LoadPrevImgHole();

  //Distortion3D slots
  void Dist3D_EnableDist3DOption();
  void Dist3D_chooseMeshesDirectory();
  void Dist3D_changeUseBackground();
  void Dist3D_chooseBackgroundsDirectory();
  void Dist3D_tirageDist3DChanged(int nbTirage);

  //Gradient Domain Degradation slots
  //void GDD_OptionCheckedGDD();
  void GDD_EnableGDDOption();
  void GDD_tirageGDDChanged(int value);
  void GDD_changeMinNbStains(int value);
  void GDD_changeMaxNbStains(int value);
  void GDD_chooseStainImagesDirectory();
  
protected:
  void enableAccordingToInputImages();
  bool askIfProceedDespiteImageInDir(const QString &path) const;

  void PageParams_connect();
  bool PageParams_isComplete() const;
  void PageParams_getParams(RandomDocumentParameters &) const;

  int computeNbGeneratedTexts() const;

  void CharDeg_updateTirageAndTotal();

  void Shadow_setPreview(dc::ShadowBinding::Border border);
  void Shadow_updatePreviewAll();
  void Shadow_updateTirageAndTotal();

  void Phantom_apply(dc::PhantomCharacter::Frequency frequency);
  void Phantom_updatePreview();
  void Phantom_updateTirageAndTotal();

  void GDD_updateTirageAndTotal();
  
  void BleedThrough_updateTirageAndTotal();

  void Blur_updateTirageAndTotal();
  int Blur_nbDegradations() const;

  void Hole_setBackgroundColor(const QColor &color);
  void Hole_updateTirageAndTotal();
  std::vector<HoleData> Hole_doHoles(QImage &recto,
                                     float scaleW,
                                     float scaleH,
                                     const QColor &color,
                                     bool randPosX,
                                     bool randPosY) const;

  void Dist3D_loadListMeshes();
  void Dist3D_loadMeshBackgrounds();
  void Dist3D_updateTirageAndTotal();

  int nbOfDegradedImages() const;
  void generateDegradedImages() const;
  void updateResults();

  void do_bleed(const QString &imageBasename,
                const QImage &recto,
                int picIndex,
                const QString &inputImageDir,
                const QString &outputImageDir) const;

  void do_charDeg(const QString &imageBasename,
                  const QImage &recto,
                  const QString &outputImageDir) const;

  void do_shadow(const QString &imageBasename,
                 const QImage &recto,
                 const QString &outputImageDir) const;

  void do_phantom(const QString &imageBasename,
                  const QImage &recto,
                  const QString &outputImageDir) const;

  void do_blur(const QString &imageBasename,
               const QImage &recto,
               const QString &outputImageDir) const;

  void do_hole(const QString &imageBasename,
               const QImage &recto,
               const QString &outputImageDir) const;

  void do_3D(const QString &imageBasename,
             const QImage &recto,
             const QString &outputImageDir) const;

  void do_GDD(const QString &imageBasename,
             const QImage &recto,
             const QString &outputImageDir) const;


  void updateProgress() const;

private:
  Ui::Assistant *ui;

  enum
  {
    Page_SyntheticOrSemiChoice,
    Page_TextType,
    Page_TextDirs,
    Page_TextRandomNb,
    Page_FontFiles,
    Page_BackgroundFiles,
    Page_PageParams,
    Page_FinalText,
    Page_ConfirmDegradations,
    Page_ImageAndGtDirs,
    Page_Bleed,
    Page_CharDeg,
    Page_GDD,
    Page_Shadow,
    Page_Phantom,
    Page_Blur,
    Page_Hole,
    Page_Dist3D,
    Page_FinalDegradations,
    Page_Results
  };
  //Warning: they must be in the same order than in Assistant.ui

  //Parameters of bleed
  QImage _BleedThrough_rectoImg;
  QImage _BleedThrough_versoImg;
  QImage _BleedThrough_rectoImgPart;
  QImage _BleedThrough_versoImgPart;
  QImage _BleedThrough_rectoImgSmall;
  QImage _BleedThrough_versoImgSmall;
  QImage _BleedThrough_bleedImgPart;
  int _BleedThrough_indexRecto = -1;

  //Parameters of charDegradation
  QImage _CharDeg_rectoImgChar;
  QImage _CharDeg_rectoImgPartChar;
  int _CharDeg_indexRecto = -1;

  //Paramètre of shadow biding
  QImage _Shadow_rectoImgShad;
  QImage _Shadow_rectoImgShadDeg;
  dc::ShadowBinding::Border _Shadow_borderStack[4]; //_shadow_nbShadSelected elts in stack
  int _Shadow_nbShadSelected;          //in [0;4]
  int _Shadow_indexRecto = -1;

  //Parameters of Phantom Character
  QImage _Phantom_rectoImgPhant;
  QImage _Phantom_rectoImgPartPhant;
  int _Phantom_nbPhantSelected;
  int _Phantom_indexRecto = -1;

  
  //Parameters of blur filter
  QImage _Blur_rectoImgBlur;
  QImage _Blur_rectoImgBlurDeg;
  int _Blur_nbBlurSelected;
  int _Blur_indexRecto = -1;

  //Parameters of hole degradation
  QImage _Hole_rectoImgHole;
  QImage _Hole_ImgHoleOriginal;
  int _Hole_nbHoleSelected;
  int _Hole_nbHoleCenterSelected;
  int _Hole_nbHoleBorderSelected;
  int _Hole_nbHoleCornerSelected;
  QStringList _Hole_CenterHoles;
  QStringList _Hole_BorderHoles;
  QStringList _Hole_CornerHoles;
  int _Hole_indexRecto = -1;

  //Parameters

  bool _semi;
  int _BleedThrough_bleedMin = 10;
  int _BleedThrough_bleedMax = 20;
  //int _BleedThrough_nbTirageBleed = 0;
  bool _BleedThrough_bleedEnable;

  int _CharDeg_charMin = 1;
  int _CharDeg_charMax = 3;
  //int _CharDeg_nbTirageChar = 0;
  bool _CharDeg_charEnable;

  bool _Shadow_shadEnable;

  
  bool _Phantom_phantEnable;
  QString _PhantomPatternsPath;

  bool _GDD_gddEnable;
  QString _GDD_stainImagesDirectory;

  bool _Blur_pattern1;
  bool _Blur_pattern2;
  bool _Blur_pattern3;
  bool _Blur_pattern4;
  bool _Blur_pattern5;
  bool _Blur_pattern6;
  bool _Blur_blurEnable;
  bool _Blur_ZoneEnable;
  bool _Blur_PageEnable;
  //int _Blur_nbTirageBlur = 0;

  bool _Hole_holeEnable;
  int _Hole_nbHoleCenter = 0;
  int _Hole_nbHoleCorner = 0;
  int _Hole_nbHoleBorder = 0;
  bool _Hole_smallHoleCorner;
  bool _Hole_mediumHoleCorner;
  bool _Hole_bigHoleCorner;
  bool _Hole_smallHoleBorder;
  bool _Hole_mediumHoleBorder;
  bool _Hole_bigHoleBorder;
  bool _Hole_smallHoleCenter;
  bool _Hole_mediumHoleCenter;
  bool _Hole_bigHoleCenter;
  QColor _Hole_colorBehind;

  bool _Dist3D_dist3DEnable;
  QString _Dist3D_meshDirectory;
  QStringList _Dist3D_meshesList;
  QString _Dist3D_meshBackgroundsDirectory;
  QStringList _Dist3D_meshBackgroundsList;

  QString _PicDirectory;
  QString _FontDirectory;
  QString _txtDirectory;
  QString _outputDegradedImageDir;
  QString _outputTxtImageDir;

  QString _originalCurrentFont;
  QList<Models::Font *> _originalFonts;
  QString _originalCurrentBackgroundName;
  QList<QString> _originalBackgrounds;

  QStringListModel _textListModel;
  QStringListModel _fontListModel;
  QStringListModel _backgroundListModel;
  QStringList _txtList;
  QStringList _fontListChoice;
  QStringList _backgroundListChoice;
  QStringList _inputImageList;
  DocumentController *_DocController;

  mutable QProgressDialog *_progressDialog;
  mutable size_t _numGeneratedImages;
};

#endif // ASSISTANT_H
