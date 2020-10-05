#include "HoleDegradationQ.hpp"

#include "Utils/convertor.h"
#include <opencv2/imgproc/imgproc.hpp>

namespace dc {

  namespace HoleDegradation {

    QImage
    addHole(const QImage &imgOriginal,
	    const QImage &pattern,
	    QPoint pos,
	    int size,
	    HoleType type,
	    HoleSide side,
	    const QColor &color,
	    const QImage &pageBelow,
	    int width,
	    float intensity)
    {
      const cv::Mat matOriginal = Convertor::getCvMat(imgOriginal);
      cv::Mat matBelow;
      if (!pageBelow.isNull()) {
	matBelow = Convertor::getCvMat(pageBelow);
	assert(matOriginal.type() == matBelow.type());
      }

      cv::Mat matPattern = Convertor::getCvMat(pattern); //TODO:OPTIM: getCvMat() produces a CV_8UC3 that we convert again. We should produce a grayscale image directly.
      if (matPattern.type() != CV_8UC1) {
	if (matPattern.type() == CV_8UC3) {
	  cv::cvtColor(matPattern, matPattern, cv::COLOR_BGR2GRAY);
	}
	else {
#if CV_MAJOR_VERSION < 3
	  const int code = CV_StsUnsupportedFormat;
#else
	  const int code = cv::Error::StsUnsupportedFormat;
#endif
	  CV_Error(code, "HoleDegradation: unhandled format");
	}
      }

      const cv::Scalar color4(color.blue(),
			      color.green(),
			      color.red(),
			      color.alpha());

      cv::Mat matOut = addHole(matOriginal,
			       matPattern,
			       cv::Point(pos.x(), pos.y()),
			       size,
			       type,
			       side,
			       color4,
			       matBelow,
			       width,
			       intensity);

      return Convertor::getQImage(matOut);
    }
    
    QImage addHoleAtRandom(const QImage &imgOriginal, const QImage &pattern, float ratioOutside, int size, HoleType type, HoleSide side, const QColor &color, const QImage &pageBelow, int width, float intensity)
    {
      
      const cv::Mat matOriginal = Convertor::getCvMat(imgOriginal);
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
#if CV_MAJOR_VERSION < 3
	const int code = CV_StsUnsupportedFormat;
#else
	const int code = cv::Error::StsUnsupportedFormat;
#endif
	CV_Error(code, "HoleDegradation: unhandled format");
	}
      }

      const cv::Scalar color4(color.blue(),
			      color.green(),
			      color.red(),
			      color.alpha());

      cv::Mat matOut = addHoleAtRandom(matOriginal,
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
    QImage finalImg = dc::HoleDegradation::addHole(_original,
						   _pattern,
						   _pos,
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
