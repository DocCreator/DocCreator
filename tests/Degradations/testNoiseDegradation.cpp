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
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

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
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

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
  cv::randn(img, cv::Scalar::all(128), cv::Scalar::all(30));

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

static
void
testSpeckleNoiseAmount(int imageType)
{
  //The noise of addSpeckleNoise() is a multiplicative factor, thus a real
  //number: a standard deviation below 0.5 must still change the image, and
  //must not drive most of the pixels to 0 or 255.
  //It was computed in CV_16S, that is rounded to an integer: a standard
  //deviation of 0.1 gave a factor of 0 everywhere, thus an unchanged image.

  const int ROWS = 200;
  const int COLS = 200;

  //a uniform image, so that everything in the output comes from the noise
  const cv::Mat img(ROWS, COLS, imageType, cv::Scalar::all(128));

  const cv::Mat out = dc::NoiseDegradation::addSpeckleNoise(img, 0.f, 0.1f, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS, 42);

  REQUIRE( out.type() == imageType );
  REQUIRE( ! checkEqual(out, img) );

  //cv::meanStdDev() gives one value per channel
  cv::Scalar mean, stddev;
  cv::meanStdDev(out, mean, stddev);

  std::vector<cv::Mat> channels;
  cv::split(out, channels);

  const int numColorChannels = std::min(out.channels(), 3);
  for (int c = 0; c < numColorChannels; ++c) {
    //the mean is unchanged and the standard deviation is 0.1*128
    REQUIRE( std::abs(mean[c] - 128.0) < 2.0 );
    REQUIRE( std::abs(stddev[c] - 12.8) < 2.0 );

    //and almost no pixel is clamped
    const cv::Mat extreme = (channels[c] == 0) | (channels[c] == 255);
    REQUIRE( cv::countNonZero(extreme) < 0.01 * channels[c].total() );
  }
}

/*
  The speckle noise is multiplicative and centred: out = in * (1 + noise).
  The mean of each channel therefore survives it, give or take the
  saturation at 0 and 255.

  This is checked per channel, because the "+ 1" is added to a whole
  cv::Mat at once: written as a float it becomes a cv::Scalar(1, 0, 0, 0)
  and reaches the first channel only, which leaves the others multiplied
  by a factor centred on zero -- a nearly black image where only the blue
  is right. Nothing else here would notice: the type, the size and the
  alpha channel all stay correct.
*/
static
void
testSpeckleNoisePreservesTheMeanOfEachChannel(int imageType)
{
  const int ROWS = 200;
  const int COLS = 201;

  //a mid grey with each channel at a different level, so that a channel
  //taking another's value is caught too
  cv::Mat img(ROWS, COLS, imageType, cv::Scalar(90, 140, 190, 255));
  cv::rectangle(img, cv::Rect(20, 20, 80, 80), cv::Scalar(30, 60, 90, 255), -1);

  const cv::Mat out =
    dc::NoiseDegradation::addSpeckleNoise(img, 0.f, .3f,
					  dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS,
					  42);

  REQUIRE( out.type() == imageType );

  std::vector<cv::Mat> inChannels;
  std::vector<cv::Mat> outChannels;
  cv::split(img, inChannels);
  cv::split(out, outChannels);
  REQUIRE( inChannels.size() == outChannels.size() );

  const size_t numColourChannels =
    (img.channels() == 4) ? 3 : inChannels.size(); //the alpha is restored as is

  for (size_t c = 0; c < numColourChannels; ++c) {
    const double meanIn = cv::mean(inChannels[c])[0];
    const double meanOut = cv::mean(outChannels[c])[0];
    REQUIRE( meanIn > 0 );
    //centred multiplicative noise: a tenth is already generous
    REQUIRE( std::abs(meanOut - meanIn) < 0.1 * meanIn );
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

  SECTION("Testing NoiseDegradation::addSpeckleNoise applies the asked amount of noise")
  {
    testSpeckleNoiseAmount(CV_8UC1);
    testSpeckleNoiseAmount(CV_8UC3);
    testSpeckleNoiseAmount(CV_8UC4);
  }

  SECTION("Testing NoiseDegradation::addSpeckleNoise keeps the mean of each channel")
  {
    testSpeckleNoisePreservesTheMeanOfEachChannel(CV_8UC1);
    testSpeckleNoisePreservesTheMeanOfEachChannel(CV_8UC3);
    testSpeckleNoisePreservesTheMeanOfEachChannel(CV_8UC4);
  }

}
