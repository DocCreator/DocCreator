#ifndef FONTFILEMANAGER_H
#define FONTFILEMANAGER_H

#include <QMap>
#include <framework_global.h>
class QXmlStreamReader;
class QXmlStreamWriter;
namespace Models {
class Character;
}
namespace Models {
class CharacterData;
}
namespace Models {
class Font;
}

namespace IOManager {
class FRAMEWORK_EXPORT FontFileManager
{
public:
  //Font Input/Output
  static Models::Font *fontFromXml(const QString &filepath);
  static Models::Font *fontFromDirectory(const QString &dirpath,
                                         const QMap<QString, QString> &matches);
  static void fontToXml(const Models::Font *font, const QString &filepath);
  static int saveBaseLineInformation(const QString &path,
                                     qreal base,
                                     qreal right,
                                     const QString &character);

private:
  // deal with unicode

  //CharacterData Input/Output
  static Models::Character *characterFromXml(QXmlStreamReader &reader);
  static void characterToXml(const Models::Character *character,
                             QXmlStreamWriter &writer);

  //Character Input/Output
  static Models::CharacterData *characterDataFromXml(QXmlStreamReader &reader);
  static void characterDataToXml(const Models::CharacterData *charData,
                                 QXmlStreamWriter &writer);
};
} // FontFileManager

#endif // FontFileManager_H
