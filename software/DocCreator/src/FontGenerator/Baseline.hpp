#ifndef BASELINE_HPP
#define BASELINE_HPP

#include "opencv2/core/core.hpp"

class Baseline
{
  // Considers that the input image is binarized
public:
  // Get median height of character in the binary image
  static std::pair<int, float> getCharacterHeight(cv::Mat img);

  // Compute baselines on the binary image
  static void computeBaselines(cv::Mat &img, std::vector<cv::Vec4i> &lines);

  // Get closest baseline from the bounding box of the character in the image
  static int getBaseline(cv::Rect r, const std::vector<cv::Vec4i> &lines);

protected:
  static int character_height;
};

#endif // BASELINE_HPP
