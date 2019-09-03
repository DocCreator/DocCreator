#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Degradations/BleedThrough.hpp"
#include "Degradations/FileUtils.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp> //DEBUG

#include "paths.hpp"


static
void
testSimple0(int imageType)
{
  //Apply BleedThrough (on random images) (at position (0,0) with 1 thread)
  //and check that the output type is the same than the input type.

  const int ROWS = 64;
  const int COLS = 64;
    
  cv::Mat imgRecto(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(imgRecto, 128, 30);

  REQUIRE( imageType == imgRecto.type() );  

  cv::Mat imgVerso;
  cv::flip(imgRecto, imgVerso, 1);
  
  REQUIRE( imgVerso.type() == imgRecto.type() );

  const int nbIters = 3;
  const int x = 0;
  const int y = 0;
  const int nbThreads = 1;
  
  const cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, x, y, nbThreads); 

  REQUIRE( out.type() == imageType );
}

static
void
testSimple0b(int imageType)
{
  //Apply BleedThrough (on random images) (at position (ROWS/2, COLS/2) with 1 thread)
  //and check that the output type is the same than the input type.

  const int ROWS = 64;
  const int COLS = 64;
    
  cv::Mat imgRecto(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(imgRecto, 128, 30);

  REQUIRE( imageType == imgRecto.type() );  

  cv::Mat imgVerso;
  cv::flip(imgRecto, imgVerso, 1);
  
  REQUIRE( imgVerso.type() == imgRecto.type() );

  const int nbIters = 3;
  const int x = ROWS/2;
  const int y = COLS/2;
  const int nbThreads = 1;
  
  const cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, x, y, nbThreads); 

  REQUIRE( out.type() == imageType );
}

static
void
testSimple0c(int imageType)
{
  //Apply BleedThrough (on random images) (at position (ROWS/3, COLS/2) with 2 threads)
  //and check that the output type is the same than the input type.

  const int ROWS = 64;
  const int COLS = 64;
    
  cv::Mat imgRecto(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(imgRecto, 128, 30);

  REQUIRE( imageType == imgRecto.type() );  

  cv::Mat imgVerso;
  cv::flip(imgRecto, imgVerso, 1);
  
  REQUIRE( imgVerso.type() == imgRecto.type() );

  const int nbIters = 3;
  const int x = ROWS/3;
  const int y = COLS/2;
  const int nbThreads = 2;
  
  const cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, x, y, nbThreads); 

  REQUIRE( out.type() == imageType );
}


template <typename T>
bool
checkEquality(const cv::Mat &res1, const cv::Mat &res2)
{
  if (res1.size() != res2.size()) {
    std::cerr<<"ERROR: images have different sizes\n";
    return false;
  }

  if (res1.type() != res2.type()) {
    std::cerr<<"ERROR: images have different types\n";
    return false;
  }
  
  const int w = res1.cols;
  const int h = res1.rows;
  assert(w == res2.cols && h == res2.rows);
  for (int i=0; i<h; ++i) {

    const T *r1 = res1.ptr<T>(i);
    const T *r2 = res2.ptr<T>(i);
    
    for (int j=0; j<w; ++j) {

      if (r1[j] != r2[j]) {
	std::cerr<<"ERROR: different at y="<<i<<"/"<<h<<"  x="<<j<<"/"<<w<<"\n";
	std::cerr<<"v1="<<r1[j]<<" v2="<<r2[j]<<"\n";
	return false;
      }
    }
  }
  
  return true;
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
  
  cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, 0, 0, nbThreads);

  REQUIRE( checkEquality<uchar>(out, imgGT) );
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
  
  cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, 0, 0, nbThreads);

  REQUIRE( checkEquality<cv::Vec3b>(out, imgGT) );
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
  
  cv::Mat out = dc::BleedThrough::bleedThrough(imgRecto, imgVerso, nbIters, 0, 0, nbThreads);

  REQUIRE( checkEquality<cv::Vec4b>(out, imgGT) );
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



TEST_CASE( "Testing BleedThrough" )
{ 

  SECTION("Testing BleedThrough produces output of same type")
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

  SECTION("Testing BleedThrough non regression with 1 thread")
  {
    const int nbThreads = 1;
    testEqualToGT1(nbThreads);
    testEqualToGT3(nbThreads);
    testEqualToGT3b(nbThreads);
    testEqualToGT4(nbThreads);
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
