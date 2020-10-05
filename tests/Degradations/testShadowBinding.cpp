#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Degradations/ShadowBinding.hpp"

#include <opencv2/imgproc/imgproc.hpp>

#include "testCommon.hpp" //checkEqual


static
void
testSimple0(int imageType)
{
  //Apply ShadowBinding of given distance
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
  
  const dc::ShadowBinding::Border border = dc::ShadowBinding::Border::LEFT;
  const int distance = ROWS/5;
  const float intensity = 0.5;
  const float angle = 45;
  
  const cv::Mat out = dc::ShadowBinding::shadowBinding(img, border, distance, intensity, angle);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
}

static
void
testSimple1(int imageType)
{
  //Apply ShadowBinding of given distance
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 101;
  const int COLS = 99;    
    
  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );  

  cv::Mat imgClone = img.clone();
  
  const dc::ShadowBinding::Border border = dc::ShadowBinding::Border::RIGHT;
  const float distanceRatio = 1.f/5.f;
  const float intensity = 0.5;
  const float angle = 45;
  
  const cv::Mat out = dc::ShadowBinding::shadowBinding2(img, distanceRatio, border, intensity, angle);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
}

TEST_CASE( "Testing ShadowBinding" )
{ 

  SECTION("Testing ShadowBinding with given shadow width produces output image of same type and size than input image")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }

  SECTION("Testing ShadowBinding with dynamic shadow width produces output image of same type and size than input image")
  {
    testSimple1(CV_8UC1);
    testSimple1(CV_8UC3);
    testSimple1(CV_8UC4);
  }

  //TODO: check that pixels outside shadows stay the same 

}
