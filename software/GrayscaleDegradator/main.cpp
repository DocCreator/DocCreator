
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

#if CV_MAJOR_VERSION < 4
  const int grayFlag = CV_LOAD_IMAGE_GRAYSCALE;
#else
  const int grayFlag = cv::IMREAD_GRAYSCALE;
#endif
  cv::Mat img = cv::imread(inputImageFilename, grayFlag);
  if (img.empty()) {
    std::cerr << "ERROR: unable to read image file: " << inputImageFilename
              << "\n";
    exit(EXIT_FAILURE);
  }

  int level = 10; //must be in [0; 10]
  float I = 0.20;
  float O = 0.30;

  cv::Mat outImg = dc::GrayscaleCharsDegradation::degradation(img, level, I, O);

  const bool writeOk = cv::imwrite(outputImageFilename, outImg);
  if (!writeOk) {
    std::cerr << "ERROR: unable to write image file: " << outputImageFilename
              << "\n";
    exit(EXIT_FAILURE);
  }

  return 0;
}
