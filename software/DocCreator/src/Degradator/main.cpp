
#include <cassert>
#include <iostream>
#include <random>

#include <QDebug>
#include <QDir>


#include "Degradations/BleedThrough.hpp"
#include "Degradations/BlurFilterQ.hpp"
#include "Degradations/Distortion3DModel/src/GLWidget.hpp"
#include "Degradations/GrayscaleCharsDegradationModel.hpp"
#include "Degradations/HoleDegradationQ.hpp"
#include "Degradations/PhantomCharacterQ.hpp"
#include "Degradations/ShadowBindingQ.hpp"
#include "Degradations/GradientDomainDegradationQ.hpp"

#include "Utils/ImageUtils.hpp" //toGray

#include "paths.hpp"

const bool do_bleedThrough = true;
const int bleedThrough_minIter = 10;
const int bleedThrough_maxIter = 30;

const bool do_charDeg = true;
const int charDeg_minLevel = 0;
const int charDeg_maxLevel= 10;


const bool do_phantom = true;
const QString phantom_patternsPath = QDir(PATH_IMAGES).absoluteFilePath("phantomPatterns");

const bool do_gradientDomain = true;
const QString gradientDomain_stainImagesPath = QDir(PATH_IMAGES).absoluteFilePath("stainImages/images");


const bool do_blur = true;
const int blur_minIntensity = 1;
const int blur_maxIntensity = 3;


const bool do_shadow = true;

const bool do_holes = true;
const int hole_minNumHoles = 0;
const int hole_maxNumHoles = 4;

