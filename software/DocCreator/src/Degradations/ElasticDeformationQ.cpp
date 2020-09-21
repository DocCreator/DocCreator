#include "ElasticDeformationQ.hpp"
#include "Utils/convertor.h"


namespace dc {

  namespace ElasticDeformation {

    QImage transform(const QImage &img,
		     float alpha,
		     float sigma,
		     BorderReplication borderMode,
		     Interpolation interpolation)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);
      
      const cv::Mat matOut = dc::ElasticDeformation::transform(matIn, alpha, sigma,
							       borderMode, interpolation);

      return Convertor::getQImage(matOut);
    }

    QImage transform2(const QImage &img,
		      float alpha,
		      float sigma,
		      float alpha_affine,
		      BorderReplication borderMode,
		      Interpolation interpolation)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);
      
      const cv::Mat matOut = dc::ElasticDeformation::transform2(matIn, alpha, sigma, alpha_affine,
								borderMode, interpolation);
      
      return Convertor::getQImage(matOut);
    }

  } //namespace ShadowBinding
  

} //namespace dc
