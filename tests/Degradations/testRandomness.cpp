/*
  The degradations are random: two successive applications of the same
  degradation, with the same parameters, must not give the same image.

  They used to, in several ways: generators seeded on the current time (two
  runs started in the same second were identical), a generator copied for
  each pixel (which restarted it from the same state), a default constructed
  generator (always the same sequence), and rand(), whose state belongs to
  the whole program.
*/

#include <set>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Degradations/ElasticDeformation.hpp"
#include "Degradations/GradientDomainDegradation.hpp"
#include "Degradations/GrayscaleCharsDegradationModel.hpp"
#include "Degradations/HoleDegradation.hpp"
#include "Degradations/NoiseDegradation.hpp"
#include "Degradations/PhantomCharacter.hpp"
#include "Degradations/Random.hpp"

#include "paths.hpp" //PHANTOM_PATTERNS_PATH, STAIN_IMAGES_PATH

#include "testCommon.hpp" //checkEqual


namespace {

  //@return an image that looks like text: dark blobs on a light background
  cv::Mat
  makeTextLikeImage(int rows = 180, int cols = 240, int imageType = CV_8UC1)
  {
    cv::Mat img(rows, cols, imageType, cv::Scalar(210, 210, 210, 255));
    cv::RNG rng(12345); //fixed: the input must not be what varies
    const int numBlobs = (rows * cols) / 900;
    for (int i = 0; i < numBlobs; ++i) {
      const int x = rng.uniform(2, cols - 12);
      const int y = rng.uniform(2, rows - 14);
      const int v = rng.uniform(20, 90);
      cv::rectangle(img,
                    cv::Rect(x, y, rng.uniform(4, 9), rng.uniform(7, 13)),
                    cv::Scalar(v, v, v, 255),
                    -1);
    }
    return img;
  }

} //namespace


TEST_CASE("Testing that the degradations are random")
{
  const cv::Mat img = makeTextLikeImage();

  SECTION("characters degradation")
  {
    const cv::Mat a = dc::GrayscaleCharsDegradation::degradation(img, 8);
    const cv::Mat b = dc::GrayscaleCharsDegradation::degradation(img, 8);

    REQUIRE_FALSE( checkEqual(a, img) ); //something was degraded
    REQUIRE_FALSE( checkEqual(a, b) );
  }

  SECTION("characters degradation varies within one noise region")
  {
    /*
      A generator was built for each pixel from a generator that never
      advanced, so every pixel of a noise region got the very same draw: the
      grey levels written must be varied.

      Measured over 20 runs on this image, the most frequent difference to
      the input covered 25.9% to 26.2% of the modified pixels before, and
      3.3% to 3.8% after. The limit below is set between the two.
    */
    const cv::Mat out = dc::GrayscaleCharsDegradation::degradation(img, 8);
    REQUIRE( out.type() == img.type() );

    std::vector<int> countPerDelta(2 * 256, 0);
    int numChanged = 0;
    for (int y = 0; y < img.rows; ++y) {
      for (int x = 0; x < img.cols; ++x) {
        const int in = img.at<uchar>(y, x);
        const int res = out.at<uchar>(y, x);
        if (in != res) {
          ++numChanged;
          ++countPerDelta[res - in + 256];
        }
      }
    }
    REQUIRE( numChanged > 100 );

    const int mostFrequent =
      *std::max_element(countPerDelta.begin(), countPerDelta.end());
    REQUIRE( mostFrequent * 10 < numChanged ); //less than 10%
  }

  SECTION("gaussian noise")
  {
    const cv::Mat a = dc::NoiseDegradation::addGaussianNoise(img, 0.f, 10.f);
    const cv::Mat b = dc::NoiseDegradation::addGaussianNoise(img, 0.f, 10.f);
    REQUIRE_FALSE( checkEqual(a, b) );
  }

  SECTION("salt and pepper noise")
  {
    const cv::Mat a = dc::NoiseDegradation::addSaltAndPepperNoise(img, 0.1f, 0.5f);
    const cv::Mat b = dc::NoiseDegradation::addSaltAndPepperNoise(img, 0.1f, 0.5f);
    REQUIRE_FALSE( checkEqual(a, b) );
  }

  SECTION("elastic deformation")
  {
    const cv::Mat a = dc::ElasticDeformation::transform(img, 2.f, 0.08f);
    const cv::Mat b = dc::ElasticDeformation::transform(img, 2.f, 0.08f);
    REQUIRE_FALSE( checkEqual(a, b) );
  }

  SECTION("elastic deformation, with the affine transform")
  {
    //[the generator of the affine part was default constructed, thus always
    // seeded with the same value]
    const cv::Mat a = dc::ElasticDeformation::transform2(img, 2.f, 0.08f, 9.f);
    const cv::Mat b = dc::ElasticDeformation::transform2(img, 2.f, 0.08f, 9.f);
    REQUIRE_FALSE( checkEqual(a, b) );
  }

  SECTION("phantom characters")
  {
    const cv::Mat a = dc::PhantomCharacter::phantomCharacter(img, 0.8f, PHANTOM_PATTERNS_PATH);
    const cv::Mat b = dc::PhantomCharacter::phantomCharacter(img, 0.8f, PHANTOM_PATTERNS_PATH);
    REQUIRE_FALSE( checkEqual(a, b) );
  }

  SECTION("gradient domain degradation")
  {
    const cv::Mat a = dc::GradientDomainDegradation::degradation(img, STAIN_IMAGES_PATH, 2);
    const cv::Mat b = dc::GradientDomainDegradation::degradation(img, STAIN_IMAGES_PATH, 2);
    REQUIRE_FALSE( checkEqual(a, b) );
  }

  SECTION("hole positions")
  {
    //[a single std::mt19937 was shared by the whole program here]
    std::set<std::pair<int, int>> positions;
    for (int i = 0; i < 50; ++i) {
      const cv::Point p =
        dc::HoleDegradation::getRandomPosition(cv::Size(500, 400),
                                               cv::Size(40, 30),
                                               dc::HoleDegradation::HoleType::CENTER,
                                               0.5f,
                                               dc::HoleDegradation::HoleSide::BORDER_TOP);
      positions.insert({ p.x, p.y });
    }
    REQUIRE( positions.size() > 10 );
  }
}



