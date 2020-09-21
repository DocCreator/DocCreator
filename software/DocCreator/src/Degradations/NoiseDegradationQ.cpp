#include "NoiseDegradationQ.hpp"
#include "Utils/convertor.h"


namespace dc {

  namespace NoiseDegradation {

    QImage addGaussianNoise(const QImage &img,
			    float average,
			    float standard_deviation,
			    AddNoiseType addType)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);

      const cv::Mat matOut = dc::NoiseDegradation::addGaussianNoise(matIn, average, standard_deviation, addType);

      return Convertor::getQImage(matOut);
    }

    QImage addSpeckleNoise(const QImage &img,
			   float average,
			   float standard_deviation,
			   AddNoiseType addType)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);
      
      const cv::Mat matOut = dc::NoiseDegradation::addSpeckleNoise(matIn, average, standard_deviation, addType);

      return Convertor::getQImage(matOut);
    }

    QImage addSaltAndPepperNoise(const QImage &img,
				 float amount,
				 float ratio)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);
      
      const cv::Mat matOut = dc::NoiseDegradation::addSaltAndPepperNoise(matIn, amount, ratio);
      
      return Convertor::getQImage(matOut);
    }

  } //namespace ShadowBinding
  

} //namespace dc
