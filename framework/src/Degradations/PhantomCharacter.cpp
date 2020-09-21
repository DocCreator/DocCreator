#include "PhantomCharacter.hpp"

#include <cassert>
#include <cmath> //sqrt
#include <ctime> //time
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "ConnectedComponent.hpp"
#include "FileUtils.hpp"
#include "Degradations/BlurFilter.hpp"

namespace dc {
  namespace PhantomCharacter {

    //Spacing minimum between two character to be able to paste a pattern
    static constexpr int SPACING_MIN = 10;

    //Spacing maximum beyond which we will consider that two character are not in the same word
    static constexpr int SPACING_MAX = 20;

    //Define how many times we will search a character where we can copy the pattern changing the size of pattern
    //static const int NB_TRY_FIT = 5;

    //Minimum spacing between a character and the degradation we paste
    static constexpr int MIN_MARGIN = 1;

    //Maximum spacing between a character and the degradation we paste
    static constexpr int MAX_MARGIN = 2;

    //Coeff with which the height of pattern will be multiplied to get the minimum height of degradation
    static constexpr float COEFF_MIN_HEIGHT = 0.9f;

    //Coeff with which the height of pattern will be multiplied to get the minimum height of degradation
    static constexpr float COEFF_MAX_HEIGHT = 1.05f;

    //Coeff with which the height of character will be multiplied. Added to the height on each side to get the height on which we want to search neighbor (to not search just in the height, but a little more)
    static constexpr float COEFF_HEIGHT_MARGIN = 0.35f;

    //Minimum of width that the pattern can take (considering also the marge)
    static constexpr int MIN_WIDTH = 5;

    //Percentage of character's width that the minimum width of pattern will take
    static constexpr int MIN_WIDTH_PRC = 10;

    //Percentage of character's width that the maximum width of pattern will take
    static constexpr int MAX_WIDTH_PRC = 30;

    //Minimum of height that the pattern can take
    static constexpr int MIN_HEIGHT = 5;

    //#define SAVE_DEGRADATIONS_IMAGE 1  //to save debug image of degradations

#ifdef SAVE_DEGRADATIONS_IMAGE
#include <opencv2/highgui/highgui.hpp>
#endif //SAVE_DEGRADATIONS_IMAGE


    /**
       Compute (axis-aligned) bounding box of connected component @a cc.

       Return minX==INT_MAX && minY==INT_MAX if @a cc is empty.
    */
    static void
    computeRect(const CC &cc, int &minX, int &maxX, int &minY, int &maxY)
    {
      const size_t s = cc.size();
      if (s == 0) {
	minX = INT_MAX;
	maxX = 0;
	minY = INT_MAX;
	maxY = 0;
	return;
      }

      minX = cc[0].x;
      maxX = cc[0].x;
      minY = cc[0].y;
      maxY = cc[0].y;
      for (size_t i = 1; i < s; ++i) { //start from 1
	const cv::Point tmp = cc[i];

	if (minX > tmp.x)
	  minX = tmp.x;
	else if (maxX < tmp.x)
	  maxX = tmp.x;

	if (minY > tmp.y)
	  minY = tmp.y;
	else if (maxY < tmp.y)
	  maxY = tmp.y;
      }

      assert(minX <= maxX && minY <= maxY);
    }

    static void
    computeSize(const CC &cc, int &width, int &height)
    {
      int minX, maxX, minY, maxY;

      computeRect(cc, minX, maxX, minY, maxY);

      width = maxX - minX + 1;
      height = maxY - minY + 1;
    }

    /**
       Compute minimal distance from point (@a xOrigin, @a yOrigin) to non-black pixel in @a pattern.

       Search in four directions.
    */
    static
    int
    getDistanceBorder(const cv::Mat &pattern, int xOrigin, int yOrigin)
    {
      int minDist = INT_MAX;

      assert(pattern.type() == CV_8UC1);

      constexpr uchar BLACK = 0;

      //search toward right
      const uchar *p = pattern.ptr<uchar>(yOrigin);
      int d = 0;
      for (int x = xOrigin; x < pattern.cols; ++x) {
	if (p[x] != BLACK) {
	  break;
	}
	++d;
      }
      if (d < minDist) {
	minDist = d;
      }

      //search toward left
      d = 0;
      const int x0 = std::max(0, xOrigin - minDist);
      for (int x = xOrigin; x >= x0; --x) {
	if (p[x] != BLACK) {
	  break;
	}
	++d;
      }
      if (d < minDist) {
	minDist = d;
      }

      if (minDist == 0) { //early out : return if already 0.
	return minDist;
      }

      //search toward bottom
      d = 0;
      const int y1 = std::min(yOrigin + minDist, pattern.rows);
      for (int y = yOrigin; y < y1; ++y) {
	p = pattern.ptr<uchar>(y);
	if (p[xOrigin] != BLACK) {
	  break;
	}
	++d;
      }
      if (d < minDist) {
	minDist = d;
      }

      //search toward top
      d = 0;
      const int y0 = std::max(yOrigin - minDist, 0);
      for (int y = yOrigin; y >= y0; --y) {
	p = pattern.ptr<uchar>(y);
	if (p[xOrigin] != BLACK) {
	  break;
	}
	++d;
      }
      if (d < minDist) {
	minDist = d;
      }

      return minDist;

      /*for (int side = 0; side < 4; ++side) //to search on the four different side of pixel
	{
	bool found = false;
	int x = xOrigin;
	int y = yOrigin;
	int dist = 0;
	while (! found && x > 0 && x < pattern.cols && y > 0 && y < pattern.rows)
        {
	if (pattern.at<cv::Vec3b>(y, x) != cv::Vec3b(0, 0, 0))
	{
	found = true;
	break; //Not essential but it's not necessary to continue
	}
	else
	++dist;

	if (side == 0) //search toward top
	--y;
	else if (side == 1) //search toward right
	++x;
	else if (side == 2) //search toward bottom
	++y;
	else if (side == 3) //search toward left
	--x;
        }

	if (dist < minDist)
        minDist = dist;
	}

	return minDist;
      */
    }

