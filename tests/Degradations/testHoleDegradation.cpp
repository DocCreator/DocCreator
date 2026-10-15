#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include "Degradations/HoleDegradation.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/version.hpp>
#if CV_VERSION_MAJOR >= 5
#include <opencv2/geometry.hpp> //boundingRect: moved out of imgproc in OpenCV 5
#endif

#include "testCommon.hpp" //checkEqual


template <typename T>
static inline
unsigned int
countPixelsOfColor(const cv::Mat &m,
		   const T &color)
{
  unsigned int count = 0;
  
  int rows = m.rows;
  int cols = m.cols;
  if (m.isContinuous()) {
    cols *= rows;
    rows = 1;
  }
  for (int i=0; i<rows; ++i) {
    const T *p = m.ptr<T>(i);
    for (int j=0; j<cols; ++j) {
      if (p[j] == color) 
	++count;
    }
  }
  
  return count;
}

static inline
unsigned int
countPixelsOfColor(const cv::Mat &m,
		   const cv::Scalar &color)
{
  if (m.type() == CV_8UC1) {
    return countPixelsOfColor<uchar>(m, (uchar)(color[0]));
  }
  else if (m.type() == CV_8UC3) {
    return countPixelsOfColor<cv::Vec3b>(m, cv::Vec3b(color[0], color[1], color[2]));
  }
  else if (m.type() == CV_8UC4) {
    return countPixelsOfColor<cv::Vec4b>(m, cv::Vec4b(color[0], color[1], color[2], color[3]));
  }
  return 0;
}

static
void
testSimple0(int imageType)
{
  //Insert one hole that takes the whole image.
  //So output image will be the hole.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 100;
  const int COLS = 100;    
    
  cv::Mat img(ROWS, COLS, imageType);

  cv::Mat imgClone = img.clone();
  
  
  const cv::Scalar COLOR(255, 110, 8);

  cv::Mat hole = cv::Mat::zeros(ROWS, COLS, CV_8UC1);

  const int xOrigin = 0;
  const int yOrigin = 0;
  const int size = 0;
  const dc::HoleDegradation::HoleType type = dc::HoleDegradation::HoleType::CENTER;
  const dc::HoleDegradation::HoleSide side = dc::HoleDegradation::HoleSide::BORDER_TOP;
    
  cv::Mat out = addHole(img, hole,
			cv::Point(xOrigin, yOrigin),
			size,
			type, side,
			COLOR);

  REQUIRE( out.type() == img.type() );
				  
  const unsigned int area = img.rows*img.cols;
  const unsigned int pixelsOfColor = countPixelsOfColor(out, COLOR);

  REQUIRE( pixelsOfColor == area );

  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
}

static
void
testSimple1(int imageType)
{
  //Insert one hole that takes a small part of the image.
  //It is not at (0, 0) but it will be completely visible.
  //So output image will be the input image with the hole in
  // the specified color.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 100;
  const int COLS = 100;    

  const int HOLE_ROWS = ROWS/10;
  const int HOLE_COLS = COLS/10;
    
  cv::Mat img = cv::Mat::ones(ROWS, COLS, imageType) * 255;

  cv::Mat imgClone = img.clone();
  
  const cv::Scalar COLOR(238, 10, 218);

  cv::Mat hole = cv::Mat::zeros(HOLE_ROWS, HOLE_COLS, CV_8UC1);

  const int xOrigin = HOLE_ROWS;
  const int yOrigin = HOLE_COLS;
  const int size = 0;
  const dc::HoleDegradation::HoleType type = dc::HoleDegradation::HoleType::CENTER;
  const dc::HoleDegradation::HoleSide side = dc::HoleDegradation::HoleSide::BORDER_TOP;
    
  cv::Mat out = addHole(img, hole,
			cv::Point(xOrigin, yOrigin),
			size,
			type, side,
			COLOR);

  REQUIRE( out.type() == img.type() );
  
  const unsigned int pixelsOfColor = countPixelsOfColor(out, COLOR);

  const unsigned int hole_area = hole.rows*hole.cols;

  REQUIRE( pixelsOfColor == hole_area );

  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
}

