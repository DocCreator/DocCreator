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

  m.doc() = R"pbdoc(
	Document image degradations.

	Every function below takes an image as a numpy array of dtype
	uint8, of shape (rows, cols) or (rows, cols, channels) with one,
	three or four channels, and gives back a degraded copy; the input
	is never modified. Anything else -- another dtype, two channels,
	fewer than two or more than three dimensions, an empty array --
	raises an exception instead of being reinterpreted.

	An image of shape (rows, cols, 1) comes back as (rows, cols): a
	single channel image has no channel axis once it has been through
	the library.

	The degradations that draw random numbers take a seed. Two calls
	with the same seed and the same parameters give the same image,
	which is what makes a generated corpus reproducible; without a
	seed, each call gives a different one.
  )pbdoc";

  //--------------

  m.def("bleedThrough", &dc::BleedThrough::bleedThrough, R"pbdoc(
	Apply a bleed-through effect of a back image onto a front image.

	The ink of the back of a page shows through its front: backImg
	diffuses into frontImg, where the two images overlap.

	frontImg and backImg must be of dtype uint8 and have the same
	number of channels, 1, 3 or 4. An exception is raised otherwise.

	backImg is not transformed. In particular, it is considered
	already mirrored.

	Parameters
	----------
	frontImg : ndarray
		Front image (recto).
	backImg : ndarray
		Back image (verso).
	numIter : int
		Number of iterations. The higher the number of iterations,
		the more the back image bleeds through the front image.
	pos : tuple
		Position (x, y) of the origin of backImg in the frame of
		frontImg. Coordinates may be negative.
	numThreads : int
		Maximum number of threads used to compute the effect. If 0
		or negative, as many threads as OpenCV uses (cf.
		cv2.setNumThreads()). The result does not depend on it.

	Returns
	-------
	ndarray
		The degraded image, of the dtype and shape of frontImg.

	)pbdoc",
	py::arg("frontImg"), py::arg("backImg"), py::arg("numIter"),
	py::arg("pos") = cv::Point(0, 0),
	py::arg("numThreads") = -1);

 //  //--------------
  
  py::enum_<dc::BlurFilter::Method>(m, "BlurMethod", "Blur filter")
    .value("GAUSSIAN", dc::BlurFilter::Method::GAUSSIAN, "Gaussian blur, cf. cv2.GaussianBlur()")
    .value("MEDIAN", dc::BlurFilter::Method::MEDIAN, "median filter, cf. cv2.medianBlur()")
    .value("NORMAL", dc::BlurFilter::Method::NORMAL, "box filter, the mean of the kernel, cf. cv2.blur()");
  
  m.def("blur", &dc::BlurFilter::blur, R"pbdoc(
	Blur an image.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.

	Parameters
	----------
	img : ndarray
		Image to blur.
	method : DocCreatorPy.BlurMethod
		Blur filter.
	kernelSize : int
		Size of the filter kernel. It must be positive, and odd for
		GAUSSIAN and MEDIAN.

	Returns
	-------
	ndarray
		The blurred image, of the dtype and shape of img.

	)pbdoc",
 	py::arg("img"), py::arg("method"), py::arg("kernelSize"));

  
 //  //TODO: other blur functions (blurArea)

 //  //--------------

  m.def("degrade3D", &dc::Distortion3D::degrade3D, R"pbdoc(
	Apply a 3D distortion to an image.

	The image is mapped as a texture onto a 3D mesh of a scanned
	document, which is rendered with a lighting, as described in:

	Kieu, Van Cuong; Journet, Nicholas; Visani, Muriel; Mullot,
	Rémy; Domenger, Jean-Philippe. Semi-synthetic Document Image
	Generation Using Texture Mapping on Scanned 3D Document Shapes.
	In: 2013 12th International Conference on Document Analysis and
	Recognition (ICDAR). IEEE, 2013, p. 489-493.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.

	The background, where the mesh does not cover the image, is
	black.

	The rendering needs OSMesa: in a module built without it, a
	warning is printed on the standard error and a copy of img
	is returned.

	Parameters
	----------
	img : ndarray
		Image to distort.
	meshFilename : str
		Mesh file, in the .obj or the .brs format. The mesh must
		have texture coordinates.
	random : bool
		Whether the lighting and the orientation of the mesh are
		random.
	seed : int, optional
		Seed of the random lighting and orientation. If None, a
		non-reproducible seed is used. Two calls with the same seed
		and the same arguments give the same result.

	Returns
	-------
	ndarray
		The distorted image, of the dtype and shape of img.

	See Also
	--------
	degrade3DN, degrade3DWithBackground

	)pbdoc",
	py::arg("img"), py::arg("meshFilename"),
	py::arg("random") = true,
	py::arg("seed") = py::none());
  
  m.def("degrade3DN", &dc::Distortion3D::degrade3DN, R"pbdoc(
	Apply numOutputImages 3D distortions to an image, and save them
	as PNG images.

	The images are saved as outputPrefix + "_" + their index, padded
	with zeros to the width of the largest one, + ".png". They have
	the dtype and shape of img, and a black background. The series
	is reproducible from one seed, and its images differ from each
	other.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.

	The rendering needs OSMesa: in a module built without it, a
	warning is printed on the standard error and no image is
	saved.

	Parameters
	----------
	img : ndarray
		Image to distort.
	numOutputImages : int
		Number of images to save.
	meshFilename : str
		Mesh file, in the .obj or the .brs format. The mesh must
		have texture coordinates.
	outputPrefix : str
		Prefix of the file names. It may contain the directory to
		save the images into.
	random : bool
		Whether the lighting and the orientation of the mesh are
		random.
	seed : int, optional
		Seed of the random lighting and orientation. If None, a
		non-reproducible seed is used. Two calls with the same seed
		and the same arguments give the same result.

	See Also
	--------
	degrade3D

	)pbdoc",
	py::arg("img"), py::arg("numOutputImages"),
	py::arg("meshFilename"),
 	py::arg("outputPrefix"), py::arg("random") = true,
	py::arg("seed") = py::none());

  m.def("degrade3DWithBackground",
	&dc::Distortion3D::degrade3DWithBackground, R"pbdoc(
	Apply a 3D distortion to an image, with another image in the
	background.

	The distortion is that of degrade3D(), as described in:

	Kieu, Van Cuong; Journet, Nicholas; Visani, Muriel; Mullot,
	Rémy; Domenger, Jean-Philippe. Semi-synthetic Document Image
	Generation Using Texture Mapping on Scanned 3D Document Shapes.
	In: 2013 12th International Conference on Document Analysis and
	Recognition (ICDAR). IEEE, 2013, p. 489-493.

	img and backgroundImg must be of dtype uint8, with ndim 2, or
	ndim 3 and shape[2] 1, 3 or 4. An exception is raised otherwise.

	The rendering needs OSMesa: in a module built without it, a
	warning is printed on the standard error and a copy of img
	is returned.

	Parameters
	----------
	img : ndarray
		Image to distort.
	meshFilename : str
		Mesh file, in the .obj or the .brs format. The mesh must
		have texture coordinates.
	backgroundImg : ndarray
		Image shown where the mesh does not cover the image.
	random : bool
		Whether the lighting and the orientation of the mesh are
		random.
	seed : int, optional
		Seed of the random lighting and orientation. If None, a
		non-reproducible seed is used. Two calls with the same seed
		and the same arguments give the same result.

	Returns
	-------
	ndarray
		The distorted image, of the size of img, and of the number
		of channels of the input image that has the most.

	)pbdoc",
	py::arg("img"), py::arg("meshFilename"),
	py::arg("backgroundImg"),
 	py::arg("random") = true,
	py::arg("seed") = py::none());
  
  m.def("degrade3DWithBackgroundN",
	&dc::Distortion3D::degrade3DWithBackgroundN, R"pbdoc(
	Apply numOutputImages 3D distortions to an image, with another
	image in the background, and save them as PNG images.

	The images are named as with degrade3DN(). They have the size
	of img, and the number of channels of the input image that has
	the most.

	img and backgroundImg must be of dtype uint8, with ndim 2, or
	ndim 3 and shape[2] 1, 3 or 4. An exception is raised otherwise.

	The rendering needs OSMesa: in a module built without it, a
	warning is printed on the standard error and no image is
	saved.

	Parameters
	----------
	img : ndarray
		Image to distort.
	numOutputImages : int
		Number of images to save.
	meshFilename : str
		Mesh file, in the .obj or the .brs format. The mesh must
		have texture coordinates.
	backgroundImg : ndarray
		Image shown where the mesh does not cover the image.
	outputPrefix : str
		Prefix of the file names. It may contain the directory to
		save the images into.
	random : bool
		Whether the lighting and the orientation of the mesh are
		random.
	seed : int, optional
		Seed of the random lighting and orientation. If None, a
		non-reproducible seed is used. Two calls with the same seed
		and the same arguments give the same result.

	)pbdoc",
	py::arg("img"), py::arg("numOutputImages"),
	py::arg("meshFilename"),
 	py::arg("backgroundImg"),
 	py::arg("outputPrefix"), py::arg("random") = true,
	py::arg("seed") = py::none());
  
 //--------------

  
 py::enum_<dc::ElasticDeformation::BorderReplication>(m, "ElasticBorderReplication", "How the pixels that come from outside the image are filled")
   .value("REPLICATE", dc::ElasticDeformation::BorderReplication::REPLICATE, "aaaaaa|abcdefgh|hhhhhhh")
   .value("REFLECT", dc::ElasticDeformation::BorderReplication::REFLECT, "fedcba|abcdefgh|hgfedcb")
   .value("WRAP", dc::ElasticDeformation::BorderReplication::WRAP, "cdefgh|abcdefgh|abcdefg")
   .value("REFLECT101", dc::ElasticDeformation::BorderReplication::REFLECT101, "gfedcb|abcdefgh|gfedcba")
   .value("BLACK", dc::ElasticDeformation::BorderReplication::BLACK, "000000|abcdefgh|000000");
  

  py::enum_<dc::ElasticDeformation::Interpolation>(m, "ElasticInterpolation", "How the deformed image is resampled")
    .value("NEAREST", dc::ElasticDeformation::Interpolation::NEAREST, "nearest neighbor")
    .value("BILINEAR", dc::ElasticDeformation::Interpolation::BILINEAR, "bilinear")
    .value("AREA", dc::ElasticDeformation::Interpolation::AREA, "resampling using pixel area relation, cf. cv2.INTER_AREA")
    .value("BICUBIC", dc::ElasticDeformation::Interpolation::BICUBIC, "bicubic, over a 4x4 neighborhood")
    .value("LANCZOS", dc::ElasticDeformation::Interpolation::LANCZOS, "Lanczos, over an 8x8 neighborhood");

  
  m.def("elasticTransform",
	&dc::ElasticDeformation::transform,  R"pbdoc(
	Apply an elastic deformation to an image.

	A random displacement field, smoothed by a Gaussian, moves every
	pixel of the image, as described in:

	Simard, Steinkraus and Platt. Best Practices for Convolutional
	Neural Networks Applied to Visual Document Analysis. In: Proc.
	of the International Conference on Document Analysis and
	Recognition, 2003.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.

	Parameters
	----------
	img : ndarray
		Image to deform.
	alpha : float
		Scaling factor, which controls the intensity of the
		deformation. It is multiplied by the smaller of the width
		and the height of img.
	sigma : float
		Elasticity coefficient: the standard deviation of the
		Gaussian that smooths the displacements, multiplied by the
		smaller of the width and the height of img. When it is
		large, the displacements become close to affine.
	borderMode : DocCreatorPy.ElasticBorderReplication
		How the pixels that come from outside the image are filled.
	interpolation : DocCreatorPy.ElasticInterpolation
		Resampling method.
	seed : int, optional
		Seed of the random generator. If None, a non-reproducible
		seed is used. Two calls with the same seed and the same
		arguments give the same result.

	Returns
	-------
	ndarray
		The deformed image, of the dtype and shape of img.

	)pbdoc",
 	py::arg("img"),
 	py::arg("alpha") = 2.0f, py::arg("sigma") = 0.08f,
 	py::arg("borderMode") = dc::ElasticDeformation::BorderReplication::BLACK,
 	py::arg("interpolation") = dc::ElasticDeformation::Interpolation::BILINEAR,
	py::arg("seed") = py::none());
  
  m.def("elasticTransform2",
	&dc::ElasticDeformation::transform2, R"pbdoc(
	Apply a random affine transformation to an image, then an
	elastic deformation.

	The elastic deformation is that of elasticTransform(), as
	described in:

	Simard, Steinkraus and Platt. Best Practices for Convolutional
	Neural Networks Applied to Visual Document Analysis. In: Proc.
	of the International Conference on Document Analysis and
	Recognition, 2003.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.

	Parameters
	----------
	img : ndarray
		Image to deform.
	alpha : float
		Scaling factor, which controls the intensity of the
		deformation. It is multiplied by the smaller of the width
		and the height of img.
	sigma : float
		Elasticity coefficient: the standard deviation of the
		Gaussian that smooths the displacements, multiplied by the
		smaller of the width and the height of img. When it is
		large, the displacements become close to affine.
	alphaAffine : float
		Scaling factor of the affine transformation.
	borderMode : DocCreatorPy.ElasticBorderReplication
		How the pixels that come from outside the image are filled.
	interpolation : DocCreatorPy.ElasticInterpolation
		Resampling method.
	seed : int, optional
		Seed of the random generator. If None, a non-reproducible
		seed is used. Two calls with the same seed and the same
		arguments give the same result.

	Returns
	-------
	ndarray
		The deformed image, of the dtype and shape of img.

	)pbdoc",
 	py::arg("img"),
	py::arg("alpha") = 2.0f,
	py::arg("sigma") = 0.08f,
	py::arg("alphaAffine") = 9.0f,
 	py::arg("borderMode") = dc::ElasticDeformation::BorderReplication::BLACK,
 	py::arg("interpolation") = dc::ElasticDeformation::Interpolation::BILINEAR,
	py::arg("seed") = py::none());

  //--------------

  py::enum_<dc::GradientDomainDegradation::InsertType>(m, "GDDInsertion", "How the stain images are inserted")
    .value("GDD_INSERT_AS_IS", dc::GradientDomainDegradation::InsertType::INSERT_AS_IS, "the stain images are inserted as they are")
    .value("GDD_INSERT_AS_GRAY", dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY, "the stain images are converted to gray before insertion")
    .value("GDD_INSERT_AS_GRAY_IF_GRAY", dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY_IF_GRAY, "the stain images are converted to gray before insertion if the image is gray");
  

  m.def("gddDegradation",
	&dc::GradientDomainDegradation::degradation, R"pbdoc(
	Insert stain images at random positions into an image.

	The stains are inserted with OpenCV seamlessClone(), as
	described in:

	Seuret, Mathias; Chen, Kai; Eichenberger, Nicole; Liwicki,
	Marcus; Ingold, Rolf. Gradient-domain degradations for improving
	historical documents images layout analysis. In: 2015 13th
	International Conference on Document Analysis and Recognition
	(ICDAR). IEEE, 2015, p. 1006-1010.

	The stain images are the image files of stainImageDir, chosen at
	random. If there is none, a warning is printed on the standard
	error and a copy of img is returned.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.

	This implementation is rather slow.

	Parameters
	----------
	img : ndarray
		Image to degrade.
	stainImageDir : str
		Directory of the stain images.
	numStainsToInsert : int
		Number of stain images to insert.
	insertType : DocCreatorPy.GDDInsertion
		Whether the stain images are converted to gray.
	doRotations : bool
		Whether the stain images are, at random, mirrored
		horizontally, vertically, or turned by 180 degrees before
		insertion.
	seed : int, optional
		Seed of the random generator. If None, a non-reproducible
		seed is used. Two calls with the same seed and the same
		arguments give the same result.

	Returns
	-------
	ndarray
		The degraded image, of the dtype and shape of img.

	See Also
	--------
	gddCopyOnto

	)pbdoc",
 	py::arg("img"),
	py::arg("stainImageDir"),
	py::arg("numStainsToInsert"),
	py::arg("insertType") = dc::GradientDomainDegradation::InsertType::INSERT_AS_GRAY_IF_GRAY,
	py::arg("doRotations") = true,
	py::arg("seed") = py::none());

  //TODO:GDD: Do not pass @a numStainsToInsert
  //but have an automatic value computed according to image size
  // or the number of connected components ?

  m.def("gddCopyOnto",
	&dc::GradientDomainDegradation::copyOnto, R"pbdoc(
	Insert one stain image into an image, with OpenCV
	seamlessClone().

	Only the part of stainImg that overlaps img is inserted; if they
	do not overlap, a copy of img is returned. stainImg is neither
	converted to gray nor turned, but the result has the dtype and
	shape of img: into a gray image, the stain is inserted in gray.
	The alpha channel of img is kept.

	img and stainImg must be of dtype uint8, with ndim 2, or ndim 3
	and shape[2] 1, 3 or 4. An exception is raised otherwise.

	Parameters
	----------
	img : ndarray
		Image to insert into.
	stainImg : ndarray
		Stain image to insert.
	posCenter : tuple
		Position (x, y), in img, of the center of stainImg.

	Returns
	-------
	ndarray
		The image with the stain, of the dtype and shape of img.

	See Also
	--------
	gddDegradation

	)pbdoc",
	py::arg("img"), py::arg("stainImg"),
	py::arg("posCenter"));

  //--------------

  m.def("characterDegradation",
	&dc::GrayscaleCharsDegradation::degradation, R"pbdoc(
	Degrade the characters of an image.

	The ink of the characters is degraded around seed points chosen
	near their contours, as described in:

	Kieu, Van Cuong; Journet, Nicholas; Visani, Muriel; Mullot,
	Rémy; Domenger, Jean-Philippe. An Efficient Parametrization of
	Character Degradation Model for Semi-synthetic Image Generation.
	In: 2nd International Workshop on Historical Document Imaging
	and Processing, 2013.

	There are three kinds of degradation spots: independent spots,
	overlapping spots and disconnection spots. The ratio of
	disconnection spots is
	1 - indepentSpotsRatio - overlappingSpotsRatio.

	The model works on gray levels: a color image is converted to
	gray, and comes back gray, in its shape. The alpha channel of a
	4 channel image is kept.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4, the two ratios in [0; 1], and their sum at most 1.
	An exception is raised otherwise.

	Parameters
	----------
	img : ndarray
		Image to degrade.
	level : int
		Scale factor, typically in [0; 10]. The number of
		degradation spots is proportional to the number of
		connected components of the image and to level: the higher
		the level, the more spots. 0 gives none.
	indepentSpotsRatio : float
		Ratio of independent spots, in [0; 1].
	overlappingSpotsRatio : float
		Ratio of overlapping spots, in [0; 1].
	seed : int, optional
		Seed of the random generator. If None, a non-reproducible
		seed is used. Two calls with the same seed and the same
		arguments give the same result.

	Returns
	-------
	ndarray
		The degraded image, of the dtype and shape of img.

	)pbdoc",
	py::arg("img"), py::arg("level"),
	py::arg("indepentSpotsRatio"),
	py::arg("overlappingSpotsRatio"),
	py::arg("seed") = py::none());

  
  //--------------

  py::enum_<dc::HoleDegradation::HoleType>(m, "HoleType", "Where a hole is on the page")
    .value("CENTER", dc::HoleDegradation::HoleType::CENTER, "anywhere in the page")
    .value("BORDER", dc::HoleDegradation::HoleType::BORDER, "on a border, cf. HoleSide")
    .value("CORNER", dc::HoleDegradation::HoleType::CORNER, "in a corner, cf. HoleSide");

  py::enum_<dc::HoleDegradation::HoleSide>(m, "HoleSide", "The border or corner where a hole is, read with the HoleType it goes with")
    .value("BORDER_TOP", dc::HoleDegradation::HoleSide::BORDER_TOP, "top border")
    .value("BORDER_RIGHT", dc::HoleDegradation::HoleSide::BORDER_RIGHT, "right border")
    .value("BORDER_BOTTOM", dc::HoleDegradation::HoleSide::BORDER_BOTTOM, "bottom border")
    .value("BORDER_LEFT", dc::HoleDegradation::HoleSide::BORDER_LEFT, "left border")
    .value("CORNER_TOPLEFT", dc::HoleDegradation::HoleSide::CORNER_TOPLEFT, "top left corner")
    .value("CORNER_TOPRIGHT", dc::HoleDegradation::HoleSide::CORNER_TOPRIGHT, "top right corner")
    .value("CORNER_BOTTOMRIGHT", dc::HoleDegradation::HoleSide::CORNER_BOTTOMRIGHT, "bottom right corner")
    .value("CORNER_BOTTOMLEFT", dc::HoleDegradation::HoleSide::CORNER_BOTTOMLEFT, "bottom left corner");

  m.def("addHole", &dc::HoleDegradation::addHole, R"pbdoc(
	Add a hole to an image, at a given position.

	img and holePattern must be of dtype uint8, with ndim 2, or ndim
	3 and shape[2] 1, 3 or 4, and belowImg, if not None, of the
	dtype and number of channels of img. An exception is raised
	otherwise.

	Parameters
	----------
	img : ndarray
		Image to add a hole to.
	holePattern : ndarray
		Pattern of the hole: its black (0) pixels are the hole. It
		may have 1, 3 or 4 channels, and is converted to gray.
	pos : tuple
		Position (x, y) of the top left corner of the pattern in
		img. The pattern may extend outside the image.
	size : int
		Number of pixels added to the width and to the height of
		the pattern, which is resized accordingly. 0 keeps its
		size.
	type : DocCreatorPy.HoleType
		Whether the hole is in the middle of the page, on a border
		or in a corner.
	side : DocCreatorPy.HoleSide
		Border or corner of the hole, for a border or a corner
		hole; not used for a center hole. Border patterns are
		drawn for the top border, and corner patterns for the top
		left corner: they are turned to side.
	belowColor : tuple
		Color of the hole where belowImg does not cover it:
		everywhere if belowImg is None.
	belowImg : ndarray, optional
		Image seen through the hole: the page below. It must be
		None, or of the dtype and number of channels of img. Where
		it does not cover the hole, belowColor is used.
	shadowBorderWidth : int
		Width, in pixels, of the shadow along the edge of the hole.
		The pixels of the hole closer than this to its edge are
		multiplied by shadowBorderIntensity. 0 adds no shadow.
	shadowBorderIntensity : float
		Factor of the shadow, in [0; 1]: the closer to 0, the
		darker the shadow (default 0.1). Values above 1
		lighten the edge instead. The alpha channel is not
		changed.

	Returns
	-------
	ndarray
		The image with the hole, of the dtype and shape of img.

	See Also
	--------
	addHoleAtRandom

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
	Add a hole to an image, at a random position.

	A center hole is anywhere in the image. A border or corner hole
	is on its border or corner, and may extend outside the image by
	up to ratioOutside of its width and height.

	img and holePattern must be of dtype uint8, with ndim 2, or ndim
	3 and shape[2] 1, 3 or 4, and belowImg, if not None, of the
	dtype and number of channels of img. An exception is raised
	otherwise.

	Parameters
	----------
	img : ndarray
		Image to add a hole to.
	holePattern : ndarray
		Pattern of the hole: its black (0) pixels are the hole. It
		may have 1, 3 or 4 channels, and is converted to gray.
	size : int
		Number of pixels added to the width and to the height of
		the pattern, which is resized accordingly. 0 keeps its
		size.
	type : DocCreatorPy.HoleType
		Whether the hole is in the middle of the page, on a border
		or in a corner.
	ratioOutside : float
		Part, in [0; 1], of the pattern that may be outside the
		image: 0.6 means 60 % of it. Not used for a center hole.
	side : DocCreatorPy.HoleSide
		Border or corner of the hole, for a border or a corner
		hole; not used for a center hole. Border patterns are
		drawn for the top border, and corner patterns for the top
		left corner: they are turned to side.
	belowColor : tuple
		Color of the hole where belowImg does not cover it:
		everywhere if belowImg is None.
	belowImg : ndarray, optional
		Image seen through the hole: the page below. It must be
		None, or of the dtype and number of channels of img. Where
		it does not cover the hole, belowColor is used.
	shadowBorderWidth : int
		Width, in pixels, of the shadow along the edge of the hole.
		The pixels of the hole closer than this to its edge are
		multiplied by shadowBorderIntensity. 0 adds no shadow.
	shadowBorderIntensity : float
		Factor of the shadow, in [0; 1]: the closer to 0, the
		darker the shadow (default 0.1). Values above 1
		lighten the edge instead. The alpha channel is not
		changed.
	seed : int, optional
		Seed of the random generator. If None, a non-reproducible
		seed is used. Two calls with the same seed and the same
		arguments give the same result.

	Returns
	-------
	ndarray
		The image with the hole, of the dtype and shape of img.

	See Also
	--------
	addHole

	)pbdoc",
	py::arg("img"), py::arg("holePattern"), 
	py::arg("size"),
	py::arg("type"),
	py::arg("ratioOutside"),
	py::arg("side"),
	py::arg("belowColor") = cv::Scalar(0., 0., 0., 255.),
	py::arg("belowImg") = cv::Mat(),
	py::arg("shadowBorderWidth") = 0,
	py::arg("shadowBorderIntensity") = 0.1f,
	py::arg("seed") = py::none());
  
  //--------------

  py::enum_<dc::NoiseDegradation::AddNoiseType>(m, "AddNoiseType", "Whether the noise added to a gray image may make it a color image")
    .value("AS_IS", dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS, "the noise is drawn independently for each channel: a gray image may become a color image")
    .value("AS_GRAY", dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY, "the same noise is added to every channel")
    .value("AS_GRAY_IF_GRAY", dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_GRAY_IF_GRAY, "the same noise is added to every channel if the image is gray, independent noise otherwise");


  m.def("addGaussianNoise",
	&dc::NoiseDegradation::addGaussianNoise, R"pbdoc(
	Add Gaussian-distributed additive noise to an image.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.
	The alpha channel of a 4 channel image is left unchanged.

	Parameters
	----------
	img : ndarray
		Image to degrade.
	average : float
		Mean of the Gaussian distribution, in gray levels.
	standardDeviation : float
		Standard deviation of the Gaussian distribution, in gray
		levels.
	addType : DocCreatorPy.AddNoiseType
		Whether the noise may make a gray image a color image.
	seed : int, optional
		Seed of the random generator. If None, a non-reproducible
		seed is used. Two calls with the same seed and the same
		arguments give the same result.

	Returns
	-------
	ndarray
		The noisy image, saturated to [0; 255], of the dtype and
		shape of img.

	)pbdoc",
	py::arg("img"),
	py::arg("average") = 0.0f, 
	py::arg("standardDeviation") = 10.0f, 
	py::arg("addType") = dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS,
	py::arg("seed") = py::none());
  
  m.def("addSpeckleNoise",
	&dc::NoiseDegradation::addSpeckleNoise, R"pbdoc(
	Add multiplicative (speckle) noise to an image: each pixel p
	becomes p + n * p, where n is drawn from a Gaussian distribution.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.
	The alpha channel of a 4 channel image is left unchanged.

	Parameters
	----------
	img : ndarray
		Image to degrade.
	average : float
		Mean of the Gaussian distribution.
	standardDeviation : float
		Standard deviation of the Gaussian distribution.
	addType : DocCreatorPy.AddNoiseType
		Whether the noise may make a gray image a color image.
	seed : int, optional
		Seed of the random generator. If None, a non-reproducible
		seed is used. Two calls with the same seed and the same
		arguments give the same result.

	Returns
	-------
	ndarray
		The noisy image, saturated to [0; 255], of the dtype and
		shape of img.

	)pbdoc",
	py::arg("img"),
	py::arg("average") = 0.0f, 
	py::arg("standardDeviation") = 1.0f, 
	py::arg("addType") = dc::NoiseDegradation::AddNoiseType::ADD_NOISE_AS_IS,
	py::arg("seed") = py::none());
  
  m.def("addSaltAndPepperNoise",
	&dc::NoiseDegradation::addSaltAndPepperNoise, R"pbdoc(
	Add salt and pepper noise to an image: random pixels become
	black (0) or white (255).

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4, and amount and ratio in [0; 1]. An exception is
	raised otherwise. The alpha channel of a 4 channel image is left
	unchanged.

	Parameters
	----------
	img : ndarray
		Image to degrade.
	amount : float
		Proportion of the pixels changed, in [0; 1].
	ratio : float
		Proportion of the changed pixels that become black, in
		[0; 1]; the others become white.
	seed : int, optional
		Seed of the random generator. If None, a non-reproducible
		seed is used. Two calls with the same seed and the same
		arguments give the same result.

	Returns
	-------
	ndarray
		The noisy image, of the dtype and shape of img.

	)pbdoc",
	py::arg("img"),
	py::arg("amount") = 0.15f, 
	py::arg("ratio") = 0.5f,
	py::arg("seed") = py::none());

  //--------------

  m.def("phantomCharacter",
	&dc::PhantomCharacter::phantomCharacter, R"pbdoc(
	Add phantom characters to an image: faint marks, taken from the
	pattern images of phantomPatternsPath, pasted next to characters,
	in the gaps between the characters of a word, as left by the ink
	of another page or of a previous impression.

	With an occurenceProbability of 0.15, phantom characters appear
	rarely; with 0.40, rather frequently; with 0.70, very
	frequently. If phantomPatternsPath holds no image file, a copy
	of img is returned.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4, and occurenceProbability in [0; 1]. An exception is
	raised otherwise.

	occurenceProbability has a default but phantomPatternsPath, which
	follows it, has none: to use the default, pass
	phantomPatternsPath by keyword.

	Parameters
	----------
	img : ndarray
		Image to degrade.
	occurenceProbability : float
		Probability that a phantom character is added at a place
		where it can be, in [0; 1].
	phantomPatternsPath : str
		Directory of the phantom patterns, which are loaded as gray
		images.
	seed : int, optional
		Seed of the random generator. If None, a non-reproducible
		seed is used. Two calls with the same seed and the same
		arguments give the same result.

	Returns
	-------
	ndarray
		The degraded image, of the dtype and shape of img.

	)pbdoc",
	py::arg("img"),
	py::arg("occurenceProbability") = 0.5f, 
	py::arg("phantomPatternsPath"),
	py::arg("seed") = py::none());

  //--------------

  
   m.def("rotateFillColor",
	&dc::RotationDegradation::rotateFillColor, R"pbdoc(
	Rotate an image, and fill the uncovered pixels with a color.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.

	Parameters
	----------
	img : ndarray
		Image to rotate.
	angle : float
		Rotation angle, in degrees. Positive values rotate
		counter-clockwise, around the center of the image.
	color : tuple
		Color of the uncovered pixels, in the order of the channels
		of img (blue, green, red and alpha).

	Returns
	-------
	ndarray
		The rotated image, of the dtype and shape of img.

	)pbdoc",
	py::arg("img"), py::arg("angle"),
	py::arg("color"));

   m.def("rotateFillImage",
	&dc::RotationDegradation::rotateFillImage, R"pbdoc(
	Rotate an image, and fill the uncovered pixels with a background
	image.

	img and backgroundImg must be of dtype uint8, with ndim 2, or
	ndim 3 and shape[2] 1, 3 or 4. An exception is raised otherwise.
	A background image is converted to the dtype and number of
	channels of img (to gray for a gray image) and resized to its
	size.

	Parameters
	----------
	img : ndarray
		Image to rotate.
	angle : float
		Rotation angle, in degrees. Positive values rotate
		counter-clockwise, around the center of the image.
	backgroundImg : ndarray
		Background image. If None, the uncovered pixels are black.

	Returns
	-------
	ndarray
		The rotated image, of the dtype and shape of img.

	)pbdoc",
	py::arg("img"),	py::arg("angle"),
	py::arg("backgroundImg"));

   m.def("rotateFillImageN",
	&dc::RotationDegradation::rotateFillImageN, R"pbdoc(
	Rotate an image, and fill the uncovered pixels with a background
	image inserted several times, as a stack of pages.

	The background image is inserted (repeats + 1) times: at angle
	0, then at angle angle / (repeats + 1), and so on, under the
	rotated image.

	img and backgroundImg must be of dtype uint8, with ndim 2, or
	ndim 3 and shape[2] 1, 3 or 4. An exception is raised otherwise.
	A background image is converted to the dtype and number of
	channels of img (to gray for a gray image) and resized to its
	size.

	Parameters
	----------
	img : ndarray
		Image to rotate.
	angle : float
		Rotation angle, in degrees. Positive values rotate
		counter-clockwise, around the center of the image.
	backgroundImg : ndarray
		Background image.
	repeats : int
		Number of intermediate images between the background image
		and the rotated image.

	Returns
	-------
	ndarray
		The rotated image, of the dtype and shape of img.

	)pbdoc",
	py::arg("img"), py::arg("angle"),
	py::arg("backgroundImg"), py::arg("repeats"));

   
   //TODO: rotateFillImages() : how to pass std::vector<cv::Mat> ???

   py::enum_<dc::RotationDegradation::BorderReplication>(m, "RotationBorderReplication", "How the uncovered pixels are filled from the image")
     .value("REPLICATE", dc::RotationDegradation::BorderReplication::REPLICATE, "aaaaaa|abcdefgh|hhhhhhh")
     .value("REFLECT", dc::RotationDegradation::BorderReplication::REFLECT, "fedcba|abcdefgh|hgfedcb")
     .value("WRAP", dc::RotationDegradation::BorderReplication::WRAP, "cdefgh|abcdefgh|abcdefg")
     .value("REFLECT101", dc::RotationDegradation::BorderReplication::REFLECT101, "gfedcb|abcdefgh|gfedcba");


   m.def("rotateFillBorder",
	&dc::RotationDegradation::rotateFillBorder, R"pbdoc(
	Rotate an image, and fill the uncovered pixels by replicating
	the image.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.

	Parameters
	----------
	img : ndarray
		Image to rotate.
	angle : float
		Rotation angle, in degrees. Positive values rotate
		counter-clockwise, around the center of the image.
	borderMode : DocCreatorPy.RotationBorderReplication
		How the image is replicated.

	Returns
	-------
	ndarray
		The rotated image, of the dtype and shape of img.

	)pbdoc",
	py::arg("img"), py::arg("angle"),
	py::arg("borderMode") = dc::RotationDegradation::BorderReplication::WRAP);
   
   m.def("rotateFillInpaint1",
	&dc::RotationDegradation::rotateFillInpaint1, R"pbdoc(
	Rotate an image, and fill the uncovered pixels by inpainting
	from the image.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.

	Parameters
	----------
	img : ndarray
		Image to rotate.
	angle : float
		Rotation angle, in degrees. Positive values rotate
		counter-clockwise, around the center of the image.
	inpaintingRatio : float
		Part of img used for the inpainting, in [0; 1], clamped to
		[0.001; 0.999]: the lower it is, the closer to its borders
		the pixels used.

	Returns
	-------
	ndarray
		The rotated image, of the dtype and shape of img.

	)pbdoc",
	py::arg("img"), py::arg("angle"),
	py::arg("inpaintingRatio") = 0.05f);
   
   m.def("rotateFillInpaint2",
	&dc::RotationDegradation::rotateFillInpaint2, R"pbdoc(
	Rotate an image, and fill the uncovered pixels by inpainting
	from a background image.

	The background image is rotated too, and the rotated image is
	copied onto it.

	img and backgroundImg must be of dtype uint8, with ndim 2, or
	ndim 3 and shape[2] 1, 3 or 4. An exception is raised otherwise.
	A background image is converted to the dtype and number of
	channels of img (to gray for a gray image) and resized to its
	size.

	Parameters
	----------
	img : ndarray
		Image to rotate.
	angle : float
		Rotation angle, in degrees. Positive values rotate
		counter-clockwise, around the center of the image.
	backgroundImg : ndarray
		Background image used for the inpainting. If None, the
		background is extracted from img.

	Returns
	-------
	ndarray
		The rotated image, of the dtype and shape of img.

	)pbdoc",
	py::arg("img"), py::arg("angle"),
	py::arg("backgroundImg"));
   
   m.def("rotateFillInpaint3",
	&dc::RotationDegradation::rotateFillInpaint3, R"pbdoc(
	Rotate an image, and fill the uncovered pixels by inpainting
	from a background image, as rotateFillInpaint2() does, but the
	rotated image is inserted into the background with
	gddCopyOnto().

	img and backgroundImg must be of dtype uint8, with ndim 2, or
	ndim 3 and shape[2] 1, 3 or 4. An exception is raised otherwise.
	A background image is converted to the dtype and number of
	channels of img (to gray for a gray image) and resized to its
	size.

	Parameters
	----------
	img : ndarray
		Image to rotate.
	angle : float
		Rotation angle, in degrees. Positive values rotate
		counter-clockwise, around the center of the image.
	backgroundImg : ndarray
		Background image used for the inpainting. If None, the
		background is extracted from img.

	Returns
	-------
	ndarray
		The rotated image, of the dtype and shape of img.

	)pbdoc",
	py::arg("img"), py::arg("angle"),
	py::arg("backgroundImg"));
   
   //--------------

  py::enum_<dc::ShadowBinding::Border>(m, "ShadowBorder", "The border of the image the shadow is on")
    .value("TOP", dc::ShadowBinding::Border::TOP, "top border")
    .value("RIGHT", dc::ShadowBinding::Border::RIGHT, "right border")
    .value("BOTTOM", dc::ShadowBinding::Border::BOTTOM, "bottom border")
    .value("LEFT", dc::ShadowBinding::Border::LEFT, "left border");

   m.def("shadowBinding",
	&dc::ShadowBinding::shadowBinding, R"pbdoc(
	Add the shadow of the binding along a border of an image, as a
	page scanned from a bound book has, after the "non-linear
	illumination model" of:

	Kanungo, Tapas; Haralick, Robert M.; Phillips, Ihsin T. Global
	and Local Document Degradation Models. In: Proc. of the
	International Conference on Document Analysis and Recognition
	(ICDAR), 1993.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.

	Parameters
	----------
	img : ndarray
		Image to degrade.
	border : DocCreatorPy.ShadowBorder
		Border on which the shadow is added.
	distance : int
		Width of the shadow, in pixels.
	intensity : float
		Intensity, in [0; 1]. The higher, the darker the shadow.
	angle : float
		Angle in degrees, in [0; 90]: the angle between the page and
		the spine of the book when the page is scanned. The higher,
		the darker the shadow.

	Returns
	-------
	ndarray
		The image with the shadow, of the dtype and shape of img.

	See Also
	--------
	shadowBinding2

	)pbdoc",
	py::arg("img"), py::arg("border"),
	py::arg("distance"), py::arg("intensity"), py::arg("angle"));

   m.def("shadowBinding2",
	&dc::ShadowBinding::shadowBinding2, R"pbdoc(
	Add the shadow of the binding along a border of an image, of a
	width relative to the image: distanceRatio times its width for
	the left and right borders, times its height for the top and
	bottom borders.

	img must be of dtype uint8, with ndim 2, or ndim 3 and shape[2]
	1, 3 or 4. An exception is raised otherwise.

	Parameters
	----------
	img : ndarray
		Image to degrade.
	distanceRatio : float
		Width of the shadow, relative to the width or the height of
		the image, in [0; 1].
	border : DocCreatorPy.ShadowBorder
		Border on which the shadow is added.
	intensity : float
		Intensity, in [0; 1]. The higher, the darker the shadow.
	angle : float
		Angle in degrees, in [0; 90]: the angle between the page and
		the spine of the book when the page is scanned. The higher,
		the darker the shadow.

	Returns
	-------
	ndarray
		The image with the shadow, of the dtype and shape of img.

	See Also
	--------
	shadowBinding

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
