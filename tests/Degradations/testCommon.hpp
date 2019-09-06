#ifndef TEST_COMMON_HPP
#define TEST_COMMON_HPP

#include <opencv2/core/core.hpp>

inline
bool
checkEqual(const cv::Mat &m1, const cv::Mat &m2)
{
  return (m1.size() == m2.size()) &&
     (m1.type() == m2.type()) &&
    (cv::sum( (m1!=m2) ) == cv::Scalar(0, 0, 0, 0));
}



#endif /* ! TEST_COMMON_HPP */
