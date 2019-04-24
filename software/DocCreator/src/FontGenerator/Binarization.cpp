#include "Binarization.hpp"

#include <iostream>

#define uset(x, y, v) at<unsigned char>(y, x) = v;
#define fset(x, y, v) at<float>(y, x) = v;


void
Binarization::preProcess(const cv::Mat &src, cv::Mat &dst, int erosion)
{
  /* Estimates a text-free background which is subtracted from the original
   * image
   * to improve the characters visualisation in a given image
   * The erosion parameter represents the size of the dilation area */
  if (erosion == 0) {
    src.copyTo(dst);
    return;
  }

  const int blur = 3;
  // Naive noise removal
  medianBlur(src, dst, blur);

  // Background estimation
  cv::Mat background;
  applyDilation(dst, background, 1, erosion);
  applyErosion(background, background, 1, erosion);

  // Background subtraction
  cv::absdiff(dst, background, dst);
  cv::cvtColor(dst, dst, cv::COLOR_BGR2GRAY);
  cv::bitwise_not(dst, dst);
}
void
Binarization::postProcess(const cv::Mat &src, cv::Mat &dst, double v, int win)
{
  /* Removes unwanted noise and enhances pixels connectivity */
  shrinkFilter(src, dst, v, win);
  // single pixel artifacts
  //connectivityFilter(src, dst);
  //medianBlur(dst, dst, win);
}

void
Binarization::shrinkFilter(const cv::Mat &src, cv::Mat &dst, double v, int win)
{
  /* Counts the number of background pixels in a given window centered on a
   * foreground pixel
   * if this number is to high, the center pixel is considered as noise
   * and thus removed */
  const int ksh = (int)(v * win * win);
  src.copyTo(dst);
  for (int i = 0; i < src.rows; ++i)
    for (int j = 0; j < src.cols; ++j) {
      if (dst.at<uchar>(i, j) == 0) { // if foreground
        int curr_k = 0;
        for (int ii = -win / 2; ii < win / 2; ++ii)
          for (int jj = -win / 2; jj < win / 2; ++jj) {
            if (i + ii < 0 || i + ii >= src.rows || j + jj < 0 ||
                j + jj >= src.cols) // deal with borders
              continue;
            if (dst.at<uchar>(i + ii, j + jj) == 255)
              ++curr_k;
          }
        if (curr_k > ksh)
          dst.at<uchar>(i, j) = 255;
      }
    }
}

void
Binarization::connectivityFilter(const cv::Mat &src, cv::Mat &dst)
{
  /* We check the symetric sides of any foreground pixel, 
   * if they belong to the same class,
   * one of them is set to foreground, the other to background */
  src.copyTo(dst);
  for (int i = 1; i < src.rows - 1; ++i)
    for (int j = 1; j < src.cols - 1; ++j) {
      if (dst.at<uchar>(i, j) == 0) { // if foreground
        if (dst.at<uchar>(i - 1, j) == dst.at<uchar>(i + 1, j)) {
          dst.at<uchar>(i - 1, j) = 0;
          dst.at<uchar>(i + 1, j) = 255;
        }

        if (dst.at<uchar>(i, j - 1) == dst.at<uchar>(i, j + 1)) {
          dst.at<uchar>(i, j - 1) = 0;
          dst.at<uchar>(i, j + 1) = 255;
        }
      }
    }
}

void
Binarization::swellFilter(const cv::Mat &src, cv::Mat &dst, int win)
{
  /* The opposite of shrink filter, counts the number of background pixels in a
   * given window centered
   *  on a background pixel if this number is to high, the center pixel is
   * considered as noise
   * and thus set as foreground */
  const int ksh = (int)(0.35 * win * win);

  src.copyTo(dst);
  for (int i = 0; i < src.rows; ++i)
    for (int j = 0; j < src.cols; ++j) {
      if (dst.at<uchar>(i, j) == 255) { // if background
        int curr_k = 0;
        for (int ii = -win; ii < win; ++ii)
          for (int jj = -win; jj < win; ++jj) {
            if (i + ii < 0 || i + ii >= src.rows || j + jj < 0 ||
                j + jj >= src.cols) // deal with borders
              continue;
            if (dst.at<uchar>(i + ii, j + jj) == 0)
              ++curr_k;
          }
        if (curr_k > ksh)
          dst.at<uchar>(i, j) = 255;
      }
    }
}

