#include "BlurFilter.hpp"

#include <cassert>
#include <algorithm> //clamp
#include <cmath> //sqrt, log
#include <cstdint>
#include <cstdlib> //abs
#include <optional>
#include <vector>

#include <opencv2/imgproc/imgproc.hpp>

#include "Validate.hpp"


namespace dc {
  namespace BlurFilter {
  
    static constexpr int COEFF_PARABOLA = 4;

  
    static void
    calcSolutions(float a, float b, float discr, float &y1, float &y2)
    {
      y1 = (-b + sqrtf(discr)) / (2 * a);
      y2 = (-b - sqrtf(discr)) / (2 * a);
    }

    //B: the four curves below return their row as a float, and toRow()
    // makes it an int: they did the conversion themselves, which is
    // undefined for log(0), at the first column.

    static constexpr float
    fctLinear(int x, int rows, float coeff, int vertical)
    {
      return rows - (x * coeff + vertical);
    }

    static float
    fctLog(int x, int rows, float coeff, int vertical)
    {
      return rows - (logf(static_cast<float>(x)) * coeff + vertical);
    }

    static constexpr float
    fctParabola(int x, int rows, float coeff, int vertical, int horizontal)
    {
      return rows - ((coeff * (x + horizontal) * (x + horizontal)) +
		     (COEFF_PARABOLA * (x + horizontal)) + vertical);
    }

    static float
    fctSinus(int x, int rows, float coeff, int vertical, int horizontal)
    {
      return rows - (coeff * sinf((x + horizontal) / 200.f) + vertical);
    }

    /*
      The rows where a conic crosses a column: none, one or two.
    */
    struct Solutions
    {
      std::optional<float> first;
      std::optional<float> second;
    };

    static Solutions
    solveQuadratic(float a, float b, float c)
    {
      const float discr = b * b - (4.f * a * c);
      if (discr > 0) {
        float y1 = 0;
        float y2 = 0;
        calcSolutions(a, b, discr, y1, y2);
        return {y1, y2};
      }
      if (discr == 0) {
        return {-b / (2 * a), std::nullopt};
      }
      return {};
    }

    static Solutions
    fctEllipse(int x, int horizontal, int vertical, float coeff)
    {
      //Pattern : 0.2*x*x + 0.5*y*y + 0.27*x*y - 330.71*x - 554.29*y = -170351.99
      //others numbers are here to ajust the coefficient chosen to Resize the ellipse. (For example, the variable y is multiplied by 0.1 when the size is increased by one). These coefficients were computed for this pattern thanks to Geogebra
      constexpr float a = 0.5f;
      const float b = (-554.29f - (coeff * 0.1f)) + (0.27f * (x + horizontal));
      const float c = (170351.99f - (coeff * 34.61f)) +
	(0.2f * (x + horizontal) * (x + horizontal)) -
	((330.71f - (coeff * 0.03f)) * (x + horizontal));

      Solutions s = solveQuadratic(a, b, c);
      for (std::optional<float> *y : {&s.first, &s.second}) {
        if (*y) {
          **y += vertical;
        }
      }
      return s;
    }

    static Solutions
    fctHyperbola(int x, int rows, int horizontal, int vertical, float coeff)
    {
      //Pattern : 0.00801*x*x - 0.03214*y*y + 0.1146*x*y - 123.95975*x - 17.44726*y = -81903.06188
      //others numbers are here to ajust the coefficient chosen to Resize the hyperbola. (For example, the variable x*x is multiplied by 0.00012 when the size is increase by one). These coefficient were computed for this pattern thanks to Geogebra
      const float a = -0.03214f - (coeff * 0.00013f);
      const float b = (0.1146f * (x + horizontal)) + (-17.44726f + (coeff * 0.24361f));
      const float c =
	(81903.06188f + (coeff * 206.34517f)) +
	((0.00801f - (coeff * 0.00012f)) * (x + horizontal) * (x + horizontal)) -
	((123.95975f + (coeff * 0.17438f)) * (x + horizontal));

      Solutions s = solveQuadratic(a, b, c);
      for (std::optional<float> *y : {&s.first, &s.second}) {
        if (*y) {
          **y = rows - (**y + vertical);
        }
      }
      return s;
    }

