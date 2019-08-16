#include "DocCreator.hpp"

#include <cassert>
#include <ctime> //time
#include <iostream>

#include <QAction>
#include <QComboBox>
#include <QDebug>
#include <QDirIterator>
#include <QDockWidget>
#include <QFileDialog>
#include <QImageReader> //supportedImageFormats
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPair>
#include <QPrintDialog>
#include <QPrinter>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QTableView>
#include <QToolBar>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QtPlugin>

#include "models/font.h"

#include "Degradations/BleedThrough.hpp"
#include "Degradations/BleedThroughParametersDialog.hpp"
#include "Degradations/BlurFilterDialog.hpp"
#include "Degradations/BlurFilterQ.hpp"
#include "Degradations/Distortion3DModel/src/MeshWindow.hpp"
#include "Degradations/GrayCharacterDegradationDialog.hpp"
#include "Degradations/GrayCharacterDegradationParameter.hpp"
#include "Degradations/GrayscaleCharsDegradationModel.hpp"
#include "Degradations/HoleDegradationDialog.hpp"
#include "Degradations/ImageGenerationFromDirDialog.hpp"
#include "Degradations/PhantomCharacterDialog.hpp"
#include "Degradations/ShadowBindingDialog.hpp"
#include "Degradations/ShadowBindingQ.hpp"
#include "Degradations/GradientDomainDegradationDialog.hpp"
#include "Degradations/GradientDomainDegradationQ.hpp"
#include "Degradations/VersoImageChanger.hpp"
#include "Document/BackGroundChanger.hpp"
#include "Document/ChooseLabelForComponentForm.hpp"
#include "Document/DocumentController.hpp"
#include "Document/ImageExporter.hpp"
#include "FontEditor/FontEditorController.hpp"
#include "FontEditor/FontEditorView.hpp"
#include "FontGenerator/BackgroundReconstructionDialog.hpp"
#include "FontGenerator/BinarizationDialog.hpp"
#include "FontGenerator/FontExtractionDialog.hpp"
#include "FontGenerator/LineDetectionDialog.hpp"
#include "FontGenerator/OCRDialog.hpp"
#include "FontGenerator/OCRSettingsDialog.hpp"
#include "FontGenerator/StructureDialog.hpp"
#include "GenerateDocument/Assistant.hpp"
#include "RandomDocument/RandomBackGroundChanger.hpp"
#include "RandomDocument/RandomDocumentCreator.hpp"
#include "RandomDocument/RandomDocumentParametersDialog.hpp"
#include "Utils/FontUtils.hpp" //computeBestLineSpacing
#include "Utils/ImageUtils.hpp"
#include "Utils/MessageHandler.hpp"
#include "VirtualKeyboard/KeyboardController.hpp"
#include "VirtualKeyboard/KeyboardView.hpp"
#include "VirtualKeyboard/KeyboardViewDirector.hpp"
#include "VirtualKeyboard/KeyboardViewXmlBuilder.hpp"
#include "appconstants.h"

#include "Document/GraphicView.hpp"
#include "Document/TextView.hpp"
#include "FontEditor/CharEditView.hpp"

#include "Document/DocumentPropertiesView.hpp"
#include "Document/DocumentView.hpp"
#include "FontEditor/FontEditorView.hpp"
#include "VirtualKeyboard/KeyboardView.hpp"

#include "Utils/ImageUtils.hpp" //getReadImageFilter()

#include "FontGenerator/FontDocumentGenerator.hpp"

DocCreator::DocCreator(QWidget *parent)
  : QMainWindow(parent)
  , _fontLabel(nullptr)
  , _fontComboBox(nullptr)
  , _backgroundComboBox(nullptr)
  , _listDialog(nullptr)
  , _matchesTable(nullptr)
  , _firstStart(true)
  , _docController(nullptr)
  , _keyboardController(nullptr)
  , _fontController(nullptr)
  , _documentView(nullptr)
  , _keyboardView(nullptr)
  , _fontEditorView(nullptr)
  , _docPropertiesView(nullptr)
  , _keyboard()
  , _newFontID(0)
{
  initialize();

  setAttribute(Qt::WA_DeleteOnClose);
  //Makes Qt delete this widget when the widget has accepted the close event (see QWidget::closeEvent()).
}

DocCreator::~DocCreator()
{
  //B
  assert(_docController);
  Doc::Document *doc = _docController->getDocument();
  Context::DocumentContext::instance()->setCurrentDocument(nullptr);
  _docController->setDocument(nullptr);
  delete doc;

  delete _docController;
  delete _keyboardController;
  delete _fontController;
  delete _documentView;
  delete _docPropertiesView;
  delete _keyboardView;
  delete _fontEditorView;

  //B:TODO: useless

  _docController = nullptr;
  _fontController = nullptr;
  _keyboardController = nullptr;
  _documentView = nullptr;
  _fontEditorView = nullptr;
  _keyboardView = nullptr;
  _docPropertiesView = nullptr;
  _firstStart = true;

  _newFontID = 0;
}

/*
  check that found data/ directory is the correct one.
 */
static bool
checkDataDir(const QDir &dir)
{
  //For now, we only check that it has a 'font' subdirectory
  return QDir(dir).cd(QStringLiteral("font"));
}

/*
  find data/ directory relative to application directory

 */
static QString
findRelativeDataDir()
{
  //std::cerr<<"findRelativeDataDir()\n";

  /*
    data/ may be:
    ../share/DocCreator/data installed on linux
    ../Resources/data installed on Mac
    ../../../data compiled on linux with build/ dir in sources
    ../../../<somthing>/data compiled on linux/mac with QtCreator
   */
  QDir dir(QCoreApplication::applicationDirPath());
  bool okup1 = dir.cdUp();
  if (okup1) {

    //std::cerr<<"dir="<<dir.absolutePath().toStdString()<<"\n";

    { //../share/DocCreator/data
      QDir dir1(dir);
      const bool ok1 = dir1.cd(QStringLiteral("share"));
      if (ok1) {
        const bool ok1b = dir1.cd(QStringLiteral("DocCreator"));
        if (ok1b) {
          const bool ok1c = dir1.cd(QStringLiteral("data"));
          if (ok1c) {
            if (checkDataDir(dir1)) {
              return dir1.absolutePath();
            }
          }
        }
      }
    }

    { //../Resources/data
      QDir dir1(dir);
      const bool ok1 = dir1.cd(QStringLiteral("Resources"));
      if (ok1) {
        const bool ok1b = dir1.cd(QStringLiteral("data"));
        if (ok1b) {
          if (checkDataDir(dir1)) {
            return dir1.absolutePath();
          }
        }
      }
    }

    { //../../../data
      QDir dir1(dir);
      const bool okup2 = dir1.cdUp();
      if (okup2) {
        const bool okup3 = dir1.cdUp();
        if (okup3) {
          const bool ok1 = dir1.cd(QStringLiteral("data"));
          if (ok1) {
            if (checkDataDir(dir1)) {
              std::cerr << "findRelativeDataDir() found "
                        << dir1.absolutePath().toStdString() << "\n";
              return dir1.absolutePath();
            }
          }
        }
      }
    }

    { //../../../*/data
      QDir dir1(dir);
      const bool okup2 = dir1.cdUp();
      if (okup2) {
        const bool okup3 = dir1.cdUp();
        if (okup3) {
          QDirIterator it(dir1.absolutePath(), QDirIterator::Subdirectories);
          while (it.hasNext()) {
            it.next();
            QDir dir2(it.filePath());
            //std::cerr<<"search "<<dir2.absolutePath().toStdString()<<"\n";
            const bool ok2 = dir2.cd(QStringLiteral("data"));
            if (ok2) {
              if (checkDataDir(dir2)) {
                std::cerr << "findRelativeDataDir() found "
                          << dir2.absolutePath().toStdString() << "\n";
                return dir2.absolutePath();
              }
            }
          }
        }
      }
    }
  }

  std::cerr << "Warning: findRelativeDataDir(): data directory not found !\n";

  return QString();
}

/*
    fontfolder=${FONT_DATA_DIRECTORY}
    backgroundfolder=${BACKGROUND_DATA_DIRECTORY}
    imagefolder=${IMAGE_DATA_DIRECTORY}
    meshfolder=${MESH_DATA_DIRECTORY}
    holepatternsfolder=${HOLEPATTERNS_DATA_DIRECTORY}
    phantompatternsfolder=${PHANTOMPATTERNS_DATA_DIRECTORY}
    blurimagesfolder=${BLURIMAGES_DATA_DIRECTORY}
    tessdataparentfolder=${Tesseract_TESSDATA_PARENT_DIR}
    xmlcheckerfolder=${XMLCHECKER_DIRECTORY}
    formatsfolder=${KEYBOARD_DATA_DIRECTORY}
  */
