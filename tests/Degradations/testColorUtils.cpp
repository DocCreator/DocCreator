#include <catch2/catch_test_macros.hpp>

#include <opencv2/core/core.hpp>

#include "Degradations/ColorUtils.hpp"


namespace {

  //@return an image whose three first channels hold the same value everywhere
  cv::Mat
  makeGrayImage(int rows, int cols, int imageType)
  {
    cv::Mat img(rows, cols, imageType);
    cv::RNG rng(42);
    for (int y = 0; y < rows; ++y) {
      uchar *p = img.ptr<uchar>(y);
      for (int x = 0; x < cols; ++x, p += img.channels()) {
        const uchar v = static_cast<uchar>(rng.uniform(0, 256));
        p[0] = v;
        p[1] = v;
        p[2] = v;
        if (img.channels() == 4)
          p[3] = static_cast<uchar>(rng.uniform(0, 256)); //alpha: ignored
      }
    }
    return img;
  }

  void
  setColored(cv::Mat &img, int y, int x)
  {
    uchar *p = img.ptr<uchar>(y) + x * img.channels();
    p[1] = static_cast<uchar>(p[0] ^ 0x55);
  }

} //namespace


TEST_CASE("Testing isGray")
{
  SECTION("a one channel image is gray")
  {
    REQUIRE( dc::isGray(cv::Mat(10, 10, CV_8UC1, cv::Scalar(37))) );
  }

  SECTION("an image with equal channels is gray")
  {
    for (int imageType : { CV_8UC3, CV_8UC4 }) {
      REQUIRE( dc::isGray(makeGrayImage(17, 23, imageType)) );
      REQUIRE( dc::isGray(makeGrayImage(1, 1, imageType)) );
      REQUIRE( dc::isGray(makeGrayImage(1, 7, imageType)) );
      REQUIRE( dc::isGray(makeGrayImage(7, 1, imageType)) );
    }
  }

  SECTION("the alpha channel is not taken into account")
  {
    cv::Mat img(8, 8, CV_8UC4, cv::Scalar(70, 70, 70, 11));
    REQUIRE( dc::isGray(img) );
  }

  SECTION("a single colored pixel is found, wherever it is")
  {
    //The scan tests four pixels at a time: a colored pixel must be found
    //whatever its position in the image, and whatever the image width modulo
    //four.
    for (int imageType : { CV_8UC3, CV_8UC4 }) {
      for (int cols = 1; cols <= 9; ++cols) {
        for (int rows = 1; rows <= 5; ++rows) {
          for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
              cv::Mat img = makeGrayImage(rows, cols, imageType);
              REQUIRE( dc::isGray(img) );
              setColored(img, y, x);
              REQUIRE_FALSE( dc::isGray(img) );
            }
          }
        }
      }
    }
  }

  SECTION("a colored pixel is found in a non continuous image")
  {
    //A region of interest is not continuous: its rows must all be scanned.
    //[Only the first row used to be, over and over.]
    for (int imageType : { CV_8UC3, CV_8UC4 }) {
      cv::Mat big = makeGrayImage(20, 30, imageType);
      const cv::Rect roi(3, 2, 20, 15);

      cv::Mat view = big(roi);
      REQUIRE_FALSE( view.isContinuous() );
      REQUIRE( dc::isGray(view) );

      for (int y = 0; y < roi.height; ++y) {
        cv::Mat colored = makeGrayImage(20, 30, imageType);
        cv::Mat coloredView = colored(roi);
        setColored(coloredView, y, roi.width / 2);
        REQUIRE_FALSE( dc::isGray(coloredView) );
      }

      //a colored pixel outside the region of interest must not be seen
      cv::Mat outside = makeGrayImage(20, 30, imageType);
      setColored(outside, 0, 0);
      REQUIRE( dc::isGray(outside(roi)) );
      REQUIRE_FALSE( dc::isGray(outside) );
    }
  }
}