double
Binarization::binarize(const cv::Mat &src,
                       cv::Mat &dst,
                       int method,
                       int thresholdType,
                       int blockSize)
{
  /* Calls the right function considering the input method */

  // Grayscale conversion
  if (src.channels() > 1)
    cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
  else
    src.copyTo(dst);

  double th = 0;

  switch (method) {
  case cv::THRESH_OTSU:
    th = cv::threshold(dst, dst, 0, 255, cv::THRESH_BINARY | method);
      break;
  case cv::ADAPTIVE_THRESH_MEAN_C:
      cv::adaptiveThreshold(dst, dst, 255, method, cv::THRESH_BINARY,
			    thresholdType, blockSize);
      break;
  }

  // A denoising medianblur is applied in case of adaptive threshold
  if (method == cv::ADAPTIVE_THRESH_MEAN_C)
    cv::medianBlur(dst, dst, 5);

  return th;
}

void
Binarization::applyErosion(const cv::Mat &src,
                           cv::Mat &dst,
                           int erosion_type,
                           int erosion_size)
{
  cv::Mat element = cv::getStructuringElement(
    erosion_type,
    cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
    cv::Point(erosion_size, erosion_size));
  cv::erode(src, dst, element);
}

void
Binarization::applyDilation(const cv::Mat &src,
                            cv::Mat &dst,
                            int dilation_type,
                            int dilation_size)
{
  cv::Mat element = cv::getStructuringElement(
    dilation_type,
    cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
    cv::Point(dilation_size, dilation_size));
  cv::dilate(src, dst, element);
}

double
Binarization::calcLocalStats(const cv::Mat &im,
                             cv::Mat &map_m,
                             cv::Mat &map_s,
                             int winx,
                             int winy)
{
  /* Computes global stats of the input image to perform 
   * Wolf & Jolion segmentation */
  cv::Mat im_sum, im_sum_sq;
  cv::integral(im, im_sum, im_sum_sq, CV_64F);

  const int wxh = winx / 2;
  const int wyh = winy / 2;
  const int x_firstth = wxh;
  const int y_lastth = std::max(im.rows - wyh - 1, 0);
  const int y_firstth = wyh;
  const double win_area = winx * winy;

  double max_s = 0;
  for (int j = y_firstth; j <= y_lastth; ++j) {
    //sum = sum_sq = 0;

    double sum = im_sum.at<double>(j - wyh + winy, winx) -
      im_sum.at<double>(j - wyh, winx) -
      im_sum.at<double>(j - wyh + winy, 0) +
      im_sum.at<double>(j - wyh, 0);
    double sum_sq = im_sum_sq.at<double>(j - wyh + winy, winx) -
      im_sum_sq.at<double>(j - wyh, winx) -
      im_sum_sq.at<double>(j - wyh + winy, 0) +
      im_sum_sq.at<double>(j - wyh, 0);

    const double m = sum / win_area;
    const double s = sqrt((sum_sq - m * sum) / win_area);
    if (s > max_s)
      max_s = s;

    map_m.fset(x_firstth, j, static_cast<float>(m));
    map_s.fset(x_firstth, j, static_cast<float>(s));

    // Shift the window, add and remove	new/old values to the histogram
    for (int i = 1; i <= im.cols - winx; ++i) {

      // Remove the left old column and add the right new column
      sum -= im_sum.at<double>(j - wyh + winy, i) -
             im_sum.at<double>(j - wyh, i) -
             im_sum.at<double>(j - wyh + winy, i - 1) +
             im_sum.at<double>(j - wyh, i - 1);
      sum += im_sum.at<double>(j - wyh + winy, i + winx) -
             im_sum.at<double>(j - wyh, i + winx) -
             im_sum.at<double>(j - wyh + winy, i + winx - 1) +
             im_sum.at<double>(j - wyh, i + winx - 1);

      sum_sq -= im_sum_sq.at<double>(j - wyh + winy, i) -
                im_sum_sq.at<double>(j - wyh, i) -
                im_sum_sq.at<double>(j - wyh + winy, i - 1) +
                im_sum_sq.at<double>(j - wyh, i - 1);
      sum_sq += im_sum_sq.at<double>(j - wyh + winy, i + winx) -
                im_sum_sq.at<double>(j - wyh, i + winx) -
                im_sum_sq.at<double>(j - wyh + winy, i + winx - 1) +
                im_sum_sq.at<double>(j - wyh, i + winx - 1);

      const double m = sum / win_area;
      const double s = sqrt((sum_sq - m * sum) / win_area);
      if (s > max_s)
        max_s = s;

      map_m.fset(i + wxh, j, static_cast<float>(m));
      map_s.fset(i + wxh, j, static_cast<float>(s));
    }
  }

  return max_s;
}

