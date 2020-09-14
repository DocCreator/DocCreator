#ifndef DOCCREATOR_H
#define DOCCREATOR_H

#include "patterns/observer.h"
#include <QMainWindow>

class DocumentController;
class DocumentPropertiesView;
class DocumentView;
class FontEditorController;
class FontEditorView;
class KeyboardController;
class KeyboardView;
class FontEditorView;

class QAction;
class QCloseEvent;
class QComboBox;
class QLabel;
class QMenu;
class QStandardItemModel;
class QTableView;
class QToolBar;

namespace Models {
class Font;
}

class DocCreator
  : public QMainWindow
  , public Patterns::Observer
{
  Q_OBJECT

public:
  explicit DocCreator(QWidget *parent = nullptr);
  ~DocCreator();

  void initialize();
  void update() override;
  //void generateImageFromFolder(const QString &strFolderIn, const QString &strFolderOut, const QString &file);

private slots:
  //void generateImagesFromDir();
  void newDocument();
  void openDocument();
  void saveDocument();
  void exportDocToTxt();
  void saveDocumentAs();
  void print();
  void undo();
  void about();
  void loadFont();
  //void importFontFromDir();
  //void saveCurrentFont();
  //void saveCurrentFontAs();
  void importBackground();
  void loadKeyboard();
  void insertTextBlock();
  void insertImageBlock();
  void removeBlock();
  void exportToImage();
  void applyBleedThrough();
  void applyBlurFilter();
  void applyShadowBinding();
  void applyHoleDegradation();
  void applyPhantomCharacter();
  void applyCharacterDegradationModel();
  void applyGradientDomainDegradation();
  void apply3DDistortionModel();
  //void extractConnectedComponents();
  //void componentsLabeling();
  void changeFont(const QString &fontName);
  void changeBackground(const QString &background);
  void binarization();
  void synthetiseImage();
  void lineDetection();
  void backgroundReconstruction();
  void fontExtraction();
  void structureDetection();
  //void loadFontFromPath(const QString &fontPath);
  void createFontDocument();

  void onFocusChanged(QWidget *old, QWidget *now);

  void addMatch();

  /*Generate Documents*/
  void generateDocuments();

private:
  void createControllers();
  void createActions();
  void createMenus();
  void createToolBars();
  void createStatusBar();
  void createDockWindows();
  void createFontSelector();
  void createBackgroundSelector();

  void addNewFont(Models::Font *font);
  void addBackground(const QString &background);

  void populateFontSelector();

  /* Save functions */
  void saveDocumentToXml(const QString &filepath);
  void saveFontToXml(const QString &filepath);

  void updateTitle(const QString &path);

  int processDisplaySaveMessage(const QString &action);

  /* Events */
  void closeEvent(QCloseEvent *event) override;

  QString getTessdataParentDir() const;

private:
  /* Menu objects */
  QMenu *_fileMenu, *_editMenu, *_insertMenu, *_viewMenu, *_degradationsMenu,
    *_helpMenu, *_imageMenu;
  QToolBar *_fileToolBar, *_editToolBar, *_insertToolBar;
  QAction *_newDocumentAct, *_saveDocumentAct, *_exportDocToTxtAct, *_printAct,
    *_undoAct;
  QAction *_loadFontAct, *_loadKeyboard;
  QAction *_aboutAct, *_quitAct, *_textBlockAct, *_imageBlockAct,
    *_removeBlockAct;
  QAction *_importBackgroundAct, *_exportToImageAct, *_openDocumentAct;
  QAction *_saveDocumentAsAct;

  QAction *_extractBackground;
  // Image action
  QMenu *_imageToolsSubMenu, *_imageDegradationSubMenu,
    *_grayScaleDegradationSubMenu, *_colorDegradationSubMenu;
  //QAction *_viewXMLAct, *_viewStructureAct, *_attributeAct;

  // Tools menu
  QMenu *_toolsMenu, *_segmentationMenu, *_classificationMenu, *_labelingMenu,
    *_OCRMenu, *_fontMenu, *_generationMenu;
  // Semi-synthetic Document Image Generation Menu

  // Semi-synthetic Document Image Generation Menu
  QMenu *_fontGeneratorMenu, *_fontGeneratorSubMenu;

  QAction *_binarization, *_lineDetection, *_backgroundReconstruction,
    *_fontExtraction, *_structureDetection, *_synthetiseImage;
  QAction *_createFontDocAct;

  /**
   * Degradations Actions
   */
  QAction *_applyBleedThrough;
  QAction *_applyBlurFilter;
  QAction *_applyShadowBinding;
  QAction *_applyHoleDegradation;
  QAction *_applyPhantomCharacter;
  QAction *_applyCharacterDegradationModel;
  QAction *_applyGradientDomainDegradation;
  QAction *_distortion3DModelAct;
  
  //DocCreator Action
  QAction *_chooseTypeDoc;

  QLabel *_fontLabel;
  QComboBox *_fontComboBox;
  QComboBox *_backgroundComboBox;

  QStandardItemModel *_listDialog;
  QTableView *_matchesTable;

  bool _firstStart;

  /* Controllers */
  DocumentController *_docController;
  KeyboardController *_keyboardController;
  FontEditorController *_fontController;

  /* Views */
  DocumentView *_documentView;
  KeyboardView *_keyboardView;
  FontEditorView *_fontEditorView;
  DocumentPropertiesView *_docPropertiesView;

  /* Keyboard */
  QString _keyboard;

  int _newFontID;
};

#endif /* DOCCREATOR_H */