void
initializeFolders()
{
  QString dataDirStr = findRelativeDataDir();
  //std::cerr<<"dataDir="<<dataDirStr.toStdString()<<"\n";
  QDir dir(dataDirStr);
  /*
    const QString AppConfigKbFolderKey = "formatsfolder" ;
    const QString AppConfigXmlCheckerFolderKey="xmlcheckerfolder";
    const QString AppConfigDocumentXSDCheckerKey="documentxsdchecker";
    
    const QString AppConfigTessdataParentFolderKey="tessdataparentfolder";
   */

  Core::ConfigurationManager::set(
    AppConfigKeyBoardGroup,
    AppConfigKbFolderKey,
    dir.absoluteFilePath(QStringLiteral("keyboard")));
  Core::ConfigurationManager::set(
    AppConfigMainGroup,
    AppConfigImageFolderKey,
    dir.absoluteFilePath(QStringLiteral("Image")));
  Core::ConfigurationManager::set(AppConfigMainGroup,
                                  AppConfigFontFolderKey,
                                  dir.absoluteFilePath(QStringLiteral("font")));
  Core::ConfigurationManager::set(
    AppConfigMainGroup,
    AppConfigBackgdFolderKey,
    dir.absoluteFilePath(QStringLiteral("background")));
  Core::ConfigurationManager::set(AppConfigMainGroup,
                                  AppConfigMeshFolderKey,
                                  dir.absoluteFilePath(QStringLiteral("Mesh")));
  Core::ConfigurationManager::set(
    AppConfigMainGroup,
    AppConfigHolePatternsFolderKey,
    dir.absoluteFilePath(QStringLiteral("Image/holePatterns")));
  Core::ConfigurationManager::set(
    AppConfigMainGroup,
    AppConfigPhantomPatternsFolderKey,
    dir.absoluteFilePath(QStringLiteral("Image/phantomPatterns")));
  Core::ConfigurationManager::set(
    AppConfigMainGroup,
    AppConfigBlurImagesFolderKey,
    dir.absoluteFilePath(QStringLiteral("Image/blurImages")));
  Core::ConfigurationManager::set(
    AppConfigMainGroup,
    AppConfigStainImagesFolderKey,
    dir.absoluteFilePath(QStringLiteral("Image/stainImages/images")));
}

void
DocCreator::initialize()
{

  initializeFolders();

  std::cout << " initializing : " << std::endl;
  _docController = nullptr;
  _fontController = nullptr;
  _keyboardController = nullptr;
  _documentView = nullptr;
  _fontEditorView = nullptr;
  _keyboardView = nullptr;
  _firstStart = true;

  srand(time(nullptr)); // initialization of rand

  _newFontID = 0;
  // Initialize font context
  std::cout << " \t........Font context 1 " << std::endl;

  QString fontpath =
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigFontFolderKey)
      .toString();
  std::cout << " \t........Font context 2 " << fontpath.toStdString()
            << std::endl;
  QString fontext =
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigFontExtKey)
      .toString();
  std::cout << " \t........Font context 3 " << fontext.toStdString()
            << std::endl;
  Context::FontContext::instance()->initialize(fontpath, fontext);
  const QString defaultFont =
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigDefaultFontKey)
      .toString();
  std::cout << " \t........Font context 4 " << defaultFont.toStdString()
            << std::endl;
  Context::FontContext::instance()->setCurrentFont(defaultFont);

  //B:TODO: if paths are wrong, no font has been loaded ! It will core-dump later !

  std::cout << " \t........Background" << std::endl;
  QString backgroundpath = Core::ConfigurationManager::get(
                             AppConfigMainGroup, AppConfigBackgdFolderKey)
                             .toString();
  std::cout << "backgroundPath=" << backgroundpath.toStdString()
            << " defaultBackgroundFile="
            << Core::ConfigurationManager::get(AppConfigMainGroup,
                                               AppConfigDefaultBackbg)
                 .toString()
                 .toStdString()
            << std::endl;
  Context::BackgroundContext::initialize(backgroundpath);
  Context::BackgroundContext::instance()->setCurrentBackground(
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigDefaultBackbg)
      .toString()); //B:TODO: why was-it commented on AntoineBilly version ???

  std::cout << " \t........Keyboard ";

  _keyboard =
    QDir(Core::ConfigurationManager::get(AppConfigKeyBoardGroup,
                                         AppConfigKbFolderKey)
           .toString())
      .absoluteFilePath(Core::ConfigurationManager::get(
                          AppConfigKeyBoardGroup, AppConfigKbDefautlFormatKey)
                          .toString());
  std::cout << _keyboard.toStdString() << "\n";

  std::cout << "\t ........ controller" << std::endl;
  createControllers();
  std::cout << " \t........Create action" << std::endl;
  createActions();
  std::cout << " \t........menus" << std::endl;
  createMenus();
  std::cout << " \t........toolbars" << std::endl;
  createToolBars();
  std::cout << " \t........statusbar" << std::endl;
  createStatusBar();
  std::cout << " \t........windows" << std::endl;
  createDockWindows();
  std::cout << " \t........Font selector" << std::endl;
  createFontSelector();
  std::cout << " \t........Background selector" << std::endl;
  createBackgroundSelector();

  std::cout << " \t........Viewer" << std::endl;
  GraphicView *central = nullptr;
  if (_documentView != nullptr) {
    central = new GraphicView(_docController, _documentView);
    _documentView->setGraphicView(central);
  }

  setCentralWidget(central);
  setUnifiedTitleAndToolBarOnMac(true);

  //setWindowState(windowState() | Qt::WindowFullScreen);
  //recommanded instead of showMaximized()
  //but no titlebar on linux
  showMaximized();

  setWindowIcon(QIcon("./images/appicon.png"));

  std::cout << " \t........Update state" << std::endl;
  update();
  std::cout << " \t........Create new doc" << std::endl;
  newDocument();
}

void
DocCreator::update()
{
  QString path = Context::DocumentContext::instance()->getCurrentDocumentPath();
  if (path.isEmpty()) {
    path = DefaultPath;
  }
  setWindowTitle(QString(AppTitle) + QString(path) +
                 (Context::DocumentContext::instance()->modified()
                    ? ModifiedStr
                    : QString()));

  _saveDocumentAct->setEnabled(
    Context::DocumentContext::instance()->modified() ||
    Context::DocumentContext::instance()->isNewDocument());

  //	_removeBlockAct->setEnabled(_documentView->graphicView()->hasCurrentBlock());

  //std::cerr<<"######### FontContext::instance()->getCurrentFontName()=+"<<FontContext::instance()->getCurrentFontName().toStdString()<<"+\n";

  //if (FontContext::instance()->getNumberOfFonts() == 0) {
  //  _fontComboBox->clear();
  //}
  //if (_fontComboBox->count() != FontContext::instance()->getNumberOfFonts()) {
  //populateFontSelector();
  //}
  int index = -1;
  QString currentFontName =
    Context::FontContext::instance()->getCurrentFontName();
  if (!currentFontName.isEmpty()) {
    index = _fontComboBox->findText(currentFontName);
    if (index == -1) {
      _fontComboBox->addItem(
        Context::FontContext::instance()->getCurrentFontName());
      currentFontName = Context::FontContext::instance()->getCurrentFontName();
      index = _fontComboBox->findText(currentFontName);
    }
  }
  if (index != -1) {
    _fontComboBox->setCurrentIndex(index);
  }
}

void
DocCreator::generateDocuments()
{
  Assistant dialog(_docController, this);

  dialog.exec(); //will generate images if accepted.
}

void
DocCreator::newDocument()
{
  if (Context::DocumentContext::instance()->modified() || _firstStart) {
    int response = QMessageBox::Discard;
    if (!_firstStart)
      response = processDisplaySaveMessage(QStringLiteral("creating new"));

    switch (response) {
      case QMessageBox::Save:
        saveDocument();
      case QMessageBox::Discard:
        break;
      case QMessageBox::Cancel:
        return;
      default:
        break;
    }
    _firstStart = false;
  }
  auto document = new Doc::Document();
  document->add(new Doc::Page(document));

  Context::DocumentContext::instance()->setCurrentDocument(document);
  if (_docController != nullptr) {
    Doc::Document *prevDoc = _docController->getDocument();
    _docController->setDocument(document);
    delete prevDoc;
  }
}

