#ifndef NOISEDEGRADATIONQ_HPP
#define NOISEDEGRADATIONQ_HPP

#include <QImage>
#include "Degradations/DocumentDegradation.hpp"
#include "Degradations/NoiseDegradation.hpp"

namespace dc {

  namespace NoiseDegradation {

    extern FRAMEWORK_EXPORT QImage addGaussianNoise(const QImage &img,
						    float average=0.0f,
						    float standard_deviation=10.0f,
						    AddNoiseType addType=AddNoiseType::ADD_NOISE_AS_IS);

    extern FRAMEWORK_EXPORT QImage addSpeckleNoise(const QImage &img,
						   float average=0.0f,
						   float standard_deviation=1.0f,
						   AddNoiseType addType=AddNoiseType::ADD_NOISE_AS_IS);

    extern FRAMEWORK_EXPORT QImage addSaltAndPepperNoise(const QImage &img,
							  float amount=0.15f,
							  float ratio=0.5f);

  } //namespace NoiseDegradation
  

} //namespace dc


#endif /* ! NOISEDEGRADATIONQ_HPP */