    /*
      @return the row @a y truncated towards zero, as the curves always were,
      or nothing for a NaN.

      B: the value is clamped first: log(0), at the first column, is
      infinite, and converting an infinite or out of range float to int is
      undefined. Any row this far from the image compares with the rows of
      the image, give or take a radius, as the exact value would.
    */
    static std::optional<int>
    toRow(float y)
    {
      if (std::isnan(y)) {
        return std::nullopt;
      }
      constexpr float LIMIT = 1 << 24;
      return static_cast<int>(std::clamp(y, -LIMIT, LIMIT));
    }

    /*
      The rows where the curve @a fct crosses the column @a x: the
      function curves cross it once, the conics none, once or twice.

      B: this filled two int out-parameters, with -1 meaning "no row", and
      a curve really crossing the column at -1 was then taken for no
      crossing. Worse, the ellipse tested the pixel row it was given,
      instead of the solution, against -FLT_MAX: with no solution,
      -FLT_MAX was converted to an int, which is undefined, and gave
      INT_MIN on x86, from which isNearFunction() subtracted the radius.
      That pixel row was used for nothing else and is no longer passed.
    */
    struct CurveRows
    {
      std::optional<int> first;
      std::optional<int> second;
    };

    static CurveRows
    curveRows(Function fct,
	      int rows,
	      int x,
	      int vertical,
	      int horizontal,
	      float coeff)
    {
      switch (fct) {
      case Function::LINEAR:
	return {toRow(fctLinear(x, rows, coeff, vertical)), std::nullopt};

      case Function::LOG:
	return {toRow(fctLog(x, rows, coeff, vertical)), std::nullopt};

      case Function::PARABOLA:
	return {toRow(fctParabola(x, rows, coeff, vertical, horizontal)), std::nullopt};

      case Function::SINUS:
	return {toRow(fctSinus(x, rows, coeff, vertical, horizontal)), std::nullopt};

      case Function::ELLIPSE:
	{
	  const Solutions s = fctEllipse(x, horizontal, vertical, coeff);
	  CurveRows c{s.first ? toRow(*s.first) : std::nullopt,
		      s.second ? toRow(*s.second) : std::nullopt};
	  //the second row only counts when the first is in the image
	  if (!c.first || *c.first < 0 || *c.first > rows) {
	    c.second.reset();
	  }
	  return c;
	}

      case Function::HYPERBOLA:
	{
	  const Solutions s = fctHyperbola(x, rows, horizontal, vertical, coeff);
	  return {s.first ? toRow(*s.first) : std::nullopt,
		  s.second ? toRow(*s.second) : std::nullopt};
	}
      }
      return {};
    }

    static bool
    isNear(int y, std::optional<int> row, int radius)
    {
      //in 64 bits: neither the difference nor the radius can overflow
      return row && std::abs(static_cast<int64_t>(y) - *row) <= radius;
    }

    /*
      The rows of the curve in each of the @a cols columns.

      B: they were computed again for every pixel, though they depend only
      on the column.
    */
    static std::vector<CurveRows>
    curveRowsOfColumns(Function fct,
		       int rows,
		       int cols,
		       int vertical,
		       int horizontal,
		       float coeff)
    {
      std::vector<CurveRows> columns(cols);
      for (int x = 0; x < cols; ++x) {
	columns[x] = curveRows(fct, rows, x, vertical, horizontal, coeff);
      }
      return columns;
    }

    static bool
    isNearFunction(int y, const CurveRows &c, int radius)
    {
      //B: the first test also required y2Function != 1, a typo for -1
      // that made a curve whose second row is 1 never near its first one
      return isNear(y, c.first, radius) || isNear(y, c.second, radius);
    }

    template <typename V>
    cv::Mat
    degradateCenter(const cv::Mat &blurredMat,
		    cv::Mat &resultMat,
		    Function function,
		    int horizontal,
		    int vertical,
		    float coeff,
		    int radius)
    {
      const int rows = blurredMat.rows;
      const int cols = blurredMat.cols;

      const std::vector<CurveRows> columns =
	curveRowsOfColumns(function, rows, cols, vertical, horizontal, coeff);

      assert(resultMat.type() == blurredMat.type());
      assert(blurredMat.size() == resultMat.size());

      for (int y = 0; y < rows; ++y) {
	const V *b = blurredMat.ptr<V>(y);
	V *r = resultMat.ptr<V>(y);
	for (int x = 0; x < cols; ++x) {
	  if (isNearFunction(y, columns[x], radius)) { //Center
	    r[x] = b[x];
	  }
	  //else
	  //r[x] = o[x];
	}
      }

      return resultMat;
    }    
    
