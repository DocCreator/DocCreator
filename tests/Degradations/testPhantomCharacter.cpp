#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include "Degradations/PhantomCharacter.hpp"

#include <opencv2/imgproc/imgproc.hpp>

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
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

  REQUIRE( imageType == img.type() );  

  cv::Mat imgClone = img.clone();

  const float occurenceProbabilities[] = {0.15f, 0.40f, 0.70f};
  const int N = sizeof(occurenceProbabilities)/sizeof(float);
  
  for (int i=0; i<N; ++i) {
    cv::Mat img2 = img.clone();
    assert(img2.type() == imageType);
    
    const cv::Mat out = dc::PhantomCharacter::phantomCharacter(img2, occurenceProbabilities[i], PHANTOM_PATTERNS_PATH);

    REQUIRE( out.type() == imageType );

    REQUIRE( checkEqual(img, imgClone) );
    REQUIRE( out.size() == img.size() );
  }
}

/*
  On the right of a narrow component near the left border, with no
  neighbour, the pattern was placed before x = 0: written out of the rows
  of the image, and then a cv::Exception from the rectangle of its blur.
*/
static
cv::Mat
makeLeftBorderImage(int imageType)
{
  //as many bars as blocks, so that both pass the filter on the areas of
  //phantomCharacter() (bars 3x20 = 60, blocks 6x21 = 126): the bars cannot
  //hold a pattern, the blocks can
  cv::Mat img(1220, 300, imageType, cv::Scalar::all(255));
  for (int k = 0; k < 30; ++k) {
    //a bar on the left border, alone on its line
    img(cv::Rect(0, 10 + 40 * k, 3, 20)).setTo(cv::Scalar::all(0));
  }
  for (int k = 0; k < 6; ++k)
    for (int j = 0; j < 5; ++j)
      img(cv::Rect(60 + 40 * j, 30 + 40 * k, 6, 21)).setTo(cv::Scalar::all(0));
  return img;
}

static
void
testLeftBorder(int imageType)
{
  const cv::Mat img = makeLeftBorderImage(imageType);
  for (uint32_t seed = 1; seed <= 20; ++seed) {
    INFO( "seed=" << seed );
    cv::Mat out;
    REQUIRE_NOTHROW( out = dc::PhantomCharacter::phantomCharacter(img, 1.f, PHANTOM_PATTERNS_PATH, seed) );
    REQUIRE( out.type() == imageType );
    REQUIRE( out.size() == img.size() );
  }
}

static
cv::Mat
makeTextImage(int imageType)
{
  cv::Mat img(500, 900, CV_8UC3, cv::Scalar(235, 235, 235));
  for (int y = 50; y < img.rows; y += 45)
    cv::putText(img, "Lorem ipsum dolor sit amet, consectetur adipiscing",
                cv::Point(20, y), cv::FONT_HERSHEY_DUPLEX, 0.9,
                cv::Scalar(20, 20, 20), 2, cv::LINE_AA);
  if (imageType == CV_8UC1)
    cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
  else if (imageType == CV_8UC4)
    cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);
  return img;
}

static
void
testTextChanges(int imageType)
{
  //Phantom characters are added to a text, the same for the same seed,
  //and the input is not modified.
  const cv::Mat img = makeTextImage(imageType);
  const cv::Mat imgClone = img.clone();
  const cv::Mat a = dc::PhantomCharacter::phantomCharacter(img, 0.5f, PHANTOM_PATTERNS_PATH, 42);
  const cv::Mat b = dc::PhantomCharacter::phantomCharacter(img, 0.5f, PHANTOM_PATTERNS_PATH, 42);
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( a.type() == imageType );
  REQUIRE_FALSE( checkEqual(a, img) );
  REQUIRE( checkEqual(a, b) );
}

TEST_CASE( "Testing PhantomCharacter" )
{ 

  SECTION("Testing PhantomCharacter near the left border of the image")
  {
    testLeftBorder(CV_8UC1);
    testLeftBorder(CV_8UC3);
    testLeftBorder(CV_8UC4);
  }

  SECTION("Testing PhantomCharacter changes a text, reproducibly")
  {
    testTextChanges(CV_8UC1);
    testTextChanges(CV_8UC3);
    testTextChanges(CV_8UC4);
  }

  SECTION("Testing PhantomCharacter produces output image of same type and size than input image")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }


}
