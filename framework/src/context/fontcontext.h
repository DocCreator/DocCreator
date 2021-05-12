#ifndef FONTCONTEXT_H
#define FONTCONTEXT_H

#include <QMap>
#include <QStringList>
#include <framework_global.h>
#include <patterns/observable.h>
#include <patterns/singleton.h>

namespace Models {
class Font;
}

namespace Context {

using FontMap = QMap<QString, Models::Font *>;
class FontContext;

class FRAMEWORK_EXPORT FontContext
  : public Patterns::Observable
  , public Patterns::Singleton<FontContext>
{
public:
  static FontContext *instance();

  FontContext();

  void initialize(const QString &path, const QString &fileExtension);

  Models::Font *getCurrentFont();

  void addFontsFromFiles(const QStringList &fontFiles); //B

  //Take ownership of added @a font.
  void addFont(Models::Font *font);

  //warning: client responsibility to delete returned font.
  Models::Font *removeFont(const QString &fontName);

  void setCurrentFont(const QString &fontName);
  Models::Font *getFont(const QString &fontName);

  int getNumberOfFonts() const;
  QStringList getFontNames() const;

  //Return short name of the font. For example "MyFont" for MyFont.of
  QString getCurrentFontName() const;

  bool fontHasChanged() const;
  bool fontCurrentModified() const;

  void setVars(const QString &path, const QString &fileExtension);

  //Return absolute filename of font file
  QString currentFontPath() const;

  //will delete all fonts
  void clear(); //B

private:
  FontContext(const FontContext &) = delete;
  FontContext &operator=(const FontContext &) = delete;

  bool checkInvariant() const;

private:
  static FontContext *_instance;

  Models::Font *_currentFont;
  FontMap _fontMap;
  bool _fontChanged;
  bool _currentFontModified;

  QString _fileExtension;
  QString _fontDir;
};
}
#endif // FONTCONTEXT_H
