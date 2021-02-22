#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Degradations/ElasticDeformation.hpp"

#include "testCommon.hpp" //checkEqual

static
void
testSimple0(int imageType)
{
  //Apply ElasticDeformation degradation on image of given type.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 398;
  const int COLS = 299;

  cv::Mat img(ROWS, COLS, imageType);
  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );  

  cv::Mat img2 = img.clone();
  assert(img2.type() == imageType);

  const float alpha = 2.0f;
  const float sigma = 0.083f;
  const dc::ElasticDeformation::BorderReplication borderMode = dc::ElasticDeformation::BorderReplication::BLACK;
  const dc::ElasticDeformation::Interpolation interpolation = dc::ElasticDeformation::Interpolation::BILINEAR;

  const cv::Mat out = dc::ElasticDeformation::transform(img2, alpha, sigma, borderMode, interpolation);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, img2) );
  REQUIRE( out.size() == img.size() );
  
}

static
void
testSimple1(int imageType)
{
  //Apply ElasticDeformation degradation (transform2) on image of given type.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.

  const int ROWS = 397;
  const int COLS = 293;

  cv::Mat img(ROWS, COLS, imageType);
  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );

  cv::Mat img2 = img.clone();
  assert(img2.type() == imageType);

  const float alpha = 2.1f;
  const float sigma = 0.087f;
  const float alpha_affine = 8.9f;
  const dc::ElasticDeformation::BorderReplication borderMode = dc::ElasticDeformation::BorderReplication::BLACK;
  const dc::ElasticDeformation::Interpolation interpolation = dc::ElasticDeformation::Interpolation::BILINEAR;

  const cv::Mat out = dc::ElasticDeformation::transform2(img2, alpha, sigma, alpha_affine,
							 borderMode, interpolation);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, img2) );
  REQUIRE( out.size() == img.size() );
}

static
void
testSimple2(int imageType)
{
  //Apply ElasticDeformation degradation on image of given type, with rectangles.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.
  //Check that we have the same number of input and transformed rectangles.

  const int ROWS = 378;
  const int COLS = 289;

  cv::Mat img(ROWS, COLS, imageType);
  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );

  cv::Mat img2 = img.clone();
  assert(img2.type() == imageType);

  const float alpha = 2.0f;
  const float sigma = 0.083f;
  const dc::ElasticDeformation::BorderReplication borderMode = dc::ElasticDeformation::BorderReplication::BLACK;
  const dc::ElasticDeformation::Interpolation interpolation = dc::ElasticDeformation::Interpolation::BILINEAR;

  std::vector<cv::Rect> rects;
  rects.push_back(cv::Rect(22, 22, 40, 20));
  rects.push_back(cv::Rect(62, 22, 39, 21));
  rects.push_back(cv::Rect(42, 52, 39, 21));
  rects.push_back(cv::Rect(ROWS/2, COLS/2, ROWS/4, COLS/4));
  std::vector<std::vector<cv::Point2f> > rects_pts;

  const cv::Mat out = dc::ElasticDeformation::transform_rects(img2,
							      rects, rects_pts,
							      alpha, sigma, borderMode, interpolation);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, img2) );
  REQUIRE( out.size() == img.size() );

  REQUIRE( rects_pts.size() == rects.size() );

  std::vector<cv::RotatedRect> rotatedRects = dc::ElasticDeformation::getRotatedRects(rects_pts);

  REQUIRE( rotatedRects.size() == rects.size() );

}

static
void
testSimple3(int imageType)
{
  //Apply ElasticDeformation degradation (transform2) on image of given type, with rectangles.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.
  //Check that we have the same number of input and transformed rectangles.

  const int ROWS = 387;
  const int COLS = 297;

  cv::Mat img(ROWS, COLS, imageType);
  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );

  cv::Mat img2 = img.clone();
  assert(img2.type() == imageType);

  const float alpha = 2.1f;
  const float sigma = 0.087f;
  const float alpha_affine = 8.9f;
  const dc::ElasticDeformation::BorderReplication borderMode = dc::ElasticDeformation::BorderReplication::BLACK;
  const dc::ElasticDeformation::Interpolation interpolation = dc::ElasticDeformation::Interpolation::BILINEAR;

  std::vector<cv::Rect> rects;
  rects.push_back(cv::Rect(22, 22, 40, 20));
  rects.push_back(cv::Rect(52, 22, 39, 21));
  rects.push_back(cv::Rect(62, 22, 39, 21));
  rects.push_back(cv::Rect(42, 52, 39, 21));
  rects.push_back(cv::Rect(ROWS/2, COLS/2, ROWS/4, COLS/4));
  std::vector<std::vector<cv::Point2f> > rects_pts;

  const cv::Mat out = dc::ElasticDeformation::transform2_rects(img2, rects, rects_pts,
							       alpha, sigma, alpha_affine,
							       borderMode, interpolation);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, img2) );
  REQUIRE( out.size() == img.size() );

  REQUIRE( rects_pts.size() == rects.size() );

  std::vector<cv::RotatedRect> rotatedRects = dc::ElasticDeformation::getRotatedRects(rects_pts);

  REQUIRE( rotatedRects.size() == rects.size() );
}


TEST_CASE( "Testing ElasticDeformation" )
{ 

  SECTION("Testing ElasticDeformation::transform produces output image of same type and size")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }

  SECTION("Testing ElasticDeformation::transform2 produces output image of same type and size")
  {
    testSimple1(CV_8UC1);
    testSimple1(CV_8UC3);
    testSimple1(CV_8UC4);
  }

  SECTION("Testing ElasticDeformation::transform_rects produces output image of same type and size, and same number of rectangles")
  {
    testSimple2(CV_8UC1);
    testSimple2(CV_8UC3);
    testSimple2(CV_8UC4);
  }

  SECTION("Testing ElasticDeformation::transform2_rects produces output image of same type and size, and same number of rectangles")
  {
    testSimple3(CV_8UC1);
    testSimple3(CV_8UC3);
    testSimple3(CV_8UC4);
  }

}
