#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "Degradations/BlurFilter.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp> //DEBUG

#include "testCommon.hpp" //checkEqual

static
void
testSimple0(int imageType)
{
  //Apply blur to whole image
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 100;
  const int COLS = 100;    
    
  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

  REQUIRE( imageType == img.type() );  

  cv::Mat imgClone = img.clone();
  
  const int kernelSize = 3;
  
  for (int i=0; i<3; ++i) {
    const dc::BlurFilter::Method method = (dc::BlurFilter::Method)(i);

    const cv::Mat img2 = img.clone();
    REQUIRE( imageType == img2.type() );  
    
    //apply blur to whole image
    const cv::Mat out = dc::BlurFilter::blur(img2, method, kernelSize);

    REQUIRE( out.type() == imageType );
    REQUIRE( checkEqual(img, imgClone) );
    REQUIRE( out.size() == img.size() );
  }
}

static
void
testSimple1(int imageType)
{
  //Apply blur to part of the image
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 100;
  const int COLS = 100;    
    
  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

  REQUIRE( imageType == img.type() );  

  cv::Mat imgClone = img.clone();
  
  const int kernelSize = 3;

  const dc::BlurFilter::Function function = dc::BlurFilter::Function::LINEAR;
  const float coeff = 1.f;
  const int vertical = 0;
  const int horizontal = 0;
  const int radius = 2;
  
  for (int i=0; i<3; ++i) {
    const dc::BlurFilter::Method method = (dc::BlurFilter::Method)(i);

    for (int j=0; j<3; ++j) {
      const dc::BlurFilter::Area area = (dc::BlurFilter::Area)(j);


      const cv::Mat img2 = img.clone();
      REQUIRE( imageType == img2.type() );

      //apply blur to part of the image
      const cv::Mat out = dc::BlurFilter::blurArea(img2, method, kernelSize, function, area, coeff, vertical, horizontal, radius);

      REQUIRE( out.type() == imageType );
      REQUIRE( checkEqual(img, imgClone) );
      REQUIRE( out.size() == img.size() );
    }
  }
}


static
void
testSimple2(int imageType)
{
  //Test makePattern/applyPattern functions
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 100;
  const int COLS = 100;

  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

  REQUIRE( imageType == img.type() );

  cv::Mat imgClone = img.clone();

  
  {
    cv::Mat img2 = img.clone();
    REQUIRE( imageType == img2.type() );
    
    const dc::BlurFilter::Function function = dc::BlurFilter::Function::LINEAR;
    const dc::BlurFilter::Area area = dc::BlurFilter::Area::UP;
    const float coeff = 1.f;
    const int vertical = 50;
    const int horizontal = 50;
    const int radius = 10;

    const cv::Mat pattern = dc::BlurFilter::makePattern(img2,
							function, area,
							coeff, vertical, horizontal, radius);

    REQUIRE( CV_8UC1 == pattern.type() );

    const dc::BlurFilter::Method method = dc::BlurFilter::Method::GAUSSIAN;
    const int kernelSize = 3;

    cv::Mat out = dc::BlurFilter::applyPattern(img2, pattern, method, kernelSize);

    REQUIRE( imageType == out.type() );
    REQUIRE( checkEqual(img, imgClone) );
    REQUIRE( out.size() == img.size() );
  }

}


/*
  The curves of a pattern are continuous, so a column of the pattern does
  not differ from both its neighbours by more than half its height.

  Two that did: the first column of a LOG pattern, where log(0) was
  converted to an int (undefined, INT_MIN on x86), and a HYPERBOLA column
  whose second row truncated to -1, which was taken for "no second row".
  The ellipse converted -FLT_MAX to an int too, with no visible effect but
  an overflow the undefined behaviour sanitizer reports.
*/
static
void
testPatternColumnsAreContinuous(int rows, int cols)
{
  using namespace dc::BlurFilter;

  const cv::Mat img(rows, cols, CV_8UC1, cv::Scalar(0));

  for (Function f : {Function::LINEAR, Function::LOG, Function::PARABOLA,
		     Function::SINUS, Function::ELLIPSE, Function::HYPERBOLA}) {
    for (Area a : {Area::UP, Area::DOWN, Area::CENTERED}) {
      for (float coeff : {1.f, 5.f}) {
	for (int vertical : {-50, 0, 100}) {
	  const cv::Mat pattern = makePattern(img, f, a, coeff, vertical, 0, 10);
	  REQUIRE( pattern.size() == img.size() );

	  std::vector<int> white(cols);
	  for (int x = 0; x < cols; ++x) {
	    white[x] = cv::countNonZero(pattern.col(x));
	  }
	  for (int x = 0; x < cols; ++x) {
	    const bool farFromPrevious =
	      (x == 0) || std::abs(white[x] - white[x-1]) > rows / 2;
	    const bool farFromNext =
	      (x == cols - 1) || std::abs(white[x] - white[x+1]) > rows / 2;
	    INFO( "function " << static_cast<int>(f) << " area " << static_cast<int>(a)
		  << " coeff " << coeff << " vertical " << vertical << " column " << x );
	    REQUIRE_FALSE( (farFromPrevious && farFromNext) );
	  }
	}
      }
    }
  }
}

