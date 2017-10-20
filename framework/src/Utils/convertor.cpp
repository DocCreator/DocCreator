#include "convertor.h"

#include <cassert>
#include <iostream>

#include <opencv2/imgproc/imgproc.hpp>

cv::Mat
Convertor::getCvMat(const QImage &image)
{
  //B:TODO:OPTIM: avoid conversion to QImage::Format_ARGB32 if in an other format !

  //QImage qimage = image.convertToFormat(QImage::Format_ARGB32);
  QImage qimage = image;
  if (image.format() != QImage::Format_ARGB32 &&
      image.format() != QImage::Format_RGB32)
    qimage = image.convertToFormat(QImage::Format_ARGB32);

  cv::Mat mat = cv::Mat(qimage.height(),
                        qimage.width(),
                        CV_8UC4,
                        (void *)qimage.constBits(),
                        qimage.bytesPerLine());
  cv::Mat mat2 = cv::Mat(mat.rows, mat.cols, CV_8UC3);
  int from_to[] = { 0, 0, 1, 1, 2, 2 };
  cv::mixChannels(&mat, 1, &mat2, 1, from_to, 3);

  return mat2;
}

cv::Mat
Convertor::binarizeOTSU(const cv::Mat &image)
{
  assert(image.type() == CV_8UC1);

  cv::Mat img_binarization;
  cv::threshold(image, img_binarization, 0, 255, cv::THRESH_OTSU); //B
  return img_binarization;
}

/*
QImage Convertor::pixToQImage(PIX *pixImage){
      int width = pixGetWidth(pixImage);
      int height = pixGetHeight(pixImage);
      int depth = pixGetDepth(pixImage);
      int bytesPerLine = pixGetWpl(pixImage) * 4;
      l_uint32 * s_data = pixGetData(pixEndianByteSwapNew(pixImage));

      QImage::Format format;
      if (depth == 1)
        format = QImage::Format_Mono;
      else if (depth == 8)
        format = QImage::Format_Indexed8;
      else
        format = QImage::Format_RGB32;

      QImage result((uchar*)s_data, width, height, bytesPerLine, format);

      // Handle palette
      QVector<QRgb> _bwCT;
      _bwCT.append(qRgb(255,255,255));
      _bwCT.append(qRgb(0,0,0));

      QVector<QRgb> _grayscaleCT(256);
      for (int i = 0; i < 256; i++)  {
        _grayscaleCT.append(qRgb(i, i, i));
      }
      if (depth == 1) {
        result.setColorTable(_bwCT);
      }  else if (depth == 8)  {
        result.setColorTable(_grayscaleCT);

      } else {
        result.setColorTable(_grayscaleCT);
      }

      if (result.isNull()) {
        static QImage none(0,0,QImage::Format_Invalid);
        //qDebug() << "***Invalid format!!!";
        return none;
      }

      return result.rgbSwapped();

}

*/

QImage
Convertor::getQImage(const cv::Mat &image)
{

  cv::Mat rgb;
  if (image.channels() == 1)
    cv::cvtColor(image, rgb, cv::COLOR_GRAY2RGB);
  else
    rgb = image;
  assert(rgb.channels() == 3 || rgb.channels() == 4);

  const int h = rgb.rows;
  const int w = rgb.cols;

  QImage qimg(w, h, QImage::Format_ARGB32);

  if (rgb.type() == CV_32FC3) {

    for (int i = 0; i < h; ++i) {
      const float *image = rgb.ptr<float>(i);
      QRgb *dst = (QRgb *)qimg.scanLine(i);
      for (int j = 0; j < w; ++j) {
        //suppose image in BGR format
        uchar b = *image;
        ++image;
        uchar g = *image;
        ++image;
        uchar r = *image;
        ++image;
        //Are values in image in [0.f; 225.f] or [0.f; 1.f] ???

        *dst = qRgb(r, g, b);
        ++dst;
      }
    }

  } else if (rgb.type() == CV_8UC3) {

    for (int i = 0; i < h; ++i) {
      const uchar *image = rgb.ptr<uchar>(i);
      QRgb *dst = (QRgb *)qimg.scanLine(i);
      for (int j = 0; j < w; ++j) {
        //suppose image in BGR format
        uchar b = *image;
        ++image;
        uchar g = *image;
        ++image;
        uchar r = *image;
        ++image;
        //shoud we cv::saturate_cast() ???

        *dst = qRgb(r, g, b);
        ++dst;
      }
    }

  } else if (rgb.type() == CV_8UC4) //For transparency
  {

    //QImage alphaImg = qimg.alphaChannel();

    for (int i = 0; i < h; ++i) {
      const uchar *image = rgb.ptr<uchar>(i);
      QRgb *dst = (QRgb *)qimg.scanLine(i);
      for (int j = 0; j < w; ++j) {
        //suppose image in BGR format
        uchar b = *image;
        ++image;
        uchar g = *image;
        ++image;
        uchar r = *image;
        ++image;
        uchar a = *image;
        ++image;
        //shoud we cv::saturate_cast() ???

        *dst = qRgba(r, g, b, a);
        ++dst;
      }
    }

  } else {
    std::cerr << "ERROR: Convertor::getQImage() : unknown type \n";
    assert(false);
  }

  return qimg;
}