    template <typename V>
    V
    makeGradient(V oldPixel, V newPixel, int distance)
    {
      if (distance > 1) {
	return newPixel;
      }

      constexpr float alpha = 0.3f;
      constexpr float oneMinusAlpha = 1.f - alpha;

      V resPixel = oldPixel; //import for V=cv::Vec4
      resPixel[0] =
	cv::saturate_cast<uchar>(alpha * oldPixel[0] + oneMinusAlpha * newPixel[0]);
      resPixel[1] =
	cv::saturate_cast<uchar>(alpha * oldPixel[1] + oneMinusAlpha * newPixel[1]);
      resPixel[2] =
	cv::saturate_cast<uchar>(alpha * oldPixel[2] + oneMinusAlpha * newPixel[2]);

      return resPixel;
    }

    template <>
    uchar
    makeGradient<uchar>(uchar oldPixel, uchar newPixel, int distance)
    {
      if (distance > 1) {
	return newPixel;
      }

      constexpr float alpha = 0.3f;
      constexpr float oneMinusAlpha = 1.f - alpha;

      const uchar resPixel = 
	cv::saturate_cast<uchar>(alpha * oldPixel + oneMinusAlpha * newPixel);

      return resPixel;
    }
    

    
    /**
     * Compute (axis-aligned) bounding box of black pixels in @a mat.
     */
    static void
    computeBoundingBox(const cv::Mat &mat,
		       int &minX,
		       int &maxX,
		       int &minY,
		       int &maxY)
    {
      assert(mat.type() == CV_8UC1 || mat.type() == CV_8UC3);

      minX = INT_MAX;
      minY = INT_MAX;
      maxX = 0;
      maxY = 0;

      const int w = mat.cols;
      const int h = mat.rows;

      if (mat.type() == CV_8UC3) {
	const cv::Vec3b BLACK(0, 0, 0);
	for (int y = 0; y < h; ++y) {
	  const cv::Vec3b *src = mat.ptr<cv::Vec3b>(y);
	  for (int x = 0; x < w; ++x) {
	    if (src[x] == BLACK) {
	      if (y > maxY) {
		maxY = y;
	      }
	      if (y < minY) {
		minY = y;
	      }
	      if (x > maxX) {
		maxX = x;
	      }
	      if (x < minX) {
		minX = x;
	      }
	    }
	  }
	}
      }
      else {
	assert(mat.type() == CV_8UC1);
	constexpr unsigned char BLACK = 0;
	for (int y = 0; y < h; ++y) {
	  const unsigned char *src = mat.ptr<unsigned char>(y);
	  for (int x = 0; x < w; ++x) {
	    if (src[x] == BLACK) {
	      if (y > maxY) {
		maxY = y;
	      }
	      if (y < minY) {
		minY = y;
	      }
	      if (x > maxX) {
		maxX = x;
	      }
	      if (x < minX) {
		minX = x;
	      }
	    }
	  }
	}
      }
    }

    /*
      Check whether image @a m1 is included in image @a m2.
      Consider both images are binarized.
    */
    static bool
    checkIncluded(const cv::Mat &m1, const cv::Mat &m2)
    {
      assert(m1.size() == m2.size());
      assert(m1.type() == CV_8UC1 || m1.type() == CV_8UC3);
      assert(m2.type() == CV_8UC1 || m2.type() == CV_8UC3);

      //For now handle only when both matrices have the same type.
      assert(m1.type() == m2.type());

      const int w = m1.cols;
      const int h = m1.rows;

      if (m1.type() == CV_8UC3) {
	const cv::Vec3b BLACK(0, 0, 0);
	assert(m1.type() == m2.type());
	for (int i = 0; i < h; ++i) {
	  const cv::Vec3b *src1 = m1.ptr<cv::Vec3b>(i);
	  const cv::Vec3b *src2 = m2.ptr<cv::Vec3b>(i);
	  for (int j = 0; j < w; ++j) {
	    if (src1[j] == BLACK && src2[j] != BLACK) {
	      return false;
	    }
	  }
	}
      }
      else {
	assert(m1.type() == CV_8UC1);
	assert(m1.type() == m2.type());
	constexpr unsigned char BLACK = 0;
	for (int i = 0; i < h; ++i) {
	  const unsigned char *src1 = m1.ptr<unsigned char>(i);
	  const unsigned char *src2 = m2.ptr<unsigned char>(i);
	  for (int j = 0; j < w; ++j) {
	    if (src1[j] == BLACK && src2[j] != BLACK) {
	      return false;
	    }
	  }
	}
      }
      return true;
    }

