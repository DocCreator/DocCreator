#include <iostream>

#include "testCommon.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

static
void
testCheckEqual(int imageType)
{
  const int rows = 11;
  const int cols = 7;
  cv::Mat m1(rows, cols, imageType);
  cv::Mat m2(rows, cols, imageType);
  cv::Mat m3(rows, cols, imageType);
  cv::Mat m4 = cv::Mat::eye(rows, cols, imageType);
  cv::Mat m5 = cv::Mat::eye(rows, cols, imageType);
  cv::Mat m6 = cv::Mat::eye(rows, cols, imageType);

  cv::Scalar v1(7,9,3,1);
  cv::Scalar v2(5,7,2,9);

  m1 = v1;
  m2 = v2;
  m3 = v1;
  m4 = m4 + v1;
  m5 = m5 + v2;
  m6 = m6 + v1;

  REQUIRE( checkEqual(m1, m3) == true );
  REQUIRE( checkEqual(m1, m2) == false );
  REQUIRE( checkEqual(m4, m6) == true );
  REQUIRE( checkEqual(m4.t(), m6) == false );
  REQUIRE( checkEqual(m5, m6) == false );
}

TEST_CASE( "Testing common functions" )
{ 

  SECTION("Testing checkEqual")
  {
    testCheckEqual(CV_8UC1);
    testCheckEqual(CV_8UC3);
    testCheckEqual(CV_8UC4);
  }


}
