/*
  Uses the installed degradation library as a program outside the tree
  would: the headers are included as <Degradations/...hpp>, and nothing else
  than the CMake package tells the build where they are, which standard to
  compile with, and what to link.
*/

#include <cstdio>

#include <Degradations/Degradations.hpp>

int
main()
{
  cv::Mat img(80, 100, CV_8UC3, cv::Scalar(210, 210, 210));
  cv::rectangle(img, cv::Rect(20, 20, 40, 30), cv::Scalar(30, 30, 30), cv::FILLED);
  const cv::Mat before = img.clone();

  const cv::Mat noisy = dc::NoiseDegradation::addGaussianNoise(
    img, 0.f, 20.f, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS, 42);
  const cv::Mat degraded = dc::GrayscaleCharsDegradation::degradation(img, 3, .3f, .3f, 42);
  const cv::Mat rotated = dc::RotationDegradation::rotateFillInpaint1(img, 5.f);

  for (const cv::Mat &out : { noisy, degraded, rotated }) {
    if (out.size() != img.size() || out.type() != img.type()) {
      std::fprintf(stderr, "a degradation gave an image of another size or type\n");
      return 1;
    }
  }
  if (cv::norm(img, before, cv::NORM_INF) != 0) {
    std::fprintf(stderr, "a degradation modified its input\n");
    return 1;
  }

  //the seed crosses the library: two calls with the same one are equal
  const cv::Mat again = dc::NoiseDegradation::addGaussianNoise(
    img, 0.f, 20.f, dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS, 42);
  if (cv::norm(noisy, again, cv::NORM_INF) != 0) {
    std::fprintf(stderr, "the same seed gave two images\n");
    return 1;
  }

  std::printf("DocCreator::Degradations works from an installed package\n");
  return 0;
}
