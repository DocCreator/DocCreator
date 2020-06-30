#include "character.h"

#include "characterdata.h"
#include <QDateTime>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif

namespace Models {
Character::Character()
  : _charValue(QStringLiteral("unassigned_name"))
  , _baseLine(100)
  , _upLine(0)
  , _leftLine(0)
  , _rightLine(100)
{}

Character::Character(const QString &s)
  : _charValue(s)
  , _baseLine(100)
  , _upLine(0)
  , _leftLine(0)
  , _rightLine(100)
{}

Character::Character(const QString &s,
                     qreal upLine,
                     qreal baseLine,
                     qreal leftLine,
                     qreal rightLine)
  : _charValue(s)
  , _baseLine(baseLine)
  , _upLine(upLine)
  , _leftLine(leftLine)
  , _rightLine(rightLine)
{}

Character::~Character()
{
  auto it = _dataList.begin();
  const auto itEnd = _dataList.end();
  for (; it != itEnd; ++it) {
    delete *it;
  }
}

void
Character::add(CharacterData *cd)
{
  auto it = _dataList.begin();
  const auto itEnd = _dataList.end();
  for (; it != itEnd; ++it) {
    if (*it == cd)
      return;
  }

  _dataList.push_back(cd);
  //B: We do not use anymore cd->getId()
}

/* Setters */
//    void Character::setCharacterUnicode(int code){
//        _unicode = code;
//    }

void
Character::setCharacterValue(const QString &s)
{
  _charValue = s;
}

void
Character::setBaseLine(qreal value)
{
  _baseLine = value;
}

void
Character::setUpLine(qreal value)
{
  _upLine = value;
}

void
Character::setLeftLine(qreal value)
{
  _leftLine = value;
}

void
Character::setRightLine(qreal value)
{
  _rightLine = value;
}

void
Character::setAllBaseLines(qreal up, qreal base, qreal left, qreal right)
{
  setBaseLine(base);
  setUpLine(up);
  setLeftLine(left);
  setRightLine(right);
}

/* Getters */
//    int Character::getCharacterUnicode() const{
//        return _unicode;
//    }

const CharacterData *
Character::getRandomCharacterData() const
{
  if (_dataList.isEmpty())
    return nullptr;

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
  const int randomIndex = qrand() % _dataList.size();
#else
  const int randomIndex = QRandomGenerator::global()->bounded(_dataList.size());
#endif
  return _dataList.at(randomIndex);
}

const CharacterData *
Character::getCharacterData(int id) const
{
  auto it = _dataList.begin();
  const auto itEnd = _dataList.end();
  for (; it != itEnd; ++it) {
    if ((*it)->getId() == id)
      return *it;
  }
  return nullptr;
}

const CharacterDataList &
Character::getAllCharacterData() const
{
  return _dataList;
}

int
Character::getCharacterDataCount() const
{

  return _dataList.size();
}

/*
void Character::initialize(const QString &s,
                           qreal upLine, qreal baseLine,
                           qreal leftLine, qreal rightLine)
{
  _charValue = s;
  _baseLine = baseLine;
  _upLine = upLine;
  _leftLine = leftLine;
  _rightLine = rightLine;
  //        _unicode =-1;
}
*/

} //namespace Models
