#include "HoleDegradation.hpp"

#include <cassert>
#include <cmath> //cos, sin
#include <random>

#include <opencv2/imgproc/imgproc.hpp>

namespace dc {
  namespace HoleDegradation {

    //static const int INTENSITY_WHITE = 255;
    static const uchar PIXEL_BLACK = 0;

    static cv::Mat
    changeBorderPattern(const cv::Mat &pattern, Border side)
    {
      assert(pattern.type() == CV_8UC1);

      //Original border patterns are on TOP side
      if (side == Border::TOP)
	return pattern;

      int rows = pattern.rows;
      int cols = pattern.cols;
      if (side != Border::BOTTOM)
	std::swap(rows, cols);

      cv::Mat changedMat(rows, cols, CV_8UC1);

      if (side == Border::RIGHT) {
	for (int y = 0; y < rows; ++y) {
	  uchar *d = changedMat.ptr<uchar>(y);
	  for (int x = 0; x < cols; ++x) {
	    assert(cols - 1 - x < pattern.rows && y < pattern.cols);
	    d[x] = pattern.at<uchar>(cols - 1 - x, y);
	  }
	}
      } else if (side == Border::BOTTOM) {
	for (int y = 0; y < rows; ++y) {
	  const uchar *p = pattern.ptr<uchar>(rows - 1 - y);
	  uchar *d = changedMat.ptr<uchar>(y);
	  for (int x = 0; x < cols; ++x)
	    d[x] = p[x];
	}
      } else if (side == Border::LEFT) {
	for (int y = 0; y < rows; ++y) {
	  uchar *d = changedMat.ptr<uchar>(y);
	  for (int x = 0; x < cols; ++x)
	    d[x] = pattern.at<uchar>(x, y);
	}
      }

      return changedMat;
    }

    static cv::Mat
    changeCornerPattern(const cv::Mat &pattern, Corner side)
    {
      assert(pattern.type() == CV_8UC1);

      //Original corner patterns are on TOPLEFT side
      if (side == Corner::TOPLEFT)
	return pattern;

      cv::Mat changedMat(pattern.rows, pattern.cols, CV_8UC1);

      const int rows = pattern.rows;
      const int cols = pattern.cols;

      if (side == Corner::TOPRIGHT) {
	for (int y = 0; y < rows; ++y) {
	  const uchar *p = pattern.ptr<uchar>(y);
	  uchar *d = changedMat.ptr<uchar>(y);
	  for (int x = 0; x < cols; ++x)
	    d[x] = p[cols - 1 - x];
	}
      } else if (side == Corner::BOTTOMRIGHT) {
	for (int y = 0; y < rows; ++y) {
	  const uchar *p = pattern.ptr<uchar>(rows - 1 - y);
	  uchar *d = changedMat.ptr<uchar>(y);
	  for (int x = 0; x < cols; ++x)
	    d[x] = p[cols - 1 - x];
	}
      } else if (side == Corner::BOTTOMLEFT) {
	for (int y = 0; y < rows; ++y) {
	  const uchar *p = pattern.ptr<uchar>(rows - 1 - y);
	  uchar *d = changedMat.ptr<uchar>(y);
	  for (int x = 0; x < cols; ++x)
	    d[x] = p[x];
	}
      }

      return changedMat;
    }


    static bool
    isInMarge(const cv::Mat &matPattern, int x, int y, int width)
    {
      assert(matPattern.type() == CV_8UC1);
      assert(x >= 0 && x < matPattern.cols);
      assert(y >= 0 && y < matPattern.rows);

      if (matPattern.at<uchar>(y, x) ==
	  PIXEL_BLACK) { //not in marge if it's not in black pattern

	for (int i = 0; i < width; ++i) {
	  if (x - i > 0 && matPattern.at<uchar>(y, x - i) != PIXEL_BLACK)
	    return true;
	  if (x + i < matPattern.cols &&
	      matPattern.at<uchar>(y, x + i) != PIXEL_BLACK)
	    return true;
	  if (y - i > 0 && matPattern.at<uchar>(y - i, x) != PIXEL_BLACK)
	    return true;
	  if (y + i < matPattern.rows &&
	      matPattern.at<uchar>(y + i, x) != PIXEL_BLACK)
	    return true;
	}
      }

      return false;
    }

    /**
       @param[in, out] pixel  modified pixel
    */
    template <typename T>
    void
    updateShadowColor(T &pixel,
		      float coeff)
    {
      //general case for Vec3b & Vec4b
      for (int i=0; i<3; ++i) { //do not process alpha in case of Vec4b
	pixel[i] = cv::saturate_cast<uchar>(pixel[i] * coeff);
      }
    }

