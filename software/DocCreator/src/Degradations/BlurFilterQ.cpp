#include "BlurFilterQ.hpp"

#include "Utils/convertor.h"

namespace dc {

  QImage
  BlurFilterQ::apply()
  {
    QImage finalImg;

    if (_mode == dc::BlurFilter::Mode::COMPLETE)
      finalImg = dc::BlurFilter::blur(_original, _method, _intensity);
    else
      finalImg = applyPattern(_original, _pattern, _method, _intensity);

    emit imageReady(finalImg);

    return finalImg;
  }

  namespace BlurFilter {
  
    QImage
    applyPattern(const QImage &originalImg,
		 const QImage &pattern,
		 Method method,
		 int intensity)
    {
      cv::Mat originalMat = Convertor::getCvMat(originalImg);
      cv::Mat patternMat = Convertor::getCvMat(pattern);

      patternMat = Convertor::binarizeOTSU(patternMat);
      assert(patternMat.type() == CV_8UC1);

      cv::Mat resultMat = applyPattern(originalMat, patternMat, method, intensity);
      assert(resultMat.type() == originalMat.type());

      return Convertor::getQImage(resultMat);
    }

    QImage
    makePattern(const QImage &originalImg,
		Function function,
		Area area,
		float coeff,
		int vertical,
		int horizontal,
		int radius)
    {
      cv::Mat originalMat = Convertor::getCvMat(originalImg);

      cv::Mat resultMat = makePattern(originalMat,
				      function, area,
				      coeff, vertical, horizontal, radius);
      assert(resultMat.type() == CV_8UC1);

      return Convertor::getQImage(resultMat);
    }

    /*
      static QImage degradateArea(QImage original, QImage blurred, Function function, Area area, float coeff, int vertical, int horizontal, int radius)
      {
      cv::Mat originalMat = Convertor::getCvMat(original);
      cv::Mat blurredMat = Convertor::getCvMat(blurred);

      cv::Mat resultMat = degradateArea(originalMat, blurredMat, function, area, coeff, vertical, horizontal, radius);

      return Convertor::getQImage(resultMat);
      }
    */

    QImage
    blur(const QImage &originalImg, Method method, int intensity)
    {
      cv::Mat matIn = Convertor::getCvMat(originalImg);

      cv::Mat matOut = dc::BlurFilter::blur(matIn, method, intensity);
      assert(matIn.type() == matOut.type());

      QImage res = Convertor::getQImage(matOut);

      return res;
    }

    QImage
    blur(const QImage &originalImg,
	 Method method,
	 int intensity,
	 Function function,
	 Area area,
	 float coeff,
	 int vertical,
	 int horizontal,
	 int radius)
    {
      cv::Mat matIn = Convertor::getCvMat(originalImg);

      cv::Mat matOut = dc::BlurFilter::blur(matIn,
					    method,
					    intensity,
					    function,
					    area,
					    coeff,
					    vertical,
					    horizontal,
					    radius);
      assert(matIn.type() == matOut.type());

      QImage res = Convertor::getQImage(matOut);

      return res;
    }

    float
    getRadiusFourier(const QImage &original)
    {
      return dc::BlurFilter::getRadiusFourier(Convertor::getCvMat(original));
    }
  
    int
    searchFitFourier(const QImage &original, float radiusExample)
    {
      return dc::BlurFilter::searchFitFourier(Convertor::getCvMat(original), radiusExample);
    }

  } //namespace BlurFilter
    
} //namespace dc
