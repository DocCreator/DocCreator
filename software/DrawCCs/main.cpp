#include <Degradations/ConnectedComponent.hpp>
#include <cstdlib>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

static void
hsv2rgb(float h,
        float s,
        float v,
        unsigned char &r,
        unsigned char &g,
        unsigned char &b)
{
  //inspired from http://www.cs.rit.edu/~ncs/color/t_convert.html

  if (s == 0) {
    //achromatic (grey)
    r = g = b = cv::saturate_cast<unsigned char>(255 * v);
    return;
  }

  h /= 60.f; //sector 0 to 5
  const int i = floor(h);
  const float f = h - i; //factorial part of h
  const float p = v * (1 - s);
  const float q = v * (1 - s * f);
  const float t = v * (1 - s * (1 - f));

  switch (i) {
    case 0:
      r = cv::saturate_cast<unsigned char>(255 * v);
      g = cv::saturate_cast<unsigned char>(255 * t);
      b = cv::saturate_cast<unsigned char>(255 * p);
      break;
    case 1:
      r = cv::saturate_cast<unsigned char>(255 * q);
      g = cv::saturate_cast<unsigned char>(255 * v);
      b = cv::saturate_cast<unsigned char>(255 * p);
      break;
    case 2:
      r = cv::saturate_cast<unsigned char>(255 * p);
      g = cv::saturate_cast<unsigned char>(255 * v);
      b = cv::saturate_cast<unsigned char>(255 * t);
      break;
    case 3:
      r = cv::saturate_cast<unsigned char>(255 * p);
      g = cv::saturate_cast<unsigned char>(255 * q);
      b = cv::saturate_cast<unsigned char>(255 * v);
      break;
    case 4:
      r = cv::saturate_cast<unsigned char>(255 * t);
      g = cv::saturate_cast<unsigned char>(255 * p);
      b = cv::saturate_cast<unsigned char>(255 * v);
      break;
    case 5:
    default:
      r = cv::saturate_cast<unsigned char>(255 * v);
      g = cv::saturate_cast<unsigned char>(255 * p);
      b = cv::saturate_cast<unsigned char>(255 * q);
      break;
  }
}

static void
copyCC(cv::Mat &outImg, dc::CC cc, cv::Vec3b color)
{
  assert(outImg.type() == CV_8UC3);

  const size_t numPts = cc.size();
  for (size_t i = 0; i < numPts; ++i) {
    cv::Point pt = cc[i];
    assert(pt.y >= 0 && pt.y < outImg.rows && pt.x >= 0 && pt.x < outImg.cols);
    outImg.at<cv::Vec3b>(pt.y, pt.x) = color;
  }
}

static void
getBB(const dc::CCs &ccs, int &xMin, int &xMax, int &yMin, int &yMax)
{
  assert(!ccs.empty());

  xMax = xMin = (ccs[0])[0].x;
  yMax = yMin = (ccs[0])[0].y;

  const size_t numCCs = ccs.size();
  for (size_t i = 0; i < numCCs; ++i) {
    const cv::Point pt = (ccs[i])[0];
    if (pt.x > xMax) {
      xMax = pt.x;
    } else if (pt.x < xMin) {
      xMin = pt.x;
    }
    if (pt.y > yMax) {
      yMax = pt.y;
    } else if (pt.y < yMin) {
      yMin = pt.y;
    }
  }
}

