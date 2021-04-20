#ifndef ELASTICDEFORMATION_HPP
#define ELASTICDEFORMATION_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace dc {
  namespace ElasticDeformation {

    enum class BorderReplication {REPLICATE=0, // aaaaaa|abcdefgh|hhhhhhh
				  REFLECT, // fedcba|abcdefgh|hgfedcb
				  WRAP, // cdefgh|abcdefgh|abcdefg
				  REFLECT101, // gfedcb|abcdefgh|gfedcba
				  BLACK // 000000|abcdefgh|000000
    };

    enum class Interpolation {NEAREST=0, //nearest-neighbour
			      BILINEAR, //bilinear
			      AREA, //
			      BICUBIC, //bicubic over 4x4 pixel neighborhood
			      LANCZOS //Lanczos over 8x8 pixel neighborhood
    };

    /**
       Apply elastic deformation to @a img as described in [Simard2003].

       [Simard2003] Simard, Steinkraus and Platt, "Best Practices for
       Convolutional Neural Networks applied to Visual Document Analysis", in
       Proc. of the International Conference on Document Analysis and
       Recognition, 2003.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
       Output image will be of the same type and size than the input img.

       @a sigma and @a alpha are multiplied with min(width, height) of @a img.

       If @a sigma is large the displacements becomes close to affine.

       @param img input image.
       @param alpha  scaling factor, controlling the intensity of the deformation.
       @param sigma  elasticity coefficiant (standard deviation of Gaussian).
       @param borderMode  border pixel interpolation method. See OpenCV cv::BorderTypes.
       @param interpolation interpolation method. See OpenCV cv::resize().
       @return modified image.
     */    
    extern FRAMEWORK_EXPORT cv::Mat transform(const cv::Mat &img,
					      float alpha = 2.0f,
					      float sigma = 0.08f,
					      BorderReplication borderMode = BorderReplication::BLACK,
					      Interpolation interpolation = Interpolation::BILINEAR);

    /**
       Apply elastic deformation to @a img as described in [Simard2003].

       Same as @see transform, but also transform @a rects and put tansformed points in @a rects_pts.
       @a rects are supposed to be inside the image frame.

       @param img input image.
       @param rects input rectangles.
       @param rects_pts output points of transformed rectangles. @see getRotatedRects.
       @param alpha  scaling factor, controling the intensity of the deformation.
       @param sigma  elasticity coefficiant (standard deviation of Gaussian).
       @param borderMode  border pixel interpolation method. See OpenCV cv::BorderTypes.
       @param interpolation interpolation method. See OpenCV cv::resize().
       @return modified image.

    */
    extern FRAMEWORK_EXPORT cv::Mat transform_rects(const cv::Mat &img,
						    const std::vector<cv::Rect> &rects,
						    std::vector<std::vector<cv::Point2f> > &rects_pts,
						    float alpha = 2.0f,
						    float sigma = 0.08f,
						    BorderReplication borderMode = BorderReplication::BLACK,
						    Interpolation interpolation = Interpolation::BILINEAR);


    /**
       Apply elastic deformation to @a img as described in [Simard2003],
       after a first affine transformation.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
       Output image will be of the same type and size than the input img.

       @param img input image.
       @param alpha  scaling factor, controling the intensity of the deformation.
       @param sigma  elasticity coefficiant (standard deviation of Gaussian). 
       @param alphaAffine scaling factor of initial affine transformation.
       @param borderMode  border pixel interpolation method. See OpenCV cv::BorderTypes.
       @param interpolation interpolation method. See OpenCV cv::resize().
       @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat transform2(const cv::Mat &img,
					       float alpha = 2.0f,
					       float sigma = 0.08f,
					       float alphaAffine = 9.0f,
					       BorderReplication borderMode = BorderReplication::BLACK,
					       Interpolation interpolation = Interpolation::BILINEAR);
    
    /**
       Apply elastic deformation to @a img as described in [Simard2003],
       after a first affine transformation.

       Same as @see transform2, but also transform @a rects and put tansformed points in @a rects_pts.
       @a rects are supposed to be inside the image frame.

       @param img input image.
       @param rects input rectangles.
       @param rects_pts output points of transformed rectangles. @see getRotatedRects.
       @param alpha  scaling factor, controling the intensity of the deformation.
       @param sigma  elasticity coefficiant (standard deviation of Gaussian).
       @param alphaAffine scaling factor of initial affine transformation.
       @param borderMode  border pixel interpolation method. See OpenCV cv::BorderTypes.
       @param interpolation interpolation method. See OpenCV cv::resize().
       @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat transform2_rects(const cv::Mat &img,
						     const std::vector<cv::Rect> &rects,
						     std::vector<std::vector<cv::Point2f> > &rects_pts,
						     float alpha = 2.0f,
						     float sigma = 0.08f,
						     float alphaAffine = 9.0f,
						     BorderReplication borderMode = BorderReplication::BLACK,
						     Interpolation interpolation = Interpolation::BILINEAR);

    /**
       Get oriented bounding rectangles from rectangles points.

       @param rects_pts input points of transformed rectangles. @see transform
       @return oriented bounding rectangles.
    */
    extern FRAMEWORK_EXPORT std::vector<cv::RotatedRect> getRotatedRects(const std::vector<std::vector<cv::Point2f> > &rects_pts);


  } //namespace ElasticDeformation
} //namespace dc


#endif /* ! ELASTICDEFORMATION_HPP */
