#include <cmath> //std::abs
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "Degradations/ElasticDeformation.hpp"

#include <opencv2/core/version.hpp>
#if CV_VERSION_MAJOR >= 5
#include <opencv2/geometry.hpp> //boundingRect
#else
#include <opencv2/imgproc.hpp>
#endif

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
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

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
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

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
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

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
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

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


/*
  The displacement field is smoothed by a Gaussian whose sigma is the one
  given multiplied by the smaller side of the image, so that a deformation
  looks the same whatever the resolution. Above a few pixels of sigma the
  field is drawn and smoothed on a coarser grid and stretched back, which
  is the only way a page of several thousand pixels is affordable: the
  kernel would otherwise be as wide as the image is large.

  The images of the cases above are 100x100, where sigma stays small
  enough for the field to be built at full size; this one is big enough
  for the other branch, so that both are covered.
*/
static
void
testLargeImage(int imageType)
{
  const int ROWS = 700;
  const int COLS = 601;

  cv::Mat img(ROWS, COLS, imageType);
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));
  const cv::Mat imgClone = img.clone();

  const float alpha = 20.f;
  const float sigma = .08f; //x 601 = 48 pixels: the coarse grid is used

  const cv::Mat a = dc::ElasticDeformation::transform(
    img, alpha, sigma,
    dc::ElasticDeformation::BorderReplication::BLACK,
    dc::ElasticDeformation::Interpolation::BILINEAR, 42);
  const cv::Mat b = dc::ElasticDeformation::transform(
    img, alpha, sigma,
    dc::ElasticDeformation::BorderReplication::BLACK,
    dc::ElasticDeformation::Interpolation::BILINEAR, 42);
  const cv::Mat c = dc::ElasticDeformation::transform(
    img, alpha, sigma,
    dc::ElasticDeformation::BorderReplication::BLACK,
    dc::ElasticDeformation::Interpolation::BILINEAR, 43);

  REQUIRE( a.type() == imageType );
  REQUIRE( a.size() == img.size() );
  REQUIRE( checkEqual(img, imgClone) ); //input not modified
  REQUIRE( checkEqual(a, b) );          //same seed => same image
  REQUIRE_FALSE( checkEqual(a, c) );    //another seed => another image
  REQUIRE_FALSE( checkEqual(a, img) );  //and something was deformed

  /*
    The deformation moves the pixels, it does not change what they are:
    the mean of the image is kept, apart from the border the deformation
    pulls in. A field built with the wrong amplitude shows up here.
  */
  const cv::Scalar meanIn = cv::mean(img);
  const cv::Scalar meanOut = cv::mean(a);
  REQUIRE( std::abs(meanOut[0] - meanIn[0]) < 0.1 * meanIn[0] );
}


/*
  transform2() gave cv::warpAffine() the border mode as its interpolation:
  the border of the affine part was always black. And BLACK itself gave
  BORDER_REPLICATE.
*/
static
void
testBorderModes(int imageType)
{
  //a uniform image, no elastic displacement, and a strong affine jitter
  //that uncovers the borders: only the border mode can change a pixel
  const cv::Mat img(90, 120, imageType, cv::Scalar::all(200));
  const float alpha = 0.f;
  const float sigma = 0.08f;
  const float alphaAffine = 15.f;
  using dc::ElasticDeformation::BorderReplication;
  for (const BorderReplication mode : {BorderReplication::REPLICATE,
                                       BorderReplication::REFLECT,
                                       BorderReplication::WRAP,
                                       BorderReplication::REFLECT101}) {
    const cv::Mat out = dc::ElasticDeformation::transform2(
      img, alpha, sigma, alphaAffine, mode,
      dc::ElasticDeformation::Interpolation::BILINEAR, 7);
    INFO( "border mode " << static_cast<int>(mode) );
    REQUIRE( out.type() == imageType );
    REQUIRE( checkEqual(out, img) );
  }

  const cv::Mat black = dc::ElasticDeformation::transform2(
    img, alpha, sigma, alphaAffine, BorderReplication::BLACK,
    dc::ElasticDeformation::Interpolation::BILINEAR, 7);
  std::vector<cv::Mat> chans;
  cv::split(black, chans);
  REQUIRE( cv::countNonZero(chans[0] == 0) > 0 );
  //and the middle of the image is still the image
  REQUIRE( chans[0].at<uchar>(img.rows / 2, img.cols / 2) == 200 );
}

