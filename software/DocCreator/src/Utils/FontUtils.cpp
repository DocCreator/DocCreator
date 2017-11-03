#include "FontUtils.hpp"

#include "models/character.h"
#include "models/characterdata.h"
#include "models/font.h"
#include <QImage>
#include <cassert>
#include <vector>

template<typename Func>
void
traverseFont(const Models::Font &f, Func &func)
{
  const Models::CharacterMap cm = f.getCharacters(); //B:TODO:OPTIM:copy

  for (Models::CharacterMap::const_iterator it = cm.cbegin(); it != cm.cend();
       ++it) {

    const Models::Character *c = it.value();
    assert(c);
    Models::CharacterDataList cds =
      c->getAllCharacterData(); //B:TODO:OPTIM:copy

    for (Models::CharacterDataList::const_iterator itC = cds.cbegin();
         itC != cds.cend();
         ++itC) {
      func((*itC)->getImage());
    }
  }
}

struct MeanCharacterHeightComputer
{
  unsigned int m_m0;
  unsigned int m_m1;

  MeanCharacterHeightComputer()
    : m_m0(0)
    , m_m1(0)
  {}

  void operator()(const QImage &img)
  {
    ++m_m0;
    m_m1 += img.height();
  }

  float getMean() const
  {
    if (m_m0 != 0u)
      return m_m1 / (float)m_m0;
    return 0;
  }
};

float
getMeanCharacterHeight(const Models::Font &f)
{
  MeanCharacterHeightComputer mchc;
  traverseFont<MeanCharacterHeightComputer>(f, mchc);
  return mchc.getMean();
}

struct MedianCharacterHeightComputer
{
  std::vector<int> m_heights;

  MedianCharacterHeightComputer()
    : m_heights()
  {}

  void operator()(const QImage &img) { m_heights.push_back(img.height()); }

  float getMedian()
  {
    if (!m_heights.empty()) {
      auto itM = m_heights.begin() + m_heights.size() / 2;
      nth_element(m_heights.begin(), itM, m_heights.end());
      return *itM;
    }
    return 0;
  }
};

float
getMedianCharacterHeight(const Models::Font &f)
{
  MedianCharacterHeightComputer mchc;
  traverseFont<MedianCharacterHeightComputer>(f, mchc);
  return mchc.getMedian();
}

struct MaxCharacterHeightComputer
{
  int m_maxHeight;

  MaxCharacterHeightComputer()
    : m_maxHeight(0)
  {}

  void operator()(const QImage &img)
  {
    const int h = img.height();
    m_maxHeight = (h > m_maxHeight ? h : m_maxHeight);
  }

  int getMax() const { return m_maxHeight; }
};

int
getMaxCharacterHeight(const Models::Font &f)
{
  MaxCharacterHeightComputer mchc;
  traverseFont<MaxCharacterHeightComputer>(f, mchc);
  return mchc.getMax();
}

template<typename Func>
void
traverseFontChar(const Models::Font &f, Func &func)
{
  const Models::CharacterMap cm = f.getCharacters(); //B:TODO:OPTIM:copy

  for (Models::CharacterMap::const_iterator it = cm.cbegin(); it != cm.cend();
       ++it) {

    const Models::Character *c = it.value();
    assert(c);
    Models::CharacterDataList cds =
      c->getAllCharacterData(); //B:TODO:OPTIM:copy

    for (Models::CharacterDataList::const_iterator itC = cds.cbegin();
         itC != cds.cend();
         ++itC) {
      func(c, (*itC)->getImage());
    }
  }
}

struct MaxCharacterHeightAndUnderBaselineComputer
{
  int m_maxHeight;
  int m_maxUnderBaseline;

  MaxCharacterHeightAndUnderBaselineComputer()
    : m_maxHeight(0)
    , m_maxUnderBaseline(0)
  {}

  void operator()(const Models::Character *c, const QImage &img)
  {
    const int h = img.height();
    m_maxHeight = (h > m_maxHeight ? h : m_maxHeight);
    assert(c);
    //assert(c->getBaseLine()<=100); //no the case for "'" character in JulesVerne.of
    const int underBaseline =
      static_cast<int>(((100. - c->getBaseLine()) / 100.) * h + 0.5);
    m_maxUnderBaseline =
      (underBaseline > m_maxUnderBaseline ? underBaseline : m_maxUnderBaseline);
  }