    static
    cv::Mat
    degradateCenter(const cv::Mat &blurredMat,
		    cv::Mat &resultMat,
		    Function function,
		    int horizontal,
		    int vertical,
		    float coeff,
		    int radius)
    {
      assert(resultMat.type() == blurredMat.type());
      assert(blurredMat.type() == CV_8UC3 || blurredMat.type() == CV_8UC4 || blurredMat.type() == CV_8UC1);

      if (blurredMat.type() == CV_8UC3) {
	return degradateCenter<cv::Vec3b>(blurredMat, resultMat, function, horizontal, vertical, coeff, radius);
      }
      else if (blurredMat.type() == CV_8UC4) {
	return degradateCenter<cv::Vec4b>(blurredMat, resultMat, function, horizontal, vertical, coeff, radius);
      }
      else if (blurredMat.type() == CV_8UC1) {
	return degradateCenter<uchar>(blurredMat, resultMat, function, horizontal, vertical, coeff, radius);
      }

      assert(false);
      return blurredMat;
    }

    static bool
    upperThan(Function fct, int y, const CurveRows &c)
    {
      if (fct == Function::ELLIPSE || fct == Function::HYPERBOLA) {
	if (!c.second) { //one solution, or none
	  return c.first && y == *c.first;
	}
	//two solutions
	return c.first && y < *c.first && y > *c.second;
      }

      return c.first && y > *c.first;
    }

    template <typename V>
    cv::Mat
    degradateBorder(const cv::Mat &blurredMat,
		    cv::Mat &resultMat,
		    Function function,
		    Area area,
		    int vertical,
		    float coeff,
		    int horizontal)
    {
      //TODO : Make it work with grayscale image too ?
      assert(blurredMat.type() == resultMat.type());
      assert(blurredMat.size() == resultMat.size());

      const int rows = resultMat.rows;
      const int cols = resultMat.cols;

      const std::vector<CurveRows> columns =
	curveRowsOfColumns(function, rows, cols, vertical, horizontal, coeff);

      if (area == Area::UP) {
	for (int y = 0; y < rows; ++y) {
	  const V *b = blurredMat.ptr<V>(y);
	  V *r = resultMat.ptr<V>(y);
	  for (int x = 0; x < cols; ++x) {
	    if (upperThan(function, y, columns[x])) {
	      r[x] = b[x];
	    }
	  }
	}
      }
      else if (area == Area::DOWN) {
	for (int y = 0; y < rows; ++y) {
	  const V *b = blurredMat.ptr<V>(y);
	  V *r = resultMat.ptr<V>(y);
	  for (int x = 0; x < cols; ++x) {
	    if (!upperThan(function, y, columns[x])) {
	      r[x] = b[x];
	    }
	  }
	}
      }
      /*
	else {
	assert(area == Area::CENTERED);

	//resultMat.at<cv::Vec3b>(y, x) = originalMat.at<cv::Vec3b>(y, x);
	}
      */

      return resultMat;
    }
    
    
    static
    cv::Mat
    degradateBorder(const cv::Mat &blurredMat,
		    cv::Mat &resultMat,
		    Function function,
		    Area area,
		    int vertical,
		    float coeff,
		    int horizontal)
    {
      //TODO : Make it work with grayscale image too ?
      assert(blurredMat.type() == CV_8UC3 || blurredMat.type() == CV_8UC4 || blurredMat.type() == CV_8UC1);
      assert(resultMat.type() == blurredMat.type());
      assert(blurredMat.size() == resultMat.size());

      if (blurredMat.type() == CV_8UC3) {
	return degradateBorder<cv::Vec3b>(blurredMat, resultMat,
					  function, area,
					  vertical, coeff, horizontal);
      }
      else if (blurredMat.type() == CV_8UC4) {
	return degradateBorder<cv::Vec4b>(blurredMat, resultMat,
					  function, area,
					  vertical, coeff, horizontal);
      }
      else if (blurredMat.type() == CV_8UC1) {
	return degradateBorder<uchar>(blurredMat, resultMat,
				      function, area,
				      vertical, coeff, horizontal);
      }
      assert(false);
      return blurredMat;
    }

