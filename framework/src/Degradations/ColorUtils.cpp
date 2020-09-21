#include "ColorUtils.hpp"

namespace dc {
  
  bool
  isGray(const cv::Mat &in)
  {
    assert(in.type() == CV_8UC1 ||
	   in.type() == CV_8UC3 ||
	   in.type() == CV_8UC4);
  
    if (in.type() == CV_8UC1) {
      return true;
    }
    if (in.type() == CV_8UC3) {
      int rows = in.rows;
      int cols = in.cols;
      if (in.isContinuous()) {
	cols *= rows;
	rows = 1;
      }
      for (int i=0; i<rows; ++i) {
	const cv::Vec3b *p = in.ptr<cv::Vec3b>(0);
	for (int j=0; j<cols; ++j) {
	  const cv::Vec3b &pj = p[j];
	  if (pj[0] != pj[1] || pj[0] != pj[2])
	    return false;
	}
      }
      return true;
    }
  
    if (in.type() == CV_8UC4) {
      int rows = in.rows;
      int cols = in.cols;
      if (in.isContinuous()) {
	cols *= rows;
	rows = 1;
      }
      for (int i=0; i<rows; ++i) {
	const cv::Vec4b *p = in.ptr<cv::Vec4b>(0);
	for (int j=0; j<cols; ++j) {
	  const cv::Vec4b &pj = p[j];
	  if (pj[0] != pj[1] || pj[0] != pj[2])
	    return false;
	}
      }
      return true;
    }

    return false;
  }


} //namespace dc
