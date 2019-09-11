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

void holeDegradation(uint8_t *imgOut, int imgOutLength,
		     uint8_t *imgIn, int rows, int cols, int channels,
                     uint8_t *pattern, int rowsPattern, int colsPattern, //int channelsPattern,
                     int xOrigin, int yOrigin, int size, int holeType, int side, int color[3],
                     uint8_t *imgBelow=nullptr, int rowsBelow=0, int colsBelow=0, int channelsBelow=0,
                     int shadowBorderWidth=0, float shadowBorderIntensity=1000.0f);

//B:TODO: here we allow only patternIg of type CV_8UC1 (we don't pass channelsPattern)
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


#endif /* ! WRAPPER_DEGRADATIONS_HPP */
