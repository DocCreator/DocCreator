#include "GrayCharacterDegradationModelQ.hpp"

#include "Utils/convertor.h"


namespace dc {
  namespace CharactersDegradation {

    QImage degradation(const QImage &img, int level, float percentOfIndepentSpots, float percentOfOverlappingSpots)
    {
      const cv::Mat matImg = Convertor::getCvMat(img);
      cv::Mat out = dc::GrayscaleCharsDegradation::degradation(matImg, level, percentOfIndepentSpots, percentOfOverlappingSpots);
      return Convertor::getQImage(out);
    }

  }

} //namespace dc