/*
void DocCreator::generateImageFromFolder(const QString &strFolderIn, const QString &strFolderOut, const QString &file)
{
  //    QDir folderIN(strFolderIn);
  //    QStringList filters;
  //    filters << "*.xml";
  //    folderIN.setNameFilters(filters);
  //    QStringList filesIN = folderIN.entryList();

  //    for (QString file : filesIN) {
  //        QString filepath = folderIN.absoluteFilePath(file);
  //        //loading files
  //        Document * document = DocumentFileManager::documentFromXml(filepath);
  //        DocumentContext::instance()->setCurrentDocument(document);
  //        DocumentContext::instance()->setCurrentDocumentPath(filepath);
  //        _docController->setDocument(document);

  //        QString imagePath = strFolderOut + file.replace(".xml",".png");
  //        _docController->exportToImage(imagePath);
  //        sleep(3);
  //    }
  QDir folderIN(strFolderIn);
  QStringList filters;
  filters << QStringLiteral("*.xml");
  folderIN.setNameFilters(filters);

  QString filepath = folderIN.absoluteFilePath(file);
  //loading files
  Doc::Document *document = IOManager::DocumentFileManager::documentFromXml(filepath);
  Context::DocumentContext::instance()->setCurrentDocument(document);
  Context::DocumentContext::instance()->setCurrentDocumentPath(filepath);
  Doc::Document *prevDoc = _docController->getDocument();

  _docController->setDocument(document);

  QString fileC = file;
  QString imagePath = strFolderOut + fileC.replace(QStringLiteral(".xml"), QStringLiteral(".png"));
  _docController->exportToImage(imagePath);

  delete document;

  _docController->setDocument(prevDoc);
}
*/

void
DocCreator::openDocument()
{
  if (Context::DocumentContext::instance()->modified()) {
    int response = processDisplaySaveMessage(QStringLiteral("openning"));

    switch (response) {
      case QMessageBox::Save:
        saveDocument();
      case QMessageBox::Discard:
        break;
      case QMessageBox::Cancel:
        return;
      default:
        break;
    }
  }

  const QList<QByteArray> listImageFormats =
    QImageReader::supportedImageFormats();

  QString allFormats(QStringLiteral("All documents (*.od *.xml "));
  const int nbFmts = listImageFormats.count();
  if (nbFmts > 0) {
    allFormats.append("*." + listImageFormats[0]);
    for (int i = 1; i < nbFmts; ++i) //start from 1
      allFormats.append(" *." + listImageFormats[i]);
  }
  allFormats.append(");;");

  QString filter;
  filter.append(allFormats);
  filter.append(QStringLiteral("Xml Old Document (*.od *.xml);;"));
  filter.append(getReadImageFilter());

  //std::cerr<<"filter="<<filter.toStdString()<<"\n";

  QString filepath = QFileDialog::getOpenFileName(
    this, tr("Open an old document..."), QString(), filter);

  if (!filepath.isEmpty()) {

    QGuiApplication::setOverrideCursor(Qt::BusyCursor);

    // open XML file
    // added by kvcuong 07/05/2012
    QStringList listStr = filepath.split('.');
    assert(!listStr.isEmpty());
    QString strExtension = listStr.at(listStr.size() - 1);
    if (strExtension.contains(QStringLiteral("od"), Qt::CaseInsensitive) ||
        strExtension.contains(QStringLiteral("xml"), Qt::CaseInsensitive)) {
      QString XsdPath = Core::ConfigurationManager::get(
                          AppConfigMainGroup, AppConfigXmlCheckerFolderKey)
                          .toString() +
                        Core::ConfigurationManager::get(
                          AppConfigMainGroup, AppConfigDocumentXSDCheckerKey)
                          .toString();
      QUrl schemaUrl(XsdPath);
      MessageHandler messageHandler;
      QXmlSchema schema;
      schema.setMessageHandler(&messageHandler);
      schema.load(schemaUrl);
      if (schema.isValid()) {
        QXmlSchemaValidator validator(schema);
        QFile file(filepath);
        file.open(QIODevice::ReadOnly);
        if (!validator.validate(&file))
          QMessageBox::warning(this,
                               tr("Error : invalid document"),
                               messageHandler.statusMessage() + "\nOn line " +
                                 QString::number(messageHandler.line()) +
                                 ", Column " +
                                 QString::number(messageHandler.column()));
      }

      /* Open and load the document */
      updateTitle(filepath);
      Doc::Document *document =
        IOManager::DocumentFileManager::documentFromXml(filepath);
      Context::DocumentContext::instance()->setCurrentDocument(document);
      Context::DocumentContext::instance()->setCurrentDocumentPath(filepath);

      Doc::Document *prevDoc = _docController->getDocument();
      _docController->setDocument(document);
      delete prevDoc;

    } else {
      /** Added by kvcuong 07/05/2012
       * Load document from image
       */

      bool isImage = false;
      for (const QByteArray &ext : listImageFormats)
        if (strExtension.contains(ext, Qt::CaseInsensitive)) {
          isImage = true;
          break;
        }
      //BORIS:TODO: we should load the image instead of checking only the extension...
      if (isImage) {

        /* Open and load the document */

        auto document = new Doc::Document();

        Context::DocumentContext::instance()->setCurrentDocument(document);
        assert(_docController != nullptr);
        Doc::Document *prevDoc = _docController->getDocument();
        _docController->setDocument(document);
        delete prevDoc;

        if (!filepath.isEmpty() && !filepath.isNull()) {
          //qDebug() << filepath;

          QImage input(filepath);
          if (!input.isNull()) {

            //B: Should we scale if necessary ???
            Core::ConfigurationManager::set(
              AppConfigMainGroup, AppConfigPageSizeX, input.width());
            Core::ConfigurationManager::set(
              AppConfigMainGroup, AppConfigPageSizeY, input.height());
            //B:TODO: recenter view !!!

            //B:TODO:UGLY !!! We don't want to write the image on disk !!!
            //input.save("image_original.jpg"); //B:useless ???

            document->add(new Doc::Page(document));
            document->setPageHeight(input.height());
            document->setPageWidth(input.width());

            BackGroundChanger bgChanger;
            bgChanger.changeBackGroundImage(input);
            //_docController->addComponentBlock("Bmt_res2812_002.tif");
          }
        }
      }
    }

    QGuiApplication::restoreOverrideCursor();
  }
}

void
DocCreator::print()
{
  auto printer = new QPrinter();
  auto dialog = new QPrintDialog(printer, this);
  dialog->setWindowTitle(tr("Print Document"));
  if (dialog->exec() == QDialog::Accepted)
    _docController->printDocument(printer);
}

void
DocCreator::saveDocument()
{
  if (Context::DocumentContext::instance()->isNewDocument())
    saveDocumentAs();
  else if (Context::DocumentContext::instance()->modified())
    saveDocumentToXml(
      Context::DocumentContext::instance()->getCurrentDocumentPath());
}

void
DocCreator::saveDocumentAs()
{
  QString filepath =
    QFileDialog::getSaveFileName(this,
                                 tr("Save current document as..."),
                                 QString(),
                                 tr("Xml Old Document (*.od *.xml)"));

  if (!filepath.isEmpty())
    saveDocumentToXml(filepath);
}

void
DocCreator::saveDocumentToXml(const QString &filepath)
{
  QFileInfo info(filepath);
  QString extension = info.suffix();
  QString lFilePath;
  if (extension != QLatin1String("od") && extension != QLatin1String("xml")) {
    lFilePath = filepath + ".xml";
  } else {
    lFilePath = filepath;
  }
  updateTitle(lFilePath);
  IOManager::DocumentFileManager::documentToXml(
    Context::DocumentContext::instance()->getCurrentDocument(), lFilePath);
  Context::DocumentContext::instance()->setCurrentDocumentPath(lFilePath);
}

void
DocCreator::exportDocToTxt()
{
  QString filepath =
    QFileDialog::getSaveFileName(this,
                                 tr("Export current document to txt file..."),
                                 QString(),
                                 tr("Txt  Document (*.txt)"));

  if (!filepath.isEmpty()) {
    IOManager::DocumentFileManager m;
    m.documentToTxt(Context::DocumentContext::instance()->getCurrentDocument(),
                    filepath);
  }
}

void
DocCreator::undo()
{
  //TODO: undo function
}

void
DocCreator::about()
{
  QMessageBox::about(
    this,
    tr("DocCreator"),
    tr("This application let you create synthetic document and apply "
       "degradation models<br/><br/>Authors: Nicholas Journet, Boris "
       "Mansencal, Antoine Billy, Kieu Van-Cuong, Vincent Rabeux, Nicolas "
       "Vidal, Jérémy Albouys, ..."));
}

void
DocCreator::loadFont()
{
  QString filePath = QFileDialog::getOpenFileName(
    this, tr("Open a font..."), QString(), tr("Xml Old font (*.of *.xml)"));

  if (!filePath.isEmpty())
    addNewFont(IOManager::FontFileManager::fontFromXml(filePath));
}

/*
void DocCreator::loadFontFromPath(const QString &fontPath)
{
  if (!fontPath.isEmpty()) //B:TODO: not enough ! File may not be readable and read may fail.
    addNewFont(IOManager::FontFileManager::fontFromXml(fontPath));
}
*/

void
DocCreator::addMatch()
{
  _listDialog->insertRow(0);
  _listDialog->setData(_listDialog->index(0, 0), QString());
  _listDialog->setData(_listDialog->index(0, 1), QString());
  _matchesTable->setModel(_listDialog);
}

/*
void DocCreator::generateImagesFromDir()
{
  auto dialog = new ImageGenerationFromDirDialog(this, _docController);
  dialog->exec();
}
*/

