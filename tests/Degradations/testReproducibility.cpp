#include <catch2/catch_test_macros.hpp>

#include <opencv2/imgproc/imgproc.hpp>

#include "Degradations/ElasticDeformation.hpp"
#include "Degradations/GradientDomainDegradation.hpp"
#include "Degradations/GrayscaleCharsDegradationModel.hpp"
#include "Degradations/HoleDegradation.hpp"
#include "Degradations/NoiseDegradation.hpp"
#include "Degradations/PhantomCharacter.hpp"

#include "paths.hpp"
#include "testCommon.hpp" //checkEqual

/*
  The other test files check the shape of the results: the type, the size,
  and that the input is left alone. They cannot check the content, because
  every degradation draws from a generator the caller could not seed.

  Each degradation that takes a seed now gets the same three checks here:

   - the same seed and the same parameters give exactly the same image,
     which is what makes a generated corpus reproducible;
   - two different seeds give different images, which is what fails when
     the generator is accidentally copied by value or re-seeded per pixel,
     and collapses the variation;
   - the image really changed, so that a degradation that quietly does
     nothing cannot pass.
*/

namespace {

  const uint32_t SEED_A = 42;
  const uint32_t SEED_B = 43;

  /*
    @return an image with @a numComponents square components on a light
    background, in grey levels so that a grayscale conversion back and
    forth is lossless.
  */
  cv::Mat
  makeTextLikeImage(int imageType = CV_8UC3, int numComponents = 12)
  {
    const int ROWS = 120;
    const int COLS = 120;
    const int BACKGROUND = 200;
    const int FOREGROUND = 60;

    cv::Mat img(ROWS, COLS, imageType,
		cv::Scalar(BACKGROUND, BACKGROUND, BACKGROUND, 255));
    for (int i = 0; i < numComponents; ++i) {
      const int x = 8 + (i % 4) * 28;
      const int y = 8 + (i / 4) * 28;
      cv::rectangle(img,
		    cv::Rect(x, y, 14, 14),
		    cv::Scalar(FOREGROUND, FOREGROUND, FOREGROUND, 255),
		    -1);
    }
    return img;
  }

  //@return a CV_8UC1 hole pattern: a white disc on black.
  cv::Mat
  makeHolePattern(int side = 40)
  {
    cv::Mat pattern(side, side, CV_8UC1, cv::Scalar(0));
    cv::circle(pattern,
	       cv::Point(side / 2, side / 2),
	       side / 3,
	       cv::Scalar(255),
	       -1);
    return pattern;
  }

  /*
    Check the three properties on a degradation called through @a degrade,
    which takes a seed and returns the degraded image.
  */
  template <typename F>
  void
  checkReproducible(const cv::Mat &img, F degrade)
  {
    const cv::Mat imgClone = img.clone();

    const cv::Mat a = degrade(SEED_A);
    const cv::Mat b = degrade(SEED_A);
    const cv::Mat c = degrade(SEED_B);

    REQUIRE( checkEqual(img, imgClone) );  //the input is left alone
    REQUIRE( checkEqual(a, b) );           //same seed => same image
    REQUIRE_FALSE( checkEqual(a, c) );     //another seed => another image
    REQUIRE_FALSE( checkEqual(a, img) );   //and something was degraded
  }

} //namespace