    static const cv::Point INVALID_POINT(-1, -1);

    /**
       Check if connected component @a charToTest from binary image @a matBin contains the pattern @a matPattern.

       @minX, @a maxX, @a minY, @a maxY is the (axis-aligned) bounding box of CC @a charToTest

       @return position of found pattern, or INVALID_POINT otherwise.

       @Warning we check only the (axis-aligned) bounding box of the CC and not the CC itself !
    */
    static
    cv::Point
    contains(const cv::Mat &matBin,
	     const CC & /*charToTest*/,
	     int minX,
	     int maxX,
	     int minY,
	     int maxY,
	     const cv::Mat &matPattern)
    {

      const int width = maxX - minX + 1;
      const int height = maxY - minY + 1;

      int p_minX, p_maxX, p_minY, p_maxY;
      computeBoundingBox(matPattern, p_minX, p_maxX, p_minY, p_maxY);
      const int p_width = p_maxX - p_minX + 1;
      const int p_height = p_maxY - p_minY + 1;

      if (p_width > width || p_height > height) {
	return INVALID_POINT;
      }
      if (p_minX == INT_MAX || p_maxX == 0 || p_minY == INT_MAX || p_maxY == 0) {
	return INVALID_POINT;
      }

      assert(p_minX + p_width <= matPattern.cols);
      assert(p_minY + p_height <= matPattern.rows);

      cv::Mat p_mat = matPattern(cv::Rect(p_minX, p_minY, p_width, p_height));
      if (p_mat.type() == CV_8UC3) {
	cvtColor(p_mat, p_mat, cv::COLOR_BGR2GRAY); //TO get a CV_8UC1 type
      }
      else if (p_mat.type() == CV_8UC4) {
	cvtColor(p_mat, p_mat, cv::COLOR_BGRA2GRAY); //TO get a CV_8UC1 type
      }
      assert(p_mat.type() == CV_8UC1);
      
      const cv::Vec3b BLACK(0, 0, 0);

      assert(p_mat.cols <= maxX + 1);
      assert(p_mat.rows <= maxY + 1);

      const int y1 = maxY + 1 - p_mat.rows;
      const int x1 = maxX + 1 - p_mat.cols;

      for (int y = minY; y < y1; ++y) {
	assert(y < matBin.rows);

	for (int x = minX; x < x1; ++x) {
	  assert(x < matBin.cols);

	  //ROI
	  assert(x + p_width <= maxX + 1);
	  assert(y + p_height <= maxY + 1);
	  cv::Mat bin_mat = matBin(cv::Rect(x, y, p_width, p_height));

	  if (checkIncluded(p_mat, bin_mat)) {
	    return cv::Point(x, y);
	  }
	}
      }

      return INVALID_POINT;
    }

    /**
       Check if connected component @a charToTest from binary image @a matBin contains the pattern @a matPattern.

    */
    static cv::Point
    contains(const cv::Mat &matBin, const CC &charToTest, const cv::Mat &matPattern)
    {
      int minX, maxX, minY, maxY;
      computeRect(charToTest, minX, maxX, minY, maxY);

      return contains(matBin, charToTest, minX, maxX, minY, maxY, matPattern);
    }

    /*
      Allow to binarize our image.
      We made a specific function here (whereas a function binarize exists in convertor.cpp)
      to be able to select the threshold that we want, and make the differents operations that we want, like erode the matrice for example
    */
    static cv::Mat
    binarize(const cv::Mat &mat, double threshold = 127)
    {
      assert(mat.channels() == 1 || mat.channels() == 3 || mat.channels() == 4);
      
      cv::Mat mat_gray;

      const int numChannels = mat.channels();
      if (numChannels == 3) {
	cvtColor(mat, mat_gray, cv::COLOR_BGR2GRAY);
      }
      else if (numChannels == 4) {
	cvtColor(mat, mat_gray, cv::COLOR_BGRA2GRAY);
      }
      else if (numChannels == 1) {
	mat_gray = mat.clone();
      }

      cv::threshold(mat_gray, mat_gray, threshold, 255, cv::THRESH_BINARY);

      /*
      //try to erode :
      int erosion_size = 1;
      cv::Mat element = cv::getStructuringElement(
						  cv::MORPH_RECT,
						  cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
						  cv::Point(
							    erosion_size,
							    erosion_size)); //http://docs.opencv.org/doc/tutorials/imgproc/erosion_dilatation/erosion_dilatation.html
      //cv::erode(mat_gray, mat_gray, element);
      //cv::dilate(mat_gray, mat_gray, element);
      //cv::dilate(mat_gray, mat_gray, element);
      */

      return mat_gray;
    }

