#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

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

  dc::GrayscaleCharsDegradationModel dcm(img);

  cv::Mat out = dcm.degradateByLevel_cv(level);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
}

TEST_CASE( "Testing CharactersDegradation" )
{ 

  SECTION("Testing Characters Degradation produces output image of same type and size")
  {
    testSimple(CV_8UC1);
    //testSimple(CV_8UC3);
    //testSimple(CV_8UC4);
  }

}