static
void
testSimple2(int imageType)
{
  //Insert one hole that takes the whole image and
  // put the same image below.
  //So output image will be the input image.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 100;
  const int COLS = 100;    

  cv::Mat img = cv::Mat::ones(ROWS, COLS, imageType);

  cv::Mat imgClone = img.clone();
  
  const cv::Scalar COLOR(208, 10, 28);

  cv::Mat hole = cv::Mat::zeros(ROWS, COLS, CV_8UC1);

  const int xOrigin = ROWS;
  const int yOrigin = COLS;
  const int size = 0;
  const dc::HoleDegradation::HoleType type = dc::HoleDegradation::HoleType::CENTER;
  const dc::HoleDegradation::HoleSide side = dc::HoleDegradation::HoleSide::BORDER_TOP;
    
  cv::Mat out = addHole(img, hole,
			cv::Point(xOrigin, yOrigin),
			size,
			type, side,
			COLOR, img);
				  
  REQUIRE( out.type() == img.type() );

  const unsigned int pixelsOfColor = countPixelsOfColor(out, COLOR);

  REQUIRE( pixelsOfColor == 0 );
  REQUIRE( checkEqual(img, out) );

  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
}

TEST_CASE( "Testing HoleDegradation" )
{ 

  SECTION("Testing simple hole at (0; 0) without background image")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }

  SECTION("Testing simple hole without background image")
  {
    testSimple1(CV_8UC1);
    testSimple1(CV_8UC3);
    testSimple1(CV_8UC4);
  }


  SECTION("Testing simple small hole with identity as background image")
  {
    testSimple2(CV_8UC1);
    testSimple2(CV_8UC3);
    testSimple2(CV_8UC4);
  }
  

}


/*
  Border and corner holes. The patterns are drawn for the top border and
  the top left corner, and turned for the other sides: what was at the top
  goes to the right, the bottom or the left. A pattern whose hole is not
  symmetrical shows each turn: its black part is a 6x5 block in the top
  left of a 20x10 pattern.
*/
namespace {

  //@return the bounding box of the pixels of @a m that are @a color
  cv::Rect
  boxOfColor(const cv::Mat &m, const cv::Vec3b &color)
  {
    cv::Mat mask;
    cv::inRange(m, cv::Scalar(color[0], color[1], color[2]),
		cv::Scalar(color[0], color[1], color[2]), mask);
    return cv::boundingRect(mask);
  }

  cv::Mat
  asymmetricPattern()
  {
    cv::Mat pattern(10, 20, CV_8UC1, cv::Scalar(255));
    pattern(cv::Rect(0, 0, 6, 5)).setTo(0); //x 0..5, y 0..4
    return pattern;
  }

} //namespace

TEST_CASE( "Testing HoleDegradation border and corner sides" )
{
  using namespace dc::HoleDegradation;

  const cv::Mat img(50, 50, CV_8UC3, cv::Scalar(255, 255, 255));
  const cv::Vec3b color(10, 20, 30);
  const cv::Scalar colorS(10, 20, 30, 255);
  const cv::Mat pattern = asymmetricPattern();

  auto box = [&](HoleType type, HoleSide side) {
    const cv::Mat out = addHole(img, pattern, cv::Point(0, 0), 0, type, side, colorS);
    REQUIRE( out.size() == img.size() );
    return boxOfColor(out, color);
  };

  SECTION("border")
  {
    //the pattern turned: 20x10 for top and bottom, 10x20 for the sides
    REQUIRE( box(HoleType::BORDER, HoleSide::BORDER_TOP) == cv::Rect(0, 0, 6, 5) );
    REQUIRE( box(HoleType::BORDER, HoleSide::BORDER_BOTTOM) == cv::Rect(0, 5, 6, 5) );
    REQUIRE( box(HoleType::BORDER, HoleSide::BORDER_LEFT) == cv::Rect(0, 0, 5, 6) );
    REQUIRE( box(HoleType::BORDER, HoleSide::BORDER_RIGHT) == cv::Rect(5, 0, 5, 6) );
  }

  SECTION("corner")
  {
    REQUIRE( box(HoleType::CORNER, HoleSide::CORNER_TOPLEFT) == cv::Rect(0, 0, 6, 5) );
    REQUIRE( box(HoleType::CORNER, HoleSide::CORNER_TOPRIGHT) == cv::Rect(14, 0, 6, 5) );
    REQUIRE( box(HoleType::CORNER, HoleSide::CORNER_BOTTOMRIGHT) == cv::Rect(14, 5, 6, 5) );
    REQUIRE( box(HoleType::CORNER, HoleSide::CORNER_BOTTOMLEFT) == cv::Rect(0, 5, 6, 5) );
  }

  SECTION("a pattern in colour is taken as its gray")
  {
    cv::Mat pattern3, pattern4;
    cv::cvtColor(pattern, pattern3, cv::COLOR_GRAY2BGR);
    cv::cvtColor(pattern, pattern4, cv::COLOR_GRAY2BGRA);
    const cv::Mat out3 = addHole(img, pattern3, cv::Point(0, 0), 0, HoleType::BORDER, HoleSide::BORDER_BOTTOM, colorS);
    const cv::Mat out4 = addHole(img, pattern4, cv::Point(0, 0), 0, HoleType::BORDER, HoleSide::BORDER_BOTTOM, colorS);
    REQUIRE( boxOfColor(out3, color) == cv::Rect(0, 5, 6, 5) );
    REQUIRE( boxOfColor(out4, color) == cv::Rect(0, 5, 6, 5) );
  }

  SECTION("a size enlarges the pattern")
  {
    const cv::Mat out = addHole(img, pattern, cv::Point(0, 0), 10, HoleType::BORDER, HoleSide::BORDER_TOP, colorS);
    //the 20x10 pattern resized to 30x20: the hole grows with it
    const cv::Rect b = boxOfColor(out, color);
    REQUIRE( b.width > 6 );
    REQUIRE( b.height > 5 );
  }
}

