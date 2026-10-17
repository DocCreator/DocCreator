#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>

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
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

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
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

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

static const dc::ShadowBinding::Border ALL_BORDERS[] = {
  dc::ShadowBinding::Border::TOP, dc::ShadowBinding::Border::RIGHT,
  dc::ShadowBinding::Border::BOTTOM, dc::ShadowBinding::Border::LEFT};

//width or height of @a img, along which the shadow of @a border goes
static
int
shadowDimension(const cv::Mat &img, dc::ShadowBinding::Border border)
{
  return (border == dc::ShadowBinding::Border::LEFT ||
          border == dc::ShadowBinding::Border::RIGHT)
    ? img.cols : img.rows;
}

//the band of @a img of width @a distance along @a border
static
cv::Rect
shadowRect(const cv::Mat &img, dc::ShadowBinding::Border border, int distance)
{
  switch (border) {
  case dc::ShadowBinding::Border::LEFT:
    return cv::Rect(0, 0, distance, img.rows);
  case dc::ShadowBinding::Border::RIGHT:
    return cv::Rect(img.cols - distance, 0, distance, img.rows);
  case dc::ShadowBinding::Border::TOP:
    return cv::Rect(0, 0, img.cols, distance);
  case dc::ShadowBinding::Border::BOTTOM:
  default:
    return cv::Rect(0, img.rows - distance, img.cols, distance);
  }
}

static
void
testDistanceOutOfImage(int imageType)
{
  //A distance larger than the image wrote out of it.
  const cv::Mat img(31, 47, imageType, cv::Scalar::all(200));
  for (const auto border : ALL_BORDERS) {
    const int dim = shadowDimension(img, border);
    REQUIRE_THROWS_AS( dc::ShadowBinding::shadowBinding(img, border, dim + 1, .5f, 45.f),
                       std::invalid_argument );
    REQUIRE_THROWS_AS( dc::ShadowBinding::shadowBinding(img, border, 10 * dim, .5f, 45.f),
                       std::invalid_argument );
    REQUIRE_THROWS_AS( dc::ShadowBinding::shadowBinding(img, border, -1, .5f, 45.f),
                       std::invalid_argument );

    //the whole image, and nothing, are accepted
    const cv::Mat full = dc::ShadowBinding::shadowBinding(img, border, dim, .5f, 45.f);
    REQUIRE( full.size() == img.size() );
    REQUIRE( full.type() == imageType );
    const cv::Mat none = dc::ShadowBinding::shadowBinding(img, border, 0, .5f, 45.f);
    REQUIRE( checkEqual(none, img) );
  }
}

static
void
testOutOfRangeParameters()
{
  const cv::Mat img(31, 47, CV_8UC3, cv::Scalar::all(200));
  const auto border = dc::ShadowBinding::Border::LEFT;
  const float nan = std::numeric_limits<float>::quiet_NaN();

  for (const float intensity : {-0.01f, 1.01f, nan}) {
    REQUIRE_THROWS_AS( dc::ShadowBinding::shadowBinding(img, border, 10, intensity, 45.f),
                       std::invalid_argument );
    REQUIRE_THROWS_AS( dc::ShadowBinding::shadowBinding2(img, .2f, border, intensity, 45.f),
                       std::invalid_argument );
  }
  for (const float angle : {-1.f, 90.5f, nan}) {
    REQUIRE_THROWS_AS( dc::ShadowBinding::shadowBinding(img, border, 10, .5f, angle),
                       std::invalid_argument );
    REQUIRE_THROWS_AS( dc::ShadowBinding::shadowBinding2(img, .2f, border, .5f, angle),
                       std::invalid_argument );
  }
  for (const float ratio : {-0.01f, 1.01f, 5.f, nan}) {
    REQUIRE_THROWS_AS( dc::ShadowBinding::shadowBinding2(img, ratio, border, .5f, 45.f),
                       std::invalid_argument );
  }

  //the bounds are accepted
  for (const auto b : ALL_BORDERS) {
    REQUIRE( dc::ShadowBinding::shadowBinding2(img, 1.f, b, 1.f, 90.f).size() == img.size() );
    REQUIRE( dc::ShadowBinding::shadowBinding2(img, 0.f, b, 0.f, 0.f).size() == img.size() );
  }
}

static
void
testOnlyTheShadowChanges(int imageType)
{
  //Pixels out of the shadow stay the same, and the border of the image,
  //where the shadow is the darkest, gets darker. Alpha is kept.
  const cv::Mat img(41, 53, imageType, cv::Scalar::all(200));
  const int distance = 12;
  for (const auto border : ALL_BORDERS) {
    const cv::Mat out = dc::ShadowBinding::shadowBinding(img, border, distance, .5f, 45.f);
    const cv::Rect shadow = shadowRect(img, border, distance);

    //out of the shadow: unchanged
    cv::Mat outCopy = out.clone();
    cv::Mat imgCopy = img.clone();
    outCopy(shadow).setTo(cv::Scalar::all(0));
    imgCopy(shadow).setTo(cv::Scalar::all(0));
    REQUIRE( checkEqual(outCopy, imgCopy) );

    //the row or column on the border itself: darker
    cv::Rect edge = shadow;
    if (border == dc::ShadowBinding::Border::LEFT || border == dc::ShadowBinding::Border::RIGHT) {
      edge.width = 1;
      edge.x = (border == dc::ShadowBinding::Border::RIGHT) ? img.cols - 1 : 0;
    }
    else {
      edge.height = 1;
      edge.y = (border == dc::ShadowBinding::Border::BOTTOM) ? img.rows - 1 : 0;
    }
    std::vector<cv::Mat> chans;
    cv::split(out(edge), chans);
    double maxV = 0;
    cv::minMaxLoc(chans[0], nullptr, &maxV);
    REQUIRE( maxV < 200 );
    if (imageType == CV_8UC4) {
      std::vector<cv::Mat> allChans;
      cv::split(out, allChans);
      REQUIRE( cv::countNonZero(allChans[3] != 200) == 0 );
    }
  }
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

  SECTION("Testing ShadowBinding refuses a distance out of the image")
  {
    testDistanceOutOfImage(CV_8UC1);
    testDistanceOutOfImage(CV_8UC3);
    testDistanceOutOfImage(CV_8UC4);
  }

  SECTION("Testing ShadowBinding refuses out-of-range parameters")
  {
    testOutOfRangeParameters();
  }

  SECTION("Testing ShadowBinding only changes the shadowed band")
  {
    testOnlyTheShadowChanges(CV_8UC1);
    testOnlyTheShadowChanges(CV_8UC3);
    testOnlyTheShadowChanges(CV_8UC4);
  }

}
