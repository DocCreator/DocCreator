#include "convertor.h"

#include <cassert>
#include <iostream>

#include <opencv2/imgproc/imgproc.hpp>

/*
  Output image is of type CV_8UC3. It is with BGR channel order.
 */
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
  assert(image.type() == CV_8UC1 || image.type() == CV_8UC3);

  cv::Mat binary_img;

  cv::Mat gray_img;
  if (image.channels() == 3) {
    cv::cvtColor(image, gray_img, cv::COLOR_BGR2GRAY);
  }
  else {
    gray_img = image;
  }

  cv::threshold(gray_img, binary_img, 0, 255, cv::THRESH_OTSU);

  return binary_img;
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
  if (image.empty()) {
    return QImage();
  }

  /*
    B: an image of a type none of the branches below handles used to be
    returned as it was built, that is with whatever was on the heap: the
    GUI then displayed it, and could save it to disk. Tell the caller
    instead. The type has to be checked before cvtColor(), which throws for
    a depth it does not handle (CV_32S, CV_64F).
  */
  const int type = image.type();
  if (type != CV_8UC1 && type != CV_8UC3 && type != CV_8UC4
      && type != CV_32FC1 && type != CV_32FC3) {
    std::cerr << "ERROR: Convertor::getQImage(): unhandled image type: "
	      << image.channels() << " channel(s), depth " << image.depth()
	      << "\n";
    return QImage();
  }

  cv::Mat rgb;
  if (image.channels() == 1) {
    cv::cvtColor(image, rgb, cv::COLOR_GRAY2RGB);
  }
  else {
    rgb = image;
  }
  assert(rgb.type() == CV_8UC3 || rgb.type() == CV_8UC4
	 || rgb.type() == CV_32FC3);

  const int h = rgb.rows;
  const int w = rgb.cols;

  QImage qimg(w, h, QImage::Format_ARGB32);
  if (qimg.isNull()) {
    std::cerr << "ERROR: Convertor::getQImage(): unable to allocate a "
	      << w << "x" << h << " image\n";
    return qimg;
  }

  if (rgb.type() == CV_32FC3) {

    for (int i = 0; i < h; ++i) {
      const float *img = rgb.ptr<float>(i);
      QRgb *dst = (QRgb *)qimg.scanLine(i);
      for (int j = 0; j < w; ++j) {
        //suppose image in BGR format, with values in [0.f; 255.f]
        //B: the conversion of a float outside [0; 256) to an integral type
        // is undefined behaviour: saturate.
        uchar b = cv::saturate_cast<uchar>(*img);
        ++img;
        uchar g = cv::saturate_cast<uchar>(*img);
        ++img;
        uchar r = cv::saturate_cast<uchar>(*img);
        ++img;

        *dst = qRgb(r, g, b);
        ++dst;
      }
    }

  }
  else if (rgb.type() == CV_8UC3) {

    for (int i = 0; i < h; ++i) {
      const uchar *img = rgb.ptr<uchar>(i);
      QRgb *dst = (QRgb *)qimg.scanLine(i);
      for (int j = 0; j < w; ++j) {
        //suppose image in BGR format
        uchar b = *img;
        ++img;
        uchar g = *img;
        ++img;
        uchar r = *img;
        ++img;
        *dst = qRgb(r, g, b);
        ++dst;
      }
    }

  }
  else if (rgb.type() == CV_8UC4) //For transparency
  {

    //QImage alphaImg = qimg.alphaChannel();

    for (int i = 0; i < h; ++i) {
      const uchar *img = rgb.ptr<uchar>(i);
      QRgb *dst = (QRgb *)qimg.scanLine(i);
      for (int j = 0; j < w; ++j) {
        //suppose image in BGR format
        uchar b = *img;
        ++img;
        uchar g = *img;
        ++img;
        uchar r = *img;
        ++img;
        uchar a = *img;
        ++img;
        *dst = qRgba(r, g, b, a);
        ++dst;
      }
    }

  }
  else {
    assert(false); //the type was checked above
  }

  return qimg;
}