    static int
    getXNearestNeighbor(cv::Mat &outputBin,
			int xOrigin,
			int yOrigin,
			int height,
			bool leftNeighbor)
    {
      assert(outputBin.type() == CV_8UC1);

      //Take a slightly greater height
      int yMax = yOrigin + height;
      if (yOrigin - (height * COEFF_HEIGHT_MARGIN) >= 1) {
	yOrigin -= (height * COEFF_HEIGHT_MARGIN); //Start a little higher
      }
      if (yMax + (height * COEFF_HEIGHT_MARGIN) < outputBin.rows) {
	yMax += (height * COEFF_HEIGHT_MARGIN); //Search a little lower
      }
      int minX = SPACING_MAX;
      unsigned char currentPix = 0;

      for (int y = yOrigin; y < yMax; ++y) {
	int x = 1;
	bool found = false;
	while (!found && x <= minX) {
	  currentPix = 0;
	  if (leftNeighbor) {
	    if (xOrigin - x > 0) {
	      currentPix = outputBin.at<unsigned char>(y, xOrigin - x);
	    }
	  }
	  else {
	    if (xOrigin + x < outputBin.cols) {
	      currentPix = outputBin.at<unsigned char>(y, xOrigin + x);
	    }
	  }
	  if (currentPix == 0) {
	    found = true;
	    if (minX > x) {
	      minX = x;
	    }
	  }
	  ++x;
	}
      }
      if (minX == SPACING_MAX) {
	return INT_MAX;
      }
      //else
      if (leftNeighbor) {
	return xOrigin - minX;
      }
      //else
      return xOrigin + minX;
    }


    //TO COPY BLUR
    /*
      copy @a src image block at point (@a origin_x, @a origin_y) in @a dst image.

    */

    template <typename V>
    void
    copyTo(const cv::Mat &src, cv::Mat &dst, int origin_x, int origin_y)
    {
      assert(dst.rows >= origin_y + src.rows);
      assert(dst.cols >= origin_x + src.cols);
      assert(src.type() == dst.type());
      assert(src.type() == CV_8UC1 || src.type() == CV_8UC3 || src.type() == CV_8UC4);

      const int bw = src.cols;
      const int bh = src.rows;

      for (int y = 0; y < bh; ++y) {
	V *d = dst.ptr<V>(origin_y + y) + origin_x;
	const V *s = src.ptr<V>(y);
	for (int x = 0; x < bw; ++x) {
	  d[x] = s[x];
	}
      }
    }
    
    void
    copyTo(const cv::Mat &src, cv::Mat &dst, int origin_x, int origin_y)
    {
      assert(src.type() == dst.type());
      if (src.type() == CV_8UC3) {
	copyTo<cv::Vec3b>(src, dst, origin_x, origin_y);
      }
      else if (src.type() == CV_8UC1) {
	copyTo<uchar>(src, dst, origin_x, origin_y);
      }
      else if (src.type() == CV_8UC4) {
	copyTo<cv::Vec4b>(src, dst, origin_x, origin_y);
      }
    }

    static int
    boundedRand(int minV, int maxV)
    {
      const int r = static_cast<int>(
				     minV + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (maxV - minV + 1)));
      assert(minV <= r && r <= maxV);
      return r;
    }

