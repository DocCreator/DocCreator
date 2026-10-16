#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include "Degradations/Distortion3D.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp> //DEBUG

#include "paths.hpp" //MESHES_PATH
#include "Degradations/FileUtils.hpp"
#include "testCommon.hpp" //checkEqual

static
void
testSimple0(int imageType)
{
  //Apply distortion3D degradation on image of given type.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.
  
  const int ROWS = 498; //bigger than all stain images to be sure they will be inserted.
  const int COLS = 499;
    
  cv::Mat img(ROWS, COLS, imageType);

  //add gaussian noise
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

  REQUIRE( imageType == img.type() );  

  const std::string meshFilename = dc::makePath(MESHES_PATH, "PlaneRegular.brs");
  const bool random = false;

  cv::Mat img2 = img.clone();
  assert(img2.type() == imageType);
  
  const cv::Mat out = dc::Distortion3D::degrade3D(img2, meshFilename, random);

  REQUIRE_FALSE( out.empty() );
  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, img2) );
  REQUIRE( out.size() == img.size() );
  //the rendered image is read back from the OpenGL context: a failed read
  //used to give a buffer that was never written to
  REQUIRE( cv::countNonZero(out.reshape(1)) > 0 );
}

void
testSimple1(int imageType, int imageBackgroundType, int outputType)
{
  //Apply distortion3D degradation on image of given type, with background image of given type.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the input background image is not modified.
  //Check that the output size is the same than the input size.
  
  const int ROWS = 498; //bigger than all stain images to be sure they will be inserted.
  const int COLS = 499;
    
  cv::Mat img(ROWS, COLS, imageType);
  //add gaussian noise
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

  cv::Mat imgBackground(ROWS+13, COLS+17, imageBackgroundType);
  //add gaussian noise
  cv::randn(imgBackground, cv::Scalar::all(128), cv::Scalar::all(30));


  REQUIRE( imageType == img.type() );  
  REQUIRE( imageBackgroundType == imgBackground.type() );  

  
  const std::string meshFilename = dc::makePath(MESHES_PATH, "PlaneRegular.brs");
  const bool random = false;

  cv::Mat img2 = img.clone();
  assert(img2.type() == imageType);
  cv::Mat imgBackground2 = imgBackground.clone();
  assert(imgBackground2.type() == imageBackgroundType);
  
  const cv::Mat out = dc::Distortion3D::degrade3DWithBackground(img2, meshFilename, imgBackground2, random);

  //B: a declaration of degrade3DWithBackground() was pasted here, inside
  // the function body. It declared a second overload taking its images by
  // non-const reference, which nothing defines and nothing called.

  REQUIRE_FALSE( out.empty() );
  REQUIRE( out.type() == outputType );
  REQUIRE( checkEqual(img, img2) );
  REQUIRE( checkEqual(imgBackground, imgBackground2) );
  REQUIRE( out.size() == img.size() );
  REQUIRE( cv::countNonZero(out.reshape(1)) > 0 );
}

static
int getType(int index)
{
  if (index == 0) {
    return CV_8UC1;
  }
  else if (index == 1) {
    return CV_8UC3;
  }
  else if (index == 2) {
    return CV_8UC4;
  }
  REQUIRE( false );

  return 0;
}



/*
  The checks above are on the shape of the result. The content needs the
  seed: it is what says the rendered image was really read back, since a
  failed read used to give a buffer nothing had written to, whose content
  changed from one run to the next.
*/
static
void
testReproducible(int imageType)
{
  const int ROWS = 200;
  const int COLS = 201;

  cv::Mat img(ROWS, COLS, imageType);
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

  const std::string meshFilename = dc::makePath(MESHES_PATH, "PlaneRegular.brs");
  const bool random = true;

  const cv::Mat a = dc::Distortion3D::degrade3D(img, meshFilename, random, 42);
  const cv::Mat b = dc::Distortion3D::degrade3D(img, meshFilename, random, 42);
  const cv::Mat c = dc::Distortion3D::degrade3D(img, meshFilename, random, 43);

  REQUIRE_FALSE( a.empty() );
  REQUIRE( checkEqual(a, b) );        //same seed => same image
  REQUIRE_FALSE( checkEqual(a, c) );  //another seed => another image
}


TEST_CASE( "Testing Distortion3D" )
{ 

  SECTION("Testing Distortion3D produces output image of same type and size")
  {
    testSimple0(CV_8UC1);
    testSimple0(CV_8UC3);
    testSimple0(CV_8UC4);
  }

  SECTION("Testing Distortion3D is reproducible from a seed")
  {
    testReproducible(CV_8UC1);
    testReproducible(CV_8UC3);
    testReproducible(CV_8UC4);
  }

  SECTION("Testing Distortion3D with background produces output image of same type and size")
  {
    for (int i=0; i<3; ++i) {
      const int imgType = getType(i);
      for (int j=0; j<3; ++j) {
	const int imgBackgroundType = getType(j);
	const int outputType = getType(std::max(i, j));
	testSimple1(imgType, imgBackgroundType, outputType);
      }
    }
  }

}
