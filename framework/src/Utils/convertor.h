#ifndef CONVERTOR_H
#define CONVERTOR_H

#include <framework_global.h>
#include <opencv2/core/core.hpp>

#include <QImage>

class FRAMEWORK_EXPORT Convertor
{
public:
  /*
    @return @a image as a cv::Mat of type CV_8UC3, with BGR channel order.
  */
  static cv::Mat getCvMat(const QImage &image);

  /*
    @return @a image as a QImage of format Format_ARGB32.

    @a image must be of type CV_8UC1, CV_8UC3, CV_8UC4, CV_32FC1 or
    CV_32FC3 (the float values are expected in [0.f; 255.f], and are
    saturated). A null QImage is returned for any other type, and for an
    empty image.
  */
  static QImage getQImage(const cv::Mat &image);
  static cv::Mat binarizeOTSU(const cv::Mat &image);
};

#endif // CONVERTOR_H