    //specialisation for uchar
    template <>
    void
    updateShadowColor<uchar>(uchar &pixel,
			     float coeff)
    {
      pixel = cv::saturate_cast<uchar>(pixel * coeff);
    }


    template <typename T>
    void
    drawBorder(cv::Mat &matOut,
	       const cv::Mat &matPattern,
	       int x,
	       int y,
	       int xOrigin,
	       int yOrigin,
	       int width,
	       float intensity)
    {
      const float x1 = matOut.cols - 1 * cos(-1) - matOut.rows - 1 * sin(-1); //B: ???
      float z1 = 3 * fabs(x1) / 30 + intensity;
      if (z1 == 0) {
	z1 = 1;
      }
      const float coeff = intensity * intensity / (z1 * z1);

      if (isInMarge(matPattern, x, y, width)) {
	updateShadowColor<T>(matOut.at<T>(y + yOrigin, x + xOrigin),
			     coeff);
      }

    }


    /**
       make a CV_8UC4 image from a CV_8UC3 image

       output image will be opage (all alpha channel is set to INTENSITY_WHITE)
    */
    /*
    static cv::Mat
    makeFourChanImage(const cv::Mat &src)
    {
      assert(src.type() == CV_8UC3);

      cv::Mat dst = cv::Mat(
			    src.rows, src.cols, CV_8UC4); //convert 3 channels mat in 4 channels mat

      int rows = src.rows;
      int cols = src.cols;
      if (src.isContinuous() && dst.isContinuous()) {
	cols *= rows;
	rows = 1;
      }
      for (int y = 0; y < rows; ++y) {
	const cv::Vec3b *s = src.ptr<cv::Vec3b>(y);
	cv::Vec4b *d = dst.ptr<cv::Vec4b>(y);
	for (int x = 0; x < cols; ++x) {
	  const cv::Vec3b v = s[x];
	  d[x] = cv::Vec4b(v[0], v[1], v[2], INTENSITY_WHITE);
	}
      }

      return dst;
    }
    */

    template <typename T>
    void
    fillHoleWithColor(cv::Mat &out,
		      const cv::Mat &holePattern,
		      T color,
		      int x0, int y0, int x1, int y1, int xOrigin, int yOrigin)
    {
      for (int y = y0; y < y1; ++y) {
	const uchar *p = holePattern.ptr<uchar>(y);
	T *d = out.ptr<T>(y + yOrigin);
	for (int x = x0; x < x1; ++x) {
	  assert(x + xOrigin >= 0 && x + xOrigin < out.cols &&
		 y + yOrigin >= 0 && y + yOrigin < out.rows);
	  if (p[x] == PIXEL_BLACK) {
	    d[x + xOrigin] = color;
	  }
	}
      }
    }

    static
    void
    fillHoleWithColor(cv::Mat &out,
		      const cv::Mat &holePattern,
		      cv::Scalar color,
		      int x0, int y0, int x1, int y1, int xOrigin, int yOrigin)
    {
      if (out.type() == CV_8UC1) {
	fillHoleWithColor<uchar>(out, holePattern,
				 color[0],
				 x0, y0, x1, y1, xOrigin, yOrigin);
      }
      else if (out.type() == CV_8UC3) {
	fillHoleWithColor<cv::Vec3b>(out, holePattern,
				     cv::Vec3b(color[0], color[1], color[2]),
				     x0, y0, x1, y1, xOrigin, yOrigin);
      }
      else if (out.type() == CV_8UC4) {
	fillHoleWithColor<cv::Vec4b>(out, holePattern,
				     cv::Vec4b(color[0], color[1], color[2], color[3]),
				     x0, y0, x1, y1, xOrigin, yOrigin);
      }
      else {
	const std::string error_msg = "holeDegradation: unhandled type";
#if CV_MAJOR_VERSION < 3
	const int code = CV_StsUnsupportedFormat;
#else
	const int code = cv::Error::StsUnsupportedFormat;
#endif
	CV_Error(code, error_msg);
      }
    }

