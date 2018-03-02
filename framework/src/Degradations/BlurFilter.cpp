#include "BlurFilter.hpp"

#include <cassert>
#include <cmath> //sqrt, log

#include "Utils/convertor.h"
#include <opencv2/imgproc/imgproc.hpp>

static const int COEFF_PARABOLA = 4;
static const cv::Vec3b WHITE_PIXEL = cv::Vec3b(255, 255, 255);
static const cv::Vec3b BLACK_PIXEL = cv::Vec3b(0, 0, 0);

void
BlurFilter::calcSolutions(float a, float b, float discr, float &y1, float &y2)
{
  y1 = (-b + sqrt(discr)) / (2 * a);
  y2 = (-b - sqrt(discr)) / (2 * a);
}

static int
fctLinear(int x, int rows, int coeff, int vertical)
{
  return rows - (x * coeff + vertical);
}

static int
fctLog(int x, int rows, int coeff, int vertical)
{
  return rows - (log(x) * coeff + vertical);
}

static int
fctParabola(int x, int rows, int coeff, int vertical, int horizontal)
{
  return rows - ((coeff * (x + horizontal) * (x + horizontal)) +
                 (COEFF_PARABOLA * (x + horizontal)) + vertical);
}

static int
fctSinus(int x, int rows, int coeff, int vertical, int horizontal)
{
  return rows - (coeff * sin((x + horizontal) / 200.f) + vertical);
}

static void
fctEllipse(int x,
           int horizontal,
           int vertical,
           float coeff,
           float &y1,
           float &y2)
{
  //Pattern : 0.2*x*x + 0.5*y*y + 0.27*x*y - 330.71*x - 554.29*y = -170351.99
  //others numbers are here to ajust the coefficient chosen to Resize the ellipse. (For example, the variable y is multiplied by 0.1 when the size is increased by one). These coefficients were computed for this pattern thanks to Geogebra
  const float a = 0.5;
  const float b = (-554.29 - (coeff * 0.1)) + (0.27 * (x + horizontal));
  const float c = (170351.99 - (coeff * 34.61)) +
                  (0.2 * (x + horizontal) * (x + horizontal)) -
                  ((330.71 - (coeff * 0.03)) * (x + horizontal));
  const float discr = b * b - (4 * a * c);

  if (discr > 0) {
    BlurFilter::calcSolutions(a, b, discr, y1, y2);
    y1 += vertical;
    y2 += vertical;
  } else if (discr == 0) {
    y1 = -b / (2 * a);
    y1 += vertical;
    y2 = -FLT_MAX;
  } else {
    y1 = -FLT_MAX;
  }
}

static void
fctHyperbola(int x,
             int rows,
             int horizontal,
             int vertical,
             float coeff,
             float &y1,
             float &y2)
{
  //Pattern : 0.00801*x*x - 0.03214*y*y + 0.1146*x*y - 123.95975*x - 17.44726*y = -81903.06188
  //others numbers are here to ajust the coefficient chosen to Resize the hyperbola. (For example, the variable x*x is multiplied by 0.00012 when the size is increase by one). These coefficient were computed for this pattern thanks to Geogebra
  const float a = -0.03214 - (coeff * 0.00013);
  const float b = (0.1146 * (x + horizontal)) + (-17.44726 + (coeff * 0.24361));
  const float c =
    (81903.06188 + (coeff * 206.34517)) +
    ((0.00801 - (coeff * 0.00012)) * (x + horizontal) * (x + horizontal)) -
    ((123.95975 + (coeff * 0.17438)) * (x + horizontal));
  const float discr = b * b - (4 * a * c);
  if (discr > 0) {
    BlurFilter::calcSolutions(a, b, discr, y1, y2);
    y1 = rows - (y1 + vertical);
    y2 = rows - (y2 + vertical);
  } else if (discr == 0) {
    y1 = -b / (2 * a);
    y1 = rows - (y1 + vertical);
    y2 = -FLT_MAX;
  }
}

