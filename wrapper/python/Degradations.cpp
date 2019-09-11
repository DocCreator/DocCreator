#include "Degradations.hpp"

#include <Degradations/BleedThrough.hpp>
#include <Degradations/BlurFilter.hpp>
#include <Degradations/GradientDomainDegradation.hpp>
#include <Degradations/GrayscaleCharsDegradationModel.hpp>
#include <Degradations/HoleDegradation.hpp>
#include <Degradations/PhantomCharacter.hpp>
#include <Degradations/ShadowBinding.hpp>

//#include <opencv2/imgproc/imgproc.hpp> //cvtColor


//TODO:lot of CODE DUPLICATION

//------------------------------------------

void bleedThrough(uint8_t *imgOut, int imgOutLength,
		  uint8_t *imgIn, int rows, int cols, int channels,
                  uint8_t *imgBelow, int rowsBelow, int colsBelow, int channelsBelow,
                  int nbIter, int x, int y, int nbThreads)
{
  assert(imgIn && rows > 0 && cols > 0 &&
	 (channels == 1 || channels == 3 || channels == 4 ));
  assert(imgBelow && rowsBelow > 0 && colsBelow > 0 &&
	 (channelsBelow == 1 || channelsBelow == 3 || channelsBelow == 4 ));  
  assert(imgOutLength == rows * cols * channels);
  
  cv::Mat recto(rows, cols, CV_MAKETYPE(CV_8U, channels), imgIn);
  cv::Mat verso(rowsBelow, colsBelow, CV_MAKETYPE(CV_8U, channelsBelow), imgBelow);
  
  cv::Mat res = dc::BleedThrough::bleedThrough(recto, verso, nbIter, x, y, nbThreads);
  
  assert(res.isContinuous());
  const uint8_t* dataPointer = reinterpret_cast<const uint8_t*>(res.data);
  memcpy(imgOut, dataPointer, imgOutLength * sizeof(uint8_t));
}


//------------------------------------------

void blur(uint8_t *imgOut, int imgOutLength,
	  uint8_t *imgIn, int rows, int cols, int channels,
	  int method, int intensity)
{
  assert(imgIn && rows > 0 && cols > 0 &&
	 (channels == 1 || channels == 3 || channels == 4 ));
  assert(imgOutLength == rows * cols * channels);

  assert(method != 2 || intensity%2==1); //B: correct ???

  cv::Mat img(rows, cols, CV_MAKETYPE(CV_8U, channels), imgIn);
  cv::Mat res = dc::BlurFilter::blur(img, (dc::BlurFilter::Method) method, intensity);

  assert(res.isContinuous());
  const uint8_t* dataPointer = reinterpret_cast<const uint8_t*>(res.data);
  memcpy(imgOut, dataPointer, imgOutLength * sizeof(uint8_t));
}

void blurArea(uint8_t *imgOut, int imgOutLength,
	      uint8_t *imgIn, int rows, int cols, int channels,
	      int method, int intensity, int function, int area,
	      float coeff, int vertical, int horizontal, int radius)
{
  assert(imgIn && rows > 0 && cols > 0 &&
	 (channels == 1 || channels == 3 || channels == 4 ));
  assert(imgOutLength == rows * cols * channels);

  assert(method != 2 || intensity%2==1); //B: correct ???

  cv::Mat img(rows, cols, CV_MAKETYPE(CV_8U, channels), imgIn);
  cv::Mat res = dc::BlurFilter::blur(img, (dc::BlurFilter::Method)method, intensity,
				     (dc::BlurFilter::Function)function,
				     (dc::BlurFilter::Area)area,
				     coeff, vertical, horizontal, radius);

  assert(res.isContinuous());
  const uint8_t* dataPointer = reinterpret_cast<const uint8_t*>(res.data);
  memcpy(imgOut, dataPointer, imgOutLength * sizeof(uint8_t));
}

//B:TODO: also wrap other public function from BlurFilter.hpp: 

//------------------------------------------

void gradientDomainDegradation(uint8_t *imgOut, int imgOutLength,
			       uint8_t *imgIn, int rows, int cols, int channels,
			       const char *stainImagePath,
			       int numStainsToInsert,
			       int insertType,
			       bool doRotations)
{
  assert(imgIn && rows > 0 && cols > 0 &&
	 (channels == 1 || channels == 3 || channels == 4 ));
  assert(imgOutLength == rows * cols * channels);
  //B:TODO: check insertType !
  
  cv::Mat img(rows, cols, CV_MAKETYPE(CV_8U, channels), imgIn);
  cv::Mat res = dc::GradientDomainDegradation::degradation(img,
							   stainImagePath,
							   numStainsToInsert,
							   (dc::GradientDomainDegradation::InsertType)insertType,
							   doRotations);
  assert(res.isContinuous());
  const uint8_t* dataPointer = reinterpret_cast<const uint8_t*>(res.data);
  memcpy(imgOut, dataPointer, imgOutLength * sizeof(uint8_t));
}