  int getMax() const { return m_maxHeight + m_maxUnderBaseline; }
};

int
getMaxCharacterHeightAndUnderBaseline(const Models::Font &f)
{
  MaxCharacterHeightAndUnderBaselineComputer mchmubc;
  traverseFontChar<MaxCharacterHeightAndUnderBaselineComputer>(f, mchmubc);
  return mchmubc.getMax();
}

#include <iostream> //DEBUG

struct MaxHeightUnderAndAboveBaselineComputer
{
  int m_maxUnderBaseline;
  int m_maxAboveBaseline;

  MaxHeightUnderAndAboveBaselineComputer()
    : m_maxUnderBaseline(0)
    , m_maxAboveBaseline(0)
  {}

  void operator()(const Models::Character *c, const QImage &img)
  {
    const int h = img.height();
    assert(c);
    //assert(c->getBaseLine()<=100); //no the case for "'" character in JulesVerne.of
    const int underBaseline =
      static_cast<int>(((100.f - c->getBaseLine()) / 100.f) * h + 0.5f);
    const int aboveBaseline =
      static_cast<int>((c->getBaseLine() / 100.f) * h + 0.5f);

    m_maxUnderBaseline =
      (underBaseline > m_maxUnderBaseline ? underBaseline : m_maxUnderBaseline);
    m_maxAboveBaseline =
      (aboveBaseline > m_maxAboveBaseline ? aboveBaseline : m_maxAboveBaseline);
  }

  int getMax() const { return m_maxUnderBaseline + m_maxAboveBaseline; }
};

int
getMaxCharacterHeightUnderAndAboveBaseline(const Models::Font &f)
{
  MaxHeightUnderAndAboveBaselineComputer mchmubc;
  traverseFontChar<MaxHeightUnderAndAboveBaselineComputer>(f, mchmubc);
  return mchmubc.getMax();
}

/*
  compute mean of @a sz first biggest elements of @a values

 */
static int
meanFirst(std::vector<int> &values, size_t sz)
{
  const size_t size = values.size();
  size_t s = sz;
  if (size > sz) {
    std::nth_element(
      values.begin(), values.begin() + sz, values.end(), std::greater<int>());
  } else {
    std::sort(values.begin(), values.end(), std::greater<int>());
    s = size;
  }

  long int sum = 0;
  for (size_t i = 0; i < s; ++i) {
    sum += values[i];
  }

  return s > 0 ? static_cast<int>(sum / (float)s + 0.5f) : 0;
}

struct MaxHeightUnderAndAboveBaselineComputerK
{
  std::vector<int> m_aboves;
  std::vector<int> m_unders;

  void operator()(const Models::Character *c, const QImage &img)
  {
    const int h = img.height();
    assert(c);
    //assert(c->getBaseLine()<=100); //no the case for "'" character in JulesVerne.of
    const int underBaseline =
      static_cast<int>(((100.f - c->getBaseLine()) / 100.f) * h + 0.5f);
    const int aboveBaseline =
      static_cast<int>((c->getBaseLine() / 100.f) * h + 0.5f);

    m_aboves.push_back(aboveBaseline);
    m_unders.push_back(underBaseline);
  }

  int getMax(size_t S = 5) //const
  {
    const int meanUnder = meanFirst(m_unders, S);
    const int meanAbove = meanFirst(m_aboves, S);

    return meanUnder + meanAbove;
  }

  int getMaxAbove(size_t S = 5)
  {
    const int meanAbove = meanFirst(m_aboves, S);

    return meanAbove;
  }
  
};

int
getMaxCharacterHeightUnderAndAboveBaseline5(const Models::Font &f)
{
  MaxHeightUnderAndAboveBaselineComputerK mchmubc;
  traverseFontChar<MaxHeightUnderAndAboveBaselineComputerK>(f, mchmubc);
  return mchmubc.getMax(5);
}

int
getMaxCharacterHeightAboveBaseline5(const Models::Font &f)
{
  MaxHeightUnderAndAboveBaselineComputerK mchmubc;
  traverseFontChar<MaxHeightUnderAndAboveBaselineComputerK>(f, mchmubc);
  return mchmubc.getMaxAbove(5);
}
