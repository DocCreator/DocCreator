#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Degradations/BlurFilter.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp> //DEBUG


static
void
testSimple0(int imageType)
{
  //Apply blur to whole image
  //and check that the output type is the same than the input type.

  const int ROWS = 100;
  const int COLS = 100;    
    
  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );  

  const int intensity = 3;
  
  for (int i=0; i<3; ++i) {
    const dc::BlurFilter::Method method = (dc::BlurFilter::Method)(i);

    const cv::Mat img2 = img.clone();
    REQUIRE( imageType == img2.type() );  
    
    //apply blur to whole image
    const cv::Mat out = dc::BlurFilter::blur(img2, method, intensity);

    REQUIRE( out.type() == imageType );
  }
}

static
void
testSimple1(int imageType)
{
  //Apply blur to part of the image
  //and check that the output type is the same than the input type.

  const int ROWS = 100;
  const int COLS = 100;    
    
  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );  

  const int intensity = 3;

  const dc::BlurFilter::Function function = dc::BlurFilter::Function::LINEAR;
  const dc::BlurFilter::Area area = dc::BlurFilter::Area::CENTERED;
  const float coeff = 1.f;
  const int vertical = 0;
  const int horizontal = 0;
  const int radius = 2;
  
  for (int i=0; i<3; ++i) {
    const dc::BlurFilter::Method method = (dc::BlurFilter::Method)(i);

    const cv::Mat img2 = img.clone();
    REQUIRE( imageType == img2.type() );  
    
    //apply blur to part of the image
    const cv::Mat out = dc::BlurFilter::blur(img2, method, intensity, function, area, coeff, vertical, horizontal, radius); 
    
    REQUIRE( out.type() == imageType );
  }
}


TEST_CASE( "Testing BlurFilter" )
{ 

  SECTION("Testing blur on whole image produces same type output")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }

  SECTION("Testing blur on part of image produces same type output")
  {
    testSimple1(CV_8UC1);
    testSimple1(CV_8UC3);
    testSimple1(CV_8UC4);
  }


}
