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

  m.def("bleedThrough", &dc::BleedThrough::bleedThrough, R"pbdoc(
	Apply a bleedthrough effect between two images.

	Apply a bleedthrough effect between two images: frontImg and
	backImg. backImg will appear as it had bled through frontImg.

	Warnings
	--------
	frontImg and backImg must have the same ndim and dtype.
	Images must have dtype==np.uint8.
	Images must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	backImg is not transformed. In particulat, it is considered
	already mirrored.

	Parameters
	----------
	frontImg : ndarray
		Front image.
	backImg : ndarray
		Back image.
	numIter : int
		Number of iterations. The higher the number of iterations,
		the more the back image will bleed through the front
		image.
	pos : tuple
		backImg is positionned at position pos in fronImage
		frame. Coordinates may be negative.
	numThreads : int
		Number of threads used to compute the effect. If negative,
		a value is automatically computed according to the image
		size and the number of available CPU cores.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than frontImg.

	)pbdoc",
	py::arg("frontImg"), py::arg("backImg"), py::arg("numIter"),
	py::arg("pos") = cv::Point(0, 0),
	py::arg("numThreads") = -1);

 //  //--------------
  
  py::enum_<dc::BlurFilter::Method>(m, "BlurMethod", "Blur method")
    .value("GAUSSIAN", dc::BlurFilter::Method::GAUSSIAN, "Gaussian blur method")
    .value("MEDIAN", dc::BlurFilter::Method::MEDIAN, "Median blur method")
    .value("NORMAL", dc::BlurFilter::Method::NORMAL, "Normal blur method");
  
  m.def("blur", &dc::BlurFilter::blur, R"pbdoc(
	Apply a blur filter to an image.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	method : DocCreatorPy.BlurMethod
		Kind of blur to apply.
	kernelSize : int
		kernel size of blur filter. It must be positive and odd.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.

	)pbdoc",
 	py::arg("img"), py::arg("method"), py::arg("kernelSize"));

  
 //  //TODO: other blur functions (blurArea)

 //  //--------------

  m.def("degrade3D", &dc::Distortion3D::degrade3D, R"pbdoc(
	Apply 3D distortion to an image.

	Algorithm described in:
	Semi-synthetic Document Image Generation Using Texture Mapping
	on Scanned 3D Document Shapes
	KIEU, Van Cuaong, JOURNET, Nicholas, VISANI, Muriel,
	MULLOT, Rémy, DOMENGER, Jean-Philippe
	In : 2013 12th International Conference on Document Analysis
	and Recognition (ICDAR). IEEE, 2013. p. 489-493.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	The background will be black.

	Parameters
	----------
	img : ndarray
		Input image.
	meshFilename : str
		Input mesh filename. Mesh file must an .obj or .brs file,
		with texture coordinates.
	random : bool
		Indicates if a random lighting and orientation must be
		used.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.

	)pbdoc",
	py::arg("img"), py::arg("meshFilename"),
	py::arg("random") = true);
  
  m.def("degrade3DN", &dc::Distortion3D::degrade3DN, R"pbdoc(
	Apply 3D distortion to an image and save N output images.

	Algorithm described in:
	Semi-synthetic Document Image Generation Using Texture Mapping
	on Scanned 3D Document Shapes
	KIEU, Van Cuaong, JOURNET, Nicholas, VISANI, Muriel,
	MULLOT, Rémy, DOMENGER, Jean-Philippe
	In : 2013 12th International Conference on Document Analysis
	and Recognition (ICDAR). IEEE, 2013. p. 489-493.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	The background will be black.

	Parameters
	----------
	img : ndarray
		Input image.
	numOutputImages : int
		Number of generated output images to save.
	meshFilename : str
		Input mesh filename. Mesh file must an .obj or .brs file,
		with texture coordinates.
	outputPrefix : str
		Prefix of output images. It may contain the path where to
		save images.
	random : bool
		Indicates if a random lighting and orientation must be
		used.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.

	)pbdoc",
	py::arg("img"), py::arg("numOutputImages"),
	py::arg("meshFilename"),
 	py::arg("outputPrefix"), py::arg("random") = true);

  m.def("degrade3DWithBackground",
	&dc::Distortion3D::degrade3DWithBackground, R"pbdoc(
	Apply 3D distortion to an image, with another image in the
	background.

	Algorithm described in:
	Semi-synthetic Document Image Generation Using Texture Mapping
	on Scanned 3D Document Shapes
	KIEU, Van Cuaong, JOURNET, Nicholas, VISANI, Muriel,
	MULLOT, Rémy, DOMENGER, Jean-Philippe
	In : 2013 12th International Conference on Document Analysis
	and Recognition (ICDAR). IEEE, 2013. p. 489-493.

	Warnings
	--------
	Images must have dtype==np.uint8.
	Images must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	meshFilename : str
		Input mesh filename. Mesh file must an .obj or .brs file,
		with texture coordinates.
	backgroundImg : ndarray
		Background image.
	random : bool
		Indicates if a random lighting and orientation must be
		used.

	Returns
	-------
	ndarray
		The modified image, with same shape[:2] and dtype than
		img. shape[3] will be the largest of the input images.

	)pbdoc",
	py::arg("img"), py::arg("meshFilename"),
	py::arg("backgroundImg"),
 	py::arg("random") = true);
  
  m.def("degrade3DWithBackgroundN",
	&dc::Distortion3D::degrade3DWithBackgroundN, R"pbdoc(
	Apply 3D distortion to an image, with another image in the
	background, and save N output images.

	Algorithm described in:
	Semi-synthetic Document Image Generation Using Texture Mapping
	on Scanned 3D Document Shapes
	KIEU, Van Cuaong, JOURNET, Nicholas, VISANI, Muriel,
	MULLOT, Rémy, DOMENGER, Jean-Philippe
	In : 2013 12th International Conference on Document Analysis
	and Recognition (ICDAR). IEEE, 2013. p. 489-493.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	The background will be black.

	Parameters
	----------
	img : ndarray
		Input image.
	numOutputImages : int
		Number of generated output images to save.
	meshFilename : str
		Input mesh filename. Mesh file must an .obj or .brs file,
		with texture coordinates.
	outputPrefix : str
		Prefix of output images. It may contain the path where to
		save images.
	random : bool
		Indicates if a random lighting and orientation must be
		used.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.

	)pbdoc",
	py::arg("img"), py::arg("numOutputImages"),
	py::arg("meshFilename"),
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

  
  m.def("elasticTransform",
	&dc::ElasticDeformation::transform,  R"pbdoc(
	Apply an elastic deformation to an image.

	Apply an elastic deformation to an image as described in:
	Best Practices for Convolutional Neural Networks applied to
	Visual Document Analysis,
	Simard, Steinkraus and Platt,
	in Proc. of the International Conference on Document Analysis
	and Recognition, 2003.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	alpha : float
		Scaling factor controlling the intensity of deformation.
	sigma : float
		Elasticity coefficient (standard deviation of Gaussian).
	borderMode : DocCreatorPy.ElasticBorderReplication
		Border replication method.
	Interpolation : DocCreatorPy.ElasticInterpolation
		Interpolation method.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.

	)pbdoc",
 	py::arg("img"),
 	py::arg("alpha") = 2.0f, py::arg("sigma") = 0.08f,
 	py::arg("borderMode") = dc::ElasticDeformation::BorderReplication::BLACK,
 	py::arg("interpolation") = dc::ElasticDeformation::Interpolation::BILINEAR);
  
  m.def("elasticTransform2",
	&dc::ElasticDeformation::transform2, R"pbdoc(
	Apply an elastic deformation to an image, after a first affine
	transformation.

	Apply an elastic deformation to an image, after a first affine
	transformation, as described in:
	Best Practices for Convolutional Neural Networks applied to
	Visual Document Analysis,
	Simard, Steinkraus and Platt,
	in Proc. of the International Conference on Document Analysis
	and Recognition, 2003.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	alpha : float
		Scaling factor controlling the intensity of deformation.
	sigma : float
		Elasticity coefficient (standard deviation of Gaussian).
	alphaAffine : float
		Scaling factor controlling the intensity of the initial
		affine deformation.
	borderMode : DocCreatorPy.ElasticBorderReplication
		Border replication method.
	Interpolation : DocCreatorPy.ElasticInterpolation
		Interpolation method.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.

	)pbdoc",
 	py::arg("img"),
	py::arg("alpha") = 2.0f,
	py::arg("sigma") = 0.08f,
	py::arg("alphaAffine") = 9.0f,
 	py::arg("borderMode") = dc::ElasticDeformation::BorderReplication::BLACK,
 	py::arg("interpolation") = dc::ElasticDeformation::Interpolation::BILINEAR);

  //--------------

  py::enum_<dc::GradientDomainDegradation::InsertType>(m, "GDDInsertion", "GradientDomainDegradation insertion type")
    .value("GDD_INSERT_AS_IS", dc::GradientDomainDegradation::InsertType::INSERT_AS_IS, "GradientDomainDegradation insertion type: stain image is not transformed before insertion")
    .value("GDD_INSERT_AS_GRAY", dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY, "GradientDomainDegradation insertion type: stain image is transformed to gray before insertion")
    .value("GDD_INSERT_AS_GRAY_IF_GRAY", dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY_IF_GRAY, "GradientDomainDegradation insertion type: stain image is transformed to gray before insertion only if the destination image is gray");
  

  m.def("gddDegradation",
	&dc::GradientDomainDegradation::degradation, R"pbdoc(
	Apply a gradient domain degradation.

	Implements gradient-domain degradation, using
	OpenCV seamlessClone(), as described in:
	Gradient-domain degradations for improving historical
	documents images layout analysis.
	SEURET, Mathias, CHEN, Kai, EICHENBERGERY, Nicole,
	LIWICKI, Markus, INGOLD, Rolf.
	In : 2015 13th International Conference on Document Analysis
	and Recognition (ICDAR). IEEE, 2015. p. 1006-1010.

	Several stain images are copied at random positions onto the
	input image.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	This implementation is rather slow.

	Parameters
	----------
	img : ndarray
		Input image.
	stainImageDir : str
		Directory to load stain images from.
	numStainsToInsert : int
		Number of stain images to insert.
	insertType : DocCreatorPy.GDDInsertion
		Insertion type.
	doRotations : bool
		Indicates whether stain images are rotated before insertion.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.

	See Also
	--------
	gddCopyOnto

	)pbdoc",
 	py::arg("img"),
	py::arg("stainImageDir"),
	py::arg("numStainsToInsert"),
	py::arg("insertType") = dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY_IF_GRAY,
	py::arg("doRotations") = true);

  //TODO:GDD: Do not pass @a numStainsToInsert
  //but have an automatic value computed according to image size
  // or the number of connected components ?

  m.def("gddCopyOnto",
	&dc::GradientDomainDegradation::copyOnto, R"pbdoc(
	Copy stain image on destination image in the gradient domain.

	Warnings
	--------
	images must have dtype==np.uint8.
	images must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	This implementation is rather slow.

	Parameters
	----------
	img : ndarray
		Input image.
	stainImg : ndarray
		Stain image to insert.
	numStainsToInsert : int
		Number of stain images to insert.
	posCenter : tuple
		Position of center of stainImg on destination image.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.

	)pbdoc",
	py::arg("img"), py::arg("stainImg"),
	py::arg("posCenter"));

  //--------------

  m.def("characterDegradation",
	&dc::GrayscaleCharsDegradation::degradation, R"pbdoc(
	Apply character degradation on image.

	Implements ink degradation model detailled in:
	An Efficient Parametrization of Character Degradation Model
	for Semi-synthetic Image Generation
	KIEU, Van Cuaong, JOURNET, Nicholas, VISANI, Muriel,
	MULLOT, Rémy, DOMENGER, Jean-Philippe
	In : 2nd International Workshop on Historical Document
	Imaging and Processing. 2013.

	There are 3 kinds of degradation spots: independent spots,
	overlapping spots and disconnection spots.
	Ratio of disconnection spots will be determined as
	(1.0 - indepentSpotsRatio - overlappingSpotsRatio).

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	level : int
		scale factor, in [0; 10]. The number of
		degradation spots will be determined according to the
		number of connected components in image and multiplied by
		this scale factor. The higher the level, the higher the
		number of degradation spots.
	indepentSpotsRatio : float
		Ratio of independent spots, in [0.0; 1.0]
	overlappingSpotsRatio : float
		Ratio of overlapping spots, in [0.0; 1.0]

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.

	)pbdoc",
	py::arg("img"), py::arg("level"),
	py::arg("indepentSpotsRatio"),
	py::arg("overlappingSpotsRatio"));

  
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

  m.def("addHole", &dc::HoleDegradation::addHole, R"pbdoc(
	Add a hole pattern to an image at a given position.

	Add a hole pattern to an image at a given position.
	Pixels inside hole are filled with pixels from belowImg if
	belowImg is not empty and visible, or with color belowColor
	otherwise.

	Warnings
	--------
	Images must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).
	holePattern must have have ndim==2 or (ndim==3 and shape[2]==1).
	If not empty, belowImg must have the same dtype and shape[2]
	than img.

	Border hole images are considered to be for the top border side.
	Corner hole images are considered to be for the left border side.
	They will be transformed according to the specified side.

	Parameters
	----------
	img : ndarray
		Input image.
	holePattern : ndarray
		Hole pattern image.
	pos : tuple
		Position of holePattern in img.
	size : int
		Increment (in each dimension) for hole pattern image.
	type : DocCreatorPy.HoleType
		Type of hole pattern.
	side : DocCreatorPy.HoleSide
		Side of hole. For a center hole, this value is node used.
		For a border or corner hole pattern image, the image will
		be transformed according to the specified side.
	belowColor : tuple
		Color used to fill the applied hole pattern image, only
		if belowImg is not empty.
	shadowBorderWidth : int
		Width in pixels of shadow added around hole border.
		If zero, no shadow is added.
	shadowBorderIntensity : float
		Intensity in [0; 1] by which the original below
		color/image is multiplied to obtain the shadow value on
		hole border. The closer to 0, the darker the shadow.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.

	)pbdoc",
	py::arg("img"), py::arg("holePattern"),
	py::arg("pos"),
	py::arg("size"),
	py::arg("type"),
	py::arg("side"),
	py::arg("belowColor") = cv::Scalar(0., 0., 0., 255.),
	py::arg("belowImg") = cv::Mat(),
	py::arg("shadowBorderWidth") = 0,
	py::arg("shadowBorderIntensity") = 0.1f);

  //B:TODO : getRandomPosition() : wrapper cv::Size

  m.def("addHoleAtRandom",
	&dc::HoleDegradation::addHoleAtRandom, R"pbdoc(
	Add a hole pattern to an image at a random position.

	Warnings
	--------
	Images must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).
	holePattern must have have ndim==2 or (ndim==3 and shape[2]==1).
	If not empty, belowImg must have the same dtype and shape[2]
	than img.

	Border hole images are considered to be for the top border side.
	Corner hole images are considered to be for the left border side.
	They will be transformed according to the specified side.

	Parameters
	----------
	img : ndarray
		Input image.
	holePattern : ndarray
		Hole pattern image.
	size : int
		Increment (in each dimension) for hole pattern image.
	type : DocCreatorPy.HoleType
		Type of hole pattern.
	ratioOutside : float
		value in [0; 1.0] that indicates the ratio of holePattern
		image that can be outside image. 0.6 means 60% of pattern
		can be outside image, 1.0 means pattern is completely
		outside.
	side : DocCreatorPy.HoleSide
		Side of hole. For a center hole, this value is node used.
		For a border or corner hole pattern image, the image will
		be transformed according to the specified side.
	belowColor : tuple
		Color used to fill the applied hole pattern image, only
		if belowImg is not empty.
	shadowBorderWidth : int
		Width in pixels of shadow added around hole border.
		If zero, no shadow is added.
	shadowBorderIntensity : float
		Intensity in [0; 1] by which the original below
		color/image is multiplied to obtain the shadow value on
		hole border. The closer to 0, the darker the shadow.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"), py::arg("holePattern"), 
	py::arg("size"),
	py::arg("type"),
	py::arg("ratioOutside"),
	py::arg("side"),
	py::arg("belowColor") = cv::Scalar(0., 0., 0., 255.),
	py::arg("belowImg") = cv::Mat(),
	py::arg("shadowBorderWidth") = 0,
	py::arg("shadowBorderIntensity") = 0.1f);
  
  //--------------

  py::enum_<dc::NoiseDegradation::AddNoiseType>(m, "AddNoiseType", "Noise add type")
    .value("AS_IS", dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS, "Added noise is independant on all channels. In particular, it may change a grayscale image in a color image")
    .value("AS_GRAY", dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY, "Added noise is the same on all channels")
    .value("AS_GRAY_IF_GRAY", dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY_IF_GRAY, "Added noise is the same on all channels only if the destination image is gray");


  m.def("addGaussianNoise",
	&dc::NoiseDegradation::addGaussianNoise, R"pbdoc(
	Add Gaussian-distributed additive noise on image.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	average : float
		Average of the Gaussian distribution.
	standardDeviation : float
		Standard deviation of the Gaussian distribution.
	addType : DocCreatorPy.AddNoiseType
		Tells whether noise may or may not change image into a
		color image.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"),
	py::arg("average") = 0.0f, 
	py::arg("standardDeviation") = 10.0f, 
	py::arg("addType") = dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS);
  
  m.def("addSpeckleNoise",
	&dc::NoiseDegradation::addSpeckleNoise, R"pbdoc(
	Add speckle/multiplicative noise on image.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	average : float
		Average of the Gaussian distribution.
	standardDeviation : float
		Standard deviation of the Gaussian distribution.
	addType : DocCreatorPy.AddNoiseType
		Tells whether noise may or may not change image into a
		color image.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"),
	py::arg("average") = 0.0f, 
	py::arg("standardDeviation") = 1.0f, 
	py::arg("addType") = dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS);
  
  m.def("addSaltAndPepperNoise",
	&dc::NoiseDegradation::addSaltAndPepperNoise, R"pbdoc(
	Add Salt and pepper noise on image.

	Replaces random pixels with 0 or 1.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	amount : float
		Amount of noise, in [0.0; 1.0].
	ratio : float
		Ratio of black pixels, in [0.0; 1.0].
		Ratio of white pixels will be 1.0-ratio.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"),
	py::arg("amount") = 0.15f, 
	py::arg("ratio") = 0.5f);

  //--------------

  m.def("phantomCharacter",
	&dc::PhantomCharacter::phantomCharacter, R"pbdoc(
	Add phantom characters to an image.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	occurenceProbability : float
		Probability of occurence of phantom characters,
		in [0.0; 1.0].
	phantomPatternsPath : str
		Directory to load phantom patterns from.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"),
	py::arg("occurenceProbability") = 0.5f, 
	py::arg("phantomPatternsPath"));

  //--------------

  
   m.def("rotateFillColor",
	&dc::RotationDegradation::rotateFillColor, R"pbdoc(
	Rotate image and fill appearing background pixels with
	uniform color.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	angle : float
		Rotation angle in degrees.
	color : tuple
		Fill color.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"), py::arg("angle"),
	py::arg("color"));

   m.def("rotateFillImage",
	&dc::RotationDegradation::rotateFillImage, R"pbdoc(
	Rotate image and fill appearing background pixels with given
	background image.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Background image may be converted to the same type than
	the input image. In particular if input image is grayscale,
	background image will be converted to gray. Background image
	may also be resized to the size of input img.

	Parameters
	----------
	img : ndarray
		Input image.
	angle : float
		Rotation angle in degrees.
	backgroundImg : ndarray
		Background image.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"),	py::arg("angle"),
	py::arg("backgroundImg"));

   m.def("rotateFillImageN",
	&dc::RotationDegradation::rotateFillImageN, R"pbdoc(
	Rotate image and fill appearing background pixels with given
	background image repeated several times.

	The background image is inserted (repeats + 1) times.
	First at angle 0, then at angle (angle/(repeast + 1)), ...

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Background image may be converted to the same type than
	the input image. In particular if input image is grayscale,
	background image will be converted to gray. Background image
	may also be resized to the size of input img.

	Parameters
	----------
	img : ndarray
		Input image.
	angle : float
		Rotation angle in degrees.
	backgroundImg : ndarray
		Background image.
	repeats : int
		Number of intermediary images between the background
		image and the foreground image.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"), py::arg("angle"),
	py::arg("backgroundImg"), py::arg("repeats"));

   
   //TODO: rotateFillImages() : how to pass std::vector<cv::Mat> ???

   py::enum_<dc::RotationDegradation::BorderReplication>(m, "RotationBorderReplication", "Rotation border replication")
     .value("REPLICATE", dc::RotationDegradation::BorderReplication::REPLICATE, "Replicate aaaaaa|abcdefgh|hhhhhhh")
     .value("REFLECT", dc::RotationDegradation::BorderReplication::REFLECT, "Reflect fedcba|abcdefgh|hgfedcb")
     .value("WRAP", dc::RotationDegradation::BorderReplication::WRAP, "Wrap cdefgh|abcdefgh|abcdefg")
     .value("REFLECT101", dc::RotationDegradation::BorderReplication::REFLECT101, "Reflect101 gfedcb|abcdefgh|gfedcba ");


   m.def("rotateFillBorder",
	&dc::RotationDegradation::rotateFillBorder, R"pbdoc(
	Rotate image and fill appearing background pixels replicating
	input image.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	angle : float
		Rotation angle in degrees.
	borderMode : DocCreatorPy.RotationBorderReplication
		mode of replication of pixels on border.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"), py::arg("angle"),
	py::arg("borderMode") = dc::RotationDegradation::BorderReplication::WRAP);
   
   m.def("rotateFillInpaint1",
	&dc::RotationDegradation::rotateFillInpaint1, R"pbdoc(
	Rotate image and fill appearing background pixels with inpainting.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	angle : float
		Rotation angle in degrees.
	inpaintingRatio : float
		Ratio of pixels of img to use for inpainting, in [0; 1].
		For a low value, only the pixels from the borders of img
		will be used.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"), py::arg("angle"),
	py::arg("inpaintingRatio") = 0.05f);
   
   m.def("rotateFillInpaint2",
	&dc::RotationDegradation::rotateFillInpaint2, R"pbdoc(
	Rotate image and fill appearing background pixels with
	inpainting.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Background image may be converted to the same type than
	the input image. In particular if input image is grayscale,
	background image will be converted to gray. Background image
	may also be resized to the size of input img.

	Parameters
	----------
	img : ndarray
		Input image.
	angle : float
		Rotation angle in degrees.
	backgroundImg : ndarray
		Background image used for inpainting.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"), py::arg("angle"),
	py::arg("backgroundImg"));
   
   m.def("rotateFillInpaint3",
	&dc::RotationDegradation::rotateFillInpaint3, R"pbdoc(
	Rotate image and fill appearing background pixels with
	inpainting.

	If backgroundImg is empty, the background of img is extracted
	and used for inpainting.
	img is rotated and copied onto rotated background with
	DocCreatorPy::copyOnto().

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Background image may be converted to the same type than
	the input image. In particular if input image is grayscale,
	background image will be converted to gray. Background image
	may also be resized to the size of input img.

	Parameters
	----------
	img : ndarray
		Input image.
	angle : float
		Rotation angle in degrees.
	backgroundImg : ndarray
		Background image used for inpainting.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"), py::arg("angle"),
	py::arg("backgroundImg"));
   
   //--------------

  py::enum_<dc::ShadowBinding::Border>(m, "ShadowBorder", "Shadow border")
    .value("TOP", dc::ShadowBinding::Border::TOP, "Shadow top border")
    .value("RIGHT", dc::ShadowBinding::Border::RIGHT, "Shadow right border")
    .value("BOTTOM", dc::ShadowBinding::Border::BOTTOM, "Shadow bottom border")
    .value("LEFT", dc::ShadowBinding::Border::LEFT, "Shadow left border");

   m.def("shadowBinding",
	&dc::ShadowBinding::shadowBinding, R"pbdoc(
	Add a shadow on a given border of image.

	angle corresponds to the angle between the page and the
	spine of the book when a page is scanned.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	border : DocCreatorPy.ShadowBorder
		Border on which the shadow is added.
	distance : int
		Size in pixels of degradation.
	intensity : float.
		Intensity, in [0.0; 1.0].
		The higher, the darker the shadow.
	angle : float.
		Angle in degrees, in [0.0; 90.0].
		The higher, the darker the shadow.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
	py::arg("img"), py::arg("border"),
	py::arg("distance"), py::arg("intensity"), py::arg("angle"));

   m.def("shadowBinding2",
	&dc::ShadowBinding::shadowBinding2, R"pbdoc(
	Add a shadow on a given border of image, proportional to image size.

	angle corresponds to the angle between the page and the
	spine of the book when a page is scanned.

	Warnings
	--------
	img must have dtype==np.uint8.
	img must have ndim==2 or (ndim==3 and shape[2]==1,3 or 4).

	Parameters
	----------
	img : ndarray
		Input image.
	distanceRatio : float
		Ratio of width or height used to compute size in pixels
		of degradation, in [0; 1].
	border : DocCreatorPy.ShadowBorder
		Border on which the shadow is added.
	intensity : float.
		Intensity, in [0.0; 1.0].
		The higher, the darker the shadow.
	angle : float.
		Angle in degrees, in [0.0; 90.0].
		The higher, the darker the shadow.

	Returns
	-------
	ndarray
		The modified image, with same shape and dtype than img.
	)pbdoc",
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