void
BlurFilter::calculFunction(Function fct,
                           int rows,
                           int x,
                           int y,
                           int vertical,
                           int horizontal,
                           float coeff,
                           int &yFunction,
                           int &y2Function)
{

  switch (fct) {
    case Function::LINEAR:
      yFunction = fctLinear(x, rows, coeff, vertical);
      break;

    case Function::LOG:
      yFunction = fctLog(x, rows, coeff, vertical);
      break;

    case Function::PARABOLA:
      yFunction = fctParabola(x, rows, coeff, vertical, horizontal);
      break;

    case Function::SINUS:
      yFunction = fctSinus(x, rows, coeff, vertical, horizontal);
      break;

    case Function::ELLIPSE: {
      float y1 = 0, y2 = 0;
      fctEllipse(x, horizontal, vertical, coeff, y1, y2);
      yFunction = (y != -FLT_MAX ? static_cast<int>(y1)
                                 : -1); //To be sure we are in ellipse
      y2Function =
        (y2 != -FLT_MAX ? static_cast<int>(y2)
                        : -1); //To know if their are just one solution

      if (yFunction < 0 || yFunction > rows)
        y2Function = -1;
    } break;

    case Function::HYPERBOLA: {
      float y1 = 0, y2 = 0;
      fctHyperbola(x, rows, horizontal, vertical, coeff, y1, y2);
      yFunction = static_cast<int>(y1);
      y2Function = (y2 != -FLT_MAX ? static_cast<int>(y2) : -1);
    } break;
  }
}

bool
BlurFilter::isNearFunction(int x,
                           int y,
                           int rows,
                           Function fct,
                           int horizontal,
                           int vertical,
                           float coeff,
                           int radius)
{
  int yFunction, y2Function = -1;

  calculFunction(
    fct, rows, x, y, vertical, horizontal, coeff, yFunction, y2Function);

  if (yFunction != -1) {
    if (yFunction - radius <= y && y <= yFunction + radius && y2Function != 1)
      return true;
    if (y2Function != -1 && y2Function - radius <= y &&
        y <= y2Function + radius)
      return true;
  }

  return false;
}

static cv::Mat
degradateCenter(const cv::Mat &blurredMat,
                cv::Mat &resultMat,
                Function function,
                int horizontal,
                int vertical,
                float coeff,
                int radius)
{
  const int rows = blurredMat.rows;
  const int cols = blurredMat.cols;

  assert(resultMat.type() == CV_8UC3);
  assert(blurredMat.type() == CV_8UC3);
  assert(blurredMat.size() == resultMat.size());

  for (int y = 0; y < rows; ++y) {
    const cv::Vec3b *b = blurredMat.ptr<cv::Vec3b>(y);
    cv::Vec3b *r = resultMat.ptr<cv::Vec3b>(y);
    for (int x = 0; x < cols; ++x) {
      if (BlurFilter::isNearFunction(
            x, y, rows, function, horizontal, vertical, coeff, radius)) //Center
        r[x] = b[x];
      //else
      //r[x] = o[x];
    }
  }

  return resultMat;
}

bool
BlurFilter::upperThan(Function fct,
                      int rows,
                      int x,
                      int y,
                      int vertical,
                      int horizontal,
                      float coeff)
{
  int yFunction, y2Function = -1;
  calculFunction(
    fct, rows, x, y, vertical, horizontal, coeff, yFunction, y2Function);

  if (fct == Function::ELLIPSE || fct == Function::HYPERBOLA) {
    if (y2Function == -1) { //Mean there are juste one solution
      if (y == yFunction)
        return true;
    } else { //Two solutions
      if (y < yFunction && y > y2Function)
        return true;
    }
  } else if (y > yFunction) {
    return true;
  }

  return false;
}

static cv::Mat
degradateBorder(const cv::Mat &blurredMat,
                cv::Mat &resultMat,
                Function function,
                Area area,
                int vertical,
                float coeff,
                int horizontal)
{
  //TODO : Make it work with grayscale image too ?
  assert(blurredMat.type() == CV_8UC3);
  assert(resultMat.type() == CV_8UC3);
  assert(blurredMat.size() == resultMat.size());

  const int rows = resultMat.rows;
  const int cols = resultMat.cols;

  if (area == Area::UP) {
    for (int y = 0; y < rows; ++y) {
      const cv::Vec3b *b = blurredMat.ptr<cv::Vec3b>(y);
      cv::Vec3b *r = resultMat.ptr<cv::Vec3b>(y);
      for (int x = 0; x < cols; ++x) {
        if (BlurFilter::upperThan(
              function, rows, x, y, vertical, horizontal, coeff))
          r[x] = b[x];
      }
    }
  } else if (area == Area::DOWN) {
    for (int y = 0; y < rows; ++y) {
      const cv::Vec3b *b = blurredMat.ptr<cv::Vec3b>(y);
      cv::Vec3b *r = resultMat.ptr<cv::Vec3b>(y);
      for (int x = 0; x < cols; ++x) {
        if (!BlurFilter::upperThan(
              function, rows, x, y, vertical, horizontal, coeff))
          r[x] = b[x];
      }
    }
  }
  /*
  else {
    assert(area == Area::CENTERED);

    //resultMat.at<cv::Vec3b>(y, x) = originalMat.at<cv::Vec3b>(y, x);
  }
  */

  return resultMat;
}

