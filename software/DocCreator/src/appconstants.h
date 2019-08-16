#ifndef APPCONSTANTS_H
#define APPCONSTANTS_H

const QString AppConfigFile = QStringLiteral(":/application.ini");

/* Groups config */
const QString AppConfigMainGroup = QStringLiteral("mainapp");
const QString AppConfigKeyBoardGroup = QStringLiteral("keyboard");
const QString AppConfigFontGroup = QStringLiteral("fonteditor");
const QString AppConfigDocGroup = QStringLiteral("doceditor");

/* keys */
const QString AppConfigImageFolderKey = QStringLiteral("imagefolder");
const QString AppConfigFontFolderKey = QStringLiteral("fontfolder");
const QString AppConfigDefaultFontKey = QStringLiteral("defaultfont");
const QString AppConfigFontExtKey = QStringLiteral("defaultfontextension");
const QString AppConfigKbFolderKey = QStringLiteral("formatsfolder");
const QString AppConfigKbDefautlFormatKey = QStringLiteral("defaultformat");
const QString AppConfigPageSizeX = QStringLiteral("pagesizex");
const QString AppConfigPageSizeY = QStringLiteral("pagesizey");
const QString AppConfigBackgdFolderKey = QStringLiteral("backgroundfolder");
const QString AppConfigDefaultBackbg = QStringLiteral("defaultbackground");
const QString AppConfigXmlCheckerFolderKey = QStringLiteral("xmlcheckerfolder");
const QString AppConfigDocumentXSDCheckerKey =
  QStringLiteral("documentxsdchecker");

const QString AppConfigMeshFolderKey = QStringLiteral("meshfolder");
const QString AppConfigHolePatternsFolderKey =
  QStringLiteral("holepatternsfolder");
const QString AppConfigPhantomPatternsFolderKey =
  QStringLiteral("phantompatternsfolder");
const QString AppConfigBlurImagesFolderKey = QStringLiteral("blurimagesfolder");
const QString AppConfigStainImagesFolderKey =
  QStringLiteral("stainimagesfolder");

const QString AppConfigTessdataParentFolderKey =
  QStringLiteral("tessdataparentfolder");

/* Some app displays */
const static QString AppTitle = QStringLiteral("DocCreator - ");
const static QString DefaultPath = QStringLiteral("New old document");
const static QString ModifiedStr = QStringLiteral("*");

#endif // APPCONSTANTS_H
