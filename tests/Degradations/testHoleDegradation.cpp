#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Degradations/HoleDegradation.hpp"

#include <opencv2/imgproc/imgproc.hpp>

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
  const int side = 0;
    
  cv::Mat out = holeDegradation(img, hole,
				xOrigin, yOrigin,
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
  const int side = 0;
    
  cv::Mat out = holeDegradation(img, hole,
				xOrigin, yOrigin,
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
  const int side = 0;
    
  cv::Mat out = holeDegradation(img, hole,
				xOrigin, yOrigin,
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