cv::Mat
applyPattern(const cv::Mat &originalMat,
             const cv::Mat &patternMat,
             Method method,
             int intensity)
{
  cv::Mat resultMat = originalMat.clone();

  cv::Mat blurredMat = blurFilter(originalMat, method, intensity);

  //resize pattern to the size of original
  resize(patternMat, patternMat, cv::Size(originalMat.cols, originalMat.rows));

  assert(patternMat.type() == CV_8UC3);
  assert(resultMat.type() == CV_8UC3);
  assert(blurredMat.type() == CV_8UC3);
  assert(patternMat.size() == originalMat.size());
  assert(blurredMat.size() == originalMat.size());
  assert(resultMat.size() == originalMat.size());

  int rows = resultMat.rows;
  int cols = resultMat.cols;
  if (patternMat.isContinuous() && resultMat.isContinuous() &&
      blurredMat.isContinuous()) {
    cols *= rows;
    rows = 1;
  }

  for (int y = 0; y < rows; ++y) {
    const cv::Vec3b *p = patternMat.ptr<cv::Vec3b>(y);
    const cv::Vec3b *b = blurredMat.ptr<cv::Vec3b>(y);
    cv::Vec3b *r = resultMat.ptr<cv::Vec3b>(y);

    for (int x = 0; x < cols; ++x) {
      if (p[x] == BLACK_PIXEL)
        r[x] = b[x];
    }
  }

  return resultMat;
}

QImage
applyPattern(const QImage &originalImg,
             const QImage &pattern,
             Method method,
             int intensity)
{
  cv::Mat originalMat = Convertor::getCvMat(originalImg);
  cv::Mat patternMat = Convertor::getCvMat(pattern);

  cv::Mat resultMat = applyPattern(originalMat, patternMat, method, intensity);

  return Convertor::getQImage(resultMat);
}

cv::Mat
makePattern(const cv::Mat &originalMat,
            Function function,
            Area area,
            float coeff,
            int vertical,
            int horizontal,
            int radius)
{
  cv::Mat resultMat =
    cv::Mat::zeros(originalMat.rows, originalMat.cols, originalMat.type());

  const int rows = originalMat.rows;
  const int cols = originalMat.cols;

  assert(resultMat.type() == CV_8UC3);

  if (area == Area::CENTERED) {
    for (int y = 0; y < rows; ++y) {
      cv::Vec3b *r = resultMat.ptr<cv::Vec3b>(y);
      for (int x = 0; x < cols; ++x) {
        if (!BlurFilter::isNearFunction(x,
                                        y,
                                        rows,
                                        function,
                                        horizontal,
                                        vertical,
                                        coeff,
                                        radius)) //Center
          r[x] = WHITE_PIXEL;
      }
    }
  } else if (area == Area::UP) {
    for (int y = 0; y < rows; ++y) {
      cv::Vec3b *r = resultMat.ptr<cv::Vec3b>(y);
      for (int x = 0; x < cols; ++x) {
        if (!BlurFilter::upperThan(
              function, rows, x, y, vertical, horizontal, coeff))
          r[x] = WHITE_PIXEL;
      }
    }
  } else {
    assert(area == Area::DOWN);
    for (int y = 0; y < rows; ++y) {
      cv::Vec3b *r = resultMat.ptr<cv::Vec3b>(y);
      for (int x = 0; x < cols; ++x) {
        if (BlurFilter::upperThan(
              function, rows, x, y, vertical, horizontal, coeff))
          r[x] = WHITE_PIXEL;
      }
    }
  }

  return resultMat;
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

  cv::Mat resultMat = makePattern(
    originalMat, function, area, coeff, vertical, horizontal, radius);

  return Convertor::getQImage(resultMat);
}

