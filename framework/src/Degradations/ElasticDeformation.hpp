#ifndef ELASTICDEFORMATION_HPP
#define ELASTICDEFORMATION_HPP

#include <cstdint>
#include <optional>

#include <framework_global.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace dc {

  /**
     Elastic deformation, as described in:

     Simard, Steinkraus and Platt. Best Practices for Convolutional Neural
     Networks Applied to Visual Document Analysis. In: Proc. of the
     International Conference on Document Analysis and Recognition, 2003.

     A random displacement field, smoothed by a Gaussian, moves every pixel
     of the image.
  */
  namespace ElasticDeformation {

    /** How the pixels that come from outside the image are filled. */
    enum class BorderReplication {
      REPLICATE=0, /**< aaaaaa|abcdefgh|hhhhhhh */
      REFLECT,     /**< fedcba|abcdefgh|hgfedcb */
      WRAP,        /**< cdefgh|abcdefgh|abcdefg */
      REFLECT101,  /**< gfedcb|abcdefgh|gfedcba */
      BLACK        /**< 000000|abcdefgh|000000 */
    };

    /** How the moved image is resampled. */
    enum class Interpolation {
      NEAREST=0, /**< nearest neighbor */
      BILINEAR,  /**< bilinear */
      AREA,      /**< resampling using pixel area relation, cf. cv::INTER_AREA */
      BICUBIC,   /**< bicubic, over a 4x4 neighborhood */
      LANCZOS    /**< Lanczos, over an 8x8 neighborhood */
    };

    /**
       Apply an elastic deformation to an image.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
       std::invalid_argument exception is thrown otherwise.

       @a alpha and @a sigma are multiplied by the smaller of the width and
       the height of @a img. When @a sigma is large, the displacements
       become close to affine.

       @param img  image to deform.
       @param alpha  scaling factor, which controls the intensity of the
       deformation.
       @param sigma  elasticity coefficient: the standard deviation of the
       Gaussian that smooths the displacements. It must be positive, and
       @a alpha finite: a std::invalid_argument exception is thrown
       otherwise.
       @param borderMode  how the pixels that come from outside the image
       are filled.
       @param interpolation  resampling method.
       @param seed  seed of the random generator. If not provided, a
       non-reproducible seed is used. Two calls with the same seed and the
       same arguments give the same result.
       @return the deformed image, of the type and size of @a img.
     */    
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat transform(const cv::Mat &img,
					      float alpha = 2.0f,
					      float sigma = 0.08f,
					      BorderReplication borderMode = BorderReplication::BLACK,
					      Interpolation interpolation = Interpolation::BILINEAR,
					      std::optional<uint32_t> seed = std::nullopt);

    /**
       Apply an elastic deformation to an image, as transform() does, and
       to rectangles of it.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
       std::invalid_argument exception is thrown otherwise.

       @param img  image to deform.
       @param rects  rectangles of @a img, supposed inside it.
       @param rects_pts  receives, for each rectangle of @a rects, the
       points of its outline moved by the deformation; those of the outline
       that are outside the image are left out. getRotatedRects() gives
       their bounding boxes.
       @param alpha, sigma, borderMode, interpolation, seed  as for
       transform().
       @return the deformed image, of the type and size of @a img.
    */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat transform_rects(const cv::Mat &img,
						    const std::vector<cv::Rect> &rects,
						    std::vector<std::vector<cv::Point2f> > &rects_pts,
						    float alpha = 2.0f,
						    float sigma = 0.08f,
						    BorderReplication borderMode = BorderReplication::BLACK,
						    Interpolation interpolation = Interpolation::BILINEAR,
						    std::optional<uint32_t> seed = std::nullopt);


    /**
       Apply a random affine transformation to an image, then an elastic
       deformation.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
       std::invalid_argument exception is thrown otherwise.

       @param img  image to deform.
       @param alpha, sigma, borderMode, interpolation, seed  as for
       transform().
       @param alphaAffine  scaling factor of the affine transformation.
       @return the deformed image, of the type and size of @a img.
    */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat transform2(const cv::Mat &img,
					       float alpha = 2.0f,
					       float sigma = 0.08f,
					       float alphaAffine = 9.0f,
					       BorderReplication borderMode = BorderReplication::BLACK,
					       Interpolation interpolation = Interpolation::BILINEAR,
					       std::optional<uint32_t> seed = std::nullopt);
    
    /**
       Apply a random affine transformation and an elastic deformation to an
       image, as transform2() does, and to rectangles of it.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
       std::invalid_argument exception is thrown otherwise.

       @param img  image to deform.
       @param rects, rects_pts  as for transform_rects().
       @param alpha, sigma, alphaAffine, borderMode, interpolation, seed  as
       for transform2().
       @return the deformed image, of the type and size of @a img.
    */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat transform2_rects(const cv::Mat &img,
						     const std::vector<cv::Rect> &rects,
						     std::vector<std::vector<cv::Point2f> > &rects_pts,
						     float alpha = 2.0f,
						     float sigma = 0.08f,
						     float alphaAffine = 9.0f,
						     BorderReplication borderMode = BorderReplication::BLACK,
						     Interpolation interpolation = Interpolation::BILINEAR,
						     std::optional<uint32_t> seed = std::nullopt);

    /**
       Get the oriented bounding rectangle of each list of points.

       @param rects_pts  lists of points, as transform_rects() and
       transform2_rects() give them.
       @return the smallest oriented rectangle that contains each list
       (cf. cv::minAreaRect()).
    */
    [[nodiscard]] extern FRAMEWORK_EXPORT std::vector<cv::RotatedRect> getRotatedRects(const std::vector<std::vector<cv::Point2f> > &rects_pts);


  } //namespace ElasticDeformation
} //namespace dc


#endif /* ! ELASTICDEFORMATION_HPP */