/*
void DocCreator::importFontFromDir()
{
  QString dirpath = QFileDialog::getExistingDirectory(this, tr("Import font from directory"));

  if (!dirpath.isEmpty())
    {
      QDialog *matchesDialog = new QDialog(this);
      QVBoxLayout *layoutDialog = new QVBoxLayout(matchesDialog);
      matchesDialog->setLayout(layoutDialog);

      _matchesTable = new QTableView();

      _listDialog =  new QStandardItemModel(0, 2, _matchesTable);
      _listDialog->setHeaderData(0, Qt::Horizontal, QObject::tr("In filename"));
      _listDialog->setHeaderData(1, Qt::Horizontal, QObject::tr("Character"));

      _matchesTable->setModel(_listDialog);

      QPushButton *addButton = new QPushButton();
      QObject::connect(addButton, SIGNAL(clicked()), this, SLOT(addMatch()));
      addButton->setText(tr("Add match"));

      QPushButton *validButton = new QPushButton();
      QObject::connect(validButton, SIGNAL(clicked()), matchesDialog, SLOT(accept()));
      validButton->setText(tr("Valid"));

      layoutDialog->addWidget(_matchesTable);
      layoutDialog->addWidget(addButton);
      layoutDialog->addWidget(validButton);

      int resultDialog = matchesDialog->exec();

      QMap<QString, QString> matches;

      if (resultDialog==QDialog::Accepted) {
    for (int i=0;i<_listDialog->rowCount(); ++i)
      matches.insert(_listDialog->data(_listDialog->index(i,0)).toString(), _listDialog->data(_listDialog->index(i,1)).toString());
      }
      addNewFont(IOManager::FontFileManager::fontFromDirectory(dirpath, matches));
    }

  saveFontToXml(Context::FontContext::instance()->currentFontPath());
}
*/

void
DocCreator::addNewFont(Models::Font *font)
{
  assert(font);
  //qDebug()<<"DocCreator::addNewFont name="<<font->getName()<<"\n";

  Context::FontContext *fontContext = Context::FontContext::instance();

  //We check if a font with this name already exists.
  //If this name is already used, we change the font name to disambiguate
  int index = 0;
  QString originalFontName = font->getName();
  QString newFontName = originalFontName;
  while (fontContext->getFont(newFontName) != nullptr)
    newFontName = originalFontName + "(" + QString::number(index++) + ")";
  if (newFontName != originalFontName)
    font->setName(newFontName);

  fontContext->addFont(font);
  fontContext->setCurrentFont(font->getName());

  //Add the font to the fontSelector
  int indexFont = _fontComboBox->findText(font->getName());
  if (indexFont == -1) {
    _fontComboBox->addItem(font->getName());
  }
  _fontComboBox->setCurrentIndex(_fontComboBox->findText(font->getName()));

  //Redraw the keyboard with new font
  _keyboardView->drawKeyboard(font);
}

/*
void DocCreator::saveCurrentFont()
{
  saveFontToXml(Context::FontContext::instance()->currentFontPath());
}

void DocCreator::saveCurrentFontAs()
{
  QString filepath = QFileDialog::getSaveFileName (this, tr("Save current font as..."), QString(), tr("Xml Old font (*.of *.xml)"));

  if (!filepath.isEmpty())
    saveFontToXml(filepath);
}

void DocCreator::saveFontToXml(const QString &filepath)
{
  IOManager::FontFileManager::fontToXml(Context::FontContext::instance()->getCurrentFont(), filepath);
}
*/

void
DocCreator::importBackground()
{
  QString imagePath = QFileDialog::getOpenFileName(
    this, tr("Import a background..."), QString(), getReadImageFilter());
  if (!imagePath.isEmpty() && !imagePath.isNull()) {
    QFile file(imagePath);
    QFileInfo fi(file);
    QString background = fi.fileName();
    if (file.copy(QString(Context::BackgroundContext::instance()->getPath() +
                          background))) {
      addBackground(background);
    } else {
      QMessageBox::warning(
        this, tr("Copy failed"), tr("Import of the background has failed."));
    }
  }
}

void
DocCreator::loadKeyboard()
{
  /*
    if (_keyboardView == nullptr)
    return;
    _keyboardView->loadKeyboard(QFileDialog::getOpenFileName(this, tr("Choose a keyboard-config file"),
    "", tr("Xkeyboard (*.kb)")));*/
}

void
DocCreator::insertTextBlock()
{
  _docController->addTextBlock();

  //B ????
  Context::AppContext::instance()->setActiveController(
    _documentView->getController());
}

void
DocCreator::insertImageBlock()
{
  QString imagePath = QFileDialog::getOpenFileName(
    this, tr("Insert an image..."), QString(), getReadImageFilter());
  if (!imagePath.isEmpty() && !imagePath.isNull())
    _docController->addImageBlock(imagePath);
}

void
DocCreator::removeBlock()
{
  _docController->removeCurrentBlock();
}

void
DocCreator::exportToImage()
{
  QString imagePath = QFileDialog::getSaveFileName(
    this, tr("Export to PNG format..."), QString(), tr("PNG file (*.png)"));

  if (!imagePath.isEmpty() && !imagePath.isNull())
    _docController->exportToImage(imagePath);
}

void
DocCreator::applyBleedThrough()
{
  BleedThroughParametersDialog dialog(this);

  //B:TODO: we need a function 'QImage getBackground()" somewehre !
  const QString path = Context::BackgroundContext::instance()->getPath();
  const QString filename =
    path + Context::BackgroundContext::instance()->getCurrentBackground();
  QImage background(filename);

  dialog.setRectoImage(background);

  if (dialog.exec()) {

    BleedThroughParameters params;
    dialog.getParameters(params);

    //        QImage verso(params->getPathToVersoImage());
    //        QImage verso(params->getPathToVersoImage());

    //        verso = verso.convertToFormat(QImage::Format_RGB32);
    //        verso = verso.mirrored(true,false);
    //        int width = backGround.width()/2;
    //        int height = backGround.height()/2;
    //        backGround = backGround.scaled(width,height);
    //        verso = verso.scaled(width,height);

    dc::BleedThrough deg(params.getNbIterations(), this);

    deg.setVerso(params.getPathToVersoImage());

    //ImageExporter exporter("bleedThrough", this);
    //connect(&deg, SIGNAL(imageReady(const QImage*)),&exporter, SLOT(writeImage(const QImage*)));
    BackGroundChanger changer;
    //RandomDocumentCreator docCreator(_docController,nullptr, this);
    //ImageExporter exporter("doc", this);

    connect(&deg,
            SIGNAL(imageReady(QImage)),
            &changer,
            SLOT(changeBackGroundImage(QImage)));
    //connect(&changer, SIGNAL(backgroundChanged()),&docCreator, SLOT(create()));
    //connect(&docCreator, SIGNAL(imageReady(const QImage*)),&exporter, SLOT(writeImage(const QImage*)));

    deg.apply(); //produced image will be saved thanks to imageReady signal.
  }
}

void
DocCreator::applyBlurFilter()
{
  BlurFilterDialog dialog(this);

  //B:TODO: we need a function 'QImage getBackground()" somewehre !
  //const QString path = Context::BackgroundContext::instance()->getPath();
  //const QString filename = path+Context::BackgroundContext::instance()->getCurrentBackground();
  //QImage background(filename);


  QImage img = _docController->toQImage(WithTextBlocks | WithImageBlocks);

  dialog.setOriginalImage(img);

  if (dialog.exec()) {
    dc::BlurFilterQ deg(img,
                   dialog.getMethod(),
                   dialog.getIntensity(),
                   dialog.getMode(),
                   dialog.getFunction(),
                   dialog.getArea(),
                   dialog.getCoeff(),
                   dialog.getVertical(),
                   dialog.getHorizontal(),
                   dialog.getRadius(),
                   dialog.getPattern(),
                   this);

    BackGroundChanger changer;

    connect(&deg,
            SIGNAL(imageReady(QImage)),
            &changer,
            SLOT(changeBackGroundImage(QImage)));

    deg.apply();
  }
}

void
DocCreator::applyShadowBinding()
{
  ShadowBindingDialog dialog(this);

  //const QString path = Context::BackgroundContext::instance()->getPath();
  //const QString filename = path+Context::BackgroundContext::instance()->getCurrentBackground();
  //QImage background(filename);

  const QImage img = _docController->toQImage(WithTextBlocks | WithImageBlocks);

  dialog.setOriginalImage(img);

  if (dialog.exec()) {

    dc::ShadowBindingQ deg(
      img,
      dialog.getBorder(),
      dialog.getDistance(),
      dialog.getIntensity(),
      dialog.getAngle(),
      this);

    BackGroundChanger changer;

    //To remove ?
    connect(&deg,
            SIGNAL(imageReady(QImage)),
            &changer,
            SLOT(changeBackGroundImage(QImage)));

    deg.apply();
  }
}

