#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "Degradations/NoiseDegradation.hpp"

#include <opencv2/imgproc/imgproc.hpp>

#include "testCommon.hpp" //checkEqual

static
void
testSimple0a(int imageType)
{
  //Apply Gaussian noise degradation with given addNoiseType.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.
  
  const int ROWS = 101;
  const int COLS = 107;    
    
  cv::Mat img(ROWS, COLS, imageType);
  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );  

  cv::Mat imgClone = img.clone();
  const float average = 0.0f;
  const float standard_deviation = 11.0f;
  
  for (int i=0; i<3; ++i) {
    cv::Mat img2 = img.clone();
    assert(img2.type() == imageType);

    const dc::NoiseDegradation::AddNoiseType addType = (dc::NoiseDegradation::AddNoiseType)(i);
    
    const cv::Mat out = dc::NoiseDegradation::addGaussianNoise(img2, average, standard_deviation, addType);
    
    REQUIRE( out.type() == imageType );
    REQUIRE( checkEqual(img, imgClone) );
    REQUIRE( out.size() == img.size() );
 }
}

static
void
testSimple0b(int imageType)
{
  //Apply Speckle noise degradation with given addNoiseType.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.
  
  const int ROWS = 97;
  const int COLS = 107;    
    
  cv::Mat img(ROWS, COLS, imageType);
  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );  

  cv::Mat imgClone = img.clone();
  const float average = 0.0f;
  const float standard_deviation = 0.8f;
  
  for (int i=0; i<3; ++i) {
    cv::Mat img2 = img.clone();
    assert(img2.type() == imageType);

    const dc::NoiseDegradation::AddNoiseType addType = (dc::NoiseDegradation::AddNoiseType)(i);
    
    const cv::Mat out = dc::NoiseDegradation::addSpeckleNoise(img2, average, standard_deviation, addType);
    
    REQUIRE( out.type() == imageType );
    REQUIRE( checkEqual(img, imgClone) );
    REQUIRE( out.size() == img.size() );
 }
}

static
void
testSimple0c(int imageType)
{
  //Apply Salt And Pepper noise degradation.
  //Check that the output type is the same than the input type.
  //Check that the input image is not modified.
  //Check that the output size is the same than the input size.
  
  const int ROWS = 103;
  const int COLS = 105;    
    
  cv::Mat img(ROWS, COLS, imageType);
  //add gaussian noise
  cv::randn(img, 128, 30);

  REQUIRE( imageType == img.type() );  

  cv::Mat imgClone = img.clone();
  const float amount = 0.20f;
  const float ratio = 0.55f;
  
  const cv::Mat out = dc::NoiseDegradation::addSaltAndPepperNoise(img, amount, ratio);
    
  REQUIRE( out.type() == imageType );
  REQUIRE( checkEqual(img, imgClone) );
  REQUIRE( out.size() == img.size() );
 
}

static
void
testAlphaIsPreserved(int imageType)
{
  //The three degradations must not change the alpha channel of a CV_8UC4
  //image.

  const int ROWS = 64;
  const int COLS = 48;

  cv::Mat img(ROWS, COLS, imageType);
  cv::randu(img, 0, 256);

  const uint32_t SEED = 42;

  std::vector<cv::Mat> outs;
  outs.push_back(dc::NoiseDegradation::addGaussianNoise(img, 0.f, 30.f, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS, SEED));
  outs.push_back(dc::NoiseDegradation::addSpeckleNoise(img, 0.f, 1.f, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS, SEED));
  outs.push_back(dc::NoiseDegradation::addSaltAndPepperNoise(img, 0.5f, 0.5f, SEED));
  //with the same noise on all channels
  outs.push_back(dc::NoiseDegradation::addGaussianNoise(img, 0.f, 30.f, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY, SEED));
  outs.push_back(dc::NoiseDegradation::addSpeckleNoise(img, 0.f, 1.f, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY, SEED));

  std::vector<cv::Mat> inChannels;
  cv::split(img, inChannels);

  for (const cv::Mat &out : outs) {
    REQUIRE( out.type() == imageType );

    std::vector<cv::Mat> outChannels;
    cv::split(out, outChannels);
    REQUIRE( checkEqual(outChannels[3], inChannels[3]) );

    //and the color channels did change
    bool changed = false;
    for (int c = 0; c < 3; ++c) {
      changed = changed || ! checkEqual(outChannels[c], inChannels[c]);
    }
    REQUIRE( changed );
  }
}

TEST_CASE( "Testing NoiseDegradation" )
{ 

  SECTION("Testing NoiseDegradation::addGaussianNoise produces output image of same type and size than input image")
  {
    testSimple0a(CV_8UC1);
    testSimple0a(CV_8UC3);
    testSimple0a(CV_8UC4);
  }

  SECTION("Testing NoiseDegradation::addSpeckleNoise produces output image of same type and size than input image")
  {
    testSimple0b(CV_8UC1);
    testSimple0b(CV_8UC3);
    testSimple0b(CV_8UC4);
  }

  SECTION("Testing NoiseDegradation::addSaltAndPepperNoise produces output image of same type and size than input image")
  {
    testSimple0c(CV_8UC1);
    testSimple0c(CV_8UC3);
    testSimple0c(CV_8UC4);
  }

  SECTION("Testing NoiseDegradation does not modify the alpha channel")
  {
    testAlphaIsPreserved(CV_8UC4);
  }

}
