#include "HoleDegradation.hpp"

#include <cassert>
#include <cmath> //cos, sin

#include "Utils/convertor.h"
#include <opencv2/imgproc/imgproc.hpp>

static const int INTENSITY_WHITE = 255;
static const cv::Vec3b PIXEL_BLACK = cv::Vec3b(0, 0, 0);

static cv::Mat
changeBorderPattern(const cv::Mat &pattern, Border side)
{
  //Original border patterns are on TOP side
  if (side == Border::TOP)
    return pattern;

  //TODO:OPTIM: use CV_8UC1 for patterns !

  int rows = pattern.rows;
  int cols = pattern.cols;
  if (side != Border::BOTTOM)
    std::swap(rows, cols);

  cv::Mat changedMat(rows, cols, CV_8UC3);

  if (side == Border::RIGHT) {
    for (int y = 0; y < rows; ++y) {
      cv::Vec3b *d = changedMat.ptr<cv::Vec3b>(y);
      for (int x = 0; x < cols; ++x) {
        assert(cols - 1 - x < pattern.rows && y < pattern.cols);
        d[x] = pattern.at<cv::Vec3b>(cols - 1 - x, y);
      }
    }
  } else if (side == Border::BOTTOM) {
    for (int y = 0; y < rows; ++y) {
      const cv::Vec3b *p = pattern.ptr<cv::Vec3b>(rows - 1 - y);
      cv::Vec3b *d = changedMat.ptr<cv::Vec3b>(y);
      for (int x = 0; x < cols; ++x)
        d[x] = p[x];
    }
  } else if (side == Border::LEFT) {
    for (int y = 0; y < rows; ++y) {
      cv::Vec3b *d = changedMat.ptr<cv::Vec3b>(y);
      for (int x = 0; x < cols; ++x)
        d[x] = pattern.at<cv::Vec3b>(x, y);
    }
  }

  return changedMat;
}

static cv::Mat
changeCornerPattern(const cv::Mat &pattern, Corner side)
{
  //Original corner patterns are on TOPLEFT side
  if (side == Corner::TOPLEFT)
    return pattern;

  //TODO:OPTIM: use CV_8UC1 for patterns !

  cv::Mat changedMat(pattern.rows, pattern.cols, CV_8UC3);

  const int rows = pattern.rows;
  const int cols = pattern.cols;

  if (side == Corner::TOPRIGHT) {
    for (int y = 0; y < rows; ++y) {
      const cv::Vec3b *p = pattern.ptr<cv::Vec3b>(y);
      cv::Vec3b *d = changedMat.ptr<cv::Vec3b>(y);
      for (int x = 0; x < cols; ++x)
        d[x] = p[cols - 1 - x];
    }
  } else if (side == Corner::BOTTOMRIGHT) {
    for (int y = 0; y < rows; ++y) {
      const cv::Vec3b *p = pattern.ptr<cv::Vec3b>(rows - 1 - y);
      cv::Vec3b *d = changedMat.ptr<cv::Vec3b>(y);
      for (int x = 0; x < cols; ++x)
        d[x] = p[cols - 1 - x];
    }
  } else if (side == Corner::BOTTOMLEFT) {
    for (int y = 0; y < rows; ++y) {
      const cv::Vec3b *p = pattern.ptr<cv::Vec3b>(rows - 1 - y);
      cv::Vec3b *d = changedMat.ptr<cv::Vec3b>(y);
      for (int x = 0; x < cols; ++x)
        d[x] = p[x];
    }
  }

  return changedMat;
}

/*
  @param[in] intensityColor
  @param[in] coeff
  @param[out] blue
  @param[out] green
  @param[out] red
 */
static void
updateShadowRGB(cv::Vec4b intensityColor,
                float coeff,
                unsigned char &blue,
                unsigned char &green,
                unsigned char &red)
{

  blue = cv::saturate_cast<unsigned char>(intensityColor.val[0] * coeff);
  green = cv::saturate_cast<unsigned char>(intensityColor.val[1] * coeff);
  red = cv::saturate_cast<unsigned char>(intensityColor.val[2] * coeff);
}

static bool
isInMarge(const cv::Mat &matPattern, int x, int y, int width)
{
  assert(x >= 0 && x < matPattern.cols);
  assert(y >= 0 && y < matPattern.rows);

  if (matPattern.at<cv::Vec3b>(y, x) ==
      PIXEL_BLACK) { //not in marge if it's not in black pattern

    for (int i = 0; i < width; ++i) {
      if (x - i > 0 && matPattern.at<cv::Vec3b>(y, x - i) != PIXEL_BLACK)
        return true;
      if (x + i < matPattern.cols &&
          matPattern.at<cv::Vec3b>(y, x + i) != PIXEL_BLACK)
        return true;
      if (y - i > 0 && matPattern.at<cv::Vec3b>(y - i, x) != PIXEL_BLACK)
        return true;
      if (y + i < matPattern.rows &&
          matPattern.at<cv::Vec3b>(y + i, x) != PIXEL_BLACK)
        return true;
    }
  }

  return false;
}

static void
drawBorder(cv::Mat &matOut,
           const cv::Mat &matPattern,
           int x,
           int y,
           int xOrigin,
           int yOrigin,
           int width,
           float intensity)
{
  const float x1 = matOut.cols - 1 * cos(-1) - matOut.rows - 1 * sin(-1);
  float z1 = 3 * fabs(x1) / 30 + intensity;
  if (z1 == 0) {
    z1 = 1;
  }
  const float coeff = intensity * intensity / (z1 * z1);

  if (isInMarge(matPattern, x, y, width)) {
    const cv::Vec4b intensityColor =
      matOut.at<cv::Vec4b>(y + yOrigin, x + xOrigin);

    unsigned char blue, green, red;
    updateShadowRGB(intensityColor, coeff, blue, green, red);
    const unsigned char alpha = intensityColor.val[3];

    matOut.at<cv::Vec4b>(y + yOrigin, x + xOrigin) =
      cv::Vec4b(blue, green, red, alpha);
  }
}

