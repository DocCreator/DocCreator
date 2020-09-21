#include "RotationDegradationQ.hpp"
#include "Utils/convertor.h"
#include <QColor>

namespace dc {

  namespace RotationDegradation {

    QImage rotateFillColor(const QImage &img,
			   float angle,
			   const QColor &color)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);

      const cv::Scalar color4(color.blue(), color.green(), color.red(), color.alpha());
      
      const cv::Mat matOut = dc::RotationDegradation::rotateFillColor(matIn, angle, color4);

      return Convertor::getQImage(matOut);
    }

    QImage rotateFillImage(const QImage &img,
			   float angle,
			   const QImage &backgroundImg)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);
      const cv::Mat matBack = Convertor::getCvMat(backgroundImg);
      
      const cv::Mat matOut = dc::RotationDegradation::rotateFillImage(matIn, angle, matBack);

      return Convertor::getQImage(matOut);
    }

    QImage rotateFillImage(const QImage &img,
			   float angle,
			   const QImage &backgroundImg,
			   int repeats)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);
      const cv::Mat matBack = Convertor::getCvMat(backgroundImg);
      
      const cv::Mat matOut = dc::RotationDegradation::rotateFillImage(matIn, angle, matBack, repeats);

      return Convertor::getQImage(matOut);
    }

    QImage rotateFillImage(const QImage &img,
			   float angle,
			   const std::vector<QImage> &backgroundImgs)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);
      std::vector<cv::Mat> matBacks;
      matBacks.reserve(backgroundImgs.size());
      for (const auto &qimg : backgroundImgs) {
	matBacks.push_back(Convertor::getCvMat(qimg));
      }
      assert(matBacks.size() == backgroundImgs.size());
      
      const cv::Mat matOut = dc::RotationDegradation::rotateFillImage(matIn, angle, matBacks);

      return Convertor::getQImage(matOut);
    }
    
    QImage rotateFillBorder(const QImage &img,
			    float angle,
			    BorderReplication borderMode)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);
      
      const cv::Mat matOut = dc::RotationDegradation::rotateFillBorder(matIn, angle, borderMode);

      return Convertor::getQImage(matOut);
    }

    QImage rotateFillInpaint1(const QImage &img,
			      float angle,
			      float inpaintingRatio)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);
      
      const cv::Mat matOut = dc::RotationDegradation::rotateFillInpaint1(matIn, angle, inpaintingRatio);

      return Convertor::getQImage(matOut);
    }

    QImage rotateFillInpaint2(const QImage &img,
			      float angle,
			      const QImage &backgroundImg)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);
      const cv::Mat matBack = Convertor::getCvMat(backgroundImg);
      
      const cv::Mat matOut = dc::RotationDegradation::rotateFillInpaint2(matIn, angle, matBack);

      return Convertor::getQImage(matOut);
    }

    QImage rotateFillInpaint3(const QImage &img,
			      float angle,
			      const QImage &backgroundImg)
    {
      const cv::Mat matIn = Convertor::getCvMat(img);
      const cv::Mat matBack = Convertor::getCvMat(backgroundImg);
      
      const cv::Mat matOut = dc::RotationDegradation::rotateFillInpaint3(matIn, angle, matBack);

      return Convertor::getQImage(matOut);
      
    }
    
    
  } //namespace ShadowBinding
  

} //namespace dc
