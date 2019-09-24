#ifndef CHARACTER_H
#define CHARACTER_H

#include <QList>
#include <QString>
#include <framework_global.h>

namespace Models {
class CharacterData;

 using CharacterDataList = QList<CharacterData *>;

class FRAMEWORK_EXPORT Character
{
public:
  Character();

  explicit Character(const QString &s);

  Character(const QString &s,
            qreal upLine,
            qreal baseLine,
            qreal leftLine,
            qreal rightLine);

  /**
   * @brief Destructor.
   *
   * Will delete all its CharacterData.
   */
  ~Character();

  /**
   * Add Character @a cd if not already present.
   *
   * @warning take ownership of @a cd.
   */
  void add(CharacterData *cd);

  /* Setters */
  //void setCharacterUnicode(int code);
  void setCharacterValue(const QString &s);
  void setBaseLine(qreal value);
  void setUpLine(qreal value);
  void setLeftLine(qreal value);
  void setRightLine(qreal value);
  void setAllBaseLines(qreal up, qreal base, qreal left, qreal right);

  /* Getters */
  const CharacterData *getRandomCharacterData() const;
  const CharacterData *getCharacterData(int id) const;
  const CharacterDataList &getAllCharacterData() const;
  int getCharacterDataCount() const;

  QString getCharacterValue() const { return _charValue; }
  qreal getBaseLine() const { return _baseLine; }
  qreal getUpLine() const { return _upLine; }
  qreal getLeftLine() const { return _leftLine; }
  qreal getRightLine() const { return _rightLine; }
  //int getCharacterUnicode() const;

private:
  //void initialize(const QString &s, qreal upLine, qreal baseLine, qreal leftLine, qreal rightLine);

private:
  //int _unicode;
  CharacterDataList _dataList;

  QString _charValue;
  qreal _baseLine;
  qreal _upLine;
  qreal _leftLine;
  qreal _rightLine;
};
} // Models

#endif // CHARACTER_H
