%module Degradations


%{
    #define SWIG_FILE_WITH_INIT
    #include "Degradations.hpp"
%}

%include "numpy.i"

%init %{
    import_array();
%}


%apply (uint8_t* IN_ARRAY3, int DIM1, int DIM2, int DIM3) {(uint8_t *imgIn, int rows, int cols, int channels),
							     (uint8_t *imgBelow, int rowsBelow, int colsBelow, int channelsBelow),
							     (uint8_t *imgBackground, int rowsBackground, int colsBackground, int channelsBackground)};
%apply (uint8_t* IN_ARRAY2, int DIM1, int DIM2) {(uint8_t *pattern, int rowsPattern, int colsPattern)};

%apply (int IN_ARRAY1[ANY]) {(int color[ANY])};
%apply (uint8_t* ARGOUT_ARRAY1, int DIM1) {(uint8_t* imgOut, int imgOutLength)};


/* Enable kwargs everywhere, this allows us the specify the name of a
   parameter before affecting a value to it so we can pass them in any
   order (e.g.: size=10) */
%feature("kwargs") bleedThrough;
%feature("kwargs") blur;
%feature("kwargs") blurArea;
%feature("kwargs") gradientDomainDegradation;
%feature("kwargs") grayscaleCharsDegradation;
%feature("kwargs") holeDegradation;
%feature("kwargs") holeDegradation2;
%feature("kwargs") phantomCharacter;
%feature("kwargs") shadowBinding;
%feature("kwargs") degrade3D;
%feature("kwargs") degrade3DWithBackground;
%feature("kwargs") addGaussianNoise;
%feature("kwargs") addSpeckleNoise;
%feature("kwargs") addSaltAndPepperNoise;
%feature("kwargs") rotateFillColor;
%feature("kwargs") rotateFillImage;
%feature("kwargs") rotateFillImageRepeats;
%feature("kwargs") rotateFillInpaint1;
%feature("kwargs") rotateFillInpaint2;
%feature("kwargs") rotateFillInpaint3;
%feature("kwargs") elasticDeformation;
%feature("kwargs") elasticDeformation2;

%include "Degradations.hpp"