    template <typename V>
    cv::Mat
    applyPattern(const cv::Mat &originalMat,
		 const cv::Mat &patternMat,
		 Method method,
		 int kernelSize)
    {
      assert(patternMat.type() == CV_8UC1);

      cv::Mat resultMat = originalMat.clone();
      assert(resultMat.type() == originalMat.type());
      assert(resultMat.size() == originalMat.size());

      cv::Mat blurredMat = blur(originalMat, method, kernelSize);
      assert(blurredMat.type() == originalMat.type());
      assert(blurredMat.size() == originalMat.size());

      //resize pattern to the size of original
      cv::Mat patternMatS;
      resize(patternMat, patternMatS, cv::Size(originalMat.cols, originalMat.rows));

      assert(patternMatS.type() == CV_8UC1);
      assert(patternMatS.size() == originalMat.size());

      int rows = resultMat.rows;
      int cols = resultMat.cols;
      if (patternMatS.isContinuous()
	  && resultMat.isContinuous()
	  && blurredMat.isContinuous()) {
	cols *= rows;
	rows = 1;
      }

      constexpr uchar BLACK = 0;
      
      for (int y = 0; y < rows; ++y) {
	const uchar *p = patternMatS.ptr<uchar>(y);
	const V *b = blurredMat.ptr<V>(y);
	V *r = resultMat.ptr<V>(y);

	for (int x = 0; x < cols; ++x) {
	  if (p[x] == BLACK) {
	    r[x] = b[x];
	  }
	}
      }

      return resultMat;
    }

    
    cv::Mat
    applyPattern(const cv::Mat &originalMat,
		 const cv::Mat &patternMat,
		 Method method,
		 int kernelSize)
    {
      static const char *func = "dc::BlurFilter::applyPattern";
      require8UC134(originalMat, "originalMat", func);
      require8UC1(patternMat, "patternMat", func);

      if (originalMat.type() == CV_8UC3) {
	return applyPattern<cv::Vec3b>(originalMat, patternMat,
				       method, kernelSize);
      }
      else if (originalMat.type() == CV_8UC4) {
	return applyPattern<cv::Vec4b>(originalMat, patternMat,
				       method, kernelSize);
      }
      else if (originalMat.type() == CV_8UC1) {
	return applyPattern<uchar>(originalMat, patternMat,
				   method, kernelSize);
      }
      
      assert(false);
      return originalMat;
    }

    cv::Mat
    makePattern(const cv::Mat &originalMat,
		Function function,
		Area area,
		float coeff,
		int vertical,
		int horizontal,
		int radius)
    {
      require8UC134(originalMat, "originalMat", "dc::BlurFilter::makePattern");

      cv::Mat resultMat =
	cv::Mat::zeros(originalMat.rows, originalMat.cols, CV_8UC1);

      constexpr uchar WHITE = 255;
      const int rows = originalMat.rows;
      const int cols = originalMat.cols;

      const std::vector<CurveRows> columns =
	curveRowsOfColumns(function, rows, cols, vertical, horizontal, coeff);

      if (area == Area::CENTERED) {
	for (int y = 0; y < rows; ++y) {
	  uchar *r = resultMat.ptr<uchar>(y);
	  for (int x = 0; x < cols; ++x) {
	    if (!isNearFunction(y, columns[x], radius)) { //Center
	      r[x] = WHITE;
	    }
	  }
	}
      }
      else if (area == Area::UP) {
	for (int y = 0; y < rows; ++y) {
	  uchar *r = resultMat.ptr<uchar>(y);
	  for (int x = 0; x < cols; ++x) {
	    if (!upperThan(function, y, columns[x])) {
	      r[x] = WHITE;
	    }
	  }
	}
      }
      else {
	assert(area == Area::DOWN);
	for (int y = 0; y < rows; ++y) {
	  uchar *r = resultMat.ptr<uchar>(y);
	  for (int x = 0; x < cols; ++x) {
	    if (upperThan(function, y, columns[x])) {
	      r[x] = WHITE;
	    }
	  }
	}
      }

      assert(resultMat.type() == CV_8UC1);
      assert(resultMat.size() == originalMat.size());

      return resultMat;
    }
    
    
    static
    cv::Mat
    degradateArea(const cv::Mat &originalMat,
		  const cv::Mat &blurredMat,
		  Function function,
		  Area area,
		  float coeff,
		  int vertical,
		  int horizontal,
		  int radius)
    {
      cv::Mat resultMat = originalMat.clone();

      if (area == Area::CENTERED) {
	degradateCenter(blurredMat, resultMat, function,
			horizontal, vertical, coeff, radius);
      }
      else {
	degradateBorder(blurredMat, resultMat, function,
			area, vertical, coeff, horizontal);
      }

      return resultMat;
    }



