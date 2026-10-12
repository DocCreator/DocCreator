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
   * Add Character @a cd if not already present. A null @a cd is ignored.
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

  /**
   * @return true if the character holds at least one CharacterData.
   */
  bool hasCharacterData() const;

  /**
   * @return one of the images of the character, drawn at random.
   *
   * @warning the character must hold at least one CharacterData, cf
   * hasCharacterData(). A character held by a Font always does: the two
   * font readers reject a letter without any, and add() ignores a null
   * one.
   */
  const CharacterData &getRandomCharacterData() const;

  /**
   * @return the CharacterData with id @a id, or nullptr if the character
   * does not hold it.
   *
   * An id is not an index: it identifies one version of the character, and
   * a document stores it to be redrawn with that very version. A document
   * opened against a font edited since may thus ask for an id that is gone.
   */
  const CharacterData *getCharacterData(int id) const;

  /**
   * @return the CharacterData with id @a id if the character holds it, and
   * its first one otherwise.
   *
   * For the places where a missing id must not stop the drawing. Unlike
   * getRandomCharacterData(), the replacement is the same on every call, so
   * a document is drawn the same way each time it is opened.
   *
   * @warning the character must hold at least one CharacterData, cf
   * hasCharacterData().
   */
  const CharacterData &getCharacterDataOrFirst(int id) const;

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

  //true once getCharacterDataOrFirst() has warned, so that it warns once
  //per character and not once per drawn letter
  mutable bool _warnedOnMissingId = false;
};
} // Models

#endif // CHARACTER_H
