#include "Degradations/BlurFilter.hpp"
#include "Utils/convertor.h"
#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <opencv2/imgproc/imgproc.hpp>

double
getGradientMax(const QImage &original)
{
  cv::Mat originalMat = Convertor::getCvMat(original);
  cv::Mat src_gray;
  cv::Mat grad;
  int scale = 1;
  int delta = 0;
  int ddepth = CV_16S;

  // Convert to gray
  cv::cvtColor(originalMat, src_gray, cv::COLOR_BGR2GRAY);

  /// Generate grad_x and grad_y
  cv::Mat
    grad_x; // = cv::Mat::zeros(originalMat.rows, originalMat.cols, CV_8UC1);
  cv::Mat
    grad_y; // = cv::Mat::zeros(originalMat.rows, originalMat.cols, CV_8UC1);
  cv::Mat abs_grad_x, abs_grad_y;

  /// Gradient X
  cv::Sobel(
    src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, cv::BORDER_DEFAULT);
  cv::convertScaleAbs(grad_x, abs_grad_x);
  //grad_x = abs(grad_x);
  /// Gradient Y
  cv::Sobel(
    src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, cv::BORDER_DEFAULT);
  cv::convertScaleAbs(grad_y, abs_grad_y);
  //grad_y = abs(grad_y);

  /*uchar* pixelX = grad_x.data;
    uchar* pixelY = grad_y.data;*/

  double MAX = 0;
  double gradient;

  //addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );

  for (int y = 0; y < abs_grad_x.rows; y++) {
    for (int x = 0; x < abs_grad_x.cols; x++) {

      float gradX = abs_grad_x.at<uchar>(y, x);
      float gradY = abs_grad_y.at<uchar>(y, x);
      gradient = sqrt((gradX * gradX) + (gradY * gradY));

      if (gradient > MAX)
        MAX = gradient;
    }
  }

  /*for(int i = 0; i < grad_x.rows * grad_x.cols; i++)
    {
      gradient = sqrt( (pixelX[i]*pixelX[i])+(pixelY[i]*pixelY[i]) );
      if (gradient > MAX)
        MAX=gradient;

      if (pixelY[i] > MAX)
        MAX=pixelY[i];
        }*/

  return MAX / 255;
}

/*QImage getFourier(QImage original)
{
  cv::Mat originalMatColor = Convertor::getCvMat(original);
  cv::Mat originalMat;
  cvtColor(originalMatColor, originalMat, cv::COLOR_BGR2GRAY);

  cv::Mat padded;                            //expand input image to optimal size
  int m = getOptimalDFTSize( originalMat.rows );
  int n = getOptimalDFTSize( originalMat.cols );
  copyMakeBorder(originalMat, padded, 0, m - originalMat.rows, 0, n - originalMat.cols, BORDER_CONSTANT, Scalar::all(0));

  cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
  cv::Mat complexI;

  cv::Mat_<float> tmpT = cv::Mat_<float>(padded);
  cv::Mat temp = cv::Mat::zeros(padded.size(), CV_32F);
  merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

  dft(complexI, complexI);

  split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
  magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
  cv::Mat magI = planes[0];

  magI += Scalar::all(1);                    // switch to logarithmic scale
  log(magI, magI);

  magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));
  int cx = magI.cols/2;
  int cy = magI.rows/2;

  cv::Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
  cv::Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
  cv::Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
  cv::Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right

  cv::Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
  q0.copyTo(tmp);
  q3.copyTo(q0);
  tmp.copyTo(q3);

  q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
  q2.copyTo(q1);
  tmp.copyTo(q2);

  normalize(magI, magI, 0, 255, cv::NORM_MINMAX); // Transform the matrix with float values into a
                                         // viewable image form (float between values 0 and 1).

  threshold(magI, magI, 127, 255, THRESH_BINARY);

  int erosion_size = 1;
  cv::Mat element = getStructuringElement(MORPH_RECT, Size( 2*erosion_size + 1, 2*erosion_size+1 ), Point(erosion_size, erosion_size ) );
  dilate(magI, magI, element);
  erode(magI, magI, element);
  erode(magI, magI, element);
  erode(magI, magI, element);
  erode(magI, magI, element);
  dilate(magI, magI, element);
  dilate(magI, magI, element);
  dilate(magI, magI, element);
  dilate(magI, magI, element);
  dilate(magI, magI, element);
  dilate(magI, magI, element);
  dilate(magI, magI, element);

  return Convertor::getQImage(magI);
  }*/

int
main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QWidget *window = new QWidget();

  /* const char *inImagePath =
  "/home/doctorant/LABRI/sDEfauts/CESR_287_0010.jpg";

  int width = 5;
  int height = 5;
  int method = 0;

  if (argc > 1) {
      inImagePath = argv[1];
  }
  if (argc > 2) {
      method = atoi(argv[2]);
  }
  if (argc > 3 && atoi(argv[3])%2 == 1) {
      width = atoi(argv[3]);
  }
  if (argc > 4 && atoi(argv[4])%2 == 1) {
      height = atoi(argv[4]);
  }

  QImage imIn = QImage(inImagePath);

  QLabel *label = new QLabel();
  QImage imOut;

  cv::Mat matIn = Convertor::getCvMat(imIn);
  cv::Mat matOut;

  //blur filter
  switch (method)
  {
  case 0 :
      cv::GaussianBlur(matIn, matOut, cv::Size(width, height), 0, 0);
      break;

  case 1 :
      cv::medianBlur(matIn, matOut, width);
      break;

  case 2 :
      cv::blur(matIn, matOut, cv::Size(width, height));
      break;

  }

  imOut = Convertor::getQImage(matOut);
  std::cout << "Le gradient max de cette image est : " << getGradientMax(imIn)
  << std::endl;
  if (argc > 5) {
      imOut.save(argv[5]);
  }

    label->setPixmap(QPixmap::fromImage(imOut));
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    window->setLayout(layout);*/

  QImage tmpImg = QImage("/home/doctorant/blanche.png");
  tmpImg = tmpImg.convertToFormat(QImage::Format_ARGB32);
  QImage tmp = tmpImg;
  //tmp = makePattern(tmp, 4, 0,  0, 0, 0);
  auto tmpLabel = new QLabel();
  tmpLabel->setPixmap(QPixmap::fromImage(tmp));
  auto layout = new QVBoxLayout;
  layout->addWidget(tmpLabel);
  window->setLayout(layout);

  window->show();

  return app.exec();
}
