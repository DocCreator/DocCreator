#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Degradations/ShadowBinding.hpp"

#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui/highgui.hpp> //DEBUG


static
void
testSimple0(int imageType)
{
  //Apply ShadowBinding of given distance
  //and check that the output type is the same than the input type.

  const int ROWS = 100;
  const int COLS = 100;    
    
  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );  

  const dc::ShadowBinding::Border border = dc::ShadowBinding::Border::LEFT;
  const int distance = ROWS/5;
  const float intensity = 0.5;
  const float angle = 45;
  
  const cv::Mat out = dc::ShadowBinding::shadowBinding(img, border, distance, intensity, angle);

  REQUIRE( out.type() == imageType );
}

static
void
testSimple1(int imageType)
{
  //Apply ShadowBinding of given distance
  //and check that the output type is the same than the input type.

  const int ROWS = 101;
  const int COLS = 99;    
    
  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );  

  const dc::ShadowBinding::Border border = dc::ShadowBinding::Border::RIGHT;
  const int distanceRatio = 1.f/5.f;
  const float intensity = 0.5;
  const float angle = 45;
  
  const cv::Mat out = dc::ShadowBinding::shadowBinding(img, distanceRatio, border, intensity, angle);

  REQUIRE( out.type() == imageType );
}

TEST_CASE( "Testing ShadowBinding" )
{ 

  SECTION("Testing ShadowBinding with given shadow width produces output image of same type")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }

  SECTION("Testing ShadowBinding with dynamic shadow width produces output image of same type")
  {
    testSimple1(CV_8UC1);
    testSimple1(CV_8UC3);
    testSimple1(CV_8UC4);
  }

  //TODO: check that pixels outside shadows stay the same 

}