TEST_CASE("Testing that a seed makes the degradations reproducible")
{
  //Same seed and same parameters: same result. Different seeds: different
  //results. This is what makes a generated dataset reproducible.
  const cv::Mat img = makeTextLikeImage();
  constexpr uint32_t SEED = 1234;
  constexpr uint32_t OTHER_SEED = 1235;

  SECTION("characters degradation")
  {
    const cv::Mat a = dc::GrayscaleCharsDegradation::degradation(img, 8, .33f, .33f, SEED);
    const cv::Mat b = dc::GrayscaleCharsDegradation::degradation(img, 8, .33f, .33f, SEED);
    const cv::Mat c = dc::GrayscaleCharsDegradation::degradation(img, 8, .33f, .33f, OTHER_SEED);

    REQUIRE( checkEqual(a, b) );
    REQUIRE_FALSE( checkEqual(a, c) );
    REQUIRE_FALSE( checkEqual(a, img) );
  }

  SECTION("gaussian noise")
  {
    using dc::NoiseDegradation::addGaussianNoise;
    using dc::NoiseDegradation::AddNoiseType;
    const AddNoiseType t = AddNoiseType::ADD_NOISE_AS_IS;
    REQUIRE( checkEqual(addGaussianNoise(img, 0.f, 10.f, t, SEED),
			addGaussianNoise(img, 0.f, 10.f, t, SEED)) );
    REQUIRE_FALSE( checkEqual(addGaussianNoise(img, 0.f, 10.f, t, SEED),
			      addGaussianNoise(img, 0.f, 10.f, t, OTHER_SEED)) );
  }

  SECTION("salt and pepper noise")
  {
    using dc::NoiseDegradation::addSaltAndPepperNoise;
    REQUIRE( checkEqual(addSaltAndPepperNoise(img, 0.1f, 0.5f, SEED),
			addSaltAndPepperNoise(img, 0.1f, 0.5f, SEED)) );
    REQUIRE_FALSE( checkEqual(addSaltAndPepperNoise(img, 0.1f, 0.5f, SEED),
			      addSaltAndPepperNoise(img, 0.1f, 0.5f, OTHER_SEED)) );
  }

  SECTION("elastic deformation")
  {
    using dc::ElasticDeformation::transform;
    using dc::ElasticDeformation::transform2;
    using dc::ElasticDeformation::BorderReplication;
    using dc::ElasticDeformation::Interpolation;
    const BorderReplication b = BorderReplication::BLACK;
    const Interpolation i = Interpolation::BILINEAR;

    REQUIRE( checkEqual(transform(img, 2.f, .08f, b, i, SEED),
			transform(img, 2.f, .08f, b, i, SEED)) );
    REQUIRE_FALSE( checkEqual(transform(img, 2.f, .08f, b, i, SEED),
			      transform(img, 2.f, .08f, b, i, OTHER_SEED)) );

    REQUIRE( checkEqual(transform2(img, 2.f, .08f, 9.f, b, i, SEED),
			transform2(img, 2.f, .08f, 9.f, b, i, SEED)) );
    REQUIRE_FALSE( checkEqual(transform2(img, 2.f, .08f, 9.f, b, i, SEED),
			      transform2(img, 2.f, .08f, 9.f, b, i, OTHER_SEED)) );
  }

  SECTION("phantom characters")
  {
    using dc::PhantomCharacter::phantomCharacter;
    REQUIRE( checkEqual(phantomCharacter(img, .8f, PHANTOM_PATTERNS_PATH, SEED),
			phantomCharacter(img, .8f, PHANTOM_PATTERNS_PATH, SEED)) );
    REQUIRE_FALSE( checkEqual(phantomCharacter(img, .8f, PHANTOM_PATTERNS_PATH, SEED),
			      phantomCharacter(img, .8f, PHANTOM_PATTERNS_PATH, OTHER_SEED)) );
  }

  SECTION("gradient domain degradation")
  {
    using dc::GradientDomainDegradation::degradation;
    using dc::GradientDomainDegradation::InsertType;
    const InsertType t = InsertType::INSERT_AS_GRAY_IF_GRAY;
    REQUIRE( checkEqual(degradation(img, STAIN_IMAGES_PATH, 2, t, true, SEED),
			degradation(img, STAIN_IMAGES_PATH, 2, t, true, SEED)) );
  }

  SECTION("hole position")
  {
    using dc::HoleDegradation::getRandomPosition;
    using dc::HoleDegradation::HoleType;
    using dc::HoleDegradation::HoleSide;
    const cv::Size imgSize(500, 400), holeSize(40, 30);
    const cv::Point a = getRandomPosition(imgSize, holeSize, HoleType::CENTER,
					  .5f, HoleSide::BORDER_TOP, SEED);
    const cv::Point b = getRandomPosition(imgSize, holeSize, HoleType::CENTER,
					  .5f, HoleSide::BORDER_TOP, SEED);
    REQUIRE( a == b );
  }
}

TEST_CASE("Testing makeRandomEngine")
{
  SECTION("the same seed gives the same sequence")
  {
    dc::RandomEngine a = dc::makeRandomEngine(7);
    dc::RandomEngine b = dc::makeRandomEngine(7);
    for (int i = 0; i < 100; ++i) {
      REQUIRE( a.next() == b.next() );
    }
  }

  SECTION("different seeds give different sequences")
  {
    //including small and consecutive seeds, and 0
    for (uint32_t seed = 0; seed < 5; ++seed) {
      dc::RandomEngine a = dc::makeRandomEngine(seed);
      dc::RandomEngine b = dc::makeRandomEngine(seed + 1);
      bool different = false;
      for (int i = 0; i < 100 && !different; ++i) {
        different = (a.next() != b.next());
      }
      REQUIRE( different );
    }
  }

  SECTION("without a seed the sequence is not reproducible")
  {
    dc::RandomEngine a = dc::makeRandomEngine();
    dc::RandomEngine b = dc::makeRandomEngine();
    bool different = false;
    for (int i = 0; i < 100 && !different; ++i) {
      different = (a.next() != b.next());
    }
    REQUIRE( different );
  }
}
