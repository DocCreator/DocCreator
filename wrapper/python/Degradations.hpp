#ifndef WRAPPER_DEGRADATIONS_HPP
#define WRAPPER_DEGRADATIONS_HPP

#include <cstdint>

//------------------------------------------

void bleedThrough(uint8_t *imgOut, int imgOutLength,
		  uint8_t *imgIn, int rows, int cols, int channels,
                  uint8_t *imgBelow, int rowsBelow, int colsBelow, int channelsBelow,
                  int nbIter, int x=0, int y=0, int nbThreads=-1);


//------------------------------------------

void blur(uint8_t *imgOut, int imgOutLength,
	  uint8_t *imgIn, int rows, int cols, int channels,
	  int method, int intensity);

void blurArea(uint8_t *imgOut, int imgOutLength,
	      uint8_t *imgIn, int rows, int cols, int channels,
	      int method, int intensity, int function, int area,
	      float coeff, int vertical, int horizontal, int radius);

//B:TODO: also wrap other public function from BlurFilter.hpp: 

//------------------------------------------

void gradientDomainDegradation(uint8_t *imgOut, int imgOutLength,
			       uint8_t *imgIn, int rows, int cols, int channels,
			       const char *stainImagePath,
			       int numStainsToInsert,
			       int insertType,
			       bool doRotations);

//B:TODO: copyOnto(): it returns two things: a bool and the modified image

//------------------------------------------

void grayscaleCharsDegradation(uint8_t *imgOut, int imgOutLength,
			       uint8_t *imgIn, int rows, int cols, int channels,
			       int level = 1, float I = 33.f, float O = 33.f, float D = 34.f);

//B:TODO: API ???  better arguments !
//B:TODO: API ???  Do we need to wrap the second method ?


//------------------------------------------

/*
  void holeDegradation(uint8_t *imgOut, int imgOutLength,
  uint8_t *imgIn, int rows, int cols, int channels,
  uint8_t *pattern, int rowsPattern, int colsPattern, //int channelsPattern,
  int xOrigin, int yOrigin, int size, int holeType, int side, int color[3],
  uint8_t *imgBelow=nullptr, int rowsBelow=0, int colsBelow=0, int channelsBelow=0,
  int shadowBorderWidth=0, float shadowBorderIntensity=1000.0f);
*/

void holeDegradation(uint8_t *imgOut, int imgOutLength,
		     uint8_t *imgIn, int rows, int cols, int channels,
                     uint8_t *pattern, int rowsPattern, int colsPattern, //int channelsPattern,
                     int xOrigin, int yOrigin, int size, int holeType, int side, int color[3]);

void holeDegradation2(uint8_t *imgOut, int imgOutLength,
		      uint8_t *imgIn, int rows, int cols, int channels,
		      uint8_t *pattern, int rowsPattern, int colsPattern, //int channelsPattern,
		      int xOrigin, int yOrigin, int size, int holeType, int side, int color[3],
		      uint8_t *imgBelow, int rowsBelow, int colsBelow, int channelsBelow,
		      int shadowBorderWidth=0, float shadowBorderIntensity=1000.0f);

//B:TODO: here we allow only pattern of type CV_8UC1 (we don't pass channelsPattern)
//B:TODO: also allow to pass CV_8UC3 ?

//B:TODO: why color[3] ????
//B: should we have default parameters for color ?

//B:TODO: wrap second method

//------------------------------------------

void phantomCharacter(uint8_t *imgOut, int imgOutLength,
		      uint8_t *imgIn, int rows, int cols, int channels,
                      int frequency, char *phantomPatternsPath);

//------------------------------------------

void shadowBinding(uint8_t *imgOut, int imgOutLength,
		   uint8_t *imgIn, int rows, int cols, int channels,
                   int border, int distance, float intensity=0.5, float angle=30);

//B:TODO: wrap second method 

//------------------------------------------

