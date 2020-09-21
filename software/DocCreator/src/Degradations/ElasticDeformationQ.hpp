#ifndef ELASTICDEFORMATIONQ_HPP
#define ELASTICDEFORMATIONQ_HPP

#include <QImage>
#include "Degradations/DocumentDegradation.hpp"
#include "Degradations/ElasticDeformation.hpp"

namespace dc {

  namespace ElasticDeformation {

    extern FRAMEWORK_EXPORT QImage transform(const QImage &img,
					     float alpha=2.0f,
					     float sigma=0.08f,
					     BorderReplication borderMode = BorderReplication::BLACK,
					     Interpolation interpolation = Interpolation::BILINEAR);

    extern FRAMEWORK_EXPORT QImage transform2(const QImage &img,
					      float alpha=2.0f,
					      float sigma=0.08f,
					      float alpha_affine=9.0f,
					      BorderReplication borderMode = BorderReplication::BLACK,
					      Interpolation interpolation = Interpolation::BILINEAR);

  } //namespace ElasticDeformation
  

} //namespace dc


#endif /* ! ELASTICDEFORMATIONQ_HPP */