    template <typename T>
    void
    fillHoleWithImage(cv::Mat &matOut,
		      const cv::Mat &matPattern,
		      const cv::Mat &matBelow,
		      const T &color,
		      int shadowBorderWidth, float shadowBorderIntensity,
		      int x0, int y0, int x1, int y1, int xOrigin, int yOrigin)
    {
      assert(matPattern.type()==CV_8UC1);
      assert(matBelow.type() == matOut.type());
      assert(y0<=y1);
      assert(x0<=x1);

      for (int y = y0; y < y1; ++y) {
	const uchar *p = matPattern.ptr<uchar>(y);
	T *d = matOut.ptr<T>(y + yOrigin);
	const T *b = nullptr;
	if (y + yOrigin >= 0 && y + yOrigin < matBelow.rows)
	  b = matBelow.ptr<T>(y + yOrigin);
	for (int x = x0; x < x1; ++x) {
	  assert(x + xOrigin >= 0 && x + xOrigin < matOut.cols &&
		 y + yOrigin >= 0 && y + yOrigin < matOut.rows);
	  if (p[x] == PIXEL_BLACK) {
	    if (b != nullptr && x + xOrigin >= 0 && x + xOrigin < matBelow.cols) {
	      d[x + xOrigin] = b[x + xOrigin];
	    }
	    else { //not over below image
	      d[x + xOrigin] = color;
	      //white4; //if page below is not below this pixel because of its size, we draw a white pixel to represent what we can see when it arrive with scanner
	    }

	    drawBorder<T>(matOut, matPattern, x, y, xOrigin, yOrigin,
			  shadowBorderWidth, shadowBorderIntensity);
	  }
	}
      }

    }

    static
    void
    fillHoleWithImage(cv::Mat &out,
		      const cv::Mat &holePattern,
		      const cv::Mat &matBelow,
		      const cv::Scalar &color,
		      int shadowBorderWidth, float shadowBorderIntensity,
		      int x0, int y0, int x1, int y1, int xOrigin, int yOrigin)
    {
      if (out.type() == CV_8UC1) {
	fillHoleWithImage<uchar>(out, holePattern, matBelow,
				 color[0],
				 shadowBorderWidth, shadowBorderIntensity,
				 x0, y0, x1, y1, xOrigin, yOrigin);
      }
      else if (out.type() == CV_8UC3) {
	fillHoleWithImage<cv::Vec3b>(out, holePattern, matBelow,
				     cv::Vec3b(color[0], color[1], color[2]),
				     shadowBorderWidth, shadowBorderIntensity,
				     x0, y0, x1, y1, xOrigin, yOrigin);
      }
      else if (out.type() == CV_8UC4) {
	fillHoleWithImage<cv::Vec4b>(out, holePattern, matBelow,
				     cv::Vec4b(color[0], color[1], color[2], color[3]),
				     shadowBorderWidth, shadowBorderIntensity,
				     x0, y0, x1, y1, xOrigin, yOrigin);
      }
      else {
	const std::string error_msg = "holeDegradation: unhandled type";

#if CV_MAJOR_VERSION < 3
	const int code = CV_StsUnsupportedFormat;
#else
	const int code = cv::Error::StsUnsupportedFormat;
#endif
	CV_Error(code, error_msg);
      }
    }

    cv::Mat
    holeDegradation(const cv::Mat &img,
		    const cv::Mat &holePattern,
		    int xOrigin,
		    int yOrigin,
		    int size,
		    HoleType type,
		    int side,
		    const cv::Scalar &color,
		    const cv::Mat &matBelow,
		    int shadowBorderWidth,
		    float shadowBorderIntensity)
    {
      assert(matBelow.empty() || matBelow.type() == img.type());
      assert(holePattern.type() == CV_8UC1);

      cv::Mat matOut = img.clone(); //img must not be modified.

      cv::Mat matPattern = holePattern;
      if (type == HoleType::BORDER && side > 0) {
	matPattern = changeBorderPattern(matPattern, (Border)side);
      }
      else if (type == HoleType::CORNER && side > 0) {
	matPattern = changeCornerPattern(matPattern, (Corner)side);
      }

      //B:TODO: remove this "size" parameter 
      if (size != 0 && matPattern.cols + size > 0 && matPattern.rows + size > 0) {
	resize(matPattern,
	       matPattern,
	       cv::Size(matPattern.cols + size, matPattern.rows + size));
      }

      assert(matPattern.type() == CV_8UC1);

      //B:TODO:OPTIM???
      const int x0 = std::max(xOrigin, 0) - xOrigin;
      const int y0 = std::max(yOrigin, 0) - yOrigin;
      const int x1 = std::min(xOrigin + matPattern.cols, matOut.cols) - xOrigin;
      const int y1 = std::min(yOrigin + matPattern.rows, matOut.rows) - yOrigin;

      //const cv::Vec4b white4(255, 255, 255, 255); //B:TODO: pass the below default color as parameter

      if (matBelow.empty()) { //without below image
	fillHoleWithColor(matOut, matPattern, color,
			  x0, y0, x1, y1, xOrigin, yOrigin);
      }
      else { //with below image

	fillHoleWithImage(matOut, matPattern, matBelow, color,
			  shadowBorderWidth, shadowBorderIntensity,
			  x0, y0, x1, y1, xOrigin, yOrigin);
      }

      assert(matOut.type() == img.type());

      return matOut;
    }


