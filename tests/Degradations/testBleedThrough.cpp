#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include "Degradations/BleedThrough.hpp"
#include "Degradations/FileUtils.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp> //DEBUG

#include "paths.hpp"
#include "testCommon.hpp" //checkEqual


static
void
testSimple0(int imageType)
{
  //Apply BleedThrough (on random images) (at position (0, 0) with 1 thread)
  //Check that the output type is the same than the input type.
  //Check that the input images are not modified.
  //Check that the output size is the same than the input recto size.

  const int ROWS = 64;
  const int COLS = 64;
    
  cv::Mat imgRecto(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(imgRecto, cv::Scalar::all(128), cv::Scalar::all(30));

  REQUIRE( imageType == imgRecto.type() );

  cv::Mat imgVerso;
  cv::flip(imgRecto, imgVerso, 1);
  
  REQUIRE( imgVerso.type() == imgRecto.type() );

  cv::Mat imgRectoClone = imgRecto.clone();
  cv::Mat imgVersoClone = imgVerso.clone();
  
  const int nbIters = 3;
  const cv::Point pos(0, 0);
  const int nbThreads = 1;
  
  const cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, pos, nbThreads);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(imgRecto, imgRectoClone) );
  REQUIRE( checkEqual(imgVerso, imgVersoClone) );
  REQUIRE( out.size() == imgRecto.size() );
}

static
void
testSimple0b(int imageType)
{
  //Apply BleedThrough (on random images) (at position (ROWS/2, COLS/2) with 1 thread)
  //Check that the output type is the same than the input type.
  //Check that the input images are not modified.
  //Check that the output size is the same than the input recto size.

  const int ROWS = 64;
  const int COLS = 64;

  cv::Mat imgRecto(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(imgRecto, cv::Scalar::all(128), cv::Scalar::all(30));

  REQUIRE( imageType == imgRecto.type() );

  cv::Mat imgVerso;
  cv::flip(imgRecto, imgVerso, 1);

  REQUIRE( imgVerso.type() == imgRecto.type() );

  cv::Mat imgRectoClone = imgRecto.clone();
  cv::Mat imgVersoClone = imgVerso.clone();

  const int nbIters = 3;
  const cv::Point pos(ROWS/2, COLS/2);
  const int nbThreads = 1;
  
  const cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, pos, nbThreads);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(imgRecto, imgRectoClone) );
  REQUIRE( checkEqual(imgVerso, imgVersoClone) );
  REQUIRE( out.size() == imgRecto.size() );
}

static
void
testSimple0c(int imageType)
{
  //Apply BleedThrough (on random images) (at position (ROWS/3, COLS/2) with 2 threads)
  //Check that the output type is the same than the input type.
  //Check that the input images are not modified.
  //Check that the output size is the same than the input recto size.

  const int ROWS = 64;
  const int COLS = 64;

  cv::Mat imgRecto(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(imgRecto, cv::Scalar::all(128), cv::Scalar::all(30));

  REQUIRE( imageType == imgRecto.type() );

  cv::Mat imgVerso;
  cv::flip(imgRecto, imgVerso, 1);

  REQUIRE( imgVerso.type() == imgRecto.type() );

  cv::Mat imgRectoClone = imgRecto.clone();
  cv::Mat imgVersoClone = imgVerso.clone();

  const int nbIters = 3;
  const cv::Point pos(ROWS/3, COLS/2);
  const int nbThreads = 2;

  const cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, pos, nbThreads);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(imgRecto, imgRectoClone) );
  REQUIRE( checkEqual(imgVerso, imgVersoClone) );
  REQUIRE( out.size() == imgRecto.size() );
}


static
void
testSimple1(int imageType)
{
  //Apply BleedThrough (on random images) at specific positions
  //Pixels outside intersection should not be modified.

  cv::Mat imgRecto(3, 4, imageType);
  cv::randn(imgRecto, cv::Scalar::all(128), cv::Scalar::all(30));

  cv::Mat imgVerso(4, 5, imageType);
  cv::randn(imgVerso, cv::Scalar::all(128), cv::Scalar::all(30));

  cv::Mat imgRectoClone = imgRecto.clone();
  cv::Mat imgVersoClone = imgVerso.clone();

  const int nbIters = 1;
  const cv::Point pos(-3, 2);
  const int nbThreads = 1;

  const cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, pos, nbThreads);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(imgRecto, imgRectoClone) );
  REQUIRE( checkEqual(imgVerso, imgVersoClone) );
  REQUIRE( out.size() == imgRecto.size() );

  //With this configuration, intersection is [x=0, y=2, w=2, h=1] for imgRecto
  //Pixels outside this area should not be modified.
  //We check with two ROIs
  const cv::Rect rect1(0, 0, 4, 2);
  const cv::Mat imgRectoPart1 = imgRecto(rect1);
  const cv::Mat outPart1 = out(rect1);
  REQUIRE( checkEqual(imgRectoPart1, outPart1) );
  const cv::Rect rect2(2, 2, 2, 1);
  const cv::Mat imgRectoPart2 = imgRecto(rect2);
  const cv::Mat outPart2 = out(rect2);
  REQUIRE( checkEqual(imgRectoPart2, outPart2) );
}