TEST_CASE( "Testing ElasticDeformation" )
{ 

  SECTION("Testing ElasticDeformation::transform2 fills the border as asked")
  {
    testBorderModes(CV_8UC1);
    testBorderModes(CV_8UC3);
    testBorderModes(CV_8UC4);
  }

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

  SECTION("Testing ElasticDeformation::transform on an image large enough for the coarse field")
  {
    testLargeImage(CV_8UC1);
    testLargeImage(CV_8UC3);
    testLargeImage(CV_8UC4);
  }

}

/*
  sigma was not checked: 0 made cv::GaussianBlur() throw a cv::Exception.
*/
TEST_CASE( "Testing ElasticDeformation refuses a sigma that is not positive" )
{
  const cv::Mat img(40, 50, CV_8UC3, cv::Scalar::all(128));
  const float nan = std::numeric_limits<float>::quiet_NaN();
  using dc::ElasticDeformation::BorderReplication;
  using dc::ElasticDeformation::Interpolation;
  for (const float sigma : {0.f, -0.1f, nan}) {
    INFO( "sigma=" << sigma );
    REQUIRE_THROWS_AS( dc::ElasticDeformation::transform(img, 2.f, sigma,
                         BorderReplication::REPLICATE, Interpolation::BILINEAR, 1u),
                       std::invalid_argument );
    REQUIRE_THROWS_AS( dc::ElasticDeformation::transform2(img, 2.f, sigma, 5.f,
                         BorderReplication::REPLICATE, Interpolation::BILINEAR, 1u),
                       std::invalid_argument );
  }
  REQUIRE_THROWS_AS( dc::ElasticDeformation::transform(img, nan, 0.08f,
                       BorderReplication::REPLICATE, Interpolation::BILINEAR, 1u),
                     std::invalid_argument );
  REQUIRE_THROWS_AS( dc::ElasticDeformation::transform2(img, 2.f, 0.08f, nan,
                       BorderReplication::REPLICATE, Interpolation::BILINEAR, 1u),
                     std::invalid_argument );
  //a small positive sigma is fine
  REQUIRE( dc::ElasticDeformation::transform(img, 2.f, 0.001f,
             BorderReplication::REPLICATE, Interpolation::BILINEAR, 1u).size() == img.size() );
}

/*
  transform2_rects() moved the points by the elastic displacement at their
  original position and then by the affine transformation, while the image
  is moved by the affine transformation first: with a strong deformation,
  the rectangles drifted from the image (3 pixels with these parameters,
  against 1 now).
*/
TEST_CASE( "Testing ElasticDeformation::transform2_rects follows the image" )
{
  cv::Mat img(300, 400, CV_8UC1, cv::Scalar(255));
  const cv::Rect rect(120, 90, 160, 110);
  img(rect).setTo(cv::Scalar(0));
  for (uint32_t seed = 1; seed <= 20; ++seed) {
    INFO( "seed=" << seed );
    std::vector<std::vector<cv::Point2f> > pts;
    const cv::Mat out = dc::ElasticDeformation::transform2_rects(
      img, {rect}, pts, 0.2f, 0.03f, 40.f,
      dc::ElasticDeformation::BorderReplication::REPLICATE,
      dc::ElasticDeformation::Interpolation::BILINEAR, seed);
    REQUIRE( pts.size() == 1 );
    std::vector<cv::Point> darkPts;
    cv::findNonZero(out < 128, darkPts);
    const cv::Rect actual = cv::boundingRect(darkPts);
    const cv::Rect given = cv::boundingRect(pts[0]);
    REQUIRE( std::abs(actual.x - given.x) <= 2 );
    REQUIRE( std::abs(actual.y - given.y) <= 2 );
    REQUIRE( std::abs(actual.br().x - given.br().x) <= 2 );
    REQUIRE( std::abs(actual.br().y - given.br().y) <= 2 );
  }
}
