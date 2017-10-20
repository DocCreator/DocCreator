#ifndef CONVERTOR_H
#define CONVERTOR_H

#include <framework_global.h>
#include <opencv2/core/core.hpp>

#include <QImage>

class FRAMEWORK_EXPORT Convertor
{
public:
  static cv::Mat getCvMat(const QImage &image);
  static QImage getQImage(const cv::Mat &image);
  static cv::Mat binarizeOTSU(const cv::Mat &image);
};

#endif // CONVERTOR_H