static cv::Mat
degradateArea(const cv::Mat &originalMat,
              const cv::Mat &blurredMat,
              Function function,
              Area area,
              float coeff,
              int vertical,
              int horizontal,
              int radius)
{
  cv::Mat resultMat = originalMat.clone();

  if (area == Area::CENTERED)
    degradateCenter(
      blurredMat, resultMat, function, horizontal, vertical, coeff, radius);
  else
    degradateBorder(
      blurredMat, resultMat, function, area, vertical, coeff, horizontal);

  return resultMat;
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
BlurFilter::apply()
{
  QImage finalImg;

  if (_mode == Mode::COMPLETE)
    finalImg = blurFilter(_original, _method, _intensity);
  else
    finalImg = applyPattern(_original, _pattern, _method, _intensity);

  emit imageReady(finalImg);

  return finalImg;
}

//Apply blur filter to whole image
cv::Mat
blurFilter(const cv::Mat &originalImg, Method method, int intensity)
{
  //B: (tested with OpenCV 2.4.12)
  //If intensity(=kSize) == 1 : the three functions do nothing
  //If intensity(=kSize) == 2 : GaussianBlur & medianBlur assert, but blur() seems ok

  cv::Mat matOut;

  switch (method) {
    case Method::GAUSSIAN:
      cv::GaussianBlur(
        originalImg, matOut, cv::Size(intensity, intensity), 0, 0);
      break;

    case Method::MEDIAN:
      cv::medianBlur(originalImg, matOut, intensity);
      break;

    case Method::NORMAL:
      cv::blur(originalImg, matOut, cv::Size(intensity, intensity));
      break;
  }

  return matOut;
}

cv::Mat
blurFilter(const cv::Mat &originalImg,
           Method method,
           int intensity,
           Function function,
           Area area,
           float coeff,
           int vertical,
           int horizontal,
           int radius)
{
  //B:TODO:OPTIM:
  // we apply the blur on the whole image
  // and then keep only the part in the computed pattern.
  // We should have a function to get the bounding box of the pattern
  // and apply the blur only in this bunding box...

  cv::Mat matOut = blurFilter(originalImg, method, intensity);
  return degradateArea(
    originalImg, matOut, function, area, coeff, vertical, horizontal, radius);
}

QImage
blurFilter(const QImage &originalImg, Method method, int intensity)
{
  cv::Mat matIn = Convertor::getCvMat(originalImg);

  cv::Mat matOut = blurFilter(matIn, method, intensity);

  QImage res = Convertor::getQImage(matOut);

  return res;
}

QImage
blurFilter(const QImage &originalImg,
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

  cv::Mat matOut = blurFilter(matIn,
                              method,
                              intensity,
                              function,
                              area,
                              coeff,
                              vertical,
                              horizontal,
                              radius);

  QImage res = Convertor::getQImage(matOut);

  return res;
}


/*
  Compute the Fourier transform of image to measure the blur
  Method taken from Emile Vinsonneau's thesis, section 3.4.2 (descriptor D5).
  Thesis here : http://www.theses.fr/s131479

  Method to get Fourier transform in OpenCV documentation :
  http://docs.opencv.org/doc/tutorials/core/discrete_fourier_transform/discrete_fourier_transform.html

*/
float
getRadiusFourier(const cv::Mat &originalMatColor)
{
  cv::Mat originalMat;
  cv::cvtColor(originalMatColor,
               originalMat,
               cv::COLOR_BGR2GRAY); //To get a grayscale image

  cv::Mat padded; //expand input image to optimal size
  int m = cv::getOptimalDFTSize(originalMat.rows);
  int n = cv::getOptimalDFTSize(originalMat.cols);
  copyMakeBorder(originalMat,
                 padded,
                 0,
                 m - originalMat.rows,
                 0,
                 n - originalMat.cols,
                 cv::BORDER_CONSTANT,
                 cv::Scalar::all(0));

  cv::Mat planes[] = { cv::Mat_<float>(padded),
                       cv::Mat::zeros(padded.size(), CV_32F) };
  cv::Mat complexI;
  cv::Mat_<float> tmpT = cv::Mat_<float>(padded);
  cv::Mat temp = cv::Mat::zeros(padded.size(), CV_32F);
  cv::merge(
    planes, 2, complexI); // Add to the expanded another plane with zeros
  cv::dft(complexI, complexI);

  cv::split(complexI, planes); // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
  cv::magnitude(planes[0], planes[1], planes[0]); // planes[0] = magnitude
  cv::Mat magI = planes[0];

  magI += cv::Scalar::all(1); // switch to logarithmic scale
  cv::log(magI, magI);

  magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));
  int cx = magI.cols / 2;
  int cy = magI.rows / 2;

  cv::Mat q0(magI,
             cv::Rect(0, 0, cx, cy)); // Top-Left - Create a ROI per quadrant
  cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));  // Top-Right
  cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));  // Bottom-Left
  cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

  cv::Mat tmp; // swap quadrants (Top-Left with Bottom-Right)
  q0.copyTo(tmp);
  q3.copyTo(q0);
  tmp.copyTo(q3);

  q1.copyTo(tmp); // swap quadrant (Top-Right with Bottom-Left)
  q2.copyTo(q1);
  tmp.copyTo(q2);

  cv::normalize(magI, magI, 0, 255, cv::NORM_MINMAX); // Transform the matrix
                                                      // with float values into
                                                      // a viewable image form
                                                      // (float between values 0
                                                      // and 1).
  cv::threshold(magI, magI, 127, 255, cv::THRESH_BINARY);
  int erosion_size = 1;
  cv::Mat element = cv::getStructuringElement(
    cv::MORPH_RECT,
    cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
    cv::Point(
      erosion_size,
      erosion_size)); //http://docs.opencv.org/doc/tutorials/imgproc/erosion_dilatation/erosion_dilatation.html

  // to compute the radius
  QImage tmpImg = Convertor::getQImage(
    magI); //To get a mat that can use cvtColor (not works if we clone directly the mat magI, [...]
  cv::Mat calcRayon = Convertor::getCvMat(
    tmpImg); //  [...] the conversion function make the operation we need)
  cv::cvtColor(calcRayon, calcRayon, cv::COLOR_BGR2GRAY);

  int pixelX = calcRayon.cols / 2;
  int pixelY = calcRayon.rows / 2;
  bool found = false;
  //start at the center of image, and search in diagonal the end of binarization
  while (!found && pixelX > 0 && pixelY > 0) {
    if (calcRayon.at<uchar>(pixelY, pixelX) == 0) {
      found = true;
    } else {
      --pixelX;
      --pixelY;
    }
  }

  //compute the diagonal
  return sqrt(
    (((calcRayon.rows / 2) - pixelY) * ((calcRayon.rows / 2) - pixelY)) +
    (((calcRayon.cols / 2) - pixelX) * ((calcRayon.cols / 2) - pixelX)));
}

