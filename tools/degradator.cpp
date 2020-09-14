/*
  This code is an example of degradation effects application.
  It will apply several degradation effects to images of an input directory and write the result images in an output directory.

  For each degradation effect <E>, we have a boolean variable do_<E> that indicates if the effect is applied. You will have to set the variable to true (resp. false) to enable (resp. disable) the corresponding degradation effect.
  If there exists a parameter <P> for the effect <E> that takes values in a range, the variables <E>_min<P> and <E>_max<P> indicate the range from which a random value will be drawn for <P> when the degradation effect <E> is applied. You may change the values of <E>_min<P> and <E>_max<P> to change the range and obtain different result images.
 Some other parameters of the effect <E> may have default values further in the code. You may change the as well if necessary.

 If N degradation effects are enabled, it is possible to decide if they are combined/cumulated for a given input image, producing one outut image, or if they are applied individually, producing N output images. Set the boolean variable "cumulate" to true (resp. false) to produce one (resp. N) output image(s).

 */

#include <cassert>
#include <iostream>
#include <random>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Degradations/BleedThrough.hpp"
#include "Degradations/BlurFilter.hpp"
#include "Degradations/Distortion3D.hpp"
#include "Degradations/ElasticDeformation.hpp"
#include "Degradations/GradientDomainDegradation.hpp"
#include "Degradations/GrayscaleCharsDegradationModel.hpp"
#include "Degradations/HoleDegradation.hpp"
#include "Degradations/NoiseDegradation.hpp"
#include "Degradations/PhantomCharacter.hpp"
#include "Degradations/RotationDegradation.hpp"
#include "Degradations/ShadowBinding.hpp"

#include "Degradations/FileUtils.hpp"

#include "paths.hpp"

const bool do_bleedThrough = true;
const int bleedThrough_minIter = 10;
const int bleedThrough_maxIter = 30;

const bool do_charDeg = true;
const int charDeg_minLevel = 0;
const int charDeg_maxLevel= 10;


const bool do_phantom = true;
const std::string phantom_patternsPath = PHANTOM_PATTERNS_PATH;

const bool do_gradientDomain = true;
const std::string gradientDomain_stainImagesPath = STAIN_IMAGES_PATH;

const bool do_gaussianNoise = true;
const float gaussianNoise_average = 0.0f;
const float gaussianNoise_stddev = 13.0f;

const bool do_blur = true;
const int blur_minIntensity = 1;
const int blur_maxIntensity = 3;

const bool do_rotation = true;
const float rotation_minAngle = -7.f;
const float rotation_maxAngle = 7.f;
const int rotation_minRepeats = 0;
const int rotation_maxRepeats = 3;

const bool do_shadow = true;

const bool do_holes = true;
const int hole_minNumHoles = 0;
const int hole_maxNumHoles = 4;

const bool do_elasticDeformation = true;
const float elasticDeformation_alpha = 2.0f;
const float elasticDeformation_sigma = 0.083f;

#if BUILD_WITH_OSMESA
const bool do_3D = true;
#else
const bool do_3D = false;
#endif


const bool cumulate = true;


std::random_device rd;
std::mt19937 mt(rd());

//return a random value in [rMin; rMax]
static
int
random_in_range(int rMin, int rMax)
{
  std::uniform_int_distribution<int> dist(rMin, rMax);
  return dist(mt);
}

static
float
random_in_range(float rMin, float rMax)
{
  std::uniform_real_distribution<float> dist(rMin,rMax);
  return dist(mt);
}

static
void
saveImage(const cv::Mat &img,
	  const std::string &outputDirectory,
	  const std::string &imageFilename,
	  const std::string &suffixe)
{
  std::string filename;
  //insert suffixe before extension
  const std::size_t pos = imageFilename.find_last_of('.');
  if (pos != std::string::npos) {
    filename = std::string(imageFilename, 0, pos)+suffixe+std::string(imageFilename, pos);
  }
  else {
    filename = imageFilename+suffixe;
  }
  filename = dc::makePath(outputDirectory, filename);

  const bool writeOk = cv::imwrite(filename, img);
  if (! writeOk) {
    std::cerr<<"ERROR: unable to save output file: "<<filename<<"\n";
  }
}