void
DocCreator::applyHoleDegradation()
{
  HoleDegradationDialog dialog(this);

  //const QString path = Context::BackgroundContext::instance()->getPath();
  //const QString filename = path+Context::BackgroundContext::instance()->getCurrentBackground();
  //QImage background(filename);

  dialog.setOriginalImage(
    _docController->toQImage(WithTextBlocks | WithImageBlocks));

  if (dialog.exec()) {
    BackGroundChanger changer;
    changer.changeBackGroundImage(dialog.getResultImage());

    //HoleDegradation deg(_docController->toQImage(WithTextBlocks | WithImageBlocks), dialog.getPattern(), dialog.getHorizontal(), dialog.getVertical(), dialog.getSize(), dialog.getType(), dialog.getSide(), dialog.getColor(), dialog.getPageBelow(), dialog.getWidth(), dialog.getIntensity(), this);

    //BackGroundChanger changer;

    //To remove ?
    //connect(&deg, SIGNAL(imageReady(const QImage &)), &changer, SLOT(changeBackGroundImage(const QImage &)));

    //deg.apply();
  }
}

void
DocCreator::applyPhantomCharacter()
{
  PhantomCharacterDialog dialog(this);

  //const QString path = Context::BackgroundContext::instance()->getPath();
  //const QString filename = path+Context::BackgroundContext::instance()->getCurrentBackground();
  //QImage background(filename);

  dialog.setOriginalImage(
    _docController->toQImage(WithTextBlocks | WithImageBlocks));

  if (dialog.exec()) {
    //PhantomCharacter deg(_docController->toQImage(WithTextBlocks | WithImageBlocks), dialog.getFrequency(), this);

    BackGroundChanger changer;
    changer.changeBackGroundImage(dialog.getResultImg());

    //To remove ?
    //connect(&deg, SIGNAL(imageReady(const QImage &)), &changer, SLOT(changeBackGroundImage(const QImage &)));

    //deg.apply();
  }
}

void
DocCreator::applyCharacterDegradationModel()
{
  GrayCharacterDegradationParameter dialog(this);

  QImage img = _docController->toQImage(WithTextBlocks | WithImageBlocks);

  dialog.setOriginalImage(img);

  if (dialog.exec()) {
    QGuiApplication::setOverrideCursor(Qt::BusyCursor);

    dc::GrayscaleCharsDegradationModel cdg(img);

    QImage dst = cdg.degradateByLevel(dialog.getLevel());

    const bool writeOk = dst.save(dialog.getOutputFilename());

    QGuiApplication::restoreOverrideCursor();

    if (! writeOk) {
      QMessageBox::critical(
			    this,
			    "Grayscale Degradation Model",
			    "The image was not correctly saved!");
    }

  }
}

void
DocCreator::applyGradientDomainDegradation()
{
  GradientDomainDegradationDialog dialog(this);

  QImage img = _docController->toQImage(WithTextBlocks | WithImageBlocks);

  dialog.setOriginalImage(img);

  if (dialog.exec()) {
    QGuiApplication::setOverrideCursor(Qt::BusyCursor);

    dc::GrayscaleCharsDegradationModel cdg(img);

    QImage dst = dc::GradientDomainDegradation::degradation(img,
							    dialog.getStainImagesPath(),
							    dialog.getNumStains(),
							    dialog.getInsertType(),
							    dialog.getDoRotations());

    const bool writeOk = dst.save(dialog.getOutputFilename());

    QGuiApplication::restoreOverrideCursor();

    if (! writeOk) {
      QMessageBox::critical(
			    this,
			    "Gradient Domain Degradation",
			    "The image was not correctly saved!");
    }

  }
}

void
DocCreator::apply3DDistortionModel()
{
  auto w = new MeshWindow(); //has WA_DeleteOnClose flag.
  QImage docImage =
    _docController->toQImage(Color | WithTextBlocks | WithImageBlocks);
  w->show();
  const QString meshPath =
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigMeshFolderKey)
      .toString();
  const QString meshFilename = QDir(meshPath).absoluteFilePath(QStringLiteral(
    "PlaneRegular.brs")); //B:TODO:UGLY: have a constant defined at compile time

  w->loadMeshFile(meshFilename);
  w->setImage(
    docImage); //must be called after show() [to have an initialized GL context] !
}



void
DocCreator::createControllers()
{
  qDebug() << "\t create controller : start";
  //Create controllers
  _docController = new DocumentController(
    Context::DocumentContext::instance()->getCurrentDocument());
  _keyboardController = new KeyboardController();
  _fontController = new FontEditorController();

  qDebug() << "\t create controller: view";
  //Create view for each controller
  _documentView = new DocumentView(_docController);
  _docPropertiesView = new DocumentPropertiesView(_docController);

  qDebug() << "\t create controller: keyboard";
  QString keyboardPath =
    _keyboard; //Core::ConfigurationManager::get(AppConfigKeyBoardGroup, AppConfigKbFolderKey).toString() + Core::ConfigurationManager::get(AppConfigKeyBoardGroup, AppConfigKbDefautlFormatKey).toString() ;
  KeyboardViewDirector kbvDirector(new KeyboardViewXmlBuilder(keyboardPath));
  kbvDirector.constructKeyboardView();
  _keyboardView = kbvDirector.getKeyboardView();

  qDebug() << "\t create controller: keyboard" << keyboardPath;

  _fontEditorView = new FontEditorView(_fontController);
  qDebug() << "\t create controller: font";
  //Add visibility of other controller if needed
  if (_documentView != nullptr)
    _documentView->setKeyboardController(_keyboardController);
  if (_fontEditorView != nullptr)
    _fontEditorView->setKeyboardController(_keyboardController);
  if (_keyboardView != nullptr) {
    _keyboardView->setKeyboardController(_keyboardController);
    _keyboardView->setDocumentController(_docController);
    _keyboardView->setFontEditorController(_fontController);
  }

  //Adding font observers
  Context::FontContext::instance()->addObserver(_keyboardController);
  Context::FontContext::instance()->addObserver(_docController);
  Context::FontContext::instance()->addObserver(this);

  //Adding background observers
  Context::BackgroundContext::instance()->addObserver(_docController);

  //Adding docController observers
  _docController->addObserver(_docPropertiesView);
  _docController->addObserver(this); // For title update

  _keyboardView->drawKeyboard(
    Context::FontContext::instance()->getCurrentFont());
  qDebug() << "\t create controller: finished";
}

