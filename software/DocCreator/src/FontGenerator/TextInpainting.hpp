#ifndef TEXTINPAINTING_H
#define TEXTINPAINTING_H

#include <opencv2/core/core.hpp>

class TextInpainting
{
public:
  static cv::Mat getBackground(const cv::Mat &img,
                               const cv::Mat &img_bin,
                               float max_text_area = 0.1,
                               float max_text_width = 0.2,
                               float max_text_height = 0.2);
  static cv::Mat getDistanceMap(const cv::Mat &img,
                                const cv::Mat &img_bin,
                                float char_height = 15);
};

#endif // TEXTINPAINTING_H
