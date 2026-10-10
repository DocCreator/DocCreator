#include "ColorUtils.hpp"

#include <cassert>

namespace dc {

  namespace {

    /*
      @return true if the first three channels of every pixel of @a in hold
      the same value. @a NC must be the number of channels of @a in.

      Four pixels are tested per branch: accumulating the comparisons with
      exclusive ors is measurably faster than branching on each pixel, and the
      scan still stops as soon as a colored pixel is found.
    */
    template <int NC>
    bool
    hasEqualChannels(const cv::Mat &in)
    {
      assert(in.channels() == NC);
      assert(in.depth() == CV_8U);

      int rows = in.rows;
      int cols = in.cols;
      if (in.isContinuous()) {
	cols *= rows;
	rows = 1;
      }

      for (int i = 0; i < rows; ++i) {
	const uchar *p = in.ptr<uchar>(i);

	int j = 0;
	for (; j + 4 <= cols; j += 4, p += 4 * NC) {
	  const int d0 = (p[0] ^ p[1]) | (p[0] ^ p[2]);
	  const int d1 = (p[NC] ^ p[NC + 1]) | (p[NC] ^ p[NC + 2]);
	  const int d2 = (p[2 * NC] ^ p[2 * NC + 1]) | (p[2 * NC] ^ p[2 * NC + 2]);
	  const int d3 = (p[3 * NC] ^ p[3 * NC + 1]) | (p[3 * NC] ^ p[3 * NC + 2]);
	  if (((d0 | d1) | (d2 | d3)) != 0) {
	    return false;
	  }
	}
	for (; j < cols; ++j, p += NC) {
	  if (((p[0] ^ p[1]) | (p[0] ^ p[2])) != 0) {
	    return false;
	  }
	}
      }

      return true;
    }

  } //anonymous namespace

  bool
  isGray(const cv::Mat &in)
  {
    assert(in.type() == CV_8UC1 ||
	   in.type() == CV_8UC3 ||
	   in.type() == CV_8UC4);

    //B: the alpha channel of a CV_8UC4 image is not taken into account.

    if (in.type() == CV_8UC1) {
      return true;
    }
    if (in.type() == CV_8UC3) {
      return hasEqualChannels<3>(in);
    }
    if (in.type() == CV_8UC4) {
      return hasEqualChannels<4>(in);
    }

    return false;
  }


} //namespace dc