void
DocCreator::createActions()
{
  _newDocumentAct =
    new QAction(QIcon(":/images/new.png"), tr("&New Document"), this);
  _newDocumentAct->setShortcuts(QKeySequence::New);
  _newDocumentAct->setStatusTip(tr("Create a new document"));
  connect(_newDocumentAct, SIGNAL(triggered()), this, SLOT(newDocument()));

  _openDocumentAct =
    new QAction(QIcon(":/images/open.png"), tr("&Open Document..."), this);
  _openDocumentAct->setShortcuts(QKeySequence::Open);
  _openDocumentAct->setStatusTip(tr("Open a document"));
  _openDocumentAct->setToolTip(tr("Open a document or an image"));
  connect(_openDocumentAct, SIGNAL(triggered()), this, SLOT(openDocument()));

  _saveDocumentAct =
    new QAction(QIcon(":/images/save.png"), tr("&Save..."), this);
  _saveDocumentAct->setShortcuts(QKeySequence::Save);
  _saveDocumentAct->setStatusTip(tr("Save the current document"));
  connect(_saveDocumentAct, SIGNAL(triggered()), this, SLOT(saveDocument()));

  _exportDocToTxtAct = new QAction(
    QIcon(":/images/save.png"), tr("&Export to text file..."), this);
  _exportDocToTxtAct->setShortcuts(QKeySequence::Save);
  _exportDocToTxtAct->setStatusTip(tr("Save the current document"));
  connect(
    _exportDocToTxtAct, SIGNAL(triggered()), this, SLOT(exportDocToTxt()));

  _saveDocumentAsAct = new QAction(tr("&Save as..."), this);
  _saveDocumentAsAct->setShortcuts(QKeySequence::SaveAs);
  _saveDocumentAsAct->setStatusTip(
    tr("Save the current document as an other name"));
  connect(
    _saveDocumentAsAct, SIGNAL(triggered()), this, SLOT(saveDocumentAs()));

  _printAct = new QAction(QIcon(":/images/print.png"), tr("&Print..."), this);
  _printAct->setShortcuts(QKeySequence::Print);
  _printAct->setStatusTip(tr("Print the current document"));
  connect(_printAct, SIGNAL(triggered()), this, SLOT(print()));

  _undoAct = new QAction(QIcon(":/images/undo.png"), tr("&Undo"), this);
  _undoAct->setShortcuts(QKeySequence::Undo);
  _undoAct->setStatusTip(tr("Undo the last editing action"));
  connect(_undoAct, SIGNAL(triggered()), this, SLOT(undo()));

  _loadFontAct = new QAction(tr("&Load font..."), this);
  _loadFontAct->setShortcut(tr("Ctrl+I"));
  _loadFontAct->setStatusTip(tr("Load a font from a file (.of or .xml)"));
  connect(_loadFontAct, SIGNAL(triggered()), this, SLOT(loadFont()));

  _importBackgroundAct = new QAction(tr("&Import background..."), this);
  _importBackgroundAct->setStatusTip(tr("Import and add a new background"));
  connect(
    _importBackgroundAct, SIGNAL(triggered()), this, SLOT(importBackground()));

  _loadKeyboard = new QAction(tr("Load &keyboard"), this);
  _loadKeyboard->setShortcut(tr("Ctrl+K"));
  _loadKeyboard->setStatusTip(tr("Load a keyboard from a file (.kb)"));
  connect(_loadKeyboard, SIGNAL(triggered()), this, SLOT(loadKeyboard()));

  _quitAct = new QAction(tr("&Quit"), this);
  _quitAct->setShortcut(tr("Ctrl+Q"));
  _quitAct->setStatusTip(tr("Quit the application"));
  connect(_quitAct, SIGNAL(triggered()), this, SLOT(close()));

  _aboutAct = new QAction(tr("&About"), this);
  _aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  _textBlockAct =
    new QAction(QIcon(":/images/textblock.png"), tr("Add &text block"), this);
  _textBlockAct->setStatusTip(tr("Insert a text block in the document"));
  connect(_textBlockAct, SIGNAL(triggered()), this, SLOT(insertTextBlock()));

  _imageBlockAct =
    new QAction(QIcon(":/images/imageblock.png"), tr("Add &image block"), this);
  _imageBlockAct->setStatusTip(tr("Insert an image block in the document"));
  connect(_imageBlockAct, SIGNAL(triggered()), this, SLOT(insertImageBlock()));

  _removeBlockAct = new QAction(
    QIcon(":/images/removeblock.png"), tr("&Remove current block"), this);
  _removeBlockAct->setStatusTip(tr("Remove the current block in the document"));
  connect(_removeBlockAct, SIGNAL(triggered()), this, SLOT(removeBlock()));

  _exportToImageAct = new QAction(tr("&Export to PNG format..."), this);
  _exportToImageAct->setStatusTip(tr("Export the document to PNG format"));
  connect(_exportToImageAct, SIGNAL(triggered()), this, SLOT(exportToImage()));

  /* DEGRADATIONS */
  _applyBleedThrough = new QAction(tr("&Bleed-Through..."), this);
  _applyBleedThrough->setStatusTip(
    tr("Apply some bleed-through to the background image"));
  connect(
    _applyBleedThrough, SIGNAL(triggered()), this, SLOT(applyBleedThrough()));

  _applyBlurFilter = new QAction(tr("&Blur-Filter..."), this);
  _applyBlurFilter->setStatusTip(tr("Apply some blur to the document"));
  connect(_applyBlurFilter, SIGNAL(triggered()), this, SLOT(applyBlurFilter()));

  _applyShadowBinding = new QAction(tr("&ShadowBinding..."), this);
  _applyShadowBinding->setStatusTip(
    tr("Apply a luminosity defect to the document caused by the binding"));
  connect(
    _applyShadowBinding, SIGNAL(triggered()), this, SLOT(applyShadowBinding()));

  _applyHoleDegradation = new QAction(tr("&HoleDegradation..."), this);
  _applyHoleDegradation->setStatusTip(tr("Add holes to the document"));
  connect(_applyHoleDegradation,
          SIGNAL(triggered()),
          this,
          SLOT(applyHoleDegradation()));

  _applyPhantomCharacter = new QAction(tr("&PhantomCharacter..."), this);
  _applyPhantomCharacter->setStatusTip(
    tr("Add phantom characters to the document"));
  connect(_applyPhantomCharacter,
          SIGNAL(triggered()),
          this,
          SLOT(applyPhantomCharacter()));

  _applyCharacterDegradationModel =
    new QAction(tr("&GrayScale Character Degradation..."), this);
  _applyCharacterDegradationModel->setStatusTip(
    tr("Apply GrayScale Character Degradation model to the document"));
  connect(_applyCharacterDegradationModel,
          SIGNAL(triggered()),
          this,
          SLOT(applyCharacterDegradationModel()));

  _applyGradientDomainDegradation = new QAction(tr("&GradientDomainDegradation..."), this);
  _applyGradientDomainDegradation->setStatusTip(
    tr("Apply a luminosity defect to the document caused by the binding"));
  connect(
    _applyGradientDomainDegradation, SIGNAL(triggered()), this, SLOT(applyGradientDomainDegradation()));
  

  //DocCreator
  _chooseTypeDoc = new QAction(
    QIcon(":/images/generate.png"), tr("&Generate Documents..."), this);
  _chooseTypeDoc->setStatusTip(tr("Generate synthetic documents"));
  connect(_chooseTypeDoc, SIGNAL(triggered()), this, SLOT(generateDocuments()));


  //B
  //_viewXMLAct = new QAction(tr("View XML"), this);
  //_viewStructureAct = new QAction(tr("View structure"), this);
  //_attributeAct = new QAction(tr("Attribute"), this);

  _distortion3DModelAct = new QAction(tr("3D distortion..."), this);
  _distortion3DModelAct->setStatusTip(
    tr("Apply 3D distortion mode to the document"));
  connect(_distortion3DModelAct,
          SIGNAL(triggered()),
          this,
          SLOT(apply3DDistortionModel()));

  _binarization = new QAction(tr("&Binarization..."), this);
  _binarization->setStatusTip(tr("Binarization methods"));
  connect(_binarization, SIGNAL(triggered()), this, SLOT(binarization()));

  _synthetiseImage = new QAction(tr("&Synthetize image..."), this);
  _synthetiseImage->setStatusTip(tr("Synthetise image"));
  connect(_synthetiseImage, SIGNAL(triggered()), this, SLOT(synthetiseImage()));

  _lineDetection = new QAction(tr("&LineDetection..."), this);
  _lineDetection->setStatusTip(tr("Text line extraction"));
  connect(_lineDetection, SIGNAL(triggered()), this, SLOT(lineDetection()));

  _fontExtraction = new QAction(tr("&Font extraction..."), this);
  _fontExtraction->setStatusTip(tr("Font extraction Recognition"));
  connect(_fontExtraction, SIGNAL(triggered()), this, SLOT(fontExtraction()));

  _backgroundReconstruction =
    new QAction(tr("&Background Reconstruction..."), this);
  _backgroundReconstruction->setStatusTip(tr("Background Reconstruction"));
  connect(_backgroundReconstruction,
          SIGNAL(triggered()),
          this,
          SLOT(backgroundReconstruction()));

  _structureDetection = new QAction(tr("&Structure detection..."), this);
  _structureDetection->setStatusTip(tr("Structure Detection"));
  connect(
    _structureDetection, SIGNAL(triggered()), this, SLOT(structureDetection()));

  _createFontDocAct = new QAction(tr("Create document with font"), this);
  _createFontDocAct->setStatusTip(
    tr("Create document with all characters from font"));
  connect(
    _createFontDocAct, SIGNAL(triggered()), this, SLOT(createFontDocument()));

  //Gestion du focus
  connect(qApp,
          SIGNAL(focusChanged(QWidget*,QWidget*)),
          this,
          SLOT(onFocusChanged(QWidget*,QWidget*)));
}


/*
void DocCreator::componentsLabeling()
{
  QString newFontName = QStringLiteral("new_font_from_image");
    newFontName.append(QString::number(_newFontID));
    auto newFont = new Models::Font(newFontName);
    QList<Doc::DocComponentBlock*> chosenComponentBlocks = _docController->getDocument()->currentPage()->getComponentBlocks();

    ChooseLabelForComponentForm* chooseLabelForm = new ChooseLabelForComponentForm(_docController, newFont, chosenComponentBlocks);
    chooseLabelForm->show();
    _newFontID = _newFontID +1;
}
*/
/** Extraction connected components function
 * kvcuong
 * 09/05/2012
 */
/*
void DocCreator::extractConnectedComponents()
{
  QImage imgDocument = _docController->toQImage(WithTextBlocks); //| WithImageBlocks
  _docController->addComponentBlock(imgDocument);
}
*/

void
DocCreator::binarization()
{
  QImage imgDocument =
    _docController->toQImage(WithTextBlocks | WithImageBlocks);
  BinarizationDialog dialog(this);

  dialog.setOriginalImage(imgDocument);

  if (dialog.exec()) {
    BackGroundChanger changer;
    changer.changeBackGroundImage(dialog.getResultImage());
  }
}

void
DocCreator::lineDetection()
{
  QImage imgDocument =
    _docController->toQImage(WithTextBlocks | WithImageBlocks);
  LineDetectionDialog dialog(this);

  dialog.setOriginalImage(imgDocument);
  if (dialog.exec()) {
    BackGroundChanger changer;
    changer.changeBackGroundImage(dialog.getResultImage());
  }
}