//B:TODO: copyOnto(): it returns two things: a bool and the modified image

//------------------------------------------

void grayscaleCharsDegradation(uint8_t *imgOut, int imgOutLength,
			       uint8_t *imgIn, int rows, int cols, int channels,
			       int level, float I, float O, float D)
{
  assert(imgIn && rows > 0 && cols > 0 &&
	 (channels == 1 || channels == 3 || channels == 4 ));
  assert(imgOutLength == rows * cols * channels);

  cv::Mat img(rows, cols, CV_MAKETYPE(CV_8U, channels), imgIn);  //CV_MAKETYPE(cv::DataDepth<uint8_t>::value ???
  dc::GrayscaleCharsDegradationModel gcdm(img);
  cv::Mat res = gcdm.degradate_cv(level, I, O, D);
  
  assert(res.isContinuous());
  const uint8_t* dataPointer = reinterpret_cast<const uint8_t*>(res.data);
  memcpy(imgOut, dataPointer, imgOutLength * sizeof(uint8_t));
}

//B:TODO: API ???  Do we need to wrap the second method ?


//------------------------------------------

//TODO: color[3] devrait être un cv::Scalar !??? 
void holeDegradation(uint8_t *imgOut, int imgOutLength,
		     uint8_t *imgIn, int rows, int cols, int channels,
                     uint8_t *pattern, int rowsPattern, int colsPattern, //int channelsPattern,
                     int xOrigin, int yOrigin, int size, int holeType, int side, int color[3],
                     uint8_t *imgBelow, int rowsBelow, int colsBelow, int channelsBelow,
                     int shadowBorderWidth, float shadowBorderIntensity)
{
  assert(imgIn && rows > 0 && cols > 0 &&
	 (channels == 1 || channels == 3 || channels == 4 ));
  assert(imgOutLength == rows * cols * channels);
  assert(pattern && rowsPattern > 0 && colsPattern > 0);// && channelsPattern == 1);
  //TODO: should we convert pattern instead of assert ???
  assert(imgBelow==nullptr || (rowsBelow > 0 && colsBelow > 0 && channelsBelow == channels));
  
  cv::Mat img(rows, cols, CV_MAKETYPE(CV_8U, channels), imgIn);
  cv::Mat patternImg(rowsPattern, colsPattern, CV_8UC1, pattern);//CV_MAKETYPE(CV_8U, channelsPattern), pattern);
  cv::Mat belowImg(rowsBelow, colsBelow, CV_MAKETYPE(CV_8U, channelsBelow), imgBelow);
  cv::Mat res = dc::HoleDegradation::holeDegradation(img, patternImg, xOrigin, yOrigin, size,
						     (dc::HoleDegradation::HoleType)holeType, side,
						     cv::Scalar(color[0], color[1], color[2]),
						     belowImg, shadowBorderWidth, shadowBorderIntensity);

  assert(res.isContinuous());
  const uint8_t* dataPointer = reinterpret_cast<const uint8_t*>(res.data);
  memcpy(imgOut, dataPointer, imgOutLength * sizeof(uint8_t));
}
//B:TODO: why color[3] ????
//B: should we have default parameters for color ?

//B:TODO: wrap second method

//------------------------------------------

void phantomCharacter(uint8_t *imgOut, int imgOutLength,
		      uint8_t *imgIn, int rows, int cols, int channels,
                      int frequency, char *phantomPatternsPath)
{
  assert(imgIn && rows > 0 && cols > 0 &&
	 (channels == 1 || channels == 3 || channels == 4 ));
  assert(imgOutLength == rows * cols * channels);
  cv::Mat img(rows, cols, CV_MAKETYPE(CV_8U, channels), imgIn);
  cv::Mat res = dc::PhantomCharacter::phantomCharacter(img,
						       (dc::PhantomCharacter::Frequency)frequency,
						       phantomPatternsPath);

  assert(res.isContinuous());
  const uint8_t* dataPointer = reinterpret_cast<const uint8_t*>(res.data);
  memcpy(imgOut, dataPointer, imgOutLength * sizeof(uint8_t));
}

//------------------------------------------

void shadowBinding(uint8_t *imgOut, int imgOutLength,
		   uint8_t *imgIn, int rows, int cols, int channels,
                   int border, int distance, float intensity, float angle)
{

  assert(imgIn && rows > 0 && cols > 0 &&
	 (channels == 1 || channels == 3 || channels == 4 ));
  assert(imgOutLength == rows * cols * channels);
  cv::Mat img(rows, cols, CV_MAKETYPE(CV_8U, channels), imgIn);
  cv::Mat res = dc::ShadowBinding::shadowBinding(img,
						 (dc::ShadowBinding::Border)border,
						 distance, intensity, angle);

  assert(res.isContinuous());
  const uint8_t* dataPointer = reinterpret_cast<const uint8_t*>(res.data);
  memcpy(imgOut, dataPointer, imgOutLength * sizeof(uint8_t));
}

