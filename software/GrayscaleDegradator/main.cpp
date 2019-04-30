
#include <Degradations/GrayscaleCharsDegradationModel.hpp>
#include <cstdlib>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>

int
main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0]
              << " inputImageFilename outputImageFilename\n";
    exit(EXIT_FAILURE);
  }

  const char *inputImageFilename = argv[1];
  const char *outputImageFilename = argv[2];

  cv::Mat img = cv::imread(inputImageFilename, CV_LOAD_IMAGE_GRAYSCALE);
  if (img.empty()) {
    std::cerr << "ERROR: unable to read image file: " << inputImageFilename
              << "\n";
    exit(EXIT_FAILURE);
  }

  int level = 10;
  float I = 20;
  float O = 30;
  float D = 50;

  dc::GrayscaleCharsDegradationModel gdm(img);

  cv::Mat outImg = gdm.degradate_cv(level, I, O, D);

  const bool writeOk = cv::imwrite(outputImageFilename, outImg);
  if (!writeOk) {
    std::cerr << "ERROR: unable to write image file: " << outputImageFilename
              << "\n";
    exit(EXIT_FAILURE);
  }

  return 0;
}
