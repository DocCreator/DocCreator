#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Degradations/PhantomCharacter.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp> //DEBUG


static const char *PATTERN_PATH = "../data/Image/phantomPatterns/";

static
void
testSimple0(int imageType)
{
  //Apply PhantomCharacter of given distance
  //and check that the output type is the same than the input type.

  const int ROWS = 100;
  const int COLS = 100;    
    
  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );  

  for (int i=0; i<3; ++i) {
    cv::Mat img2 = img.clone();
    assert(img2.type() == imageType);
    
    const dc::PhantomCharacter::Frequency frequency = (dc::PhantomCharacter::Frequency)(i);

    const cv::Mat out = dc::PhantomCharacter::phantomCharacter(img, frequency, PATTERN_PATH);

    REQUIRE( out.type() == imageType );
  }
}

TEST_CASE( "Testing PhantomCharacter" )
{ 

  SECTION("Testing PhantomCharacter produces same type output")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }


}
