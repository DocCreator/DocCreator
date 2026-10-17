#ifndef CHARACTERSDEGRADATIONQ_HPP
#define CHARACTERSDEGRADATIONQ_HPP

#include <QImage>
#include <QObject>

#include "Degradations/GrayscaleCharsDegradationModel.hpp"


namespace dc {

  namespace CharactersDegradation {

    /*
      @a indepentSpotsRatio and @a overlappingSpotsRatio are ratios in
      [0; 1], of sum at most 1 (the rest is for disconnection spots), as
      for dc::GrayscaleCharsDegradation::degradation(), which throws
      std::invalid_argument otherwise.
    */
    extern FRAMEWORK_EXPORT QImage degradation(const QImage &img, int level = 1, float indepentSpotsRatio = .33f, float overlappingSpotsRatio = .33f);

  } //namespace CharactersDegradation


} //namespace dc


#endif /* ! CHARACTERSDEGRADATIONQ_HPP */