float
getRadiusFourier(const QImage &original)
{
  return getRadiusFourier(Convertor::getCvMat(original));
}
  
static const int MIN_BLUR_FOURIER = 1;
static const int MAX_BLUR_FOURIER = 21;


/*
  Search the size of filter (represented here by intensity) that we have to apply to fit with the example selected.

  QImage original is the image on which we want to apply blur.
  radiusExample is the radius (computed by the fonction getRadiusFourier) of the example selected.
*/
int
searchFitFourier(const cv::Mat &original, float radiusExample)
{
  bool found = false;
  int intensity = 0;

  int intensityMin = MIN_BLUR_FOURIER, intensityMax = MAX_BLUR_FOURIER;

  while (!found && intensityMax - intensityMin >
                     2) //binary search to find intensity that we need
  {
    intensity = ((intensityMax + intensityMin) / 2);
    if (intensity % 2 == 0)
      --intensity; // to get an odd number

    float radius = getRadiusFourier(blurFilter(
      original,
      Method::GAUSSIAN,
      intensity)); // compute the radius of result image with actual intensity
    if (radius > radiusExample + INTERVAL_FOURIER) //not enough blur
      intensityMin = intensity;
    else if (radius < radiusExample - INTERVAL_FOURIER) // too blur
      intensityMax = intensity;
    else
      found = true;
  }

  return intensity;
}

int
searchFitFourier(const QImage &original, float radiusExample)
{
  return searchFitFourier(Convertor::getCvMat(original), radiusExample);
}