void
DocCreator::backgroundReconstruction()
{
  QImage imgDocument =
    _docController->toQImage(WithTextBlocks | WithImageBlocks);
  BinarizationDialog binadialog(this);

  binadialog.setOriginalImage(imgDocument);

  if (binadialog.exec()) {
    BackgroundReconstructionDialog dialog(this);
    dialog.setOriginalImage(imgDocument);
    dialog.setBinarizedImage(binadialog.getResultImage());
    if (dialog.exec()) {
      QGuiApplication::setOverrideCursor(Qt::BusyCursor);
      dialog.process();
      BackGroundChanger changer;
      changer.changeBackGroundImage(dialog.getResultImage());
      QGuiApplication::restoreOverrideCursor();
    }
  }
}

void
DocCreator::structureDetection()
{
  QImage imgDocument =
    _docController->toQImage(WithTextBlocks | WithImageBlocks);
  BinarizationDialog binadialog(this);

  binadialog.setOriginalImage(imgDocument);

  if (binadialog.exec()) { // Binarization

    const QImage binaryImage = binadialog.getResultImage();

#if 0
      BackgroundReconstructionDialog bkgdialog(this);
      bkgdialog.setOriginalImage(imgDocument);
      bkgdialog.setBinarizedImage(binaryImage);
      
      if (bkgdialog.exec()) { // Background Reconstruction
	
	QGuiApplication::setOverrideCursor(Qt::BusyCursor);
	
	bkgdialog.process();
	QImage background = bkgdialog.getResultImage();
	
	QGuiApplication::restoreOverrideCursor();  
	
	StructureDialog structdialog(_docController, this);
	
	structdialog.init(imgDocument, binaryImage, background);
	
	if(structdialog.exec()){ // Structure detection
	  
	  QGuiApplication::setOverrideCursor(Qt::BusyCursor);
	  
	  BackGroundChanger changer;
	  changer.changeBackGroundImage(structdialog.getResultImage());
	  
	  structdialog.loremIpsum();
	  
	  QGuiApplication::restoreOverrideCursor();  
	}
      }

#else
    //Debug version:
    //Here we do not extract the background, but create a white background image.
    //It is much faster.

    QImage background(
      imgDocument.width(), imgDocument.height(), QImage::Format_RGB32);
    background.fill(0xffffffff);

    StructureDialog structdialog(_docController, this);

    structdialog.init(imgDocument, binaryImage, background);

    if (structdialog.exec()) { // Structure detection

      QGuiApplication::setOverrideCursor(Qt::BusyCursor);

      BackGroundChanger changer;
      changer.changeBackGroundImage(structdialog.getResultImage());

      structdialog.loremIpsum();

      QGuiApplication::restoreOverrideCursor();
    }

#endif //0
  }
}

void
DocCreator::createFontDocument()
{
  buildDocumentWithWholeFont(_docController);
}

void
DocCreator::synthetiseImage()
{
  QImage imgDocument =
    _docController->toQImage(WithTextBlocks | WithImageBlocks);

  BinarizationDialog binaDialog(this);

  binaDialog.setOriginalImage(imgDocument);

  if (binaDialog.exec()) { // Binarization

    BackgroundReconstructionDialog bkgDialog(this);
    bkgDialog.setOriginalImage(imgDocument);
    QImage binaryImage = binaDialog.getResultImage();
    bkgDialog.setBinarizedImage(binaryImage);

    if (bkgDialog.exec()) { // Background Reconstruction

      QGuiApplication::setOverrideCursor(Qt::BusyCursor);

      bkgDialog.process();
      QImage background = bkgDialog.getResultImage();

      QGuiApplication::restoreOverrideCursor();

      OCRSettingsDialog ocrSettings(this, getTessdataParentDir());
      if (ocrSettings.exec()) {

        OCRDialog ocrDialog(this);

        const QString tessdataParentDir = ocrSettings.tessdataParentDir();
        const QString lang = ocrSettings.language();
        ocrDialog.setParameters(tessdataParentDir, lang);

        QGuiApplication::setOverrideCursor(Qt::BusyCursor);

        ocrDialog.init(imgDocument, binaryImage);

        QGuiApplication::restoreOverrideCursor();

        if (ocrDialog.exec()) { // Font extraction

          const QString fontName = ocrDialog.saveFont();
          Models::Font *font =
            IOManager::FontFileManager::fontFromXml(fontName);
          assert(font);
          addNewFont(font);
          //B:TODO:ugly: we save to disk and reload !!!
          //B:TODO:addNewFont will add the font to the application ! We do not want that !?
          // we just want to add the font to the style of the document ? (cf
          // RandomDocumentGenerator.cpp)
          // or just do :
          // FontContext::instance()->setCurrentFont(FontFileManager::fontFromXml(fontName));
          // //?
          //B: Do we need to set the font here ? before StructureDialog ?

          //B:TODO !!!!!
          //const int newLineSpacing = computeLineSpacing(font);

          StructureDialog structDialog(_docController, this);

          structDialog.init(imgDocument, binaryImage, background);

          if (structDialog.exec()) { // Structure detection

            BackGroundChanger changer;
            changer.changeBackGroundImage(structDialog.getResultImage());

            structDialog.loremIpsum();
          }
        }
      }
    }
  }
}

QString
DocCreator::getTessdataParentDir() const
{
  QString d = Core::ConfigurationManager::get(AppConfigMainGroup,
                                              AppConfigTessdataParentFolderKey)
                .toString();
  if (d.isEmpty()) {
    //path relative to executable path
    /*
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("share");
    dir.cd("DocCreator");
    d = dir.absolutePath();
    */
    //tessdata/ at same level than data/, thus same parent.
    QDir dir(findRelativeDataDir());
    const bool ok = dir.cdUp();
    if (ok) {
      d = dir.absolutePath();
    }
  }
  std::cerr << "getTessdataParentDir(): " << d.toStdString() << "\n";
  return d;
}

void
DocCreator::fontExtraction()
{
  QImage imgDocument =
    _docController->toQImage(WithTextBlocks | WithImageBlocks);
  BinarizationDialog dialog(this);

  dialog.setOriginalImage(imgDocument);

  if (dialog.exec()) {

    OCRSettingsDialog ocrSettings(this, getTessdataParentDir());
    if (ocrSettings.exec()) {

      OCRDialog ocrDialog(this);

      const QString tessdataParentDir = ocrSettings.tessdataParentDir();
      const QString lang = ocrSettings.language();
      //std::cerr<<"tessdataParentDir="<<tessdataParentDir.toStdString()<<"\n";
      ocrDialog.setParameters(tessdataParentDir, lang);

      QGuiApplication::setOverrideCursor(Qt::BusyCursor);

      ocrDialog.init(imgDocument, dialog.getResultImage());

      QGuiApplication::restoreOverrideCursor();

      if (ocrDialog.exec()) {
        const QString fontName = ocrDialog.saveFont();
        Models::Font *font = IOManager::FontFileManager::fontFromXml(fontName);
        assert(font);
        addNewFont(font);
        const int lineSpacing = computeBestLineSpacing(*font);
        _docController->setParagraphLineSpacing(lineSpacing);
      }
    }
  }
}

