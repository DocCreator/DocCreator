#include "HoleDegradationQ.hpp"

#include "Utils/convertor.h"
#include <opencv2/imgproc/imgproc.hpp>

namespace dc {

  namespace HoleDegradation {

    QImage
    holeDegradation(const QImage &imgOriginal,
		    const QImage &pattern,
		    int xOrigin,
		    int yOrigin,
		    int size,
		    HoleType type,
		    int side,
		    const QColor &color,
		    const QImage &pageBelow,
		    int width,
		    float intensity)
    {
      cv::Mat matOriginal = Convertor::getCvMat(imgOriginal);
      cv::Mat matBelow;
      if (!pageBelow.isNull()) {
	matBelow = Convertor::getCvMat(pageBelow);
	assert(matOriginal.type() == matBelow.type());
      }

      cv::Mat matPattern = Convertor::getCvMat(pattern); //TODO:OPTIM: getCvMat() procudes a CV_8UC3 that we convert again. We should produce a grayscale image directly.
      if (matPattern.type() != CV_8UC1) {
	if (matPattern.type() == CV_8UC3) {
	  cv::cvtColor(matPattern, matPattern, cv::COLOR_BGR2GRAY);
	}
	else {
	  CV_Error(cv::Error::StsUnsupportedFormat, "HoleDegradation: unhandled format");
	}
      }

      const cv::Scalar color4(
			      color.blue(), color.green(), color.red(), color.alpha());

      cv::Mat matOut = holeDegradation(matOriginal,
				       matPattern,
				       xOrigin,
				       yOrigin,
				       size,
				       type,
				       side,
				       color4,
				       matBelow,
				       width,
				       intensity);

      return Convertor::getQImage(matOut);
    }
    
    QImage holeDegradation(const QImage &imgOriginal, const QImage &pattern, float ratioOutside, int size, HoleType type, int side, const QColor &color, const QImage &pageBelow, int width, float intensity)
    {
      
      cv::Mat matOriginal = Convertor::getCvMat(imgOriginal);
      cv::Mat matBelow;
      if (!pageBelow.isNull()) {
	matBelow = Convertor::getCvMat(pageBelow);
	assert(matOriginal.type() == matBelow.type());
      }

      cv::Mat matPattern = Convertor::getCvMat(pattern); //TODO:OPTIM: getCvMat() procudes a CV_8UC3 that we convert again. We should produce a grayscale image directly.
      if (matPattern.type() != CV_8UC1) {
	if (matPattern.type() == CV_8UC3) {
	  cv::cvtColor(matPattern, matPattern, cv::COLOR_BGR2GRAY);
	}
	else {
	  CV_Error(cv::Error::StsUnsupportedFormat, "HoleDegradation: unhandled format");
	}
      }

      const cv::Scalar color4(
			      color.blue(), color.green(), color.red(), color.alpha());

      cv::Mat matOut = holeDegradation(matOriginal,
				       matPattern,
				       size,
				       type,
				       ratioOutside, 
				       side,
				       color4,
				       matBelow,
				       width,
				       intensity);

      return Convertor::getQImage(matOut);
    }
    
  } //namespace HoleDegradation

  QImage
  HoleDegradationQ::apply()
  {
    QImage finalImg = dc::HoleDegradation::holeDegradation(_original,
							   _pattern,
							   _xOrigin,
							   _yOrigin,
							   _size,
							   _type,
							   _side,
							   _color,
							   _pageBelow,
							   _width,
							   _intensity);
    
    emit imageReady(finalImg);
    
    return finalImg;
  }


  
} //namespace dc
