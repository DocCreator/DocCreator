#ifndef STRUCTUREDETECTION_H
#define STRUCTUREDETECTION_H

#include <opencv2/core/core.hpp>

class StructureDetection
{
public:
  static std::vector<cv::Rect> getBlocks(const cv::Mat &img);
  static std::vector<cv::Rect> getBlocks(
    const cv::Mat &distanceMap,
    int char_height = 1); //char_height=dilateFactor
  static std::vector<cv::Rect> getWordBlocks(const cv::Mat &img_bin);
  static cv::Mat getDistanceMap(const cv::Mat &img_bin);

  static int getCharacterHeight(const cv::Mat &img);
};

#endif // STRUCTUREDETECTION_H