void
DocCreator::createMenus()
{
  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addAction(_newDocumentAct);
  _fileMenu->addAction(_openDocumentAct);
  _fileMenu->addAction(_chooseTypeDoc);
  _fileMenu->addSeparator();
  _fileMenu->addAction(_saveDocumentAct);
  _fileMenu->addAction(_saveDocumentAsAct);
  _fileMenu->addSeparator();
  _fileMenu->addAction(_exportToImageAct);
  _fileMenu->addAction(_exportDocToTxtAct);
  _fileMenu->addAction(_printAct);
  _fileMenu->addSeparator();
  _fileMenu->addAction(_quitAct);

  _viewMenu = menuBar()->addMenu(tr("&View"));

  //_editMenu = menuBar()->addMenu(tr("&Edit"));
  //_editMenu->addAction(_undoAct);

  //    _insertMenu = menuBar()->addMenu(tr("&Insert"));
  //    _insertMenu->addAction(_textBlockAct);
  //    _insertMenu->addAction(_imageBlockAct);
  //    _insertMenu->addAction(_removeBlockAct);

  _imageMenu = menuBar()->addMenu(tr("&Image"));

  _imageDegradationSubMenu = _imageMenu->addMenu(tr("Degradation"));

  _grayScaleDegradationSubMenu =
    _imageDegradationSubMenu->addMenu(tr("GrayScale Degradation"));
  _colorDegradationSubMenu =
    _imageDegradationSubMenu->addMenu(tr("Color Degradation"));

  _grayScaleDegradationSubMenu->addAction(_applyBleedThrough);
  _grayScaleDegradationSubMenu->addAction(_applyCharacterDegradationModel);

  _colorDegradationSubMenu->addAction(_distortion3DModelAct);
  _colorDegradationSubMenu->addAction(_applyBlurFilter);
  _colorDegradationSubMenu->addAction(_applyShadowBinding);
  _colorDegradationSubMenu->addAction(_applyHoleDegradation);
  _colorDegradationSubMenu->addAction(_applyPhantomCharacter);
  _colorDegradationSubMenu->addAction(_applyGradientDomainDegradation);

  //B
  //_imageMenu->addAction(_viewXMLAct);
  //_imageMenu->addAction(_viewStructureAct);
  //_imageMenu->addAction(_attributeAct);
  _imageToolsSubMenu = _imageMenu->addMenu(tr("Edit"));
  _imageToolsSubMenu->addAction(_textBlockAct);
  _imageToolsSubMenu->addAction(_imageBlockAct);
  _imageToolsSubMenu->addAction(_removeBlockAct);

  _toolsMenu = menuBar()->addMenu(tr("&Tools"));
  _toolsMenu->addAction(_loadFontAct);
  _toolsMenu->addAction(_importBackgroundAct);
  _toolsMenu->addAction(_loadKeyboard);

  _fontGeneratorMenu = menuBar()->addMenu(tr("Synthetise"));
  _fontGeneratorMenu->addAction(_backgroundReconstruction);
  _fontGeneratorMenu->addAction(_fontExtraction);
  _fontGeneratorMenu->addAction(_synthetiseImage);

  _fontGeneratorSubMenu = _fontGeneratorMenu->addMenu(tr("Details"));
  _fontGeneratorSubMenu->addAction(_binarization);
  _fontGeneratorSubMenu->addAction(_lineDetection);
  _fontGeneratorSubMenu->addAction(_structureDetection);

  _fontGeneratorMenu->addSeparator();
  _fontGeneratorMenu->addAction(_createFontDocAct);

  menuBar()->addSeparator();

  _helpMenu = menuBar()->addMenu(tr("&Help"));
  _helpMenu->addAction(_aboutAct);
}

void
DocCreator::createToolBars()
{
  _fileToolBar = addToolBar(tr("File"));
  _fileToolBar->addAction(_newDocumentAct);
  _fileToolBar->addAction(_openDocumentAct);
  _fileToolBar->addAction(_saveDocumentAct);
  _fileToolBar->addAction(_printAct);

  _fileToolBar->addSeparator();
  _fileToolBar->addAction(_chooseTypeDoc);

  //_editToolBar = addToolBar(tr("Edit"));
  //_editToolBar->addAction(_undoAct);

  _insertToolBar = addToolBar(tr("Insert"));
  _insertToolBar->addAction(_textBlockAct);
  _insertToolBar->addAction(_imageBlockAct);
  _insertToolBar->addAction(_removeBlockAct);
}

void
DocCreator::createStatusBar()
{
  statusBar()->showMessage(tr("Ready"));
}

void
DocCreator::createDockWindows()
{
  QDockWidget *dock = new QDockWidget(tr("TextEditor"), this);
  dock->setAllowedAreas(Qt::AllDockWidgetAreas);
  auto textView = new TextView(_docController, _documentView);
  _documentView->setTextView(textView);
  assert(textView != nullptr);
  textView->setParent(dock);
  dock->setWidget(textView);
  addDockWidget(Qt::LeftDockWidgetArea, dock);
  _viewMenu->addAction(dock->toggleViewAction());

  dock = new QDockWidget(tr("FontEditor"), this);
  _fontEditorView->setParent(dock);
  dock->setWidget(_fontEditorView);
  addDockWidget(Qt::RightDockWidgetArea, dock);
  _viewMenu->addAction(dock->toggleViewAction());

  dock = new QDockWidget(tr("Virtual Keyboard"), this);
  _keyboardView->setParent(dock);
  dock->setWidget(_keyboardView);
  dock->setMinimumSize(QSize(_keyboardView->width(), _keyboardView->height()));
  addDockWidget(Qt::BottomDockWidgetArea, dock);
  _viewMenu->addAction(dock->toggleViewAction());

  dock = new QDockWidget(tr("Document properties"), this);
  _docPropertiesView->setParent(dock);
  dock->setWidget(_docPropertiesView);
  addDockWidget(Qt::BottomDockWidgetArea, dock);
  _viewMenu->addAction(dock->toggleViewAction());
}

void
DocCreator::populateFontSelector()
{
  //qDebug()<<"DocCreator::populateFontSelector ";
  assert(_fontComboBox);

  _fontComboBox->clear();

  QStringList fontNames = Context::FontContext::instance()->getFontNames();
  for (const QString &fontName : fontNames)
    _fontComboBox->addItem(fontName);

  const QString currentFontName =
    Context::FontContext::instance()->getCurrentFontName();
  const int index = _fontComboBox->findText(currentFontName);
  if (index != -1)
    _fontComboBox->setCurrentIndex(index);
}

void
DocCreator::createFontSelector()
{
  _fontLabel = new QLabel(tr("Font: "));
  _fontComboBox = new QComboBox();

  populateFontSelector();

  QToolBar *t = addToolBar(tr("FontSelector"));
  t->addWidget(_fontLabel);
  t->addWidget(_fontComboBox);

  connect(_fontComboBox,
          SIGNAL(currentIndexChanged(QString)),
          this,
          SLOT(changeFont(QString)));
}

void
DocCreator::changeFont(const QString &fontName)
{
  if (!fontName.isEmpty()) {
    //qDebug()<<"DocCreator::changeFont fontName="<<fontName;

    Context::FontContext::instance()->setCurrentFont(fontName);
    QWidget *central = centralWidget();
    if (central != nullptr)
      central->setFocus();
  }
}

void
DocCreator::addBackground(const QString &background)
{
  QString path = Context::BackgroundContext::instance()->getPath();

  path.append(background);

  QImage icon(path);

  _backgroundComboBox->addItem(QIcon(QPixmap::fromImage(icon).scaled(21, 30)),
                               background);
}

void
DocCreator::createBackgroundSelector()
{

  QLabel *backgroundLabel = new QLabel(tr("Background: "));
  _backgroundComboBox = new QComboBox();

  Context::BackgroundList backgroundList =
    Context::BackgroundContext::instance()->getBackgrounds();

  //    std::cout << backgroundList.size() << " " <<
  //    BackgroundContext::instance()->getCurrentBackground().toStdString()<<
  //    std::endl;

  for (const QString &background : backgroundList) {
    //        std::cout << background.toStdString()<< std::endl;
    addBackground(background);
  }

  //    std::cout << backgroundList.size() << " " <<
  //    BackgroundContext::instance()->getCurrentBackground().toStdString()<<
  //    std::endl;

  _backgroundComboBox->setCurrentIndex(_backgroundComboBox->findText(
    Context::BackgroundContext::instance()->getCurrentBackground()));
  //    std::cout << backgroundList.size() << " " <<
  //    BackgroundContext::instance()->getCurrentBackground().toStdString()<<
  //    std::endl;

  QToolBar *t = addToolBar(tr("BackgroundSelector"));
  t->addWidget(backgroundLabel);
  t->addWidget(_backgroundComboBox);

  connect(_backgroundComboBox,
          SIGNAL(currentIndexChanged(QString)),
          this,
          SLOT(changeBackground(QString)));
}

void
DocCreator::changeBackground(const QString &background)
{
  Context::BackgroundContext::instance()->setCurrentBackground(background);
}

void
DocCreator::onFocusChanged(QWidget * /*old*/, QWidget *now)
{
  ADocumentView<Doc::Document> *docView =
    dynamic_cast<ADocumentView<Doc::Document> *>(now);
  if (docView != nullptr) {
    Context::AppContext::instance()->setActiveController(
      docView->getController());
  } else {
    Mvc::IView *fontView = dynamic_cast<FontEditorView *>(now);
    if (fontView == nullptr)
      fontView = dynamic_cast<CharEditView *>(now);
    if (fontView != nullptr)
      Context::AppContext::instance()->setActiveController(
        _fontEditorView->getController());
  }
}

void
DocCreator::updateTitle(const QString &path)
{
  Context::DocumentContext::instance()->setModified(false);
  Context::DocumentContext::instance()->setCurrentDocumentPath(path);
  update();
}

void
DocCreator::closeEvent(QCloseEvent *event)
{
  if (Context::DocumentContext::instance()->modified()) {
    const int response = processDisplaySaveMessage(QStringLiteral("closing"));

    switch (response) {
      case QMessageBox::Save:
        saveDocument();
        break;
      case QMessageBox::Cancel:
        event->ignore();
        break;
      default:
        break;
    }
  }
}

int
DocCreator::processDisplaySaveMessage(const QString &action)
{
  return QMessageBox::warning(
    this,
    "Saving before " + action + "...",
    "Do you want to save your old document before " + action + " ?",
    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
    QMessageBox::Save);
}