static
void
testSimple1b(int imageType)
{
  //Apply BleedThrough (on random images) at specific positions
  //Pixels outside intersection should not be modified.

  cv::Mat imgRecto(3, 4, imageType);
  cv::randn(imgRecto, cv::Scalar::all(128), cv::Scalar::all(30));

  cv::Mat imgVerso(7, 4, imageType);
  cv::randn(imgVerso, cv::Scalar::all(128), cv::Scalar::all(30));

  cv::Mat imgRectoClone = imgRecto.clone();
  cv::Mat imgVersoClone = imgVerso.clone();

  const int nbIters = 1;
  const cv::Point pos(2, -5);
  const int nbThreads = 1;

  const cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, pos, nbThreads);

  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(imgRecto, imgRectoClone) );
  REQUIRE( checkEqual(imgVerso, imgVersoClone) );
  REQUIRE( out.size() == imgRecto.size() );

  //With this configuration, intersection is [x=2, y=0, w=2, h=2] for imgRecto
  //Pixels outside this area should not be modified.
  //We check with two ROIs
  const cv::Rect rect1(0, 0, 2, 2);
  const cv::Mat imgRectoPart1 = imgRecto(rect1);
  const cv::Mat outPart1 = out(rect1);
  REQUIRE( checkEqual(imgRectoPart1, outPart1) );
  const cv::Rect rect2(0, 2, 4, 1);
  const cv::Mat imgRectoPart2 = imgRecto(rect2);
  const cv::Mat outPart2 = out(rect2);
  REQUIRE( checkEqual(imgRectoPart2, outPart2) );
}





static
void
testEqualToGT1_aux(const std::string &imgFilename,
		   const std::string &gtFilename,
		   int nbIters,
		   int nbThreads)
{
  //test BleedThrough on grayscale image


  const std::string imgFilepath = dc::makePath(BLEEDTHROUGH_TEST_IMAGES_PATH, imgFilename);
  const std::string gtFilepath = dc::makePath(BLEEDTHROUGH_TEST_IMAGES_PATH, gtFilename);
  
#if CV_MAJOR_VERSION < 4
  const int flag = CV_LOAD_IMAGE_GRAYSCALE;
#else
  const int flag = cv::IMREAD_GRAYSCALE;
#endif

  cv::Mat imgGT = cv::imread(gtFilepath, flag);
  REQUIRE( ! imgGT.empty() );
  REQUIRE( imgGT.type() == CV_8UC1 );


  cv::Mat imgRecto = cv::imread(imgFilepath, flag);
  REQUIRE( ! imgRecto.empty() );
  REQUIRE( imgRecto.type() == CV_8UC1 );

  cv::Mat imgVerso;
  cv::flip(imgRecto, imgVerso, 1);
  REQUIRE( ! imgVerso.empty() );
  REQUIRE( imgVerso.type() == CV_8UC1 );

  cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, cv::Point(0, 0), nbThreads);

  REQUIRE( checkEqual(out, imgGT) );
}

static
void
testEqualToGT1(int nbThreads)
{
  //test BleedThrough on grayscale image

  const std::string imgFilename = "in1Gray.png";
  const std::string gtFilename = "out1Gray.png";
  const int nbIters = 30;

  testEqualToGT1_aux(imgFilename, gtFilename, nbIters, nbThreads);
}


static
void
testEqualToGT3_aux(const std::string &imgFilename,
		   const std::string &gtFilename,
		   int nbIters,
		   int nbThreads)
{
  //test BleedThrough on BGR image

  const std::string imgFilepath = dc::makePath(BLEEDTHROUGH_TEST_IMAGES_PATH, imgFilename);
  const std::string gtFilepath = dc::makePath(BLEEDTHROUGH_TEST_IMAGES_PATH, gtFilename);

#if CV_MAJOR_VERSION < 4
  const int flag = CV_LOAD_IMAGE_COLOR;
#else
  const int flag = cv::IMREAD_COLOR;
#endif

  cv::Mat imgGT = cv::imread(gtFilepath, flag);
  REQUIRE( ! imgGT.empty() );
  REQUIRE( imgGT.type() == CV_8UC3 );


  cv::Mat imgRecto = cv::imread(imgFilepath, flag);
  REQUIRE( ! imgRecto.empty() );
  REQUIRE( imgRecto.type() == CV_8UC3 );

  cv::Mat imgVerso;
  cv::flip(imgRecto, imgVerso, 1);
  REQUIRE( ! imgVerso.empty() );
  REQUIRE( imgVerso.type() == CV_8UC3 );

  cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, cv::Point(0, 0), nbThreads);

  REQUIRE( checkEqual(out, imgGT) );
}

static
void
testEqualToGT3(int nbThreads)
{
  //test BleedThrough on BGR image

  const std::string imgFilename = "in2Color.png";
  const std::string gtFilename = "out2Color.png";
  const int nbIters = 30;

  testEqualToGT3_aux(imgFilename, gtFilename, nbIters, nbThreads);
}

