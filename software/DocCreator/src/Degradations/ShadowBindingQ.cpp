#include "ShadowBindingQ.hpp"
#include "Utils/convertor.h"

namespace dc {

  namespace ShadowBinding {

    QImage
    shadowBinding(const QImage &imgOriginal,
		  dc::ShadowBinding::Border border,
		  int distance,
		  float intensity,
		  float angle)
    {
      const cv::Mat matIn = Convertor::getCvMat(imgOriginal);

      const cv::Mat matOut = dc::ShadowBinding::shadowBinding(matIn, border, distance, intensity, angle);

      const QImage out = Convertor::getQImage(matOut);

      return out;
    }

    QImage
    shadowBinding(const QImage &imgOriginal,
		  float distanceRatio,
		  dc::ShadowBinding::Border border,
		  float intensity,
		  float angle)
    {
      const cv::Mat matIn = Convertor::getCvMat(imgOriginal);

      const cv::Mat matOut = dc::ShadowBinding::shadowBinding(matIn, distanceRatio, border, intensity, angle);

      const QImage out = Convertor::getQImage(matOut);

      return out;  
    }

    

  } //namespace ShadowBinding




  QImage
  ShadowBindingQ::apply()
  {
    const QImage out =
      shadowBinding(_original, _border, _distance, _intensity, _angle);

    emit imageReady(out);

    return out;
  }
  

} //namespace dc