TEST_CASE( "Testing the reproducibility of the seeded degradations" )
{
  SECTION("GrayscaleCharsDegradation::degradation")
  {
    for (int imageType : { CV_8UC1, CV_8UC3, CV_8UC4 }) {
      const cv::Mat img = makeTextLikeImage(imageType);
      checkReproducible(img, [&img](uint32_t seed) {
	return dc::GrayscaleCharsDegradation::degradation(img, 8, .33f, .33f,
							  seed);
      });
    }
  }

  SECTION("NoiseDegradation::addGaussianNoise")
  {
    for (int imageType : { CV_8UC1, CV_8UC3, CV_8UC4 }) {
      const cv::Mat img = makeTextLikeImage(imageType);
      checkReproducible(img, [&img](uint32_t seed) {
	return dc::NoiseDegradation::addGaussianNoise(
	  img, 0.f, 20.f, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS,
	  seed);
      });
    }
  }

  SECTION("NoiseDegradation::addSpeckleNoise")
  {
    for (int imageType : { CV_8UC1, CV_8UC3, CV_8UC4 }) {
      const cv::Mat img = makeTextLikeImage(imageType);
      checkReproducible(img, [&img](uint32_t seed) {
	return dc::NoiseDegradation::addSpeckleNoise(
	  img, 0.f, .5f, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS,
	  seed);
      });
    }
  }

  SECTION("NoiseDegradation::addSaltAndPepperNoise")
  {
    for (int imageType : { CV_8UC1, CV_8UC3, CV_8UC4 }) {
      const cv::Mat img = makeTextLikeImage(imageType);
      checkReproducible(img, [&img](uint32_t seed) {
	return dc::NoiseDegradation::addSaltAndPepperNoise(img, .15f, .5f,
							   seed);
      });
    }
  }

  SECTION("PhantomCharacter::phantomCharacter")
  {
    for (int imageType : { CV_8UC1, CV_8UC3, CV_8UC4 }) {
      const cv::Mat img = makeTextLikeImage(imageType);
      checkReproducible(img, [&img](uint32_t seed) {
	return dc::PhantomCharacter::phantomCharacter(img, .7f,
						      PHANTOM_PATTERNS_PATH,
						      seed);
      });
    }
  }

  SECTION("ElasticDeformation::transform")
  {
    for (int imageType : { CV_8UC1, CV_8UC3, CV_8UC4 }) {
      const cv::Mat img = makeTextLikeImage(imageType);
      checkReproducible(img, [&img](uint32_t seed) {
	return dc::ElasticDeformation::transform(
	  img, 20.f, .08f,
	  dc::ElasticDeformation::BorderReplication::BLACK,
	  dc::ElasticDeformation::Interpolation::BILINEAR, seed);
      });
    }
  }

  SECTION("ElasticDeformation::transform2")
  {
    for (int imageType : { CV_8UC1, CV_8UC3, CV_8UC4 }) {
      const cv::Mat img = makeTextLikeImage(imageType);
      checkReproducible(img, [&img](uint32_t seed) {
	return dc::ElasticDeformation::transform2(
	  img, 20.f, .08f, 9.f,
	  dc::ElasticDeformation::BorderReplication::BLACK,
	  dc::ElasticDeformation::Interpolation::BILINEAR, seed);
      });
    }
  }

  SECTION("HoleDegradation::addHoleAtRandom")
  {
    const cv::Mat pattern = makeHolePattern();
    for (int imageType : { CV_8UC1, CV_8UC3, CV_8UC4 }) {
      const cv::Mat img = makeTextLikeImage(imageType);
      checkReproducible(img, [&img, &pattern](uint32_t seed) {
	return dc::HoleDegradation::addHoleAtRandom(
	  img, pattern, 0, dc::HoleDegradation::HoleType::CENTER, 0.f,
	  dc::HoleDegradation::HoleSide::BORDER_TOP,
	  cv::Scalar(0, 0, 0, 255), cv::Mat(), 0, 1000.f, seed);
      });
    }
  }

  SECTION("HoleDegradation::getRandomPosition")
  {
    const cv::Size imgSize(120, 120);
    const cv::Size patternSize(40, 40);
    const cv::Point a = dc::HoleDegradation::getRandomPosition(
      imgSize, patternSize, dc::HoleDegradation::HoleType::CENTER, .5f,
      dc::HoleDegradation::HoleSide::BORDER_TOP, SEED_A);
    const cv::Point b = dc::HoleDegradation::getRandomPosition(
      imgSize, patternSize, dc::HoleDegradation::HoleType::CENTER, .5f,
      dc::HoleDegradation::HoleSide::BORDER_TOP, SEED_A);
    const cv::Point c = dc::HoleDegradation::getRandomPosition(
      imgSize, patternSize, dc::HoleDegradation::HoleType::CENTER, .5f,
      dc::HoleDegradation::HoleSide::BORDER_TOP, SEED_B);
    REQUIRE( a == b );
    REQUIRE_FALSE( a == c );
  }

  SECTION("GradientDomainDegradation::degradation")
  {
    //slow: one image type and two stains are enough to pin the seed
    const cv::Mat img = makeTextLikeImage(CV_8UC3);
    checkReproducible(img, [&img](uint32_t seed) {
      return dc::GradientDomainDegradation::degradation(
	img, STAIN_IMAGES_PATH, 2,
	dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY_IF_GRAY,
	true, seed);
    });
  }
}

/*
  The variation a degradation adds has to be spread over the image. With the
  generator copied by value instead of shared, every spot started from the
  same state and the deviation collapsed; the degradation still "changed"
  the image, so the checks above would not have caught it.
*/
TEST_CASE( "Testing the spread of the noise added by a degradation" )
{
  const cv::Mat img = makeTextLikeImage(CV_8UC1);

  SECTION("gaussian noise has about the requested standard deviation")
  {
    const float SIGMA = 20.f;
    const cv::Mat out = dc::NoiseDegradation::addGaussianNoise(
      img, 0.f, SIGMA, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS,
      SEED_A);

    cv::Mat diff;
    cv::absdiff(out, img, diff);
    cv::Scalar mean;
    cv::Scalar stddev;
    cv::meanStdDev(diff, mean, stddev);

    //the absolute difference of a centred gaussian: mean ~ sigma*sqrt(2/pi),
    //clamped at the image borders, so a generous floor is enough here
    REQUIRE( mean[0] > SIGMA / 4 );
    REQUIRE( stddev[0] > 1. );
  }

  SECTION("the character degradation touches more than one place")
  {
    const cv::Mat out =
      dc::GrayscaleCharsDegradation::degradation(img, 10, .33f, .33f, SEED_A);

    cv::Mat changed;
    cv::compare(out, img, changed, cv::CMP_NE);
    REQUIRE( cv::countNonZero(changed) > 20 );

    //the changes must not all sit in one row or one column
    cv::Mat rowCounts;
    cv::reduce(changed / 255, rowCounts, 1, cv::REDUCE_SUM, CV_32S);
    REQUIRE( cv::countNonZero(rowCounts) > 2 );

    cv::Mat colCounts;
    cv::reduce(changed / 255, colCounts, 0, cv::REDUCE_SUM, CV_32S);
    REQUIRE( cv::countNonZero(colCounts) > 2 );
  }
}
