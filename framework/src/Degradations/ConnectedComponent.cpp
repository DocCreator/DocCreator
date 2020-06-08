#include "ConnectedComponent.hpp"

#include <algorithm>
#include <cassert>
#include <deque>

#include <opencv2/imgproc/imgproc.hpp>

namespace dc {

  namespace ConnectedComponent {

  static constexpr uchar BACKGROUND = 255;

  /*
    Region growing from @a seed, according to @a connectivity.
    @a input is considered binarized, and of type CV_8UC1.
    Will modify @a input and fill @a cc.

    Black pixels (0) are considered foreground, 
    white pixels (255) are considered background.
  */
  void
  extractConnectedComponent(cv::Mat &input,
			    const cv::Point &seed,
			    CC &cc,
			    int connectivity)
  {
    assert(input.type() == CV_8U);
    assert(seed.x < input.cols && seed.y < input.rows && seed.x >= 0 &&
	   seed.y >= 0);
    //  assert(input.at<unsigned char>(seed.y, seed.x));
    assert(connectivity == 4 || connectivity==8);

    cc.clear();

    std::deque<cv::Point> ptsQueue;
    uchar &pixS = input.at<unsigned char>(seed.y, seed.x);
    if (pixS != BACKGROUND) {
      ptsQueue.push_back(seed);
      pixS = BACKGROUND;
    }

    while (!ptsQueue.empty()) {

      const cv::Point current = ptsQueue.front();
      ptsQueue.pop_front();

      cc.push_back(current);

      // enqueue neighboors
      const cv::Point e(current.x + 1, current.y);
      const cv::Point w(current.x - 1, current.y);
      const cv::Point n(current.x, current.y - 1);
      const cv::Point s(current.x, current.y + 1);
      if (n.y >= 0) {
	uchar &pix = input.at<unsigned char>(n.y, n.x);
	if (pix != BACKGROUND) {
	  ptsQueue.push_back(n);
	  pix = BACKGROUND;
	}
      }
      if (e.x < input.cols) {
	uchar &pix = input.at<unsigned char>(e.y, e.x);
	if (pix != BACKGROUND) {
	  ptsQueue.push_back(e);
	  pix = BACKGROUND;
	}
      }
      if (s.y < input.rows) {
	uchar &pix = input.at<unsigned char>(s.y, s.x);
	if (pix != BACKGROUND) {
	  ptsQueue.push_back(s);
	  pix = BACKGROUND;
	}
      }
      if (w.x >= 0) {
	uchar &pix = input.at<unsigned char>(w.y, w.x);
	if (pix != BACKGROUND) {
	  ptsQueue.push_back(w);
	  pix = BACKGROUND;
	}
      }
      if (connectivity == 8) {
	const cv::Point ne(current.x + 1, current.y - 1);
	const cv::Point no(current.x - 1, current.y - 1);
	const cv::Point se(current.x + 1, current.y + 1);
	const cv::Point so(current.x - 1, current.y + 1);
	if (ne.x < input.cols && ne.y >= 0) {
	  uchar &pix = input.at<unsigned char>(ne.y, ne.x);
	  if (pix != BACKGROUND) {
	    ptsQueue.push_back(ne);
	    pix = BACKGROUND;
	  }
	}
	if (se.x < input.cols && se.y < input.rows) {
	  uchar &pix = input.at<unsigned char>(se.y, se.x);
	  if (pix != BACKGROUND) {
	    ptsQueue.push_back(se);
	    pix = BACKGROUND;
	  }
	}
	if (so.x >= 0 && so.y < input.rows) {
	  uchar &pix = input.at<unsigned char>(so.y, so.x);
	  if (pix != BACKGROUND) {
	    ptsQueue.push_back(so);
	    pix = BACKGROUND;
	  }
	}
	if (no.x >= 0 && no.y >= 0) {
	  uchar &pix = input.at<unsigned char>(no.y, no.x);
	  if (pix != BACKGROUND) {
	    ptsQueue.push_back(no);
	    pix = BACKGROUND;
	  }
	}
      }
    }
  }

  void
  extractAllConnectedComponents(const cv::Mat &input,
				CCs &ccs,
				int connectivity)
  {
    assert(input.type() == CV_8U);
    assert(connectivity == 4 || connectivity==8);

    ccs.clear();
    ccs.reserve(input.rows); //arbitrary

    cv::Mat tmp = input.clone();

    CC cc;

    for (int i = 0; i < input.rows; ++i) {
      const uchar *r = tmp.ptr<uchar>(i);
      for (int j = 0; j < input.cols; ++j) {
	if (r[j] != BACKGROUND) {
	  cc.clear();
	  extractConnectedComponent(
				    tmp, cv::Point(j, i), cc, connectivity); //modify tmp
	  ccs.push_back(cc);
	}
      }
    }
  }

  } //namespace ConnectedComponent

} //namespace dc
