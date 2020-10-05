#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <Degradations/BleedThrough.hpp>
#include <Degradations/BlurFilter.hpp>
#include <Degradations/Distortion3D.hpp>
#include <Degradations/ElasticDeformation.hpp>
#include <Degradations/GradientDomainDegradation.hpp>
#include <Degradations/GrayscaleCharsDegradationModel.hpp>
#include <Degradations/HoleDegradation.hpp>
#include <Degradations/NoiseDegradation.hpp>
#include <Degradations/PhantomCharacter.hpp>
#include <Degradations/RotationDegradation.hpp>
#include <Degradations/ShadowBinding.hpp>

#include "ndarray_converter.h"
#include "opencv_converter.hpp"

namespace py = pybind11;


PYBIND11_MODULE(DocCreatorPy, m) {

  NDArrayConverter::init_numpy();

  //--------------

  m.def("bleedThrough", &dc::BleedThrough::bleedThrough,
	"Apply bleedthrough effect between two images",
	py::arg("imgRecto"), py::arg("imgVerso"), py::arg("nbIter"),
	py::arg("pos") = cv::Point(0, 0), py::arg("numThreads") = -1);

 //  //--------------
  
  py::enum_<dc::BlurFilter::Method>(m, "BlurMethod", "Blur method")
    .value("GAUSSIAN", dc::BlurFilter::Method::GAUSSIAN, "Gaussian blur method")
    .value("MEDIAN", dc::BlurFilter::Method::MEDIAN, "Median blur method")
    .value("NORMAL", dc::BlurFilter::Method::NORMAL, "Normal blur method");
  
  m.def("blur", &dc::BlurFilter::blur,
 	"Apply blur filter to whole image",
 	py::arg("img"), py::arg("method"), py::arg("kernelSize"));

  
 //  //TODO: other blur functions (blurArea)

 //  //--------------

  m.def("degrade3D", &dc::Distortion3D::degrade3D,
 	"Apply 3D distortion to an image",
 	py::arg("img"), py::arg("meshFilename"), py::arg("random") = true);
  
  m.def("degrade3DN", &dc::Distortion3D::degrade3DN,
 	"Apply 3D distortion to an image and save N output images",
 	py::arg("img"), py::arg("numOutputImages"), py::arg("meshFilename"),
 	py::arg("outputPrefix"), py::arg("random") = true);

  m.def("degrade3DWithBackground", &dc::Distortion3D::degrade3DWithBackground,
 	"Apply 3D distortion to an image, with another image in the background",
 	py::arg("img"), py::arg("meshFilename"), py::arg("backgroundImg"),
 	py::arg("random") = true);
  
  m.def("degrade3DWithBackgroundN", &dc::Distortion3D::degrade3DWithBackgroundN,
 	"Apply 3D distortion to an image, with another image in the background, and save N output images",
 	py::arg("img"), py::arg("numOutputImages"), py::arg("meshFilename"),
 	py::arg("backgroundImg"),
 	py::arg("outputPrefix"), py::arg("random") = true);
  
 //--------------

  
 py::enum_<dc::ElasticDeformation::BorderReplication>(m, "ElasticBorderReplication", "Elastic border replication")
   .value("REPLICATE", dc::ElasticDeformation::BorderReplication::REPLICATE, "Replicate aaaaaa|abcdefgh|hhhhhhh")
   .value("REFLECT", dc::ElasticDeformation::BorderReplication::REFLECT, "Reflect fedcba|abcdefgh|hgfedcb")
   .value("WRAP", dc::ElasticDeformation::BorderReplication::WRAP, "Wrap cdefgh|abcdefgh|abcdefg")
   .value("REFLECT101", dc::ElasticDeformation::BorderReplication::REFLECT101, "Reflect101 gfedcb|abcdefgh|gfedcba ")
   .value("BLACK", dc::ElasticDeformation::BorderReplication::BLACK, "Black 000000|abcdefgh|000000");
  

  py::enum_<dc::ElasticDeformation::Interpolation>(m, "ElasticInterpolation", "Elastic interpolation")
    .value("NEAREST", dc::ElasticDeformation::Interpolation::NEAREST, "Nearest-neighbour interpolation")
    .value("BILINEAR", dc::ElasticDeformation::Interpolation::BILINEAR, "Bilinear interpolation")
    .value("AREA", dc::ElasticDeformation::Interpolation::AREA, "Area interpolation")
    .value("BICUBIC", dc::ElasticDeformation::Interpolation::BICUBIC, "Bicubic interpolation")
    .value("LANCZOS", dc::ElasticDeformation::Interpolation::LANCZOS, "Lanczos interpolation");

  
  m.def("elasticTransform", &dc::ElasticDeformation::transform,
 	"Apply elastic deformation to @a img as described in [Simard2003]", 
 	py::arg("img"),
 	py::arg("alpha") = 2.0f, py::arg("sigma") = 0.08f,
 	py::arg("borderMode") = dc::ElasticDeformation::BorderReplication::BLACK,
 	py::arg("interpolation") = dc::ElasticDeformation::Interpolation::BILINEAR);
  
  m.def("elasticTransform2", &dc::ElasticDeformation::transform2,
 	"Apply elastic deformation to @a img as described in [Simard2003], after a first affine transformation", 
 	py::arg("img"),
 	py::arg("alpha") = 2.0f, py::arg("sigma") = 0.08f, py::arg("alpha_affine") = 9.0f, 
 	py::arg("borderMode") = dc::ElasticDeformation::BorderReplication::BLACK,
 	py::arg("interpolation") = dc::ElasticDeformation::Interpolation::BILINEAR);

  //--------------

  py::enum_<dc::GradientDomainDegradation::InsertType>(m, "GDDInsertion", "GradientDomainDegradation insertion type")
    .value("GDD_INSERT_AS_IS", dc::GradientDomainDegradation::InsertType::INSERT_AS_IS, "GradientDomainDegradation insertion type: stain image is not transformed before insertion")
    .value("GDD_INSERT_AS_GRAY", dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY, "GradientDomainDegradation insertion type: stain image is transformed to gray before insertion")
    .value("GDD_INSERT_AS_GRAY_IF_GRAY", dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY_IF_GRAY, "GradientDomainDegradation insertion type: stain image is transformed to gray before insertion only if the destination image is gray");
  

  m.def("gddDegradation", &dc::GradientDomainDegradation::degradation,
	"Apply gradient domain degradation",
 	py::arg("img"),
	py::arg("stainImageDir"),
	py::arg("numStainsToInsert"),
	py::arg("insertType") = dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY_IF_GRAY,
	py::arg("doRotations") = true);

  //TODO:GDD: Ne pas donner @a numStainsToInsert mais avoir un chiffre calculée automatiquement en fonction de la taille ou du nombre de composantes connexes dans l'image ? 
  

  m.def("gddCopyOnto", &dc::GradientDomainDegradation::copyOnto,
	"Copy stain image on destination image in the gradient domain",
	py::arg("dstImg"), py::arg("stainImg"), py::arg("posCenter"));

  //--------------

  m.def("characterDegradation", &dc::GrayscaleCharsDegradation::degradation,
	"Apply character degradation on image",
	py::arg("img"), py::arg("level"),
	py::arg("percentOfIndepentSpots"), py::arg("percentOfOverlappingSpots"));

  
  //--------------

  py::enum_<dc::HoleDegradation::HoleType>(m, "HoleType", "Hole type")
    .value("CENTER", dc::HoleDegradation::HoleType::CENTER, "Hole center")
    .value("BORDER", dc::HoleDegradation::HoleType::BORDER, "Hole border")
    .value("CORNER", dc::HoleDegradation::HoleType::CORNER, "Hole corner");

  py::enum_<dc::HoleDegradation::HoleSide>(m, "HoleSide", "Hole side")
    .value("BORDER_TOP", dc::HoleDegradation::HoleSide::BORDER_TOP, "Hole top border")
    .value("BORDER_RIGHT", dc::HoleDegradation::HoleSide::BORDER_RIGHT, "Hole right border")
    .value("BORDER_BOTTOM", dc::HoleDegradation::HoleSide::BORDER_BOTTOM, "Hole bottom border")
    .value("BORDER_LEFT", dc::HoleDegradation::HoleSide::BORDER_LEFT, "Hole left border")
    .value("CORNER_TOPLEFT", dc::HoleDegradation::HoleSide::CORNER_TOPLEFT, "Hole top left corner")
    .value("CORNER_TOPRIGHT", dc::HoleDegradation::HoleSide::CORNER_TOPRIGHT, "Hole top right corner")
    .value("CORNER_BOTTOMRIGHT", dc::HoleDegradation::HoleSide::CORNER_BOTTOMRIGHT, "Hole bottom right corner")
    .value("CORNER_BOTTOMLEFT", dc::HoleDegradation::HoleSide::CORNER_BOTTOMLEFT, "Hole bottom left corner");

  m.def("addHole", &dc::HoleDegradation::addHole,
	"Add a hole pattern on image",
	py::arg("img"), py::arg("holePattern"),
	py::arg("pos"),
	py::arg("size"),
	py::arg("type"),
	py::arg("side"),
	py::arg("color") = cv::Scalar(0., 0., 0., 255.),
	py::arg("belowImg") = cv::Mat(),
	py::arg("shadowBorderWidth") = 0,
	py::arg("shadowBorderIntensity") = 0.1f);

  //B:TODO : getRandomPosition() : wrapper cv::Size

  m.def("addHoleAtRandom", &dc::HoleDegradation::addHoleAtRandom,
	"Add a hole pattern at a random position on image",
	py::arg("img"), py::arg("holePattern"), 
	py::arg("size"),
	py::arg("type"),
	py::arg("ratioOutside"),
	py::arg("side"),
	py::arg("color") = cv::Scalar(0., 0., 0., 255.),
	py::arg("belowImg") = cv::Mat(),
	py::arg("shadowBorderWidth") = 0,
	py::arg("shadowBorderIntensity") = 0.1f);
  
  //--------------

  py::enum_<dc::NoiseDegradation::AddNoiseType>(m, "AddNoiseType", "Noise add type")
    .value("AS_IS", dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS, "Added noise is independant on all channels. In particular, it may change a grayscale image in a color image")
    .value("AS_GRAY", dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY, "Added noise is the same on all channels")
    .value("AS_GRAY_IF_GRAY", dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY_IF_GRAY, "Added noise is the same on all channels only if the destination image is gray");


  m.def("addGaussianNoise", &dc::NoiseDegradation::addGaussianNoise,
	"Add Gaussian noise on image",
	py::arg("img"),
	py::arg("average") = 0.0f, 
	py::arg("standardDeviation") = 10.0f, 
	py::arg("addType") = dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS);
  
  m.def("addSpeckleNoise", &dc::NoiseDegradation::addSpeckleNoise,
	"Add speckle/multiplicative noise on image",
	py::arg("img"),
	py::arg("average") = 0.0f, 
	py::arg("standardDeviation") = 1.0f, 
	py::arg("addType") = dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS);
  
  m.def("addSaltAndPepperNoise", &dc::NoiseDegradation::addSaltAndPepperNoise,
	"Add Gaussian-distributed additive noise on image",
	py::arg("img"),
	py::arg("amount") = 0.15f, 
	py::arg("ratio") = 0.5f);

  //--------------

  m.def("phantomCharacter", &dc::PhantomCharacter::phantomCharacter,
	"Add phantom characters to image",
	py::arg("img"),
	py::arg("occurenceProbability") = 0.5f, 
	py::arg("phantomPatternsPath"));

  //--------------

  
   m.def("rotateFillColor", &dc::RotationDegradation::rotateFillColor,
	"Rotate image and fill appearing background pixels with uniform color",
	py::arg("img"), py::arg("angle"), 
	py::arg("color"));

   m.def("rotateFillImage", &dc::RotationDegradation::rotateFillImage,
	"Rotate image and fill appearing background pixels with given background image",
	py::arg("img"),	py::arg("angle"), 
	py::arg("backgroundImg"));

   m.def("rotateFillImageN", &dc::RotationDegradation::rotateFillImageN,
	 "Rotate image and fill appearing background pixels with given background image repeated several times",
	 py::arg("img"), py::arg("angle"), 
	 py::arg("backgroundImg"), py::arg("repeats"));

   
   //TODO: rotateFillImages() : how to pass std::vector<cv::Mat> ???

   py::enum_<dc::RotationDegradation::BorderReplication>(m, "RotationBorderReplication", "Rotation border replication")
     .value("REPLICATE", dc::RotationDegradation::BorderReplication::REPLICATE, "Replicate aaaaaa|abcdefgh|hhhhhhh")
     .value("REFLECT", dc::RotationDegradation::BorderReplication::REFLECT, "Reflect fedcba|abcdefgh|hgfedcb")
     .value("WRAP", dc::RotationDegradation::BorderReplication::WRAP, "Wrap cdefgh|abcdefgh|abcdefg")
     .value("REFLECT101", dc::RotationDegradation::BorderReplication::REFLECT101, "Reflect101 gfedcb|abcdefgh|gfedcba ");

   
   m.def("rotateFillBorder", &dc::RotationDegradation::rotateFillBorder,
	 "Rotate image and fill appearing background pixels replicating input image",
	 py::arg("img"), py::arg("angle"), 
	 py::arg("borderMode") = dc::RotationDegradation::BorderReplication::WRAP);
   
   m.def("rotateFillInpaint1", &dc::RotationDegradation::rotateFillInpaint1,
	 "Rotate image and fill appearing background pixels with inpainting",
	 py::arg("img"), py::arg("angle"), 
	 py::arg("inpaintingRatio") = 0.05f);
   
   m.def("rotateFillInpaint2", &dc::RotationDegradation::rotateFillInpaint2,
	 "Rotate image and fill appearing background pixels with inpainting",
	 py::arg("img"), py::arg("angle"), 
	 py::arg("backgroundImg"));
   
   m.def("rotateFillInpaint3", &dc::RotationDegradation::rotateFillInpaint3,
	 "Rotate image and fill appearing background pixels with inpainting",
	 py::arg("img"), py::arg("angle"), 
	 py::arg("backgroundImg"));
   
   //--------------

  py::enum_<dc::ShadowBinding::Border>(m, "ShadowBorder", "Shadow border")
    .value("TOP", dc::ShadowBinding::Border::TOP, "Shadow top border")
    .value("RIGHT", dc::ShadowBinding::Border::RIGHT, "Shadow right border")
    .value("BOTTOM", dc::ShadowBinding::Border::BOTTOM, "Shadow bottom border")
    .value("LEFT", dc::ShadowBinding::Border::LEFT, "Shadow left border");

   m.def("shadowBinding", &dc::ShadowBinding::shadowBinding,
	 "Add a shadow on a given border of image",
	 py::arg("img"), py::arg("border"), 
	 py::arg("distance"), py::arg("intensity"), py::arg("angle"));

   m.def("shadowBinding2", &dc::ShadowBinding::shadowBinding2,
	 "Add a shadow on a given border of image, proportional to image size",
	 py::arg("img"), 
	 py::arg("distanceRatio"), py::arg("border"),
	 py::arg("intensity"), py::arg("angle"));

  //--------------


  //TODO !!!
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO; 
#else
    m.attr("__version__") = "dev";
#endif
}