void degrade3D(uint8_t *imgOut, int imgOutLength,
	       uint8_t *imgIn, int rows, int cols, int channels,
	       char *meshFilename,
	       bool random=true);
	       
void degrade3DWithBackground(uint8_t *imgOut, int imgOutLength,
			     uint8_t *imgIn, int rows, int cols, int channels,
			     char *meshFilename,
			     uint8_t *imgBackground, int rowsBackground, int colsBackground, int channelsBackground,
			     bool random=true);

//TODO
//void degrade3D(const cv::Mat &img, size_t numOutputImages, const std::string &meshFilename, const std::string &outputPrefix, bool random=true);
//void degrade3DWithBackground(const cv::Mat &img, size_t numOutputImages, const std::string &meshFilename, const cv::Mat &backgroundImg, const std::string &outputPrefix, bool random=true);
			     
//------------------------------------------

void addGaussianNoise(uint8_t *imgOut, int imgOutLength,
		      uint8_t *imgIn, int rows, int cols, int channels,
		      float average=0.0f,
		      float standard_deviation=10.0f,
		      int addType=0);

void addSpeckleNoise(uint8_t *imgOut, int imgOutLength,
		     uint8_t *imgIn, int rows, int cols, int channels,
		     float average=0.0f,
		     float standard_deviation=10.0f,
		     int addType=0);

void addSaltAndPepperNoise(uint8_t *imgOut, int imgOutLength,
			   uint8_t *imgIn, int rows, int cols, int channels,
			   float amount=0.15f,
			   float ratio=0.5f);

//------------------------------------------

void rotateFillColor(uint8_t *imgOut, int imgOutLength,
		     uint8_t *imgIn, int rows, int cols, int channels,
		     float angle,
		     int color[3]);

void rotateFillImage(uint8_t *imgOut, int imgOutLength,
		     uint8_t *imgIn, int rows, int cols, int channels,
		     float angle,
		     uint8_t *imgBackground, int rowsBackground, int colsBackground, int channelsBackground);

void rotateFillImageRepeats(uint8_t *imgOut, int imgOutLength,
			    uint8_t *imgIn, int rows, int cols, int channels,
			    float angle,
			    uint8_t *imgBackground, int rowsBackground, int colsBackground, int channelsBackground,
			    int repeats);

//TODO: rotateFillImage(const cv::Mat &img, float angle, const std::vector<cv::Mat> &backgroundImgs)


void rotateFillBorder(uint8_t *imgOut, int imgOutLength,
		      uint8_t *imgIn, int rows, int cols, int channels,
		      float angle,
		      int borderMode);

void rotateFillInpaint1(uint8_t *imgOut, int imgOutLength,
			uint8_t *imgIn, int rows, int cols, int channels,
			float angle,
			float inpaintingRatio = 0.05);

void rotateFillInpaint2(uint8_t *imgOut, int imgOutLength,
			uint8_t *imgIn, int rows, int cols, int channels,
			float angle,
			uint8_t *imgBackground, int rowsBackground, int colsBackground, int channelsBackground);

void rotateFillInpaint3(uint8_t *imgOut, int imgOutLength,
			uint8_t *imgIn, int rows, int cols, int channels,
			float angle,
			uint8_t *imgBackground, int rowsBackground, int colsBackground, int channelsBackground);

//------------------------------------------

void elasticDeformation(uint8_t *imgOut, int imgOutLength,
			uint8_t *imgIn, int rows, int cols, int channels,
			float alpha = 2.0f,
			float sigma = 0.08f,
			int borderMode = 4, int interpolation = 1);

void elasticDeformation2(uint8_t *imgOut, int imgOutLength,
			 uint8_t *imgIn, int rows, int cols, int channels,
			 float alpha = 2.0f,
			 float sigma = 0.08f,
			 float alpha_affine = 9.0f,
			 int borderMode = 4, int interpolation = 1);

//------------------------------------------



#endif /* ! WRAPPER_DEGRADATIONS_HPP */
