#define _USE_MATH_DEFINES //for M_PI with Visual

#include "ShadowBinding.hpp"

#include <cassert>
#include <cmath> //cos, M_PI
//#include <iostream>//DEBUG

#include "Utils/convertor.h"
#include <opencv2/imgproc/imgproc.hpp>

/**
   Darken rectangle @a rect of image @a img with intensity @a intensity.
   
   @param[in, out] img
   
*/
static void
darkenImagePart(cv::Mat &img, const cv::Rect &rect, float intensity)
{
  assert(img.type() == CV_8UC3 || img.type() == CV_8UC1);
  assert(rect.x + rect.width <= img.cols);
  assert(rect.y + rect.height <= img.rows);

  const float coeff = intensity;

  const int x0 = rect.x;
  const int x1 = rect.x + rect.width;
  const int y0 = rect.y;
  const int y1 = rect.y + rect.height;

  if (img.type() == CV_8UC3) {
    for (int y = y0; y < y1; ++y) {
      cv::Vec3b *p = img.ptr<cv::Vec3b>(y) + x0;
      for (int x = x0; x < x1; ++x) {
        (*p)[0] = cv::saturate_cast<uchar>((*p)[0] * coeff);
        (*p)[1] = cv::saturate_cast<uchar>((*p)[1] * coeff);
        (*p)[2] = cv::saturate_cast<uchar>((*p)[2] * coeff);
        ++p;
      }
    }
  } else {
    assert(img.type() == CV_8UC1);
    for (int y = y0; y < y1; ++y) {
      uchar *p = img.ptr<uchar>(y) + x0;
      for (int x = x0; x < x1; ++x) {
        (*p) = cv::saturate_cast<uchar>((*p) * coeff);
        ++p;
      }
    }
  }
}

/**
   @param[in, out] matOut.
 */
void
shadowBinding(cv::Mat &matOut,
              ShadowBorder border,
              int distance,
              float intensity,
              float angle)
{
  cv::Rect rec;

  const float theta = angle * M_PI / 180.f;
  const float radius = distance;

  assert(fabs(intensity) <= 1.f);

  const float l0 = intensity * 100 + 1;

  for (int i = 0; i < distance; ++i) {
    const float step = 1;
    switch (border) {
      case ShadowBorder::LEFT:
        rec = cv::Rect(i, 0, step, matOut.rows);
        break;
      case ShadowBorder::TOP:
        rec = cv::Rect(0, i, matOut.cols, step);
        break;
      case ShadowBorder::RIGHT:
        rec = cv::Rect(matOut.cols - 1 - i, 0, step, matOut.rows);
        break;
      case ShadowBorder::BOTTOM:
      default:
        rec = cv::Rect(0, matOut.rows - 1 - i, matOut.cols, step);
        break;
    }

    //transform i from [0; distance[ to [theta; 0[
    const float phi = theta * (1 - i / static_cast<float>(distance));
    const float c = (l0 / (l0 + radius * (1 - cos(phi))));
    const float coeff = c * c;

    //std::cerr<<"distance="<<distance<<" intensity="<<intensity<<" angle="<<angle<<" i="<<i<<" => c="<<coeff<<"\n";

    darkenImagePart(matOut, rec, coeff);
  }
}

static inline int
getDistance(ShadowBorder border, float distanceRatio, int w, int h)
{
  return (border == ShadowBorder::LEFT || border == ShadowBorder::RIGHT)
           ? distanceRatio * w
           : distanceRatio * h;
}

/**
   @param[in, out] matOut.
 */
void
shadowBinding(cv::Mat &matOut,
              float distanceRatio,
              ShadowBorder border,
              float intensity,
              float angle)
{
  const int distance =
    getDistance(border, distanceRatio, matOut.cols, matOut.rows);

  shadowBinding(matOut, border, distance, intensity, angle);
}

QImage
shadowBinding(const QImage &imgOriginal,
              ShadowBorder border,
              int distance,
              float intensity,
              float angle)
{
  cv::Mat matOut = Convertor::getCvMat(imgOriginal);

  shadowBinding(matOut, border, distance, intensity, angle);

  const QImage out = Convertor::getQImage(matOut);

  return out;
}

QImage
shadowBinding(const QImage &imgOriginal,
              float distanceRatio,
              ShadowBorder border,
              float intensity,
              float angle)
{
  const int distance = getDistance(
    border, distanceRatio, imgOriginal.width(), imgOriginal.height());

  return shadowBinding(imgOriginal, border, distance, intensity, angle);
}

ShadowBinding::ShadowBinding(const QImage &original,
                             float distanceRatio,
                             ShadowBorder border,
                             float intensity,
                             float angle,
                             QObject *parent)
  : DocumentDegradation(parent)
  , _border(border)
  , _distance(10)
  , _intensity(intensity)
  , _angle(angle)
  , _original(original)
{
  _distance =
    getDistance(border, distanceRatio, original.width(), original.height());
}

QImage
ShadowBinding::apply()
{
  const QImage out =
    shadowBinding(_original, _border, _distance, _intensity, _angle);

  emit imageReady(out);

  return out;
}