/*
  The shadow: the pixels of the hole within shadowBorderWidth - 1 of its
  edge get the colour of the hole times shadowBorderIntensity (the alpha of
  a 4 channel image excepted). With a black hole, the default, nothing
  shows.
*/
TEST_CASE( "Testing HoleDegradation shadow border" )
{
  using namespace dc::HoleDegradation;

  //a 10x10 hole in the middle of a 20x20 pattern
  cv::Mat pattern(20, 20, CV_8UC1, cv::Scalar(255));
  pattern(cv::Rect(5, 5, 10, 10)).setTo(0);

  const cv::Scalar color(200, 200, 200, 255);
  const int RING = 36; //outer ring of a 10x10 square
  const int INSIDE = 64;

  SECTION("gray")
  {
    const cv::Mat img(20, 20, CV_8UC1, cv::Scalar(255));
    const cv::Mat out = addHole(img, pattern, cv::Point(0, 0), 0, HoleType::CENTER, HoleSide::BORDER_TOP, color, cv::Mat(), 2, .5f);
    REQUIRE( countPixelsOfColor<uchar>(out, 100) == RING );
    REQUIRE( countPixelsOfColor<uchar>(out, 200) == INSIDE );
  }

  SECTION("colour, and the alpha left alone")
  {
    const cv::Mat img3(20, 20, CV_8UC3, cv::Scalar(255, 255, 255));
    const cv::Mat out3 = addHole(img3, pattern, cv::Point(0, 0), 0, HoleType::CENTER, HoleSide::BORDER_TOP, color, cv::Mat(), 2, .5f);
    REQUIRE( countPixelsOfColor<cv::Vec3b>(out3, cv::Vec3b(100, 100, 100)) == RING );
    REQUIRE( countPixelsOfColor<cv::Vec3b>(out3, cv::Vec3b(200, 200, 200)) == INSIDE );

    const cv::Mat img4(20, 20, CV_8UC4, cv::Scalar(255, 255, 255, 255));
    const cv::Mat out4 = addHole(img4, pattern, cv::Point(0, 0), 0, HoleType::CENTER, HoleSide::BORDER_TOP, color, cv::Mat(), 2, .5f);
    REQUIRE( countPixelsOfColor<cv::Vec4b>(out4, cv::Vec4b(100, 100, 100, 255)) == RING );
  }

  SECTION("an intensity above 1 saturates")
  {
    const cv::Mat img(20, 20, CV_8UC1, cv::Scalar(255));
    const cv::Mat out = addHole(img, pattern, cv::Point(0, 0), 0, HoleType::CENTER, HoleSide::BORDER_TOP, color, cv::Mat(), 2, 2.f);
    REQUIRE( countPixelsOfColor<uchar>(out, 255) == 20 * 20 - INSIDE );
  }

  SECTION("no width, no shadow")
  {
    const cv::Mat img(20, 20, CV_8UC1, cv::Scalar(255));
    const cv::Mat out = addHole(img, pattern, cv::Point(0, 0), 0, HoleType::CENTER, HoleSide::BORDER_TOP, color, cv::Mat(), 0, .5f);
    REQUIRE( countPixelsOfColor<uchar>(out, 200) == RING + INSIDE );
  }
}
