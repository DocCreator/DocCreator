#ifndef ELASTICDEFORMATION_HPP
#define ELASTICDEFORMATION_HPP

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
       @param alpha  scaling factor, controling the intensity of the deformation.
       @param sigma  elasticity coefficiant (standard deviation of Gaussian). 
       @param borderMode  pixel interpolation method. See OpenCV cv::BorderTypes.
       @param interpolation interpolation metod. See OpenCV cv::resize(). 
       @return modified image.
     */    
    cv::Mat transform(const cv::Mat &img,
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
       @param alpha_affine scaling factor of initial affine transformation.
       @param borderMode  pixel interpolation method. See OpenCV cv::BorderTypes.
       @param interpolation interpolation metod. See OpenCV cv::resize(). 
       @return modified image.
    */
    cv::Mat transform2(const cv::Mat &img,
		       float alpha = 2.0f,
		       float sigma = 0.08f,
		       float alpha_affine = 9.0f,
		       BorderReplication borderMode = BorderReplication::BLACK,
		       Interpolation interpolation = Interpolation::BILINEAR);
    

  } //namespace ElasticDeformation
} //namespace dc


#endif /* ! ELASTICDEFORMATION_HPP */
