#ifndef CHARACTERSDEGRADATIONQ_HPP
#define CHARACTERSDEGRADATIONQ_HPP

#include <QImage>
#include <QObject>

#include "Degradations/GrayscaleCharsDegradationModel.hpp"


namespace dc {

  namespace CharactersDegradation {

    extern FRAMEWORK_EXPORT QImage degradation(const QImage &img, int level = 1, float percentOfIndepentSpots = 33, float percentOfOverlappingSpots = 33);

  } //namespace CharactersDegradation


} //namespace dc


#endif /* ! CHARACTERSDEGRADATIONQ_HPP */