    template <typename V>
    void
    transferPattern(const cv::Mat &pattern,
		    const cv::Point dstPoint,
		    const cv::Point srcPoint,
		    cv::Mat &output
#ifdef SAVE_DEGRADATIONS_IMAGE
		    , cv::Mat &degrads
#endif //SAVE_DEGRADATIONS_IMAGE
		    )
    {
      const V BLACK(0, 0, 0);

      const int p_w = std::min(pattern.cols, output.cols - dstPoint.x);
      const int p_h = std::min(pattern.rows, output.rows - dstPoint.y);

      for (int y = 0; y < p_h; ++y) {
	assert(dstPoint.y + y >= 0 && dstPoint.y + y < output.rows);
	assert(srcPoint.y + y >= 0 && srcPoint.y + y < output.rows);
#ifdef SAVE_DEGRADATIONS_IMAGE
	assert(dstPoint.y + y >= 0 && dstPoint.y + y < degrads.rows);
#endif //SAVE_DEGRADATIONS_IMAGE

	const V *p = pattern.ptr<V>(y);
	const V *s = output.ptr<V>(srcPoint.y + y);
	V *d = output.ptr<V>(dstPoint.y + y);
#ifdef SAVE_DEGRADATIONS_IMAGE
	V *dg = degrads.ptr<V>(dstPoint.y + y);
#endif //SAVE_DEGRADATIONS_IMAGE

	for (int x = 0; x < p_w; ++x) {
	  assert(dstPoint.x + x >= 0 && dstPoint.x + x < output.cols);
	  assert(srcPoint.x + x >= 0 && srcPoint.x + x < output.cols);
#ifdef SAVE_DEGRADATIONS_IMAGE
	  assert(dstPoint.x + x >= 0 && dstPoint.x + x < degrads.cols);
#endif //SAVE_DEGRADATIONS_IMAGE

	  if (p[x] == BLACK) {
	    const V oldPixel = d[dstPoint.x + x];
	    // = output.at<V>(y+dstPoint.y, x+dstPoint.x)
	    const V newPixel = s[srcPoint.x + x];
	    // = output.at<V>(y+srcPoint.y, x+srcPoint.x)

	    const uchar oldGray = (oldPixel[0] + oldPixel[1] + oldPixel[2]) / 3;
	    const uchar newGray = (newPixel[0] + newPixel[1] + newPixel[2]) / 3;
	    if (oldGray > newGray) {
	      d[dstPoint.x + x] =
		makeGradient(oldPixel, newPixel, getDistanceBorder(pattern, x, y));
#ifdef SAVE_DEGRADATIONS_IMAGE
	      dg[dstPoint.x + x] = BLACK;
#endif //SAVE_DEGRADATIONS_IMAGE
	    }
	  }
	}
      }
    }

    template <>
    void
    transferPattern<uchar>(const cv::Mat &pattern,
			   const cv::Point dstPoint,
			   const cv::Point srcPoint,
			   cv::Mat &output
#ifdef SAVE_DEGRADATIONS_IMAGE
			   , cv::Mat &degrads
#endif //SAVE_DEGRADATIONS_IMAGE
		    )
    {
      constexpr uchar BLACK = 0;

      const int p_w = std::min(pattern.cols, output.cols - dstPoint.x);
      const int p_h = std::min(pattern.rows, output.rows - dstPoint.y);

      for (int y = 0; y < p_h; ++y) {
	assert(dstPoint.y + y >= 0 && dstPoint.y + y < output.rows);
	assert(srcPoint.y + y >= 0 && srcPoint.y + y < output.rows);
#ifdef SAVE_DEGRADATIONS_IMAGE
	assert(dstPoint.y + y >= 0 && dstPoint.y + y < degrads.rows);
#endif //SAVE_DEGRADATIONS_IMAGE

	const uchar *p = pattern.ptr<uchar>(y);
	const uchar *s = output.ptr<uchar>(srcPoint.y + y);
	uchar *d = output.ptr<uchar>(dstPoint.y + y);
#ifdef SAVE_DEGRADATIONS_IMAGE
	uchar *dg = degrads.ptr<uchar>(dstPoint.y + y);
#endif //SAVE_DEGRADATIONS_IMAGE

	for (int x = 0; x < p_w; ++x) {
	  assert(dstPoint.x + x >= 0 && dstPoint.x + x < output.cols);
	  assert(srcPoint.x + x >= 0 && srcPoint.x + x < output.cols);
#ifdef SAVE_DEGRADATIONS_IMAGE
	  assert(dstPoint.x + x >= 0 && dstPoint.x + x < degrads.cols);
#endif //SAVE_DEGRADATIONS_IMAGE

	  if (p[x] == BLACK) {
	    const uchar oldPixel = d[dstPoint.x + x];
	    // = output.at<uchar>(y+dstPoint.y, x+dstPoint.x)
	    const uchar newPixel = s[srcPoint.x + x];
	    // = output.at<uchar>(y+srcPoint.y, x+srcPoint.x)

	    const uchar oldGray = oldPixel;
	    const uchar newGray = newPixel;
	    if (oldGray > newGray) {
	      d[dstPoint.x + x] =
		makeGradient(oldPixel, newPixel, getDistanceBorder(pattern, x, y));
#ifdef SAVE_DEGRADATIONS_IMAGE
	      dg[dstPoint.x + x] = BLACK;
#endif //SAVE_DEGRADATIONS_IMAGE
	    }
	  }
	}
      }
    }
    
      
    /**
       Copy pixels from @a srcPoint to @a dstPoint in @a output
       when pixels are black in pattern @a pattern.

    */
    static
    void
    transferPattern(const cv::Mat &pattern,
		    const cv::Point dstPoint,
		    const cv::Point srcPoint,
		    cv::Mat &output
#ifdef SAVE_DEGRADATIONS_IMAGE
		    ,
		    cv::Mat &degrads
#endif //SAVE_DEGRADATIONS_IMAGE
		    )
    {
      assert(pattern.type() == CV_8UC1);
      assert(output.type() == CV_8UC1 || output.type() == CV_8UC3 || output.type() == CV_8UC4);
#ifdef SAVE_DEGRADATIONS_IMAGE
      assert(degrads.type() == output.type());
#endif //SAVE_DEGRADATIONS_IMAGE

      assert(dstPoint.x < output.cols && dstPoint.y < output.rows);
      assert(srcPoint.x < output.cols && srcPoint.y < output.rows);
      assert(srcPoint.x + pattern.cols <= output.cols &&
	     srcPoint.y + pattern.rows <= output.rows);


      if (output.type() == CV_8UC3) {
	transferPattern<cv::Vec3b>(pattern, dstPoint, srcPoint, output
#ifdef SAVE_DEGRADATIONS_IMAGE
				  , cv::Mat &degrads
#endif //SAVE_DEGRADATIONS_IMAGE
				  );
      }  
      else if (output.type() == CV_8UC4) {
	transferPattern<cv::Vec4b>(pattern, dstPoint, srcPoint, output
#ifdef SAVE_DEGRADATIONS_IMAGE
				  , cv::Mat &degrads
#endif //SAVE_DEGRADATIONS_IMAGE
				  );
      }				   
      else if (output.type() == CV_8UC1) {
	transferPattern<uchar>(pattern, dstPoint, srcPoint, output
#ifdef SAVE_DEGRADATIONS_IMAGE
				  , cv::Mat &degrads
#endif //SAVE_DEGRADATIONS_IMAGE
				  );
      }				   

    }

