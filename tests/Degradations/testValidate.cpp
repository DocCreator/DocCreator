#include <stdexcept>

#include <catch2/catch_test_macros.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Degradations/BleedThrough.hpp"
#include "Degradations/BlurFilter.hpp"
#include "Degradations/ConnectedComponent.hpp"
#include "Degradations/ElasticDeformation.hpp"
#include "Degradations/GradientDomainDegradation.hpp"
#include "Degradations/GrayscaleCharsDegradationModel.hpp"
#include "Degradations/HoleDegradation.hpp"
#include "Degradations/NoiseDegradation.hpp"
#include "Degradations/PhantomCharacter.hpp"
#include "Degradations/RotationDegradation.hpp"
#include "Degradations/ShadowBinding.hpp"
#include "Degradations/Validate.hpp"

/*
  The preconditions documented on the exported degradations used to be
  checked with assert() only, thus not at all in the default build type
  (RelWithDebInfo defines NDEBUG).
*/

namespace {

  const int ROWS = 60;
  const int COLS = 80;

  cv::Mat
  makeImage(int type)
  {
    cv::Mat img(ROWS, COLS, type, cv::Scalar::all(220));
    cv::rectangle(img, cv::Rect(10, 10, 15, 20), cv::Scalar::all(0), -1);
    cv::rectangle(img, cv::Rect(40, 10, 15, 20), cv::Scalar::all(0), -1);
    return img;
  }

  //a 4-dimensional CV_8U mat, as the python layer builds for an array of
  //any dimensionality. Its rows and cols are both -1.
  cv::Mat
  makeNonImage()
  {
    const int sizes[] = { 2, ROWS, COLS, 2 };
    return cv::Mat(4, sizes, CV_8U, cv::Scalar::all(128));
  }

} //namespace

TEST_CASE("check8UC134() and check8UC1() diagnose the input")
{
  SECTION("accepted images")
  {
    for (int type : { CV_8UC1, CV_8UC3, CV_8UC4 }) {
      REQUIRE( dc::check8UC134(makeImage(type)) == dc::ImgError::Ok );
    }
    REQUIRE( dc::check8UC1(makeImage(CV_8UC1)) == dc::ImgError::Ok );
  }

  SECTION("rejected images")
  {
    REQUIRE( dc::check8UC134(cv::Mat()) == dc::ImgError::Empty );
    REQUIRE( dc::check8UC134(makeNonImage()) == dc::ImgError::BadDimensions );
    REQUIRE( dc::check8UC134(makeImage(CV_16UC1)) == dc::ImgError::BadDepth );
    REQUIRE( dc::check8UC134(makeImage(CV_32FC1)) == dc::ImgError::BadDepth );
    REQUIRE( dc::check8UC134(makeImage(CV_8UC2)) == dc::ImgError::BadChannels );
    REQUIRE( dc::check8UC1(makeImage(CV_8UC3)) == dc::ImgError::BadChannels );
  }

  SECTION("the message names the function and the argument")
  {
    bool thrown = false;
    try {
      dc::require8UC134(makeImage(CV_8UC2), "img", "dc::Test::func");
    }
    catch (const std::invalid_argument &e) {
      thrown = true;
      const std::string msg = e.what();
      REQUIRE( msg.find("dc::Test::func") != std::string::npos );
      REQUIRE( msg.find("img") != std::string::npos );
      REQUIRE( msg.find("2-channel") != std::string::npos );
    }
    REQUIRE( thrown );
  }
}

