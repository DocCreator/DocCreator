#include "LineDetectionDialog.hpp"
#include "ui_LineDetectionDialog.h"

#include <Utils/convertor.h>
#include <cassert>
#include <opencv2/imgproc/imgproc.hpp>

//B: some CODE DUPLICATION with baseline.hpp/.cpp ????

static const double MIN_LINE_ANGLE = 2.8;
static const int IMG_WIDTH = 400;
static const int IMG_HEIGHT = 600;

LineDetectionDialog::LineDetectionDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::LineDetectionDialog)
  , _thumbnail(nullptr)
  , _originalLabel(nullptr)
  , _originalImg()
  , _houghImg()
  , _character_height(0)
{
  ui->setupUi(this);
}

LineDetectionDialog::~LineDetectionDialog()
{
  delete ui;
}

void
LineDetectionDialog::setOriginalImage(const QImage &img)
{
  _originalImg = img;
  process();
}

void
LineDetectionDialog::updateView()
{
  _thumbnail = ui->thumbnail;
  _thumbnail->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  _thumbnail->setText(tr("thumbnail"));
  _thumbnail->setPixmap(QPixmap::fromImage(_houghImg.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation)));
}

int
LineDetectionDialog::getCharacterHeight(const cv::Mat &img)
{
  /* Returns the median height of a character in a given binary image */

  // Smooth the image to remove unwanted noise from extracted CCs
  cv::medianBlur(img, img, 9);

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
  return cv::boundingRect(contours[median]).height;
}

static void
peel(cv::Mat &src, cv::Mat &dst, int thickness, int line_gap)
{
  src.copyTo(dst);
  // Here we shrink the lines to keep only the bottom pixels
  for (int i = 0; i < dst.rows - line_gap; i++) {
    for (int j = 0; j < dst.cols; j++) {
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

void
LineDetectionDialog::drawBaseLines(const cv::Mat &img,
                                   cv::Mat &dst,
                                   const std::vector<cv::Vec4i> &lines)
{
  // Smooth the image to remove unwanted noise from extracted CCs
  cv::Mat tmp;
  // Smooth the image to remove unwanted noise from extracted CCs
  cv::medianBlur(img, tmp, 9);

  // CCs extraction
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(tmp, contours, cv::RETR_CCOMP, cv::CHAIN_APPROX_TC89_KCOS);

  cv::cvtColor(img, dst, cv::COLOR_GRAY2BGR);

  for (const auto &contour : contours) {
    const cv::Rect r = cv::boundingRect(contour);
    cv::Point p = r.br();
    // We create a point in the bottom-middle of the bounding box of the letter
    p.x -= r.width / 2;

    double min_dist = std::numeric_limits<double>::max();
    cv::Vec4i closest_line;

    // We look for the closest line
    for (cv::Vec4i l :
         lines) { //int line=0; line < (int)lines.size(); ++line) {
                  //cv::Vec4i l = lines[line];
      const double d =
        distance_to_line(cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), p);
      const double angle = atan2(l[1] - l[3], l[0] - l[2]);
      // We discard lines with not almost horizontal angle

      if (d < min_dist && fabs(angle) > MIN_LINE_ANGLE) {
        min_dist = d;
        closest_line = l;
      }
    }

    // Global line slope
    assert(closest_line[0] != closest_line[2]);
    const double slope = (double)(closest_line[1] - closest_line[3]) /
                         (closest_line[0] - closest_line[2]);
    int y = (int)(slope * (p.x - closest_line[0]) + closest_line[1]);

    // Discarded if the angle is to high
    if (abs(y - p.y) > 1.2 * r.height)
      y = p.y;

    cv::Point baseline(p.x - r.width / 2, y);
    cv::Point baseline2(p.x + r.width / 2, y);

    cv::line(dst, baseline2, baseline, cv::Scalar(100, 150, 255));
  }
}

void
LineDetectionDialog::process()
{
  cv::Mat src(Convertor::getCvMat(_originalImg));
  cv::Mat src2;

  if (src.channels() > 1)
    cvtColor(src, src, cv::COLOR_BGR2GRAY);

  src.copyTo(src2);
  _character_height = getCharacterHeight(src.clone());

  cv::Mat dst, cdst;

  // horizontal blur
  cv::blur(src, dst, cv::Size(6 * _character_height, 0.5 * _character_height));

  // binarization + median filtering
  cv::threshold(dst, dst, 190, 255, 1);
  cv::medianBlur(dst, dst, 9);

  QImage blurred = Convertor::getQImage(dst);

  peel(dst, dst, 2, static_cast<int>(0.8 * _character_height));

  _originalLabel = ui->originalLabel;
  _originalLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  _originalLabel->setText(tr("original label"));
  _originalLabel->setPixmap(QPixmap::fromImage(blurred.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation)));

  cv::cvtColor(src, src2, cv::COLOR_GRAY2BGR);
  std::vector<cv::Vec4i> lines;

  // Hough transform allows to detect global lines
  cv::HoughLinesP(dst,
                  lines,
                  1,
                  CV_PI / 180,
                  20,
                  4 * _character_height,
                  5 * _character_height);

  drawBaseLines(src, src2, lines);

  _houghImg = Convertor::getQImage(src2);
  updateView();
}
