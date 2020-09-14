#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Degradations/GradientDomainDegradation.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp> //DEBUG

#include "paths.hpp" //PATH_STAIN_IMAGES
#include "testCommon.hpp" //checkEqual

static
void
testSimple0(int imageType)
{
  //Apply gradient-domain degradation with given insertType
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.
  
  const int ROWS = 498; //bigger than all stain images to be sure they will be inserted.
  const int COLS = 499;
    
  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );  

  cv::Mat imgClone = img.clone();
  
  const size_t numStainsToInsert = 2; //low to not take too long...
  const bool doRotations = true;
  
  for (int i=0; i<3; ++i) {
    cv::Mat img2 = img.clone();
    assert(img2.type() == imageType);
    
    const dc::GradientDomainDegradation::InsertType insertType = (dc::GradientDomainDegradation::InsertType)(i);

    const cv::Mat out = dc::GradientDomainDegradation::degradation(img2, STAIN_IMAGES_PATH, numStainsToInsert, insertType, doRotations);

    REQUIRE( out.type() == imageType );
    REQUIRE( checkEqual(img, imgClone) );
    REQUIRE( out.size() == img.size() );
  }
}

TEST_CASE( "Testing GradientDomainDegradation" )
{ 

  SECTION("Testing GradientDomainDegradation produces output image of same type and size than input image")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }


}