TEST_CASE("the exported degradations reject an image of the wrong type")
{
  const cv::Mat empty;
  const cv::Mat wrongDepth = makeImage(CV_16UC1);
  const cv::Mat wrongChannels = makeImage(CV_8UC2);
  const cv::Mat nonImage = makeNonImage();
  const cv::Mat pattern = makeImage(CV_8UC1);

  SECTION("empty image")
  {
    REQUIRE_THROWS_AS( dc::GrayscaleCharsDegradation::degradation(empty, 4), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::NoiseDegradation::addGaussianNoise(empty), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::NoiseDegradation::addSpeckleNoise(empty), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::NoiseDegradation::addSaltAndPepperNoise(empty), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::BlurFilter::blur(empty, dc::BlurFilter::Method::GAUSSIAN, 3), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::ShadowBinding::shadowBinding(empty, dc::ShadowBinding::Border::LEFT, 10, 0.5f, 90.f), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::ElasticDeformation::transform(empty, 2.f, 0.07f), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::RotationDegradation::rotateFillColor(empty, 10.f, cv::Scalar(0, 0, 0)), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::PhantomCharacter::phantomCharacter(empty, 0.5f, "/nonexistent"), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::GradientDomainDegradation::degradation(empty, "/nonexistent", 1), std::invalid_argument );
  }

  SECTION("not a 8-bit image")
  {
    REQUIRE_THROWS_AS( dc::GrayscaleCharsDegradation::degradation(wrongDepth, 4), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::NoiseDegradation::addGaussianNoise(wrongDepth), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::BlurFilter::blur(wrongDepth, dc::BlurFilter::Method::GAUSSIAN, 3), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::BlurFilter::getRadiusFourier(wrongDepth), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::ElasticDeformation::transform2(wrongDepth, 2.f, 0.07f, 0.09f), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::ShadowBinding::shadowBinding2(wrongDepth, 0.1f, dc::ShadowBinding::Border::TOP, 0.5f, 90.f), std::invalid_argument );
  }

  SECTION("unsupported number of channels")
  {
    REQUIRE_THROWS_AS( dc::GrayscaleCharsDegradation::degradation(wrongChannels, 4), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::RotationDegradation::rotateFillInpaint1(wrongChannels, 10.f, 0.1f), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::GradientDomainDegradation::copyOnto(wrongChannels, makeImage(CV_8UC3), cv::Point(10, 10)), std::invalid_argument );
    dc::CCs ccs;
    REQUIRE_THROWS_AS( dc::ConnectedComponent::extractAllConnectedComponents(makeImage(CV_8UC3), ccs), std::invalid_argument );
  }

  SECTION("not a two-dimensional image")
  {
    //these used to return the input unchanged, without any diagnostic
    REQUIRE_THROWS_AS( dc::ShadowBinding::shadowBinding(nonImage, dc::ShadowBinding::Border::LEFT, 10, 0.5f, 90.f), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::GrayscaleCharsDegradation::degradation(nonImage, 4), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::PhantomCharacter::phantomCharacter(nonImage, 0.5f, "/nonexistent"), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::NoiseDegradation::addGaussianNoise(nonImage), std::invalid_argument );
  }

  SECTION("mismatched image types")
  {
    const cv::Mat gray = makeImage(CV_8UC1);
    const cv::Mat color = makeImage(CV_8UC3);

    //bleedThroughInc() used to dispatch on frontImg's type and to read
    //originalFrontImg with the same accessor: with a 1-channel
    //originalFrontImg and a 3-channel frontImg, it read three times past
    //the end of each row (reported by AddressSanitizer).
    REQUIRE_THROWS_AS( dc::BleedThrough::bleedThroughInc(gray, color, color, 2), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::BleedThrough::bleedThrough(gray, color, 2), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::HoleDegradation::addHole(color, pattern, cv::Point(5, 5), 0,
						    dc::HoleDegradation::HoleType::CENTER,
						    dc::HoleDegradation::HoleSide::BORDER_TOP,
						    cv::Scalar(0, 0, 0), gray),
		       std::invalid_argument );
    REQUIRE_THROWS_AS( dc::BlurFilter::applyPattern(color, color, dc::BlurFilter::Method::GAUSSIAN, 3), std::invalid_argument );
  }

  SECTION("out-of-range values")
  {
    const cv::Mat img = makeImage(CV_8UC3);
    REQUIRE_THROWS_AS( dc::BlurFilter::blur(img, dc::BlurFilter::Method::GAUSSIAN, 0), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::PhantomCharacter::phantomCharacter(img, 1.5f, "/nonexistent"), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::GrayscaleCharsDegradation::degradation(img, 4, -0.5f, 0.5f), std::invalid_argument );
    REQUIRE_THROWS_AS( dc::GrayscaleCharsDegradation::degradation(img, 4, 0.8f, 0.8f), std::invalid_argument );
    dc::CCs ccs2;
    REQUIRE_THROWS_AS( dc::ConnectedComponent::extractAllConnectedComponents(pattern, ccs2, 6), std::invalid_argument );
  }
}

TEST_CASE("the exported degradations accept every documented image type")
{
  for (int type : { CV_8UC1, CV_8UC3, CV_8UC4 }) {
    const cv::Mat img = makeImage(type);

    REQUIRE_NOTHROW( dc::NoiseDegradation::addGaussianNoise(img, 0.f, 10.f, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS, 42) );
    REQUIRE_NOTHROW( dc::NoiseDegradation::addSaltAndPepperNoise(img, 0.1f, 0.5f, 42) );
    REQUIRE_NOTHROW( dc::BlurFilter::blur(img, dc::BlurFilter::Method::GAUSSIAN, 3) );
    REQUIRE_NOTHROW( dc::ShadowBinding::shadowBinding(img, dc::ShadowBinding::Border::LEFT, 10, 0.5f, 90.f) );
    REQUIRE_NOTHROW( dc::ElasticDeformation::transform(img, 2.f, 0.07f, dc::ElasticDeformation::BorderReplication::REPLICATE, dc::ElasticDeformation::Interpolation::NEAREST, 42) );
    REQUIRE_NOTHROW( dc::RotationDegradation::rotateFillColor(img, 10.f, cv::Scalar(0, 0, 0)) );
    REQUIRE_NOTHROW( dc::GrayscaleCharsDegradation::degradation(img, 2, 0.33f, 0.33f, 42) );

    //getRadiusFourier() used to assert() CV_8UC3 and then to call
    //cvtColor(COLOR_BGR2GRAY), which throws on a 1-channel image.
    REQUIRE_NOTHROW( dc::BlurFilter::getRadiusFourier(img) );
  }
}
