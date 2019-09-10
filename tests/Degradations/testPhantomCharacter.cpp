#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Degradations/PhantomCharacter.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp> //DEBUG

#include "paths.hpp"
#include "testCommon.hpp" //checkEqual

static
void
testSimple0(int imageType)
{
  //Apply PhantomCharacter with given frequency
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
  
  for (int i=0; i<3; ++i) {
    cv::Mat img2 = img.clone();
    assert(img2.type() == imageType);
    
    const dc::PhantomCharacter::Frequency frequency = (dc::PhantomCharacter::Frequency)(i);

    const cv::Mat out = dc::PhantomCharacter::phantomCharacter(img2, frequency, PHANTOM_PATTERNS_PATH);

    REQUIRE( out.type() == imageType );

    REQUIRE( checkEqual(img, imgClone) );
    REQUIRE( out.size() == img.size() );
  }
}

TEST_CASE( "Testing PhantomCharacter" )
{ 

  SECTION("Testing PhantomCharacter produces output image of same type and size")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }


}