static
void
testEqualToGT3b(int nbThreads)
{
  //test BleedThrough on BGR image

  const std::string imgFilename = "in3Color.png";
  const std::string gtFilename = "out3Color.png";
  const int nbIters = 30;

  testEqualToGT3_aux(imgFilename, gtFilename, nbIters, nbThreads);
}


static
void
testEqualToGT4_aux(const std::string &imgFilename,
		   const std::string &gtFilename,
		   int nbIters,
		   int nbThreads)
{
  //test BleedThrough on BGR image
  
  const std::string imgFilepath = dc::makePath(BLEEDTHROUGH_TEST_IMAGES_PATH, imgFilename);
  const std::string gtFilepath = dc::makePath(BLEEDTHROUGH_TEST_IMAGES_PATH, gtFilename);

#if CV_MAJOR_VERSION < 4
  const int flag = - CV_LOAD_IMAGE_COLOR;
#else
  const int flag = cv::IMREAD_UNCHANGED;
#endif

  cv::Mat imgGT = cv::imread(gtFilepath, flag);
  REQUIRE( ! imgGT.empty() );
  REQUIRE( imgGT.type() == CV_8UC4 );


  cv::Mat imgRecto = cv::imread(imgFilepath, flag);
  REQUIRE( ! imgRecto.empty() );
  REQUIRE( imgRecto.type() == CV_8UC4 );

  cv::Mat imgVerso;
  cv::flip(imgRecto, imgVerso, 1);
  REQUIRE( ! imgVerso.empty() );
  REQUIRE( imgVerso.type() == CV_8UC4 );

  cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, cv::Point(0, 0), nbThreads);

  REQUIRE( checkEqual(out, imgGT) );
}

static
void
testEqualToGT4(int nbThreads)
{
  //test BleedThrough on BGRA image

  const std::string imgFilename = "in5ColorRGBA.png";
  const std::string gtFilename = "out5ColorRGBA.png";
  const int nbIters = 30;

  testEqualToGT4_aux(imgFilename,
		     gtFilename,
		     nbIters,
		     nbThreads);
}



/*
  numThreads is the maximum number of threads used, 0 or negative meaning
  as many as OpenCV uses: the result must not depend on it. The verso is
  placed at an offset, so that only part of the recto is degraded, and a
  height of 37 rows does not divide evenly between the threads.
*/
static
void
testIndependentOfNumThreads(int imageType)
{
  cv::Mat recto(37, 53, imageType);
  cv::Mat verso(41, 47, imageType);
  cv::randn(recto, cv::Scalar::all(128), cv::Scalar::all(40));
  cv::randn(verso, cv::Scalar::all(100), cv::Scalar::all(40));

  const int nbIters = 7;
  const cv::Point pos(5, -3);

  const cv::Mat ref = dc::BleedThrough::bleedThrough(recto, verso, nbIters, pos, 1);
  const cv::Mat refInc = dc::BleedThrough::bleedThroughInc(recto, ref, verso, nbIters, pos, 1);
  REQUIRE_FALSE( checkEqual(ref, recto) );

  for (int numThreads : {-1, 0, 2, 3, 7, 100}) {
    INFO( "numThreads=" << numThreads );
    REQUIRE( checkEqual(dc::BleedThrough::bleedThrough(recto, verso, nbIters, pos, numThreads), ref) );
    REQUIRE( checkEqual(dc::BleedThrough::bleedThroughInc(recto, ref, verso, nbIters, pos, numThreads), refInc) );
  }
}


TEST_CASE( "Testing BleedThrough" )
{ 

  SECTION("Testing BleedThrough produces output of same type and size than input image")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);

    testSimple0b(CV_8UC1);
    testSimple0b(CV_8UC3);
    testSimple0b(CV_8UC4);

    testSimple0c(CV_8UC1);
    testSimple0c(CV_8UC3);
    testSimple0c(CV_8UC4);
  }

  SECTION("Testing BleedThrough modifies only overlapping part")
  {
    testSimple1(CV_8UC1);
    testSimple1(CV_8UC3);
    testSimple1(CV_8UC4);

    testSimple1b(CV_8UC1);
    testSimple1b(CV_8UC3);
    testSimple1b(CV_8UC4);
  }

  SECTION("Testing BleedThrough non regression with 1 thread")
  {
    const int nbThreads = 1;
    testEqualToGT1(nbThreads);
    testEqualToGT3(nbThreads);
    testEqualToGT3b(nbThreads);
    testEqualToGT4(nbThreads);
  }

  SECTION("Testing BleedThrough gives the same result for any numThreads")
  {
    testIndependentOfNumThreads(CV_8UC1);
    testIndependentOfNumThreads(CV_8UC3);
    testIndependentOfNumThreads(CV_8UC4);
  }

  SECTION("Testing BleedThrough non regression with 3 threads")
  {
    const int nbThreads = 3;
    testEqualToGT1(nbThreads);
    testEqualToGT3(nbThreads);
    testEqualToGT3b(nbThreads);
    testEqualToGT4(nbThreads);
  }
  
}