    /**
     * @param[in,out] output
     */
    static bool
    degradeComposant(cv::Mat &output,
		     const CC &degradedCC,
		     const CCs &ccs,
		     int posDegraded
#ifdef SAVE_DEGRADATIONS_IMAGE
		     ,
		     cv::Mat &degrads
#endif //SAVE_DEGRADATIONS_IMAGE
		     , 
		     const std::string &phantomPatternsPath
		     )
    {
      int minX, maxX, minY, maxY;
      computeRect(degradedCC, minX, maxX, minY, maxY);
      const int width = maxX - minX + 1;
      const int height = maxY - minY + 1;
      bool applied = false;

      cv::Mat outputBin = binarize(output);
      assert(outputBin.type() == CV_8UC1);

      const int maxXLeft = getXNearestNeighbor(outputBin, minX, minY, height, true);
      const int minXRight =
	getXNearestNeighbor(outputBin, maxX, minY, height, false);

      assert(maxXLeft == INT_MAX || maxXLeft <= minX);
      assert(minXRight == INT_MAX || minXRight >= maxX);

      const bool leftCharFound = (maxXLeft != INT_MAX);
      const bool rightCharFound = (minXRight != INT_MAX);

      int firstSide = 0, lastSide = 1; //to define which side we have to work on

      if ((leftCharFound && minX - maxXLeft < SPACING_MIN) || height == 0) {
	firstSide = 1;
      }
      if ((rightCharFound && minXRight - maxX < SPACING_MIN) || height == 0) {
	lastSide = 0;
      }

      cv::Mat patternMat;
      cv::Point origin = INVALID_POINT;

      //to be sure to not make a degradation lighter than the old pixel
      //cv::Vec3b oldPixel, newPixel;

      if (firstSide == 1 && lastSide == 0) {
	return false;
      }

      const std::vector<std::string> patterns = dc::listDirectory(phantomPatternsPath);
      if (patterns.empty()) {
	return false;
      }
      const int numPatterns = static_cast<int>(patterns.size());

      for (int side = firstSide; side <= lastSide; ++side) { //0 = left, 1 = right

	const int apply = rand() % 2;
	if (apply == 1) { // apply degradation on this side or not

	  const int yPattern = minY;

	  const int pattern = rand() % numPatterns; //choice of pattern
	  assert((size_t)pattern < patterns.size());
	  const std::string patternFilename = makePath(phantomPatternsPath, patterns.at(pattern));
#if CV_MAJOR_VERSION < 4
	  const int grayFlag = CV_LOAD_IMAGE_GRAYSCALE;
#else
	  constexpr int grayFlag = cv::IMREAD_GRAYSCALE;
#endif
	  patternMat = cv::imread(patternFilename, grayFlag);
	  assert(!patternMat.empty());
     
	  int widthPattern=0, maxWidth=0, minWidth=0;
	  int heightPattern=0, minHeight=0, maxHeight=0;

	  if (side == 0 && leftCharFound) {
	    maxWidth = (minX - maxXLeft) / 2;
	  }
	  else if (side == 1 && rightCharFound) {
	    maxWidth = (minXRight - maxX) / 2;
	  }
	  else {
	    maxWidth = (width * MAX_WIDTH_PRC) / 100;
	  }

	  minWidth = (width * MIN_WIDTH_PRC) / 100;

	  if (maxWidth <= minWidth) {
	    maxWidth = minWidth + 1;
	  }

	  widthPattern = boundedRand(
				     minWidth, maxWidth); //rand()% (maxWidth - minWidth) + minWidth ;

	  minHeight = height * COEFF_MIN_HEIGHT;
	  maxHeight = height * COEFF_MAX_HEIGHT;

	  heightPattern = boundedRand(
				      minHeight, maxHeight); //rand()% (maxHeight - minHeight) + minHeight;

	  if (widthPattern == 0) {
	    widthPattern = minWidth;
	  }
	  if (heightPattern == 0) {
	    heightPattern = MIN_HEIGHT;
	  }
	  if (widthPattern < MIN_WIDTH) {
	    widthPattern = MIN_WIDTH;
	  }

	  resize(patternMat, patternMat, cv::Size(widthPattern, heightPattern));

	  origin =
	    contains(outputBin, degradedCC, minX, maxX, minY, maxY, patternMat);

	  int pos = 0;
	  bool minReach = false, maxReach = false;

	  /*
	    Search in extracted characters where we can copy the pattern we have chosen
	    We search in first place around the character that we want degrade because we estimate there color will be closest to the character's color that we degraded as the distant character'color. (If there are changements of color in the document, there is more chance that it is away). We supposed that the closest CC in vector are the closest character on document (it's not exactly like that).
	    origin == INVALID_POINT -> We continue while we have don't found
	    (!minReach || !maxReach) -> We stop when we have reached the begin and the end of the ccs (so when we have try with all the characters)
	  */
	  while (origin == INVALID_POINT && (!minReach || !maxReach)) {
	    if (!maxReach) {
	      origin = contains(outputBin, ccs[posDegraded + pos], patternMat);
	    }
	    if (origin != INVALID_POINT && !minReach) {
	      origin = contains(outputBin, ccs[posDegraded - pos], patternMat);
	    }
	    ++pos;
	    if (posDegraded + pos >= static_cast<int>(ccs.size())) {
	      maxReach = true;
	    }
	    if (posDegraded - pos < 0) {
	      minReach = true;
	    }
	  }

	  /*RESIZE
	    origin = cv::Point(1, 0);//to be sure that we will enter one time in the while

	    int i=0;
	    //To try with a smaller size with we don't find where copy pattern

	    cv::Point lastOrigin = cv::Point(0, 0);//to be sure lastOrigin is different than (0, 0) at the start
	    cv::Mat patternMatResized = patternMat.clone();

	    while (origin != cv::Point(0,0) && i<NB_TRY_FIT)
	    {
	    resize(patternMat, patternMatResized, cv::Size(widthPattern+i, heightPattern+i));
	    int pos = 1;
	    bool minReach = false, maxReach=false;

	    origin = contains(output, ccs[posDegraded], patternMatResized); //search in character that we degrade

	    while (origin == cv::Point(0,0) && (!minReach || !maxReach) )//&& it!=sortedBySize.begin()+nb_char_tested);
	    {
	    if (!maxReach && ccs[posDegraded] != ccs[ccs.size()-1])
	    origin = contains(output, ccs[posDegraded+pos], patternMatResized);

	    if (origin != cv::Point(0, 0) && !minReach && ccs[posDegraded] != ccs[0])
	    origin = contains(output, ccs[posDegraded-pos], patternMatResized);

	    ++pos;
	    if (posDegraded+pos >= ccs.size())
	    maxReach = true;
	    if (posDegraded-pos < 0)
	    minReach = true;
	    }

	    if (origin != cv::Point(0, 0))
	    lastOrigin = origin;        //save the last Point where we could copy

	    ++i;
	    }

	    origin = lastOrigin;
	    i--; //we select the last round where we had found a character in which copy
	    resize(patternMat, patternMat, cv::Size(widthPattern+i, heightPattern+i)); //resize to the bigger size we could

	    // TO REMOVE : Test resize
	    QImage tmp = Convertor::getQImage(patternMat);
	    tmp.save("testPattern.png");*/

	  if (origin !=
	      INVALID_POINT) { //If we found a place where copy, we can apply

	    applied = true;
	    int xMinPattern = 0, xMaxPattern = 0;

	    if (side == 0) { //LEFT

	      if (leftCharFound) {
		xMinPattern = ((minX + maxXLeft) / 2);
		//The center between the character and his neighbor
	      }
	      else {
		xMinPattern = minX - MAX_MARGIN;
		//if there is not neighbor, we take a fixed maximum width between character and degradation
	      }
	      
	      if (xMinPattern < 0) {
		xMinPattern = 0; //to not go out the document
	      }
	      
	      xMaxPattern = minX - MIN_MARGIN; // we take a fixed minimum width
	      // between character and its
	      // degradation

	      if (xMinPattern >= xMaxPattern) {
		xMaxPattern = xMinPattern + 1;
		//to be sure that max > min
		// (+1 to shift toward right and don't go out the document)
	      }
	      
	    }
	    else if (side == 1) { //RIGHT

	      if (rightCharFound) {
		xMaxPattern = ((maxX + minXRight) / 2) - (patternMat.cols / 2);
		//The center between the character and his neighbor
		// (and shift to be symetric with the left degradation)
	      }
	      else {
		xMaxPattern = maxX + MAX_MARGIN - patternMat.cols;
		//if there is not neighbor, we take a fixed maximum (and shift to be symetric with the left degradation)
	      }
	      
	      if (xMaxPattern > output.cols) {
		xMaxPattern = output.cols; //to not go out the document
	      }
	      
	      xMinPattern = maxX + MIN_MARGIN; // we take a fixed minimum width
	      // between character and its
	      // degradation

	      if (xMinPattern >= xMaxPattern) {
		xMinPattern = xMaxPattern - 1;
		//to be sure that max > min
		// (-1 to shift toward left and don't go out the document)
	      }
	    }

	    const int xPattern = boundedRand(
					     xMinPattern,
					     xMaxPattern); //(int) (rand()%(xMaxPattern - xMinPattern) +  xMinPattern); //random

	    transferPattern(patternMat,
			    cv::Point(xPattern, yPattern),
			    origin,
			    output
#ifdef SAVE_DEGRADATIONS_IMAGE
			    ,
			    degrads
#endif //SAVE_DEGRADATIONS_IMAGE
			    );

	    //APPLY BLUR
	    widthPattern = std::min(
				    widthPattern,
				    output.cols - xPattern); //to be sure to not copy outside the mat
	    heightPattern = std::min(heightPattern, output.rows - yPattern);
	    cv::Mat boundingBox = cv::Mat(output,
					  cv::Rect(xPattern, yPattern, widthPattern, heightPattern));

	    cv::Mat boundingBoxBlur = dc::BlurFilter::blur(boundingBox, dc::BlurFilter::Method::GAUSSIAN, 3);
	    copyTo(boundingBoxBlur, output, xPattern, yPattern);
	  }
	}
      }

      return applied;
    }

