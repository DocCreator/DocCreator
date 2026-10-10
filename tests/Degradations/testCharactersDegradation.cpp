#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include <opencv2/imgproc/imgproc.hpp>

#include "Degradations/GrayscaleCharsDegradationModel.hpp"

#include "testCommon.hpp" //checkEqual


static
void
testSimple(int imageType)
{
  //Apply GrayscaleCharsDegradationModel on random image
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.
  
  const int ROWS = 100;
  const int COLS = 100;    
    
  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );  

  cv::Mat imgClone = img.clone();
  
  const int level = 5;

  cv::Mat out = dc::GrayscaleCharsDegradation::degradation(img, level);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
}


static
void
testSimple2(int imageType)
{
  //Apply GrayscaleCharsDegradationModel on image with one connected componant
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 100;
  const int COLS = 100;

  cv::Mat img(ROWS, COLS, imageType);
  const cv::Scalar WHITE(255, 255, 255, 255);
  const cv::Scalar BLACK(0, 0, 0, 0);
  img = WHITE;
  img.rowRange(ROWS/4, (ROWS*3)/4).colRange(COLS/4, (COLS*3)/4) = BLACK;

  REQUIRE( imageType == img.type() );

  cv::Mat imgClone = img.clone();

  const int level = 4;

  cv::Mat out = dc::GrayscaleCharsDegradation::degradation(img, level);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
}



/*
  @return a black & white image with @a numComponents square connected
  components. Black & white, so that converting it to grayscale and back is
  lossless, and an untouched image can be compared to its input.
*/
static
cv::Mat
makeImageWithComponents(int numComponents, int imageType)
{
  const int ROWS = 120;
  const int COLS = 120;

  //Grey levels, and not pure black & white: a degraded pixel is clamped to
  //[0; 255], so it could otherwise equal its input value by accident.
  const int BACKGROUND = 200;
  const int FOREGROUND = 60;

  cv::Mat img(ROWS,
              COLS,
              imageType,
              cv::Scalar(BACKGROUND, BACKGROUND, BACKGROUND, 255));
  for (int i = 0; i < numComponents; ++i) {
    const int x = 10 + (i % 4) * 28;
    const int y = 10 + (i / 4) * 28;
    cv::rectangle(img,
                  cv::Rect(x, y, 14, 14),
                  cv::Scalar(FOREGROUND, FOREGROUND, FOREGROUND, 255),
                  -1);
  }
  return img;
}


static
void
testAllLevels(int imageType)
{
  //Whatever the level and the number of connected components, the degradation
  //must keep the type and the size of the input image, and leave it untouched.

  for (int numComponents = 0; numComponents <= 5; ++numComponents) {
    for (int level = 0; level <= 10; ++level) {
      const cv::Mat img = makeImageWithComponents(numComponents, imageType);
      const cv::Mat imgClone = img.clone();

      const cv::Mat out = dc::GrayscaleCharsDegradation::degradation(img, level);

      REQUIRE( out.type() == imageType );
      REQUIRE( out.size() == img.size() );
      REQUIRE( checkEqual(img, imgClone) );
    }
  }
}


static
void
testLevelOutOfRange(int imageType)
{
  //level is documented in [0; 10], but an out-of-range value must not
  //crash nor throw.
  const int levels[] = { -1, 20, 1000 };

  for (int level : levels) {
    const cv::Mat img = makeImageWithComponents(4, imageType);
    const cv::Mat imgClone = img.clone();

    cv::Mat out;
    REQUIRE_NOTHROW( out = dc::GrayscaleCharsDegradation::degradation(img, level) );

    REQUIRE( out.type() == imageType );
    REQUIRE( out.size() == img.size() );
    REQUIRE( checkEqual(img, imgClone) );
  }
}


TEST_CASE( "Testing CharactersDegradation" )
{ 

  SECTION("Testing Characters Degradation produces output image of same type and size than input image")
  {
    testSimple(CV_8UC1);
    testSimple(CV_8UC3);
    testSimple(CV_8UC4);

    testSimple2(CV_8UC1);
    testSimple2(CV_8UC3);
    testSimple2(CV_8UC4);
  }

  SECTION("Testing Characters Degradation for every level")
  {
    testAllLevels(CV_8UC1);
    testAllLevels(CV_8UC3);
    testAllLevels(CV_8UC4);
  }

  SECTION("Testing Characters Degradation with an out-of-range level")
  {
    testLevelOutOfRange(CV_8UC1);
    testLevelOutOfRange(CV_8UC3);
    testLevelOutOfRange(CV_8UC4);
  }

}


