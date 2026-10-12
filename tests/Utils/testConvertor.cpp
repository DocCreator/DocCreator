#include <catch2/catch_test_macros.hpp>

#include <QImage>

#include <opencv2/core/core.hpp>

#include "Utils/convertor.h"

/*
  Convertor::getQImage() used to build its QImage, which Qt does not
  initialize, and to return it as it was for an image of a type it does not
  handle: the GUI then displayed heap content, and could save it to a file.
*/

TEST_CASE("Convertor::getQImage() handles the documented types")
{
  const int W = 7;
  const int H = 5;

  SECTION("a gray image")
  {
    cv::Mat img(H, W, CV_8UC1, cv::Scalar::all(37));
    const QImage qimg = Convertor::getQImage(img);
    REQUIRE_FALSE( qimg.isNull() );
    REQUIRE( qimg.width() == W );
    REQUIRE( qimg.height() == H );
    REQUIRE( qimg.pixel(3, 2) == qRgb(37, 37, 37) );
  }

  SECTION("a color image, BGR order")
  {
    cv::Mat img(H, W, CV_8UC3, cv::Scalar(10, 20, 30)); //B, G, R
    const QImage qimg = Convertor::getQImage(img);
    REQUIRE_FALSE( qimg.isNull() );
    REQUIRE( qimg.pixel(3, 2) == qRgb(30, 20, 10) );
  }

  SECTION("an image with an alpha channel")
  {
    cv::Mat img(H, W, CV_8UC4, cv::Scalar(10, 20, 30, 128));
    const QImage qimg = Convertor::getQImage(img);
    REQUIRE_FALSE( qimg.isNull() );
    REQUIRE( qimg.pixel(3, 2) == qRgba(30, 20, 10, 128) );
  }

  SECTION("a float image is saturated and not wrapped around")
  {
    //the conversion of a float outside [0; 256) to an integral type is
    //undefined behaviour
    cv::Mat img(H, W, CV_32FC3, cv::Scalar(-10.f, 128.4f, 300.f));
    const QImage qimg = Convertor::getQImage(img);
    REQUIRE_FALSE( qimg.isNull() );
    REQUIRE( qimg.pixel(3, 2) == qRgb(255, 128, 0) );
  }

  SECTION("a round trip through getCvMat()")
  {
    QImage src(W, H, QImage::Format_ARGB32);
    src.fill(qRgb(12, 34, 56));
    const cv::Mat mat = Convertor::getCvMat(src);
    REQUIRE( mat.type() == CV_8UC3 );
    const QImage dst = Convertor::getQImage(mat);
    REQUIRE( dst.pixel(3, 2) == qRgb(12, 34, 56) );
  }
}

TEST_CASE("Convertor::getQImage() returns a null image for the other types")
{
  const int W = 7;
  const int H = 5;

  REQUIRE( Convertor::getQImage(cv::Mat()).isNull() );

  for (int type : { CV_16UC1, CV_16UC3, CV_32SC1, CV_32SC3, CV_64FC1,
		    CV_64FC3 }) {
    cv::Mat img(H, W, type, cv::Scalar::all(1));
    REQUIRE( Convertor::getQImage(img).isNull() );
  }

  //a CV_32FC1 image, on the other hand, is converted to CV_32FC3 by
  //cvtColor() and handled
  cv::Mat img32f(H, W, CV_32FC1, cv::Scalar::all(42));
  const QImage qimg32f = Convertor::getQImage(img32f);
  REQUIRE_FALSE( qimg32f.isNull() );
  REQUIRE( qimg32f.pixel(3, 2) == qRgb(42, 42, 42) );

  //a 2-channel image is not converted to RGB, and was not handled either
  cv::Mat img2(H, W, CV_8UC2, cv::Scalar::all(1));
  REQUIRE( Convertor::getQImage(img2).isNull() );
}
