#ifndef BINARIZATION_H
#define BINARIZATION_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class Binarization
{

public:
  enum NiblackVersion
  {
    NIBLACK = 0,
    SAUVOLA,
    WOLFJOLION,
  };

  static double binarize(const cv::Mat &src,
                         cv::Mat &dst,
                         int method = CV_THRESH_OTSU,
                         int thresholdType = 17,
                         int blockSize = 11);
  static void preProcess(const cv::Mat &src, cv::Mat &dst, int erosion = 12);
  static void postProcess(const cv::Mat &src,
                          cv::Mat &dst,
                          double v = 0.9,
                          int win = 7);

  static void NiblackSauvolaWolfJolion(const cv::Mat &imOrig,
                                       cv::Mat &output,
                                       NiblackVersion version,
                                       double dR = 128,
                                       int winx = 40,
                                       int winy = 40,
                                       double k = 0.34);

  static void applyErosion(const cv::Mat &src,
                           cv::Mat &dst,
                           int erosion_type = 2,
                           int erosion_size = 2);
  static void applyDilation(const cv::Mat &src,
                            cv::Mat &dst,
                            int dilation_type = 2,
                            int dilation_size = 2);

  static void shrinkFilter(const cv::Mat &src,
                           cv::Mat &dst,
                           double v = 0.9,
                           int win = 5);
  static void swellFilter(const cv::Mat &src, cv::Mat &dst, int win = 16);
  static void connectivityFilter(const cv::Mat &src, cv::Mat &dst);

protected:
  static double calcLocalStats(const cv::Mat &im,
                               cv::Mat &map_m,
                               cv::Mat &map_s,
                               int winx,
                               int winy);
};

#endif // BINARIZATION_H