QImage
HoleDegradation::apply()
{
  QImage finalImg = holeDegradation(_original,
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

/**
   make a CV_8UC4 image from a CV_8UC3 image

   output image will be opage (all alpha channel is set to INTENSITY_WHITE)
*/
static cv::Mat
makeFourChanImage(const cv::Mat &src)
{
  assert(src.type() == CV_8UC3);

  cv::Mat dst = cv::Mat(
    src.rows, src.cols, CV_8UC4); //convert 3 channels mat in 4 channels mat

  int rows = src.rows;
  int cols = src.cols;
  if (src.isContinuous() && dst.isContinuous()) {
    cols *= rows;
    rows = 1;
  }
  for (int y = 0; y < rows; ++y) {
    const cv::Vec3b *s = src.ptr<cv::Vec3b>(y);
    cv::Vec4b *d = dst.ptr<cv::Vec4b>(y);
    for (int x = 0; x < cols; ++x) {
      const cv::Vec3b v = s[x];
      d[x] = cv::Vec4b(v[0], v[1], v[2], INTENSITY_WHITE);
    }
  }

  return dst;
}

cv::Mat
holeDegradation(const cv::Mat &matOriginal,
                const cv::Mat &pattern,
                int xOrigin,
                int yOrigin,
                int size,
                HoleType type,
                int side,
                const QColor &color,
                const cv::Mat &matBelow,
                int width,
                float intensity)
{
  assert(matOriginal.type() == CV_8UC3);

  cv::Mat matOut = makeFourChanImage(matOriginal);

  cv::Mat matPattern = pattern;
  if (type == HoleType::BORDER && side > 0)
    matPattern = changeBorderPattern(matPattern, (Border)side);
  else if (type == HoleType::CORNER && side > 0)
    matPattern = changeCornerPattern(matPattern, (Corner)side);

  if (size != 0 && matPattern.cols + size > 0 && matPattern.rows + size > 0)
    resize(matPattern,
           matPattern,
           cv::Size(matPattern.cols + size, matPattern.rows + size));

  //B:TODO:OPTIM???

  int x0 = std::max(xOrigin, 0) - xOrigin;
  int y0 = std::max(yOrigin, 0) - yOrigin;
  int x1 = std::min(xOrigin + matPattern.cols, matOut.cols) - xOrigin;
  int y1 = std::min(yOrigin + matPattern.rows, matOut.rows) - yOrigin;

  //std::cerr<<"holeDegradation x0="<<x0<<" x1="<<x1<<"  y0="<<y0<<" y1="<<y1<<"\n";

  const cv::Vec4b color4(
    color.blue(), color.green(), color.red(), color.alpha());
  const cv::Vec4b white4(
    255, 255, 255, 255); //B:TODO: pass the below color as parameter

  if (matBelow.empty()) { //without below image

    for (int y = y0; y < y1; ++y) {
      const cv::Vec3b *p = matPattern.ptr<cv::Vec3b>(y);
      cv::Vec4b *d = matOut.ptr<cv::Vec4b>(y + yOrigin);
      for (int x = x0; x < x1; ++x) {
        assert(x + xOrigin >= 0 && x + xOrigin < matOut.cols &&
               y + yOrigin >= 0 && y + yOrigin < matOut.rows);
        if (p[x] == PIXEL_BLACK)
          d[x + xOrigin] = color4;
      }
    }

  } else { //with below image

    for (int y = y0; y < y1; ++y) {
      const cv::Vec3b *p = matPattern.ptr<cv::Vec3b>(y);
      cv::Vec4b *d = matOut.ptr<cv::Vec4b>(y + yOrigin);
      const cv::Vec3b *b = nullptr;
      if (y + yOrigin >= 0 && y + yOrigin < matBelow.rows)
        b = matBelow.ptr<cv::Vec3b>(y + yOrigin);
      for (int x = x0; x < x1; ++x) {
        assert(x + xOrigin >= 0 && x + xOrigin < matOut.cols &&
               y + yOrigin >= 0 && y + yOrigin < matOut.rows);
        if (p[x] == PIXEL_BLACK) {
          if (b != nullptr && x + xOrigin >= 0 && x + xOrigin < matBelow.cols) {
            const cv::Vec3b below = b[x + xOrigin];
            d[x + xOrigin] = cv::Vec4b(below[0], below[1], below[2], 255);
          } else { //not over below image
            d[x + xOrigin] =
              white4; //if page below is not below this pixel because of its size, we draw a white pixel to represent what we can see when it arrive with scanner
          }

          drawBorder(
            matOut, matPattern, x, y, xOrigin, yOrigin, width, intensity);
        }
      }
    }
  }

  return matOut;
}

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
  cv::Mat matPattern = Convertor::getCvMat(pattern);
  cv::Mat matBelow;
  if (!pageBelow.isNull())
    matBelow = Convertor::getCvMat(pageBelow);

  cv::Mat matOut = holeDegradation(matOriginal,
                                   matPattern,
                                   xOrigin,
                                   yOrigin,
                                   size,
                                   type,
                                   side,
                                   color,
                                   matBelow,
                                   width,
                                   intensity);

  return Convertor::getQImage(matOut);
}
