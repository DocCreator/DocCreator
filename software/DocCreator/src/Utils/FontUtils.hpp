#ifndef FONT_UTILS_HPP
#define FONT_UTILS_HPP

namespace Models {
class Font;
}

extern float
getMeanCharacterHeight(const Models::Font &f);

extern float
getMedianCharacterHeight(const Models::Font &f);

extern int
getMaxCharacterHeight(const Models::Font &f);

extern int
getMaxCharacterHeightAndUnderBaseline(const Models::Font &f);

/*
  Compute the sum of the maximum part of character above the baseline
  and the maximum part of character under the baseline.
 */
extern int
getMaxCharacterHeightUnderAndAboveBaseline(const Models::Font &f);

/*
  Compute the sum of the mean of the 5 biggest parts of character above the baseline
  and the mean of the 5 biggest parts of character under the baseline.
 */
extern int
getMaxCharacterHeightUnderAndAboveBaseline5(const Models::Font &f);

/*
  Compute the mean of the 5 biggest parts of character above the baseline.
 */
extern int
getMaxCharacterHeightAboveBaseline5(const Models::Font &f);


static inline int
computeBestLineSpacing(const Models::Font &f)
{
  return getMaxCharacterHeightUnderAndAboveBaseline5(f);
}

static inline int
computeBestAboveBaselineHeight(const Models::Font &f)
{
  return getMaxCharacterHeightAboveBaseline5(f);
}



#endif /* ! FONT_UTILS_HPP */