void
Binarization::NiblackSauvolaWolfJolion(const cv::Mat &imOrig,
                                       cv::Mat &output,
                                       NiblackVersion version,
                                       double dR,
                                       int winx,
                                       int winy,
                                       double k)
{

  const int wxh = winx / 2;
  const int wyh = winy / 2;
  const int x_firstth = wxh;
  const int x_lastth = std::max(imOrig.cols - wxh - 1, 0);
  const int y_lastth = std::max(imOrig.rows - wyh - 1, 0);
  const int y_firstth = wyh;

  output.create(imOrig.rows, imOrig.cols, CV_8UC1);

  if (y_firstth > y_lastth) {
    output = 255; //white everywhere
    return;
  }

  cv::Mat im = imOrig;
  if (im.channels() > 1) {
    cv::cvtColor(imOrig, im, cv::COLOR_BGR2GRAY);
  }

  // Create local statistics and store them in a double matrices
  cv::Mat map_m = cv::Mat::zeros(im.rows, im.cols, CV_32F);
  cv::Mat map_s = cv::Mat::zeros(im.rows, im.cols, CV_32F);
  const double max_s = calcLocalStats(im, map_m, map_s, winx, winy);

  double min_I, max_I;
  cv::minMaxLoc(im, &min_I, &max_I);

  cv::Mat thsurf = cv::Mat::zeros(im.rows, im.cols, CV_32F);

  // Create the threshold surface, including border processing
  // ----------------------------------------------------

  double th = 0;

  assert(y_firstth >= 0 && y_lastth < map_m.rows - 1);
  assert(im.cols - winx + wxh <= map_m.cols - 1);

  for (int j = y_firstth; j <= y_lastth; ++j) {

    // NORMAL, NON-BORDER AREA IN THE MIDDLE OF THE WINDOW:
    for (int i = 0; i <= im.cols - winx; ++i) {

      const double m = map_m.at<float>(j, i + wxh);
      const double s = map_s.at<float>(j, i + wxh);

      // Calculate the threshold
      switch (version) {

        case NIBLACK:
          th = m + k * s;
          break;

        case SAUVOLA:
          th = m * (1 + k * (s / dR - 1));
          break;

        case WOLFJOLION:
          th = m + k * (s / max_s - 1) * (m - min_I);
          break;

        default:
          std::cerr << "Unknown threshold type in "
                       "ImageThresholder::surfaceNiblackImproved()\n";
          exit(1);
      }

      thsurf.fset(i + wxh, j, th);

      if (i == 0) {
        // LEFT BORDER
        for (int i = 0; i <= x_firstth; ++i)
          thsurf.fset(i, j, th);

        // LEFT-UPPER CORNER
        if (j == y_firstth)
          for (int u = 0; u < y_firstth; ++u)
            for (int i = 0; i <= x_firstth; ++i)
              thsurf.fset(i, u, th);

        // LEFT-LOWER CORNER
        if (j == y_lastth)
          for (int u = y_lastth + 1; u < im.rows; ++u)
            for (int i = 0; i <= x_firstth; ++i)
              thsurf.fset(i, u, th);
      }

      // UPPER BORDER
      if (j == y_firstth)
        for (int u = 0; u < y_firstth; ++u)
          thsurf.fset(i + wxh, u, th);

      // LOWER BORDER
      if (j == y_lastth)
        for (int u = y_lastth + 1; u < im.rows; ++u)
          thsurf.fset(i + wxh, u, th);
    }

    // RIGHT BORDER
    for (int i = x_lastth; i < im.cols; ++i)
      thsurf.fset(i, j, th);

    // RIGHT-UPPER CORNER
    if (j == y_firstth)
      for (int u = 0; u < y_firstth; ++u)
        for (int i = x_lastth; i < im.cols; ++i)
          thsurf.fset(i, u, th);

    // RIGHT-LOWER CORNER
    if (j == y_lastth)
      for (int u = y_lastth + 1; u < im.rows; ++u)
        for (int i = x_lastth; i < im.cols; ++i)
          thsurf.fset(i, u, th);
  }


  int rows = im.rows;
  int cols = im.cols;
  if (im.isContinuous() && thsurf.isContinuous() && output.isContinuous()) {
    cols *= rows;
    rows = 1;
  }
  for (int y = 0; y < rows; ++y) {
    const float *ths = thsurf.ptr<float>(y);
    const uchar *img = im.ptr<uchar>(y);
    uchar *dst = output.ptr<uchar>(y);
    for (int x = 0; x < cols; ++x) {
      if (img[x] >= ths[x]) {
	//(img.at<unsigned char>(y, x) >= thsurf.at<float>(y, x))
        dst[x] = 255;
      }
      else {
        dst[x] = 0;
      }
    }
  }
}
