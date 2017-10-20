#include "TextInpainting.hpp"

#include "Binarization.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/photo/photo.hpp>

cv::Mat
TextInpainting::getBackground(const cv::Mat &img,
                              const cv::Mat &img_bin,
                              float max_text_area,
                              float max_text_width,
                              float max_text_height)
{
  cv::Mat background;

  cv::Mat bin = cv::Mat::zeros(img_bin.rows, img_bin.cols, CV_8U);
  cv::bitwise_not(img_bin, bin);

  // We consider too big connected components as images, they are then removed
  // from the mask which is to be inpainted
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(
    bin.clone(), contours, CV_RETR_CCOMP, CV_CHAIN_APPROX_TC89_KCOS);

  // We dilate the mask to inpaint a wider area and remove letters contour
  Binarization::applyDilation(bin, bin, 2, 3);

  for (size_t i = 0; i < contours.size(); ++i) {
    cv::Rect r = cv::boundingRect(contours[i]);
    if (r.width > img_bin.cols * max_text_width ||
        r.height > img_bin.rows * max_text_height ||
        r.area() > (img_bin.cols * img_bin.rows) * max_text_area) {
      cv::drawContours(bin, contours, i, cv::Scalar(0), CV_FILLED);
    }
  }

  cv::inpaint(img, bin, background, 1, cv::INPAINT_TELEA);
  //B: cv::inpaint() is very slow...

  return background;
}

cv::Mat
TextInpainting::getDistanceMap(const cv::Mat &img,
                               const cv::Mat &img_bin,
                               float char_height)
{

  cv::Mat distance = cv::Mat(img.rows, img.cols, CV_32SC1);

  // We compute the number of successive white pixels in the four directions for
  // each pixel
  for (int row = 0; row < img.rows; ++row)
    for (int col = 0; col < img.cols; ++col) {

      int sum_h = 0;
      int sum_v = 0;

      // Horizontal
      // left
      for (int i = col; i >= 0; --i) {
        if (img_bin.at<uchar>(row, i) == 0)
          break;
        ++sum_h;
      }
      // right
      for (int i = col; i < img.cols; ++i) {
        if (img_bin.at<uchar>(row, i) == 0)
          break;
        ++sum_h;
      }

      // Vertical
      // left
      for (int i = col; i >= 0; --i) {
        if (img_bin.at<uchar>(i, col) == 0)
          break;
        ++sum_v;
      }
      // right
      for (int i = col; i < img.rows; ++i) {
        if (img_bin.at<uchar>(i, col) == 0)
          break;
        ++sum_v;
      }

      distance.at<int>(row, col) = sum_h * img.rows + sum_v * img.cols;
    }

  cv::Mat distance2 = cv::Mat(img.rows, img.cols, CV_8U);

  // We normalize to get an understandable image
  cv::normalize(distance, distance2, 0, 255, cv::NORM_MINMAX, CV_8U);

  // Then the images are binarized
  cv::Mat distance3 = cv::Mat(img.rows, img.cols, CV_8U);
  //double th =
  Binarization::binarize(distance2, distance3);

  cv::bitwise_not(distance3, distance3);

  // Then we close the image
  char_height /= 2;
  cv::Mat element = cv::getStructuringElement(
    2,
    cv::Size(2 * char_height + 1, 2 * char_height + 1),
    cv::Point(char_height, char_height));
  //B:TODO?: truncation from float to int !?

  cv::dilate(distance3, distance3, element);
  cv::erode(distance3, distance3, element);

  return distance3;
}