const bool do_3D = false; //true;


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
void
saveImage(const QImage &img,
	  const QString &outputDirectory,
	  const QString &imageFilename,
	  const QString &suffixe)
{
  QString filename;
  //insert suffixe before extension
  const int pos = imageFilename.lastIndexOf('.');
  if (pos != -1) {
    filename = imageFilename.left(pos)+suffixe+imageFilename.mid(pos);
  }
  else {
    filename = imageFilename+suffixe;
  }
  filename = QDir(outputDirectory).absoluteFilePath(filename);

  const bool writeOk = img.save(filename);
  if (! writeOk) {
    qDebug()<<"ERROR: unable to save output file: "<<filename;
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

  QString inputImageDirectory = QString::fromLatin1(argv[1]);
  QString outputImageDirectory = QString::fromLatin1(argv[2]);

  QStringList filters;
  filters <<"*.png" << "*.jpg";
  const QStringList imageList = QDir(inputImageDirectory).entryList(filters);

  const int numImages = imageList.size();

  std::cerr<<numImages<<" images\n";

  QString hole_patterns_dirs[3];
  QStringList hole_patterns[3];
  if (do_holes) {
    hole_patterns_dirs[0] = QDir(PATH_IMAGES).absoluteFilePath("holePatterns/centerHoles");
    hole_patterns_dirs[1] = QDir(PATH_IMAGES).absoluteFilePath("holePatterns/borderHoles");
    hole_patterns_dirs[2] = QDir(PATH_IMAGES).absoluteFilePath("holePatterns/cornerHoles");
    hole_patterns[0] = QDir(hole_patterns_dirs[0]).entryList(filters); 
    hole_patterns[1] = QDir(hole_patterns_dirs[1]).entryList(filters); 
    hole_patterns[2] = QDir(hole_patterns_dirs[2]).entryList(filters);
  }

  QStringList meshList;
  if (do_3D) {
      QStringList meshFilters;
      meshFilters <<"*.brs";
      meshList = QDir(PATH_MESHES).entryList(meshFilters); 
  }

  for (int i=0; i<numImages; ++i) {

    QString suffixe = "";

    const QString imageFilename = QDir(inputImageDirectory).absoluteFilePath(imageList[i]);
    qDebug()<<imageFilename<<"\n";
    QImage currImg(imageFilename);
    if (currImg.isNull()) {
      qDebug()<<"Warning: unable to load file: "<<imageFilename;
      continue;
    }

    if (do_bleedThrough) {
      const int versoIndex = random_in_range(0, numImages-1);
      const QString versoFilename = QDir(inputImageDirectory).absoluteFilePath(imageList[versoIndex]);
      QImage versoImg(versoFilename);
      if (versoImg.isNull()) {
	qDebug()<<"Warning: unable to load file: "<<imageFilename;
      }
      else {
	const int nbIter = random_in_range(bleedThrough_minIter, bleedThrough_maxIter);
	const QImage imgBleed = dc::bleedThrough(currImg, versoImg, nbIter);
	if (cumulate) {
	  currImg = imgBleed;
	  suffixe += "_bt";
	}
	else {
	  saveImage(currImg, outputImageDirectory, imageList[i], "_bt");
	}
      }
    }

    
    if (do_charDeg) {
      const int level = random_in_range(charDeg_minLevel, charDeg_maxLevel);
      QImage currImgGray = toGray(currImg);
      dc::GrayscaleCharsDegradationModel deg(currImgGray);
      const QImage imgCharDeg = deg.degradateByLevel(level);
      if (cumulate) {
	currImg = imgCharDeg;
	suffixe += "_cd";
      }
      else {
	saveImage(currImg, outputImageDirectory, imageList[i], "_cd");
      }	
    }

    if (do_phantom) {
      const dc::PhantomCharacter::Frequency frequency = (dc::PhantomCharacter::Frequency)random_in_range(0, 2);
      QImage imgPhant = dc::PhantomCharacter::phantomCharacter(currImg, frequency, phantom_patternsPath);
      if (cumulate) {
	currImg = imgPhant;
	suffixe += "_ph";
      }
      else {
	saveImage(currImg, outputImageDirectory, imageList[i], "_ph");
      }	
    }

    if (do_gradientDomain) {
      const dc::GradientDomainDegradation::InsertType insertType = dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY_IF_GRAY;
      const bool doRotations = true;
      size_t numStains = (size_t)random_in_range(10, 20);
      QImage imgGDD = dc::GradientDomainDegradation::degradation(currImg, gradientDomain_stainImagesPath, numStains, insertType, doRotations);
      if (cumulate) {
	currImg = imgGDD;
	suffixe += "_gdd";
      }
      else {
	saveImage(currImg, outputImageDirectory, imageList[i], "_gdd");	
      }
    }

    
    if (do_blur) {
      const dc::BlurFilter::Method m = (dc::BlurFilter::Method)random_in_range(0, 2);
      const int intensity = 1+2*random_in_range(blur_minIntensity, blur_maxIntensity);
      QImage imgBlur = dc::BlurFilter::blur(currImg, m, intensity);
      if (cumulate) {
	currImg = imgBlur;
	suffixe += "_bl";
      }
      else {
	saveImage(currImg, outputImageDirectory, imageList[i], "_bl");
      }
    }

    if (do_shadow) {
      const dc::ShadowBinding::Border border = (dc::ShadowBinding::Border)random_in_range(0, 3);
      const int distance = random_in_range(0, int(0.25*std::min(currImg.width(), currImg.height())));
      const float intensity = random_in_range(0, 255)/255.f;
      const float angle = random_in_range(0, 90);
      QImage imgShad = dc::ShadowBinding::shadowBinding(currImg, border, distance, intensity, angle);
      if (cumulate) {
	currImg = imgShad;
	suffixe += "_sh";
      }
      else {
	saveImage(currImg, outputImageDirectory, imageList[i], "_sh");
      }
    }

    if (do_holes) {
      const int numHoles = random_in_range(hole_minNumHoles, hole_maxNumHoles);
      QImage imgHole = currImg;
      for (int j=0; j<numHoles; ++j) {
	const dc::HoleDegradation::HoleType holeType = (dc::HoleDegradation::HoleType)random_in_range(0, 2);
	const QStringList &patterns = hole_patterns[(int)holeType];
	const int holeIndex = random_in_range(0, patterns.size()-1);
	const QString holePatternFilename = QDir(hole_patterns_dirs[(int)holeType]).absoluteFilePath(patterns[holeIndex]);
	QImage holePattern(holePatternFilename);
	if (holePattern.isNull()) {
	  qDebug()<<"Warning: unable to load file: "<<patterns[holeIndex];
	  continue;
	}
	
	const float ratioOutside = 0.3f;
	const int size = 0;
	const int side = random_in_range(0, 3);
	const QColor color(0, 0, 0);
	
	imgHole = dc::HoleDegradation::holeDegradation(imgHole, holePattern, ratioOutside, size, holeType, side, color);

      }

      if (cumulate) {
	currImg = imgHole;
	suffixe += "_ho";
      }
      else {
	saveImage(currImg, outputImageDirectory, imageList[i], "_ho");
      }
	
    }
      
    if (do_3D) {

      auto w = new GLWidget();
      w->show();
      w->setTexture(currImg);
      w->setUseTexture(true);

      const int meshIndex = random_in_range(0, meshList.size()-1);
      const QString &meshFilename = meshList[meshIndex];
      w->loadMesh(meshFilename);
      QImage img3D = w->takeScreenshotHiRes();
	if (cumulate) {
	  currImg = img3D;
	  suffixe += "_td";
	}
	else {
	  saveImage(currImg, outputImageDirectory, imageList[i], "_td");
	}
      
    }

    if (cumulate) {
      saveImage(currImg, outputImageDirectory, imageList[i], suffixe);
    }
    
  }
  


  return 0;
}