    cv::Mat
    phantomCharacter(const cv::Mat &img, Frequency frequency, const std::string &phantomPatternsPath)
    {
      cv::Mat output = img.clone();

      CCs ccs;
      cv::Mat imgBin = binarize(img);
      ConnectedComponent::extractAllConnectedComponents(imgBin, ccs, 4);

      if (ccs.empty()) {
	return output;
      }

      const size_t sz = ccs.size();
      std::vector<size_t> areas(sz, 0);

      //compute average & standard deviation (via moments) of ccs' AABB area and fill areas;
      const size_t m0 = ccs.size();
      size_t m1 = 0, m2 = 0;

      for (size_t i = 0; i < sz; ++i) {
	int width, height;
	computeSize(ccs[i], width, height);
	const size_t areaAABB = width * static_cast<size_t>(height);

	areas[i] = areaAABB;
	m1 += areaAABB;
	m2 += areaAABB * areaAABB;
      }
      assert(m0 > 0);
      const float inv_m0 = 1.f / m0;
      const float avg = m1 * inv_m0;
      const float standard_deviation = std::sqrt((m2 - m1 * m1 * inv_m0) * inv_m0);

      //Filtering of ccs
      constexpr float factor = 1.f;
      const float lowTh = avg * 0.6f;
      const float highTh = avg + (factor * standard_deviation);
      CCs filteredCCs;
      filteredCCs.reserve(sz);
      for (size_t i = 0; i < sz; ++i) {
	const float areaAABB = static_cast<float>(areas[i]);
	if (areaAABB > lowTh && areaAABB <= highTh) {
	  filteredCCs.push_back(ccs[i]);
	}
      }
      std::swap(ccs, filteredCCs);
      //sz = ccs.size();

      //Create default
      srand(time(nullptr)); // initialization
      int probOccurence = 0;

      if (frequency == Frequency::RARE) {
	probOccurence = 15; // 15 %
      }
      else if (frequency == Frequency::FREQUENT) {
	probOccurence = 40;
      }
      else if (frequency == Frequency::VERY_FREQUENT) {
	probOccurence = 70;
      }

#ifdef SAVE_DEGRADATIONS_IMAGE
      cv::Mat degrads(img.rows,
		      img.cols,
		      img.type(),
		      cv::Scalar(255, 255, 255));
#endif //SAVE_DEGRADATIONS_IMAGE

      for (int unsigned i = 0; i < ccs.size(); ++i) {
	//Random : choose if we apply a default on this characters or not
	const int chosen = rand() % 100; // random number between 0 and 100

	if (chosen < probOccurence) { // if random number is included in the x % (x
	  // which was chosen by the user)
	  degradeComposant(output,
			   ccs[i],
			   ccs,
			   i
#ifdef SAVE_DEGRADATIONS_IMAGE
			   ,
			   degrads
#endif //SAVE_DEGRADATIONS_IMAGE
			   , phantomPatternsPath);
	}
      }

#ifdef SAVE_DEGRADATIONS_IMAGE
      const char *degradationsFilename = "/tmp/imageWhite.png";
      cv::imwrite(degradationsFilename, degrads);
      std::cerr << "wrote " << degradationsFilename << "\n";
#endif //SAVE_DEGRADATIONS_IMAGE

      return output;
    }

  } //namespace PhantomCharacter

} //namespace dc


