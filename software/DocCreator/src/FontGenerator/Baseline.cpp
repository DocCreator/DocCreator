#include "Baseline.hpp"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <Utils/convertor.h>

static const double MIN_LINE_ANGLE = 2.8;
static const double MIN_LINE_DIST_COEFF = 1.2;

int Baseline::character_height = 0;

static void
peel(cv::Mat &src, cv::Mat &dst, int thickness, int line_gap)
{
  src.copyTo(dst);
  // Here we shrink the lines to keep only the bottom pixels
  for (int i = 0; i < dst.rows - line_gap; ++i) {
    for (int j = 0; j < dst.cols; ++j) {
      if (dst.at<uchar>(i, j) == 255) { // if foreground
        for (int l = thickness; l < line_gap; ++l) {
          if (dst.at<uchar>(i + l, j) == 255) {
            dst.at<uchar>(i, j) = 0;
            break;
          }
        }
      }
    }
  }
}

void
Baseline::computeBaselines(cv::Mat &img, std::vector<cv::Vec4i> &lines)
{
  cv::Mat dst;
  img.copyTo(dst);

  if (img.channels() > 1)
    cvtColor(dst, dst, cv::COLOR_BGR2GRAY);

  const std::pair<int, float> pair = getCharacterHeight(dst.clone());
  const int characterHeight = pair.first;
  const float characterDensity = pair.second;

  //B: What are these constants ????
  const int densityThreshold =
    (int)(212.453255 * (characterDensity * characterDensity) -
          306.0151077 * characterDensity + 244.3721012);

  // horizontal blur
  cv::blur(dst, dst, cv::Size(6 * characterHeight, 0.5 * characterHeight));

  // binarization + median filtering
  cv::threshold(dst, dst, densityThreshold, 255, 1);
  //cv::threshold(dst, dst, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

  cv::medianBlur(dst, dst, 9);

  // Mask peeling
  peel(dst, dst, 2, (int)(0.8 * characterHeight));

  // Hough transform line detection
  cv::HoughLinesP(
    dst, lines, 1, CV_PI / 180, 20, 4 * characterHeight, 5 * characterHeight);
}

static inline double
cross_product(cv::Point a, cv::Point b)
{
  return a.x * b.y - a.y * b.x;
}

static double
distance_to_line(cv::Point begin, cv::Point end, cv::Point x)
{
  //translate the begin to the origin
  end -= begin;
  x -= begin;

  const double area = cross_product(x, end);
  return fabs(area / cv::norm(end));
}

int
Baseline::getBaseline(cv::Rect r, const std::vector<cv::Vec4i> &lines)
{
  cv::Point p = r.br();
  // We create a point in the bottom-middle of the bounding box of the letter
  p.x -= r.width / 2;
  p.y -= r.height / 2;

  double min_dist = std::numeric_limits<double>::max();
  cv::Vec4i closest_line;

  // We look for the closest line
  for (const auto &l : lines) {
    const double d =
      distance_to_line(cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), p);
    const double angle = atan2(l[1] - l[3], l[0] - l[2]);
    // We discard lines with not almost horizontal angle

    if (d < min_dist && fabs(angle) > MIN_LINE_ANGLE) {
      min_dist = d;
      closest_line = l;
    }
  }

  // We compute the projection of p (character center point) on the baseline
  // equation
  assert(closest_line[0] != closest_line[2]);
  const double slope = (double)(closest_line[1] - closest_line[3]) /
                       (closest_line[0] - closest_line[2]);
  int y = (int)(slope * (p.x - closest_line[0]) + closest_line[1]);

  // If the found baseline is too far from the character, we take the bottom as
  // reference
  if (abs(y - p.y) > MIN_LINE_DIST_COEFF * r.height)
    y = r.br().y;

  return y;
}

std::pair<int, float>
Baseline::getCharacterHeight(cv::Mat img)
{
  /* Returns the median height of a character in a given binary image */

  // Smooth the image to remove unwanted noise from extracted CCs
  cv::medianBlur(img, img, 9);

  cv::Mat symbol = img(cv::Rect((int)img.cols / 3,
                                (int)img.rows / 3,
                                (int)img.cols / 3,
                                (int)img.rows / 3));

  int sum = 0;
  for (int i = 0; i < symbol.rows; ++i)
    for (int j = 0; j < symbol.cols; ++j)
      if (symbol.at<uchar>(i, j) == 0)
        ++sum;

  const float density = (float)sum / (symbol.rows * symbol.cols);

  // CCs extraction
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(img, contours, cv::RETR_CCOMP, cv::CHAIN_APPROX_TC89_KCOS);

  // We sort the array with respect to their height
  sort(contours.begin(),
       contours.end(),
       [](const std::vector<cv::Point> &c1, const std::vector<cv::Point> &c2) {
         return cv::boundingRect(c1).height < cv::boundingRect(c2).height;
       });

  // Merely return the median's height
  const int median = (int)contours.size() / 2;
  const cv::Rect r = cv::boundingRect(contours[median]);

  return std::pair<int, float>(r.height, density);
}