/*
  A curve crossing a column at row -1 is a curve, -1 having been used to
  mean "no row": it is near the first radius rows of the image.
*/
static
void
testCurveAtRowMinusOne()
{
  using namespace dc::BlurFilter;

  const int rows = 300;
  const int cols = 400;
  const int radius = 10;
  const cv::Mat img(rows, cols, CV_8UC1, cv::Scalar(0));

  //LINEAR, coeff 1, vertical 0: row = rows - x, so -1 at x = rows + 1
  const cv::Mat pattern =
    makePattern(img, Function::LINEAR, Area::CENTERED, 1.f, 0, 0, radius);

  const cv::Mat column = pattern.col(rows + 1);
  //the pattern is white where the image is not near the curve
  REQUIRE( cv::countNonZero(column.rowRange(0, radius)) == 0 );
  REQUIRE( cv::countNonZero(column.rowRange(radius, rows)) == rows - radius );
}


TEST_CASE( "Testing BlurFilter" )
{ 

  SECTION("Testing blur on whole image produces output image of same type and size than input image")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }

  SECTION("Testing blur on part of image produces output image of same type and size than input image")
  {
    testSimple1(CV_8UC1);
    testSimple1(CV_8UC3);
    testSimple1(CV_8UC4);
  }

  SECTION("Testing blur makePattern/applyPattern")
  {
    testSimple2(CV_8UC1);
    testSimple2(CV_8UC3);
    testSimple2(CV_8UC4);
  }

  SECTION("Testing the curves of a pattern are continuous")
  {
    testPatternColumnsAreContinuous(300, 400);
    testPatternColumnsAreContinuous(1600, 1200);
  }

  SECTION("Testing a curve crossing row -1 is a curve")
  {
    testCurveAtRowMinusOne();
  }

  //TODO: test blur with even kernelSize (ex: 2, 4, ...)

}

/*
  getRadiusFourier() read a CV_32F image with at<uchar>(): its radius was
  0 whatever the blur, and searchFitFourier() always gave 11. The radius
  also did not change with the blur once the floats were read.
*/

static
cv::Mat
makeTextImage()
{
  cv::Mat img(400, 600, CV_8UC3, cv::Scalar(235, 235, 235));
  for (int y = 40; y < img.rows; y += 36)
    cv::putText(img, "Lorem ipsum dolor sit amet, consectetur adipiscing elit",
                cv::Point(10, y), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                cv::Scalar(20, 20, 20), 1, cv::LINE_AA);
  return img;
}

TEST_CASE( "Testing the Fourier radius of BlurFilter" )
{
  const cv::Mat img = makeTextImage();

  SECTION("the more blurred the image, the smaller the radius")
  {
    float previous = dc::BlurFilter::getRadiusFourier(img);
    REQUIRE( previous > 0 );
    for (int k = 5; k <= 21; k += 4) {
      const float r = dc::BlurFilter::getRadiusFourier(
        dc::BlurFilter::blur(img, dc::BlurFilter::Method::GAUSSIAN, k));
      INFO( "kernelSize=" << k );
      REQUIRE( r < previous );
      previous = r;
    }
  }

  SECTION("the radius is the same for a gray, a color or a BGRA image")
  {
    cv::Mat gray, bgra;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(img, bgra, cv::COLOR_BGR2BGRA);
    const float r = dc::BlurFilter::getRadiusFourier(img);
    REQUIRE( dc::BlurFilter::getRadiusFourier(gray) == r );
    REQUIRE( dc::BlurFilter::getRadiusFourier(bgra) == r );
  }

  SECTION("searchFitFourier finds the blur of an image back")
  {
    for (const int k : {3, 7, 11, 15, 19}) {
      const float r = dc::BlurFilter::getRadiusFourier(
        dc::BlurFilter::blur(img, dc::BlurFilter::Method::GAUSSIAN, k));
      const int found = dc::BlurFilter::searchFitFourier(img, r);
      INFO( "kernelSize=" << k << " radius=" << r << " found=" << found );
      REQUIRE( std::abs(found - k) <= 2 );
    }
  }

  SECTION("searchFitFourier gives an odd kernel size in [3; 19]")
  {
    for (const float r : {0.f, 1.f, 1e6f}) {
      const int found = dc::BlurFilter::searchFitFourier(img, r);
      REQUIRE( found % 2 == 1 );
      REQUIRE( 3 <= found );
      REQUIRE( found <= 19 );
    }
    REQUIRE( dc::BlurFilter::searchFitFourier(img, 0.f) == 19 );
    REQUIRE( dc::BlurFilter::searchFitFourier(img, 1e6f) == 3 );
  }
}

/*
  An even kernel size, documented as invalid for Method::GAUSSIAN and
  Method::MEDIAN, was not checked: OpenCV threw a cv::Exception.
*/
TEST_CASE( "Testing BlurFilter refuses an even kernel size" )
{
  const cv::Mat img(40, 50, CV_8UC3, cv::Scalar::all(128));
  for (const int k : {2, 4, 10}) {
    INFO( "kernelSize=" << k );
    REQUIRE_THROWS_AS( dc::BlurFilter::blur(img, dc::BlurFilter::Method::GAUSSIAN, k),
                       std::invalid_argument );
    REQUIRE_THROWS_AS( dc::BlurFilter::blur(img, dc::BlurFilter::Method::MEDIAN, k),
                       std::invalid_argument );
    //any positive size for the normal blur
    REQUIRE( dc::BlurFilter::blur(img, dc::BlurFilter::Method::NORMAL, k).size() == img.size() );
  }
  REQUIRE( dc::BlurFilter::blur(img, dc::BlurFilter::Method::GAUSSIAN, 3).size() == img.size() );
  REQUIRE( dc::BlurFilter::blur(img, dc::BlurFilter::Method::MEDIAN, 3).size() == img.size() );
}
