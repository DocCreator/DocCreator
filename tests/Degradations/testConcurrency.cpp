/*
  The degradations are exported to python, where callers use threads. They
  must therefore be reentrant: a call must not touch anything another call
  can see.

  Each degradation here is run once alone, then NUM_THREADS times at the
  same time with the same arguments, and every result must be the one it
  gives alone. A generator shared between calls, a static buffer or a
  srand() in the library breaks it.
*/

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Degradations/BleedThrough.hpp"
#include "Degradations/BlurFilter.hpp"
#include "Degradations/ElasticDeformation.hpp"
#include "Degradations/GradientDomainDegradation.hpp"
#include "Degradations/GrayscaleCharsDegradationModel.hpp"
#include "Degradations/HoleDegradation.hpp"
#include "Degradations/NoiseDegradation.hpp"
#include "Degradations/PhantomCharacter.hpp"
#include "Degradations/RotationDegradation.hpp"
#include "Degradations/ShadowBinding.hpp"

#include "paths.hpp" //PHANTOM_PATTERNS_PATH, STAIN_IMAGES_PATH

#include "testCommon.hpp" //checkEqual

namespace {

  constexpr int NUM_THREADS = 8;
  constexpr uint32_t SEED = 4321;

  //@return an image that looks like text: dark blobs on a light background
  cv::Mat
  makeTextLikeImage(int rows = 120, int cols = 160, int imageType = CV_8UC1)
  {
    cv::Mat img(rows, cols, imageType, cv::Scalar(210, 210, 210, 255));
    cv::RNG rng(12345);
    for (int i = 0; i < 40; ++i) {
      const cv::Point p(rng.uniform(5, cols - 5), rng.uniform(5, rows - 5));
      cv::circle(img, p, rng.uniform(2, 6), cv::Scalar(30, 30, 30, 255), -1);
    }
    return img;
  }

  /*
    Run @a degradation once, then NUM_THREADS times at the same time, and
    check that every concurrent result is the one of the single call.
  */
  void
  checkReentrant(const std::string &name,
		 const std::function<cv::Mat()> &degradation)
  {
    INFO("degradation: " << name);

    const cv::Mat expected = degradation();
    REQUIRE_FALSE( expected.empty() );

    std::vector<cv::Mat> results(NUM_THREADS);
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    //start them all before any of them can finish, so that the calls do
    //overlap
    std::atomic<int> ready{ 0 };
    std::atomic<bool> go{ false };

    for (int i = 0; i < NUM_THREADS; ++i) {
      threads.emplace_back([&, i]() {
	++ready;
	while (! go.load()) {
	  std::this_thread::yield();
	}
	results[i] = degradation();
      });
    }

    while (ready.load() != NUM_THREADS) {
      std::this_thread::yield();
    }
    go.store(true);

    for (std::thread &t : threads) {
      t.join();
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
      INFO("thread " << i);
      REQUIRE( checkEqual(results[i], expected) );
    }
  }

} //namespace

TEST_CASE("The degradations give the same result called concurrently")
{
  const cv::Mat img = makeTextLikeImage();
  const cv::Mat imgColor = makeTextLikeImage(120, 160, CV_8UC3);

  SECTION("characters degradation")
  {
    checkReentrant("characterDegradation", [&]() {
      return dc::GrayscaleCharsDegradation::degradation(img, 6, .33f, .33f, SEED);
    });
  }

  SECTION("gaussian noise")
  {
    checkReentrant("addGaussianNoise", [&]() {
      return dc::NoiseDegradation::addGaussianNoise(
	img, 0.f, 10.f, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS, SEED);
    });
  }

  SECTION("speckle noise")
  {
    checkReentrant("addSpeckleNoise", [&]() {
      return dc::NoiseDegradation::addSpeckleNoise(
	img, 0.f, .3f, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS, SEED);
    });
  }

  SECTION("salt and pepper noise")
  {
    checkReentrant("addSaltAndPepperNoise", [&]() {
      return dc::NoiseDegradation::addSaltAndPepperNoise(img, .1f, .5f, SEED);
    });
  }

  SECTION("elastic deformation")
  {
    checkReentrant("transform", [&]() {
      return dc::ElasticDeformation::transform(
	img, 10.f, .08f, dc::ElasticDeformation::BorderReplication::BLACK,
	dc::ElasticDeformation::Interpolation::BILINEAR, SEED);
    });
  }

  SECTION("hole degradation")
  {
    //a hole pattern: white outside, black inside, as the shipped ones are
    cv::Mat pattern(40, 30, CV_8UC1, cv::Scalar(255));
    cv::circle(pattern, cv::Point(15, 20), 12, cv::Scalar(0), -1);

    checkReentrant("addHoleAtRandom", [&]() {
      return dc::HoleDegradation::addHoleAtRandom(
	img, pattern, 0, dc::HoleDegradation::HoleType::CENTER, .5f,
	dc::HoleDegradation::HoleSide::BORDER_TOP, cv::Scalar(0, 0, 0, 255),
	cv::Mat(), 0, .1f, SEED);
    });
  }

  SECTION("phantom character")
  {
    checkReentrant("phantomCharacter", [&]() {
      return dc::PhantomCharacter::phantomCharacter(
	img, .1f, PHANTOM_PATTERNS_PATH, SEED);
    });
  }

  SECTION("gradient domain degradation")
  {
    checkReentrant("degradation", [&]() {
      return dc::GradientDomainDegradation::degradation(
	imgColor, STAIN_IMAGES_PATH, 3,
	dc::GradientDomainDegradation::InsertType::INSERT_AS_IS, false, SEED);
    });
  }

  SECTION("shadow binding")
  {
    checkReentrant("shadowBinding", [&]() {
      return dc::ShadowBinding::shadowBinding(
	img, dc::ShadowBinding::Border::TOP, 20, .5f, 1.f);
    });
  }

  SECTION("blur")
  {
    checkReentrant("blur", [&]() {
      return dc::BlurFilter::blur(img, dc::BlurFilter::Method::GAUSSIAN, 3);
    });
  }

  SECTION("bleed through")
  {
    const cv::Mat verso = makeTextLikeImage(120, 160);
    checkReentrant("bleedThrough", [&]() {
      return dc::BleedThrough::bleedThrough(img, verso, 10);
    });
  }

  SECTION("rotation")
  {
    checkReentrant("rotateFillColor", [&]() {
      return dc::RotationDegradation::rotateFillColor(img, 12.5, cv::Scalar(255, 255, 255, 255));
    });
  }
}
