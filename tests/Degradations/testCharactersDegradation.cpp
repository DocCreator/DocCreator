#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include <vector>

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
makeImageWithComponents(int numComponents, int imageType, int alpha = 255)
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
              cv::Scalar(BACKGROUND, BACKGROUND, BACKGROUND, alpha));
  for (int i = 0; i < numComponents; ++i) {
    const int x = 10 + (i % 4) * 28;
    const int y = 10 + (i / 4) * 28;
    cv::rectangle(img,
                  cv::Rect(x, y, 14, 14),
                  cv::Scalar(FOREGROUND, FOREGROUND, FOREGROUND, alpha),
                  -1);
  }
  return img;
}


static
void
testLowLevelOnFewComponents(int imageType)
{
  //The number of degradation spots is (2 * nbConnectedComponents * level / 5),
  //an integer division that gives 0 for level 1 on an image with a single
  //connected component. One spot is now always requested for a positive
  //level. [It used to request none, and to read one element past the end of
  //a vector while doing so.]
  //
  //B:TODO: we cannot check here that the image really changed: a single spot
  // covers a handful of pixels and often writes values equal to the ones
  // already there, and the random generator is seeded on the current time,
  // so the result is not reproducible. A seed parameter on degradation()
  // would make this testable.

  for (int level = 1; level <= 3; ++level) {
    const cv::Mat img = makeImageWithComponents(1, imageType);
    const cv::Mat imgClone = img.clone();

    const cv::Mat out = dc::GrayscaleCharsDegradation::degradation(img, level);

    REQUIRE( out.type() == imageType );
    REQUIRE( out.size() == img.size() );
    REQUIRE( checkEqual(img, imgClone) ); //input not modified
  }
}


static
void
testNoDegradationAtLevelZero(int imageType)
{
  //A null level means no degradation spot: the image must come back
  //unchanged. [The input is grey, so the conversion to grayscale and back
  //is lossless.]

  const cv::Mat img = makeImageWithComponents(3, imageType);
  const cv::Mat imgClone = img.clone();

  const cv::Mat out = dc::GrayscaleCharsDegradation::degradation(img, 0);

  REQUIRE( out.type() == imageType );
  REQUIRE( out.size() == img.size() );
  REQUIRE( checkEqual(img, imgClone) );  //input not modified
  REQUIRE( checkEqual(out, img) );       //and nothing degraded
}


static
void
testAlphaIsPreserved()
{
  //The degradation works on a grayscale image: the alpha channel of a
  //4-channel input must be restored as is in the output.
  //[cv::cvtColor(COLOR_GRAY2BGRA) used to set it to 255.]

  const int alphas[] = { 0, 37, 128, 255 };

  for (int alpha : alphas) {
    const cv::Mat img = makeImageWithComponents(3, CV_8UC4, alpha);
    const cv::Mat imgClone = img.clone();

    const cv::Mat out = dc::GrayscaleCharsDegradation::degradation(img, 5);

    REQUIRE( out.type() == CV_8UC4 );
    REQUIRE( out.size() == img.size() );
    REQUIRE( checkEqual(img, imgClone) ); //input not modified

    std::vector<cv::Mat> inChannels;
    std::vector<cv::Mat> outChannels;
    cv::split(img, inChannels);
    cv::split(out, outChannels);
    REQUIRE( inChannels.size() == 4 );
    REQUIRE( outChannels.size() == 4 );

    REQUIRE( cv::countNonZero(inChannels[3] != outChannels[3]) == 0 );
  }
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


static
void
testLargeComponents(int imageType)
{
  //The maximum component size is derived from the number of pixels of the
  //components, and its square used to overflow an int: the wrapped value
  //being negative, no component was accepted and the image came back
  //untouched. Components of 400x400 pixels are enough.

  const int SIDE = 400;
  cv::Mat img(1500, 1500, imageType, cv::Scalar(210, 210, 210, 255));
  for (int i = 0; i < 4; ++i) {
    cv::rectangle(img,
                  cv::Rect(30 + (i % 2) * 700, 30 + (i / 2) * 700, SIDE, SIDE),
                  cv::Scalar(50, 50, 50, 255),
                  -1);
  }
  const cv::Mat imgClone = img.clone();

  const cv::Mat out = dc::GrayscaleCharsDegradation::degradation(img, 8);

  REQUIRE( out.type() == imageType );
  REQUIRE( out.size() == img.size() );
  REQUIRE( checkEqual(img, imgClone) );  //input not modified
  REQUIRE_FALSE( checkEqual(out, img) ); //and something was degraded
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

  SECTION("Testing Characters Degradation with a low level on few components")
  {
    testLowLevelOnFewComponents(CV_8UC1);
    testLowLevelOnFewComponents(CV_8UC3);
    testLowLevelOnFewComponents(CV_8UC4);
  }

  SECTION("Testing Characters Degradation does not degrade at level 0")
  {
    testNoDegradationAtLevelZero(CV_8UC1);
    testNoDegradationAtLevelZero(CV_8UC3);
    testNoDegradationAtLevelZero(CV_8UC4);
  }

  SECTION("Testing Characters Degradation on large connected components")
  {
    testLargeComponents(CV_8UC1);
    testLargeComponents(CV_8UC3);
    testLargeComponents(CV_8UC4);
  }

  SECTION("Testing Characters Degradation preserves the alpha channel")
  {
    testAlphaIsPreserved();
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