    //Apply blur filter to whole image
    cv::Mat
    blur(const cv::Mat &img, Method method, int kernelSize)
    {
      static const char *func = "dc::BlurFilter::blur";
      require8UC134(img, "img", func);
      requireArgument(kernelSize > 0, "kernelSize must be positive", func);

      //B: (tested with OpenCV 2.4.12)
      //If kernelSize(=kSize) == 1 : the three functions do nothing
      //If kernelSize(=kSize) == 2 : GaussianBlur & medianBlur assert, but blur() seems ok

      cv::Mat matOut;

      switch (method) {
      case Method::GAUSSIAN:
	cv::GaussianBlur(img, matOut,
			 cv::Size(kernelSize, kernelSize), 0, 0);
	break;

      case Method::MEDIAN:
	cv::medianBlur(img, matOut, kernelSize);
	break;

      case Method::NORMAL:
	cv::blur(img, matOut, cv::Size(kernelSize, kernelSize));
	break;
      }

      assert(matOut.type() == img.type());

      return matOut;
    }

    cv::Mat
    blurArea(const cv::Mat &img,
	 Method method,
	 int kernelSize,
	 Function function,
	 Area area,
	 float coeff,
	 int vertical,
	 int horizontal,
	 int radius)
    {
      require8UC134(img, "img", "dc::BlurFilter::blurArea");

      //B:TODO:OPTIM:
      // we apply the blur on the whole image
      // and then keep only the part in the computed pattern.
      // We should have a function to get the bounding box of the pattern
      // and apply the blur only in this bounding box...

      cv::Mat matOut = blur(img, method, kernelSize);
      assert(matOut.type() == img.type());
      return degradateArea(img, matOut,
			   function, area, coeff, vertical, horizontal, radius);
    }



    /*
      Compute the Fourier transform of image to measure the blur
      Method taken from Emile Vinsonneau's thesis, section 3.4.2 (descriptor D5).
      Thesis here : http://www.theses.fr/s131479

      Method to get Fourier transform in OpenCV documentation :
      http://docs.opencv.org/doc/tutorials/core/discrete_fourier_transform/discrete_fourier_transform.html

    */
    float
    getRadiusFourier(const cv::Mat &img)
    {
      require8UC134(img, "img", "dc::BlurFilter::getRadiusFourier");

      //B: this used to assert() CV_8UC3 and then call cvtColor(BGR2GRAY),
      // which throws a cv::Exception on a 1-channel image. The assertion
      // being inert in a NDEBUG build, a grayscale image coming from the C++
      // or python API ended up in cvtColor().
      cv::Mat originalMat;
      if (img.channels() == 1) {
	originalMat = img;
      }
      else if (img.channels() == 4) {
	cv::cvtColor(img, originalMat, cv::COLOR_BGRA2GRAY);
      }
      else {
	cv::cvtColor(img, originalMat, cv::COLOR_BGR2GRAY); //To get a grayscale image
      }

      cv::Mat padded; //expand input image to optimal size
      const int m = cv::getOptimalDFTSize(originalMat.rows);
      const int n = cv::getOptimalDFTSize(originalMat.cols);
      copyMakeBorder(originalMat,
		     padded,
		     0,
		     m - originalMat.rows,
		     0,
		     n - originalMat.cols,
		     cv::BORDER_CONSTANT,
		     cv::Scalar::all(0));

      cv::Mat planes[] = { cv::Mat_<float>(padded),
			   cv::Mat::zeros(padded.size(), CV_32F) };
      cv::Mat complexI;
      //cv::Mat_<float> tmpT = cv::Mat_<float>(padded); //B:unused
      //cv::Mat temp = cv::Mat::zeros(padded.size(), CV_32F); //B:unused
      cv::merge(
		planes, 2, complexI); // Add to the expanded another plane with zeros
      cv::dft(complexI, complexI);

      cv::split(complexI, planes); // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
      cv::magnitude(planes[0], planes[1], planes[0]); // planes[0] = magnitude
      cv::Mat magI = planes[0];

      magI += cv::Scalar::all(1); // switch to logarithmic scale
      cv::log(magI, magI);

      magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));
      const int cx = magI.cols / 2;
      const int cy = magI.rows / 2;

