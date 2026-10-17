#include "GrayCharacterDegradationModelQ.hpp"

#include "Utils/convertor.h"


namespace dc {
  namespace CharactersDegradation {

    QImage degradation(const QImage &img, int level, float indepentSpotsRatio, float overlappingSpotsRatio)
    {
      const cv::Mat matImg = Convertor::getCvMat(img);
      cv::Mat out = dc::GrayscaleCharsDegradation::degradation(matImg, level, indepentSpotsRatio, overlappingSpotsRatio);
      return Convertor::getQImage(out);
    }

  }

} //namespace dc
