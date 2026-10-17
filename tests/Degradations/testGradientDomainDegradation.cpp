#include <iostream>
#include <vector>

#include <catch2/catch_test_macros.hpp>

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
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

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

/*
  copyOnto() threw on every 4-channel image: the alpha channel was taken
  from an image that has none.
*/

//a stain with strong gradients, so that MIXED_CLONE keeps it
static
cv::Mat
makeStripedStain(int imageType)
{
  cv::Mat stain(20, 20, CV_8UC3, cv::Scalar::all(255));
  for (int x = 0; x < stain.cols; x += 4)
    stain.colRange(x, x + 2).setTo(cv::Scalar::all(0));
  if (imageType == CV_8UC4) {
    cv::Mat stain4;
    cv::cvtColor(stain, stain4, cv::COLOR_BGR2BGRA);
    return stain4;
  }
  if (imageType == CV_8UC1) {
    cv::Mat stain1;
    cv::cvtColor(stain, stain1, cv::COLOR_BGR2GRAY);
    return stain1;
  }
  return stain;
}

static
void
testCopyOnto(int imageType, int stainType)
{
  const int ROWS = 60;
  const int COLS = 80;
  cv::Mat img(ROWS, COLS, imageType, cv::Scalar::all(200));
  if (imageType == CV_8UC4) {
    //an alpha channel that is not uniform, to check that it is kept
    std::vector<cv::Mat> chans;
    cv::split(img, chans);
    for (int y = 0; y < ROWS; ++y)
      chans[3].row(y).setTo(cv::Scalar(50 + 2 * y));
    cv::merge(chans, img);
  }
  const cv::Mat imgClone = img.clone();
  const cv::Mat stain = makeStripedStain(stainType);

  const cv::Mat out =
    dc::GradientDomainDegradation::copyOnto(img, stain, cv::Point(COLS / 2, ROWS / 2));

  REQUIRE( out.type() == imageType );
  REQUIRE( out.size() == img.size() );
  REQUIRE( checkEqual(img, imgClone) );

  std::vector<cv::Mat> outChans, imgChans;
  cv::split(out, outChans);
  cv::split(img, imgChans);
  //the stain is inserted around the center...
  cv::Mat diff;
  cv::absdiff(outChans[0], imgChans[0], diff);
  REQUIRE( cv::countNonZero(diff(cv::Rect(COLS / 2 - 8, ROWS / 2 - 8, 16, 16))) > 0 );
  //...and nothing changes far from it
  REQUIRE( cv::countNonZero(diff(cv::Rect(0, 0, 10, 10))) == 0 );
  if (imageType == CV_8UC4) {
    REQUIRE( checkEqual(outChans[3], imgChans[3]) );
  }
}

TEST_CASE( "Testing GradientDomainDegradation" )
{ 

  SECTION("Testing copyOnto keeps the type, the size and the alpha channel of the image")
  {
    for (const int imageType : {CV_8UC1, CV_8UC3, CV_8UC4}) {
      testCopyOnto(imageType, CV_8UC3);
    }
    testCopyOnto(CV_8UC4, CV_8UC4);
    testCopyOnto(CV_8UC4, CV_8UC1);
  }

  SECTION("Testing GradientDomainDegradation produces output image of same type and size than input image")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }


}

/*
  cv::seamlessClone() threw (cv::Exception, or std::length_error) when the
  stain, or the part of it that overlaps the image, was 1, 3 or 4 pixels
  wide or high: a stain near a border failed whatever its size.
*/
TEST_CASE( "Testing copyOnto with small stains and on the borders" )
{
  for (const int imageType : {CV_8UC1, CV_8UC3, CV_8UC4}) {
    INFO( "imageType=" << imageType );
    const cv::Mat img(60, 80, imageType, cv::Scalar::all(200));
    const cv::Point positions[] = {cv::Point(40, 30), cv::Point(0, 0), cv::Point(79, 59),
                                   cv::Point(2, 30), cv::Point(-3, 30)};
    for (const cv::Point &pos : positions) {
      for (int w = 1; w <= 12; ++w) {
        for (int h = 1; h <= 12; ++h) {
          INFO( "stain " << w << "x" << h << " at (" << pos.x << ", " << pos.y << ")" );
          const cv::Mat stain(h, w, CV_8UC3, cv::Scalar::all(0));
          cv::Mat out;
          REQUIRE_NOTHROW( out = dc::GradientDomainDegradation::copyOnto(img, stain, pos) );
          REQUIRE( out.type() == imageType );
          REQUIRE( out.size() == img.size() );
        }
      }
    }

    //too small to be inserted: the image as it was
    REQUIRE( checkEqual(dc::GradientDomainDegradation::copyOnto(img, cv::Mat(3, 3, CV_8UC3, cv::Scalar::all(0)), cv::Point(40, 30)), img) );
    //a stain large enough is still inserted
    cv::Mat stain(20, 20, CV_8UC3, cv::Scalar::all(255));
    for (int x = 0; x < stain.cols; x += 4)
      stain.colRange(x, x + 2).setTo(cv::Scalar::all(0));
    REQUIRE_FALSE( checkEqual(dc::GradientDomainDegradation::copyOnto(img, stain, cv::Point(40, 30)), img) );
  }
}