int
main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0]
              << " inputImageDirectory outputImageDirectory\n";
    exit(EXIT_FAILURE);
  }

  std::string inputImageDirectory = argv[1];
  std::string outputImageDirectory = argv[2];

  const std::vector<std::string> imageList = dc::listDirectory(inputImageDirectory);

  const size_t numImages = imageList.size();

  std::cerr<<numImages<<" input images\n";

  std::string hole_patterns_dirs[3];
  std::vector<std::string> hole_patterns[3];
  if (do_holes) {
    hole_patterns_dirs[0] = dc::makePath(HOLES_PATTERNS_PATH, "centerHoles");
    hole_patterns_dirs[1] = dc::makePath(HOLES_PATTERNS_PATH, "borderHoles");
    hole_patterns_dirs[2] = dc::makePath(HOLES_PATTERNS_PATH, "cornerHoles");
    hole_patterns[0] = dc::listDirectory(hole_patterns_dirs[0]);
    hole_patterns[1] = dc::listDirectory(hole_patterns_dirs[1]);
    hole_patterns[2] = dc::listDirectory(hole_patterns_dirs[2]);
  }

  std::vector<std::string> meshList;
  if (do_3D) {
    meshList = dc::listDirectory(MESHES_PATH);
  }

  for (size_t i=0; i<numImages; ++i) {

    std::string suffixe = "";

    const std::string imageFilename = dc::makePath(inputImageDirectory, imageList[i]);
    std::cerr<<imageFilename<<"\n";
    cv::Mat currImg = cv::imread(imageFilename);
    if (currImg.empty()) {
      std::cerr<<"Warning: unable to load image file: "<<imageFilename<<"\n";
      continue;
    }

    if (do_bleedThrough) {
      const int versoIndex = random_in_range(0, numImages-1);
      const std::string versoFilename = dc::makePath(inputImageDirectory, imageList[versoIndex]);
      cv::Mat versoImg = cv::imread(versoFilename);
      if (versoImg.empty()) {
	std::cerr<<"Warning: unable to load verso image file: "<<versoFilename<<"\n";
	std::cerr<<"Warning: will not apply bleedThrough to image file: "<<imageFilename<<"\n";
      }
      else {
	const int nbIter = random_in_range(bleedThrough_minIter, bleedThrough_maxIter);
	const cv::Mat imgBleed = dc::BleedThrough::bleedThrough(currImg, versoImg, nbIter);
	if (cumulate) {
	  currImg = imgBleed;
	  suffixe += "_bt";
	}
	else {
	  saveImage(imgBleed, outputImageDirectory, imageList[i], "_bt");
	}
      }
    }

    
    if (do_charDeg) {
      const int level = random_in_range(charDeg_minLevel, charDeg_maxLevel);
      cv::Mat currImgGray;
      cv::cvtColor(currImg, currImgGray, cv::COLOR_BGR2GRAY);
      dc::GrayscaleCharsDegradationModel deg(currImgGray);
      const cv::Mat imgCharDeg = deg.degradateByLevel_cv(level);
      if (cumulate) {
	currImg = imgCharDeg;
	suffixe += "_cd";
      }
      else {
	saveImage(imgCharDeg, outputImageDirectory, imageList[i], "_cd");
      }	
    }

    
    if (do_phantom) {
      const dc::PhantomCharacter::Frequency frequency = (dc::PhantomCharacter::Frequency)random_in_range(0, 2);
      cv::Mat imgPhant = dc::PhantomCharacter::phantomCharacter(currImg, frequency, phantom_patternsPath);
      if (cumulate) {
	currImg = imgPhant;
	suffixe += "_ph";
      }
      else {
	saveImage(imgPhant, outputImageDirectory, imageList[i], "_ph");
      }	
    }

    
    if (do_gradientDomain) {
      const dc::GradientDomainDegradation::InsertType insertType = dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY_IF_GRAY;
      const bool doRotations = true;
      size_t numStains = (size_t)random_in_range(10, 20);
      cv::Mat imgGDD = dc::GradientDomainDegradation::degradation(currImg, gradientDomain_stainImagesPath, numStains, insertType, doRotations);
      if (cumulate) {
	currImg = imgGDD;
	suffixe += "_gdd";
      }
      else {
	saveImage(imgGDD, outputImageDirectory, imageList[i], "_gdd");	
      }
    }

    if (do_gaussianNoise) {
      const dc::NoiseDegradation::AddNoiseType addType = dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY_IF_GRAY;

      cv::Mat imgND = dc::NoiseDegradation::addGaussianNoise(currImg, gaussianNoise_average, gaussianNoise_stddev, addType);
      if (cumulate) {
	currImg = imgND;
	suffixe += "_nd";
      }
      else {
	saveImage(imgND, outputImageDirectory, imageList[i], "_nd");
      }
    }

    
    if (do_blur) {
      const dc::BlurFilter::Method m = (dc::BlurFilter::Method)random_in_range(0, 2);
      const int intensity = 1+2*random_in_range(blur_minIntensity, blur_maxIntensity);
      cv::Mat imgBlur = dc::BlurFilter::blur(currImg, m, intensity);
      if (cumulate) {
	currImg = imgBlur;
	suffixe += "_bl";
      }
      else {
	saveImage(imgBlur, outputImageDirectory, imageList[i], "_bl");
      }
    }

    if (do_rotation) {
      const float angle = random_in_range(rotation_minAngle, rotation_maxAngle);
      const int backgroundIndex = random_in_range(0, numImages-1);
      const std::string backgroundFilename = dc::makePath(inputImageDirectory, imageList[backgroundIndex]);
      cv::Mat backgroundImg = cv::imread(backgroundFilename);
      cv::Mat rotatedImg;
      if (backgroundImg.empty()) {
	std::cerr<<"Warning: unable to load background image file: "<<backgroundFilename<<"\n";
	rotatedImg = dc::RotationDegradation::rotateFillColor(currImg, angle, cv::Scalar(0, 0, 0));
      }
      else {
	const int repeats = random_in_range(rotation_minRepeats, rotation_maxRepeats);
	rotatedImg = dc::RotationDegradation::rotateFillImage(currImg, angle, backgroundImg, repeats);
      }
      if (cumulate) {
	currImg = rotatedImg;
	suffixe += "_rt";
      }
      else {
	saveImage(rotatedImg, outputImageDirectory, imageList[i], "_rt");
      }
    }
    
    
    if (do_shadow) {
      const dc::ShadowBinding::Border border = (dc::ShadowBinding::Border)random_in_range(0, 3);
      const int distance = random_in_range(0, int(0.25*std::min<int>(currImg.cols, currImg.rows)));
      const float intensity = random_in_range(0, 255)/255.f;
      const float angle = random_in_range(0, 90);
      cv::Mat imgShad = dc::ShadowBinding::shadowBinding(currImg, border, distance, intensity, angle);
      if (cumulate) {
	currImg = imgShad;
	suffixe += "_sh";
      }
      else {
	saveImage(imgShad, outputImageDirectory, imageList[i], "_sh");
      }
    }

    
    if (do_holes) {
      const int numHoles = random_in_range(hole_minNumHoles, hole_maxNumHoles);
      cv::Mat imgHole = currImg;
      for (int j=0; j<numHoles; ++j) {
	const dc::HoleDegradation::HoleType holeType = (dc::HoleDegradation::HoleType)random_in_range(0, 2);
	const std::vector<std::string> &patterns = hole_patterns[(int)holeType];
	const int holeIndex = random_in_range(0, patterns.size()-1);
	const std::string holePatternFilename = dc::makePath(hole_patterns_dirs[(int)holeType], patterns[holeIndex]);
	cv::Mat holePattern = cv::imread(holePatternFilename);
	if (holePattern.empty()) {
	  std::cerr<<"Warning: unable to load hole pattern file: "<<patterns[holeIndex]<<"\n";
	  continue;
	}
	
	const float ratioOutside = 0.3f;
	const int size = 0;
	const int side = random_in_range(0, 3);
	const cv::Scalar color(0, 0, 0);
	
	imgHole = dc::HoleDegradation::holeDegradation(imgHole, holePattern, size, holeType, ratioOutside, side, color);

      }

      if (cumulate) {
	currImg = imgHole;
	suffixe += "_ho";
      }
      else {
	saveImage(imgHole, outputImageDirectory, imageList[i], "_ho");
      }
	
    }

    if (do_elasticDeformation) {

      const cv::Mat imgED = dc::ElasticDeformation::transform(currImg, elasticDeformation_alpha, elasticDeformation_sigma);
      if (cumulate) {
	currImg = imgED;
	suffixe += "_ed";
      }
      else {
	saveImage(imgED, outputImageDirectory, imageList[i], "_ed");
      }

    }
    
    if (do_3D) {

      const int meshIndex = random_in_range(0, meshList.size()-1);
      const std::string &meshFilename = meshList[meshIndex];
      const bool random = true;

      const cv::Mat img3D = dc::Distortion3D::degrade3D(currImg, meshFilename, random);
      if (cumulate) {
	currImg = img3D;
	suffixe += "_td";
      }
      else {
	saveImage(img3D, outputImageDirectory, imageList[i], "_td");
      }
      
    }

    if (cumulate) {
      saveImage(currImg, outputImageDirectory, imageList[i], suffixe);
    }
    
  }
  


  return 0;
}
