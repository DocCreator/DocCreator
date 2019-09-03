#include "GrayCharacterDegradationModelQ.hpp"

#include "Utils/convertor.h"


namespace dc {

GrayscaleCharsDegradationModelQ::GrayscaleCharsDegradationModelQ(const QImage &img) :
  _cdm(Convertor::getCvMat(img))
{

}

/*
void
GrayscaleCharsDegradationModelQ::initialize(const QImage &input)
{

}
*/

QImage
GrayscaleCharsDegradationModelQ::degradate(int level, float I, float O, float D)
{
  const cv::Mat img = _cdm.degradate_cv(level, I, O, D);
  return Convertor::getQImage(img);
}

QImage
GrayscaleCharsDegradationModelQ::degradateByLevel(int level)
{
  const cv::Mat img = _cdm.degradateByLevel_cv(level);
  return Convertor::getQImage(img);
}


} //namespace dc
