#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Degradations/RotationDegradation.hpp"

#include <opencv2/imgproc/imgproc.hpp>

#include "testCommon.hpp" //checkEqual

static
void
testSimple0a(int imageType)
{
  //Apply RotationDegradation::rotateFillColor
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
  
  const float angle = 11;
  const cv::Scalar color(100, 150, 200);
  const cv::Mat out = dc::RotationDegradation::rotateFillColor(img, angle, color);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
}

static
void
testSimple0b(int imageType)
{
  //Apply RotationDegradation::rotateFillImage
  //Background image is not necessarily of the same type and size than input image.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the input background image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 100;
  const int COLS = 100;
    
  cv::Mat img(ROWS, COLS, imageType);

  cv::Mat backgroundImg(ROWS*2, COLS/3, CV_8UC3);

  //add gaussian noise
  cv::randn(img, 128, 30);

  cv::randn(backgroundImg, 128, 30);

  REQUIRE( imageType == img.type() );

  cv::Mat imgClone = img.clone();
  cv::Mat backgroundImgClone = backgroundImg.clone();
  
  const float angle = 11;
  const cv::Mat out = dc::RotationDegradation::rotateFillImage(img, angle, backgroundImg);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( checkEqual(backgroundImg, backgroundImgClone) );
  REQUIRE( out.size() == img.size() );
}


static
void
testSimple0b2(int imageType)
{
  //Apply RotationDegradation::rotateFillImage (with repeats)
  //Background image is not necessarily of the same type and size than input image.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the input background image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 100;
  const int COLS = 100;
    
  cv::Mat img(ROWS, COLS, imageType);

  cv::Mat backgroundImg(ROWS*2, COLS/3, CV_8UC3);

  //add gaussian noise
  cv::randn(img, 128, 30);

  cv::randn(backgroundImg, 128, 30);

  REQUIRE( imageType == img.type() );

  cv::Mat imgClone = img.clone();
  cv::Mat backgroundImgClone = backgroundImg.clone();
  
  const float angle = 17;
  const int repeats = 3;
  const cv::Mat out = dc::RotationDegradation::rotateFillImageN(img, angle, backgroundImg, repeats);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( checkEqual(backgroundImg, backgroundImgClone) );
  REQUIRE( out.size() == img.size() );
}

static
void
testSimple0b3(int imageType)
{
  //Apply RotationDegradation::rotateFillImage
  //Background image is an empty image.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the input background image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 100;
  const int COLS = 100;
    
  cv::Mat img(ROWS, COLS, imageType);

  cv::Mat backgroundImg;

  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );

  cv::Mat imgClone = img.clone();
  cv::Mat backgroundImgClone = backgroundImg.clone();
  
  const float angle = 17;
  const cv::Mat out = dc::RotationDegradation::rotateFillImage(img, angle, backgroundImg);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( checkEqual(backgroundImg, backgroundImgClone) );
  REQUIRE( out.size() == img.size() );
}


static
void
testSimple0b4(int imageType)
{
  //Apply RotationDegradation::rotateFillImage (with repeats)
  //Background image is an empty image.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the input background image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 100;
  const int COLS = 100;
    
  cv::Mat img(ROWS, COLS, imageType);

  cv::Mat backgroundImg;

  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );

  cv::Mat imgClone = img.clone();
  cv::Mat backgroundImgClone = backgroundImg.clone();
  
  const float angle = 17;
  const int repeats = 3;
  const cv::Mat out = dc::RotationDegradation::rotateFillImageN(img, angle, backgroundImg, repeats);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( checkEqual(backgroundImg, backgroundImgClone) );
  REQUIRE( out.size() == img.size() );
}

static
void
testSimple0c(int imageType)
{
  //Apply RotationDegradation::rotateFillBorder
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
  
  const float angle = 11;

  for (int i=0; i<4; ++i) {
    dc::RotationDegradation::BorderReplication borderMode = (dc::RotationDegradation::BorderReplication)(i);

    const cv::Mat out = dc::RotationDegradation::rotateFillBorder(img, angle, borderMode);

    REQUIRE( out.type() == imageType );
    REQUIRE( checkEqual(img, imgClone) );
    REQUIRE( out.size() == img.size() );
  }
  
}

static
void
testSimple0d(int imageType)
{
  //Apply RotationDegradation::rotateFillInpaint1
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
  
  const float angle = 11;
  const float inpaintingRatio = 0.1f;
  const cv::Mat out = dc::RotationDegradation::rotateFillInpaint1(img, angle, inpaintingRatio);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
}

static
void
testSimple0e(int imageType)
{
  //Apply RotationDegradation::rotateFillInpaint2
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
  
  const float angle = 11;
  const cv::Mat out = dc::RotationDegradation::rotateFillInpaint2(img, angle);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
}

static
void
testSimple0f(int imageType)
{
  //Apply RotationDegradation::rotateFillInpaint3
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
  
  const float angle = 11;
  const cv::Mat out = dc::RotationDegradation::rotateFillInpaint3(img, angle);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
}




TEST_CASE( "Testing RotationDegradation" )
{ 

  SECTION("Testing RotationDegradation::rotateFillColor produces output image of same type and size than input image")
  {
    testSimple0a(CV_8UC1);
    testSimple0a(CV_8UC3);
    testSimple0a(CV_8UC4);
  }

  SECTION("Testing RotationDegradation::rotateFillImage produces output image of same type and size than input image")
  {
    testSimple0b(CV_8UC1);
    testSimple0b(CV_8UC3);
    testSimple0b(CV_8UC4);

    testSimple0b2(CV_8UC1);
    testSimple0b2(CV_8UC3);
    testSimple0b2(CV_8UC4);

    testSimple0b3(CV_8UC1);
    testSimple0b3(CV_8UC3);
    testSimple0b3(CV_8UC4);

    testSimple0b4(CV_8UC1);
    testSimple0b4(CV_8UC3);
    testSimple0b4(CV_8UC4);
  }

  SECTION("Testing RotationDegradation::rotateFillBorder produces output image of same type and size than input image")
  {
    testSimple0c(CV_8UC1);
    testSimple0c(CV_8UC3);
    testSimple0c(CV_8UC4);
  }

  SECTION("Testing RotationDegradation::rotateFillInpaint1 produces output image of same type and size than input image")
  {
    testSimple0d(CV_8UC1);
    testSimple0d(CV_8UC3);
    testSimple0d(CV_8UC4);
  }

  SECTION("Testing RotationDegradation::rotateFillInpaint2 produces output image of same type and size than input image")
  {
    testSimple0e(CV_8UC1);
    testSimple0e(CV_8UC3);
    testSimple0e(CV_8UC4);
  }

  SECTION("Testing RotationDegradation::rotateFillInpaint3 produces output image of same type and size than input image")
  {
    testSimple0f(CV_8UC1);
    testSimple0f(CV_8UC3);
    testSimple0f(CV_8UC4);
  }


}
