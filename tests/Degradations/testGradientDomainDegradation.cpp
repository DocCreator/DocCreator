#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Degradations/GradientDomainDegradation.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp> //DEBUG

#include "paths.hpp" //PATH_STAIN_IMAGES


static
void
testSimple0(int imageType)
{
  //Apply gradient-domain degradation with given insertType
  //and check that the output type is the same than the input type.

  const int ROWS = 498; //bigger than all stain images to be sure they will be inserted.
  const int COLS = 498;    
    
  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, 128, 30);

  const size_t numStainsToInsert = 2; //low to not take too long...
  const bool doRotations = true;
  
  REQUIRE( imageType == img.type() );  

  for (int i=0; i<3; ++i) {
    cv::Mat img2 = img.clone();
    assert(img2.type() == imageType);
    
    const dc::GradientDomainDegradation::InsertType insertType = (dc::GradientDomainDegradation::InsertType)(i);

    const cv::Mat out = dc::GradientDomainDegradation::degradation(img2, STAIN_IMAGES_PATH, numStainsToInsert, insertType, doRotations);

    REQUIRE( out.type() == imageType );
  }
}

TEST_CASE( "Testing GradientDomainDegradation" )
{ 

  SECTION("Testing GradientDomainDegradation produces output image of same type")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }


}