//progressive colors
//It allows to see (somehow) the order of CCs.
static cv::Mat
drawCCs(const cv::Mat &img)
{
  cv::Mat img_bin;
  cv::threshold(img, img_bin, 0, 255, cv::THRESH_OTSU);

  dc::CCs ccs;
  const int connectivity = 8;
  dc::ConnectedComponent::extractAllConnectedComponents(
    img_bin, ccs, connectivity);

  int xMin, xMax, yMin, yMax;
  getBB(ccs, xMin, xMax, yMin, yMax);

  const size_t numCCs = ccs.size();

  cv::Mat outImg(img.rows, img.cols, CV_8UC3);

  //compute color in HSV and convert to RGB.
  //H is computed according to component indice.
  //S & V computed according to first point in CC.

  //H is in [0;360], S & V are in [0;1]
  //We change S in [S_min; Smax] & V in [V_min; V_max]

  const float H_MAX = 360;
  const float h_step = H_MAX / static_cast<float>(numCCs);

  const float S_min = 0.5;
  const float S_max = 1.0;
  const float V_min = 0.5;
  const float V_max = 1.0;

  //const float s = 1.f;
  //const float v = 1.f;

  std::cerr << "numCCs=" << numCCs << "\n";

  for (size_t i = 0; i < numCCs; ++i) {

    const float h = i * h_step;

    assert(!ccs[i].empty());
    const cv::Point pt = (ccs[i])[0];
    const float s =
      S_min + (S_max - S_min) * (pt.x - xMin) / static_cast<float>(xMax - xMin);
    const float v =
      V_min + (V_max - V_min) * (pt.y - yMin) / static_cast<float>(yMax - yMin);

    unsigned char r, g, b;
    hsv2rgb(h, s, v, r, g, b);

    cv::Vec3b color(b, g, r); //BGR !

    copyCC(outImg, ccs[i], color);
  }

  return outImg;
}

static cv::Mat
drawCCsRandom(const cv::Mat &img)
{
  cv::Mat img_bin;
  cv::threshold(img, img_bin, 0, 255, cv::THRESH_OTSU);

  dc::CCs ccs;
  const int connectivity = 8;
  dc::ConnectedComponent::extractAllConnectedComponents(
    img_bin, ccs, connectivity);

  int xMin, xMax, yMin, yMax;
  getBB(ccs, xMin, xMax, yMin, yMax);

  const size_t numCCs = ccs.size();

  cv::Mat outImg(img.rows, img.cols, CV_8UC3);

  const float golden_ratio_conjugate = 0.618033988749895;

  //compute color in HSV and convert to RGB.
  //H is computed according to component indice.
  //S & V computed according to first point in CC.

  //H is in [0;360], S & V are in [0;1]
  //We change S in [S_min; Smax] & V in [V_min; V_max]

  const float H_MAX = 360;
  //const float h_step = H_MAX / static_cast<float>(numCCs);

  const float S_min = 0.5;
  const float S_max = 1.0;
  const float V_min = 0.5;
  const float V_max = 1.0;

  for (size_t i = 0; i < numCCs; ++i) {

    const float h = fmodf(rand() + golden_ratio_conjugate, H_MAX);

    assert(!ccs[i].empty());
    const cv::Point pt = (ccs[i])[0];
    const float s =
      S_min + (S_max - S_min) * (pt.x - xMin) / static_cast<float>(xMax - xMin);
    const float v =
      V_min + (V_max - V_min) * (pt.y - yMin) / static_cast<float>(yMax - yMin);
    //const float s = S_min + (S_max-S_min)*pt.x/static_cast<float>(img.cols);
    //const float v = V_min + (V_max-V_min)*pt.y/static_cast<float>(img.rows);

    unsigned char r, g, b;
    hsv2rgb(h, s, v, r, g, b);

    cv::Vec3b color(b, g, r); //BGR !

    copyCC(outImg, ccs[i], color);
  }

  return outImg;
}

int
main(int argc, char *argv[])
{
  if (argc != 3 && argc != 4) {
    std::cerr
      << "Usage: " << argv[0]
      << " inputImageFilename outputImageFilename [outImageFilename2]\n";
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

  {
    cv::Mat outImg = drawCCs(img);

    const bool writeOk = cv::imwrite(outputImageFilename, outImg);
    if (!writeOk) {
      std::cerr << "ERROR: unable to write image file: " << outputImageFilename
		<< "\n";
      exit(EXIT_FAILURE);
    }
  }

  if (argc == 4) {
    const char *outputImageFilename2 = argv[3];

    cv::Mat outImg = drawCCsRandom(img);

    const bool writeOk = cv::imwrite(outputImageFilename2, outImg);
    if (!writeOk) {
      std::cerr << "ERROR: unable to write image file: " << outputImageFilename2
                << "\n";
      exit(EXIT_FAILURE);
    }
  }

#if 0
  cv::imshow("orig", img);
  cv::imshow("out", outImg);
  cv::waitKey(0);
#endif

  return 0;
}