    namespace {
      std::random_device rd;
      std::mt19937 mt(rd());
      
      //return a random value in [rMin; rMax]
      int
      random_in_range(int rMin, int rMax)
      {
	std::uniform_int_distribution<int> dist(rMin, rMax);
	return dist(mt);
      }
    } //anonymous namespace


    
    cv::Point
    getRandomPosition(const cv::Size &matOriginalSize,
		      const cv::Size &holePatternSize,
		      HoleType type,
		      float ratioOutside,
		      int side)
    {
      //b: we want to draw xOrigin in [startX, endX] and yOrigin in [startY, endY]
      
      int startX = 0;
      int startY = 0;
      int endX = matOriginalSize.width;
      int endY = matOriginalSize.height;

      //B: Here we move the corner/border pattern
      //   according to its 'side'.
      //top left corner of pattern x coord will be in [startX; endX]
      // and y coord will be in [startY; endY]

      const int holeW = holePatternSize.width;
      const int holeH = holePatternSize.height;
      const int imgW = matOriginalSize.width;
      const int imgH = matOriginalSize.height;
      
      const int rw = std::max(static_cast<int>(holeW * ratioOutside), 1);
      const int rh = std::max(static_cast<int>(holeH * ratioOutside), 1);
      
      if (type == dc::HoleDegradation::HoleType::CORNER) {
	//holePattern is oriented to be top-left corner. But pattern geometry does not change for other corners.
	switch (static_cast<dc::HoleDegradation::Corner>(side)) {
	case dc::HoleDegradation::Corner::TOPLEFT:
	  startX = -rw;
	  endX = 1;
	  startY = -rh;
	  endY = 1;
	  break;
	case dc::HoleDegradation::Corner::TOPRIGHT:
	  startX = imgW - holeW;
	  endX = imgW + rw - holeW;
	  startY = -rh;
	  endY = 1;
	  break;
	case dc::HoleDegradation::Corner::BOTTOMRIGHT:
	  startX = imgW - holeW;
	  endX = imgW + rw - holeW;
	  startY = imgH - holeH;
	  endY = imgH + rh - holeH;
	  break;
	case dc::HoleDegradation::Corner::BOTTOMLEFT:
	default:
	  startX = -rw;
	  endX = 1;
	  startY = imgH - holeH;
	  endY = imgH + rh - holeH;
	  break;
	}

	//std::cerr<<"   corner side="<<side<<" startX="<<startX<<" endX="<<endX<<" startY="<<startY<<" endY="<<endY<<"\n";
      }
      else if (type == dc::HoleDegradation::HoleType::BORDER) {
	//Warning: hole pattern is oriented for top border. Its geometry changes for borders LEFT & RIGHT
	switch (static_cast<dc::HoleDegradation::Border>(side)) {
	case dc::HoleDegradation::Border::TOP:
	  startX = -rw;
	  endX = imgW + rw - holeW;
	  startY = -rh; //0
	  endY = 1;     //1
	  break;
	case dc::HoleDegradation::Border::RIGHT:
	  startX = imgW - holeH;    //imgW-holeH;
	  endX = imgW - holeH + rh; //imgW-holeH+1;
	  startY = -rw;
	  endY = imgH + rw - holeW;
	  break;
	case dc::HoleDegradation::Border::BOTTOM:
	  startX = -rw;
	  endX = imgW + rw - holeW;
	  startY = imgH - holeH;
	  endY = imgH + rh - holeH; //imgH-holeH+1;
	  break;
	case dc::HoleDegradation::Border::LEFT:
	  startX = -rh; //0;
	  endX = 1;
	  startY = -rw;
	  endY = imgH + holeW - rw;
	  break;
	}
      }
      

      int xOrigin = random_in_range(startX, endX-1);
      int yOrigin = random_in_range(startY, endY-1);
      return cv::Point(xOrigin, yOrigin);
    }
    

    cv::Mat
    holeDegradation(const cv::Mat &img,
		    const cv::Mat &holePattern,
		    int size,
		    HoleType type,
		    float ratioOutside,
		    int side,
		    const cv::Scalar &color,
		    const cv::Mat &matBelow,
		    int shadowBorderWidth,
		    float shadowBorderIntensity)
    {
      const cv::Point pos = getRandomPosition(img.size(),
					      holePattern.size(),
					      type,
					      ratioOutside,
					      side);
      
      return holeDegradation(img,
			     holePattern,
			     pos.x,
			     pos.y,
			     size,
			     type,
			     side,
			     color,
			     matBelow,
			     shadowBorderWidth,
			     shadowBorderIntensity);


    }


    
  }//namespace HoleDegradation

}//namespace dc