      cv::Mat q0(magI,
		 cv::Rect(0, 0, cx, cy)); // Top-Left - Create a ROI per quadrant
      cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));  // Top-Right
      cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));  // Bottom-Left
      cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

      cv::Mat tmp; // swap quadrants (Top-Left with Bottom-Right)
      q0.copyTo(tmp);
      q3.copyTo(q0);
      tmp.copyTo(q3);

      q1.copyTo(tmp); // swap quadrant (Top-Right with Bottom-Left)
      q2.copyTo(q1);
      tmp.copyTo(q2);

      cv::normalize(magI, magI, 0, 255, cv::NORM_MINMAX); // Transform the matrix
      // with float values into
      // a viewable image form
      // (float between values 0
      // and 1).
      cv::threshold(magI, magI, 127, 255, cv::THRESH_BINARY);
      /*
      const int erosion_size = 1;
      cv::Mat element = cv::getStructuringElement(
						  cv::MORPH_RECT,
						  cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
						  cv::Point(
							    erosion_size,
							    erosion_size)); //http://docs.opencv.org/doc/tutorials/imgproc/erosion_dilatation/erosion_dilatation.html
      */

      // to compute the radius
      //B: magI was converted to RGB and back to gray, which keeps its
      // depth, CV_32F, and then read with at<uchar>(): the bytes of the
      // floats were read, not their values, and the radius did not depend
      // on the blur (0 for any image). The binarized magnitude, 0 or 255,
      // is now converted to CV_8U.
      cv::Mat calcRayon;
      magI.convertTo(calcRayon, CV_8U);

      //B: the radius was the distance from the center to the first pixel
      // under the threshold on the diagonal: a few pixels, inside the peak
      // of the low frequencies, which the blur does not change. It is now
      // the radius of the disk of the same area as the pixels over the
      // threshold: the blur removes the high frequencies, and this area
      // shrinks with it.
      return std::sqrt(static_cast<float>(cv::countNonZero(calcRayon)) /
                       static_cast<float>(CV_PI));
    }

    //odd kernel sizes searched by searchFitFourier(), cf BlurFilter.hpp
    static constexpr int MIN_BLUR_FOURIER = 3;
    static constexpr int MAX_BLUR_FOURIER = 19;


    /*
      Search the size of filter (represented here by kernelSize) that we have to apply to fit with the example selected.

      @a img is the image on which we want to apply blur.
      @a dstRadius is the radius (computed by the fonction getRadiusFourier) of the example selected.

      B: the search stopped at the first kernel size whose radius was
      within INTERVAL_FOURIER of @a dstRadius: with a strong blur, the radii
      of several kernel sizes are that close, and the first one tried, 11,
      was taken. The radius decreasing as the kernel size increases, the
      kernel size whose radius is the closest to @a dstRadius is now
      searched, by dichotomy.
    */
    int
    searchFitFourier(const cv::Mat &img, float dstRadius)
    {
      require8UC134(img, "img", "dc::BlurFilter::searchFitFourier");

      //kernel size 2*i+1, i in [iMin; iMax]
      constexpr int iMin = (MIN_BLUR_FOURIER - 1) / 2;
      constexpr int iMax = (MAX_BLUR_FOURIER - 1) / 2;
      std::vector<std::optional<float>> radii(iMax + 1);
      const auto radius = [&](int i) {
	if (!radii[i])
	  radii[i] = getRadiusFourier(blur(img, Method::GAUSSIAN, 2 * i + 1));
	return *radii[i];
      };

      //first i whose radius is not larger than dstRadius
      int lo = iMin;
      int hi = iMax + 1;
      while (lo < hi) {
	const int mid = (lo + hi) / 2;
	if (radius(mid) > dstRadius)
	  lo = mid + 1;
	else
	  hi = mid;
      }

      int best = std::min(lo, iMax);
      if (lo > iMin &&
	  (lo > iMax ||
	   std::abs(radius(lo - 1) - dstRadius) < std::abs(radius(lo) - dstRadius)))
	best = lo - 1;
      return 2 * best + 1;
    }

  } //namespace BlurFilter
} //namespace dc
