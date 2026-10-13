#include "EllipseRasteriser.hpp"

#include <algorithm>
#include <cassert>
#include <climits>

#include <opencv2/imgproc/imgproc.hpp>

namespace dc {

  static constexpr uchar WHITE = 255;

  size_t
  EllipseRasteriser::rasterise(const cv::Mat &grayImage,
			       int localZone,
			       std::vector<NoiseLine> &lines,
			       cv::Rect &rec,
			       int &max_B,
			       int &min_B,
			       cv::Point centre,
			       double semi_minor_axis,
			       double semi_major_axis,
			       double theta)
  {
    const int width = grayImage.cols;
    const int height = grayImage.rows;
    /*
      We only ever look at a window of (2*localZone) pixels around the
      centre, thus:
      1) the filled ellipse is drawn in a small buffer of that size, kept from
         one call to the next, instead of a buffer as big as the whole image
         allocated (and cleared) for each seed point;
      2) the points of each line, from the centre to a point of the ellipse
         contour, are walked with a Bresenham iterator instead of being drawn
         in that buffer and searched back by their color.

      Coordinates are kept in the frame of the former full-image buffer,
      that is image coordinates translated by (half, half), as the returned
      @a rec is expressed in it.
    */

    //the lines of the previous seed point are dropped, but their point
    //vectors are kept, cf the comment on the declaration
    size_t numLines = 0;

    const int ex = static_cast<int>(semi_major_axis) + 10;
    const int half = ex / 2;
    const int boxCols = width + ex;
    const int boxRows = height + ex;
    const cv::Point newCentre(centre.x + half, centre.y + half);

    //window of interest, clipped to the former buffer
    const int localZone2 = 2 * localZone;
    const int xr = std::max(newCentre.x - localZone2, 0);
    const int yr = std::max(newCentre.y - localZone2, 0);
    const int wr = (newCentre.x - xr) + std::min(localZone2, boxCols - newCentre.x);
    const int hr = (newCentre.y - yr) + std::min(localZone2, boxRows - newCentre.y);
    assert(wr > 0 && wr <= 2 * localZone2);
    assert(hr > 0 && hr <= 2 * localZone2);
    const cv::Rect r(xr, yr, wr, hr);

    if (m_ellipse.empty()) {
      m_ellipse.create(2 * localZone2, 2 * localZone2, CV_8UC1);
    }
    //Drawing the ellipse directly in the window clips it exactly as drawing
    //it in the whole image and cropping the window did.
    cv::Mat win = m_ellipse(cv::Rect(0, 0, r.width, r.height));
    win.setTo(cv::Scalar(0));
    cv::ellipse(win,
		cv::Point(newCentre.x - r.x, newCentre.y - r.y),
		cv::Size(static_cast<int>(semi_major_axis),
			 static_cast<int>(semi_minor_axis)),
		theta,
		0,
		360,
		cv::Scalar(WHITE),
		-1);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(win,
		     contours,
		     hierarchy,
		     cv::RETR_TREE,
		     cv::CHAIN_APPROX_NONE,
		     cv::Point(0, 0));

    if (contours.empty()) { //B: the ellipse is entirely outside the window
      rec = cv::Rect(0, 0, 0, 0);
      return 0;
    }

    const std::vector<cv::Point> &v = contours[0];
    if (lines.size() < v.size()) {
      lines.resize(v.size());
    }

    int min_x = INT_MAX, min_y = INT_MAX;
    int max_x = INT_MIN, max_y = INT_MIN;

    for (const cv::Point &pWin : v) {

      //contour point, in the former buffer frame
      const cv::Point p(pWin.x + r.x, pWin.y + r.y);

      if (min_x > p.x)
	min_x = p.x;
      if (max_x < p.x)
	max_x = p.x;
      if (min_y > p.y)
	min_y = p.y;
      if (max_y < p.y)
	max_y = p.y;

      //Points of the segment from the centre to the contour point, in image
      //coordinates. cv::LineIterator walks the very pixels cv::line() would
      //draw, and clips the segment to the image.
      //the vector of the line about to be filled, kept from the previous call
      std::vector<cv::Point> &pointInLine = lines[numLines].points;
      pointInLine.clear();
      {
	cv::LineIterator lineIt(grayImage, centre, cv::Point(p.x - half, p.y - half), 8);
	for (int i = 0; i < lineIt.count; ++i, ++lineIt) {
	  const cv::Point px = lineIt.pos();
	  assert(px.x >= 0 && px.x < grayImage.cols &&
		 px.y >= 0 && px.y < grayImage.rows);
	  pointInLine.push_back(px);
	}
      }

      // new line
      if (!pointInLine.empty()) {

	//B: check translated point is really in image
	cv::Point pt(p.x - half, p.y - half);

	if (pt.y >= grayImage.rows)
	  pt.y = grayImage.rows - 1;
	else if (pt.y < 0)
	  pt.y = 0;
	if (pt.x >= grayImage.cols)
	  pt.x = grayImage.cols - 1;
	else if (pt.x < 0)
	  pt.x = 0;

	assert(pt.y >= 0 && pt.y < grayImage.rows && pt.x >= 0 &&
	       pt.x < grayImage.cols);
	const int gray = grayImage.at<uchar>(pt.y, pt.x);

	if (max_B < gray)
	  max_B = gray;
	if (min_B > gray)
	  min_B = gray;

	//B: should we use p or pt here ???
	lines[numLines].distanceToCentre =
	  sqrtf(static_cast<float>((p.x - newCentre.x) * (p.x - newCentre.x) +
				   (p.y - newCentre.y) * (p.y - newCentre.y)));
	lines[numLines].borderGrayLevel = gray;
	++numLines;
      }
    }

    rec.x = min_x - half;
    rec.y = min_y - half;
    rec.width = max_x - min_x;
    rec.height = max_y - min_y;

    return numLines;
  }

} //namespace dc
