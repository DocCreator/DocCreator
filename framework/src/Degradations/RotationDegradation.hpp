#ifndef ROTATIONDEGRADATION_HPP
#define ROTATIONDEGRADATION_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  /**
     Rotation of an image, and the ways to fill the corners the rotation
     uncovers: a color, background images, the border of the image
     replicated, or inpainting.
  */
  namespace RotationDegradation {

    /**
       Rotate an image, and fill the uncovered pixels with a color.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
       std::invalid_argument exception is thrown otherwise.

       @param img  image to rotate.
       @param angle  rotation angle, in degrees. Positive values rotate
       counter-clockwise, around the center of the image.
       @param color  color of the uncovered pixels, in the order of the
       channels of @a img (blue, green, red and alpha).
       @return the rotated image, of the type and size of @a img.
     */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillColor(const cv::Mat &img,
						    float angle,
						    const cv::Scalar &color);

    /**
       Rotate an image, and fill the uncovered pixels with a background
       image.

       If @a backgroundImg is empty, the uncovered pixels are black.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4, and the
       background images too. A std::invalid_argument exception is thrown
       otherwise. A background image is converted to the type of @a img (to
       gray for a gray image) and resized to its size.

       @param img  image to rotate.
       @param angle  rotation angle, in degrees. Positive values rotate
       counter-clockwise, around the center of the image.
       @param backgroundImg  background image.
       @return the rotated image, of the type and size of @a img.
     */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillImage(const cv::Mat &img,
						    float angle,
						    const cv::Mat &backgroundImg);

    /**
       Rotate an image, and fill the uncovered pixels with a background
       image inserted several times, as a stack of pages.

       The background image is inserted (@a repeats + 1) times: at angle 0,
       then at angle @a angle / (@a repeats + 1), and so on, under the
       rotated image.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4, and the
       background images too. A std::invalid_argument exception is thrown
       otherwise. A background image is converted to the type of @a img (to
       gray for a gray image) and resized to its size.

       @param img  image to rotate.
       @param angle  rotation angle, in degrees. Positive values rotate
       counter-clockwise, around the center of the image.
       @param backgroundImg  background image.
       @param repeats  number of intermediate images between the background
       image and the rotated image.
       @return the rotated image, of the type and size of @a img.
     */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillImageN(const cv::Mat &img,
						     float angle,
						     const cv::Mat &backgroundImg,
						     int repeats);
    
    /**
       Rotate an image, and fill the uncovered pixels with several
       background images, as a stack of pages.

       Each background image is inserted once: @a backgroundImgs[0] at angle
       0, then @a backgroundImgs[1] at angle @a angle /
       @a backgroundImgs.size(), and so on. If @a backgroundImgs is empty,
       the uncovered pixels are black.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4, and the
       background images too. A std::invalid_argument exception is thrown
       otherwise. A background image is converted to the type of @a img (to
       gray for a gray image) and resized to its size.

       @param img  image to rotate.
       @param angle  rotation angle, in degrees. Positive values rotate
       counter-clockwise, around the center of the image.
       @param backgroundImgs  background images.
       @return the rotated image, of the type and size of @a img.
     */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillImages(const cv::Mat &img,
						     float angle,
						     const std::vector<cv::Mat> &backgroundImgs);
    
    

    /** How the uncovered pixels are filled from the image. */
    enum class BorderReplication {
      REPLICATE=0, /**< aaaaaa|abcdefgh|hhhhhhh */
      REFLECT,     /**< fedcba|abcdefgh|hgfedcb */
      WRAP,        /**< cdefgh|abcdefgh|abcdefg */
      REFLECT101   /**< gfedcb|abcdefgh|gfedcba */
    };

    
    /**
       Rotate an image, and fill the uncovered pixels by replicating the
       image.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
       std::invalid_argument exception is thrown otherwise.

       @param img  image to rotate.
       @param angle  rotation angle, in degrees. Positive values rotate
       counter-clockwise, around the center of the image.
       @param borderMode  how the image is replicated.
       @return the rotated image, of the type and size of @a img.
    */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillBorder(const cv::Mat &img,
						     float angle,
						     BorderReplication borderMode = BorderReplication::WRAP);
    

    /**
       Rotate an image, and fill the uncovered pixels by inpainting from
       the image.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
       std::invalid_argument exception is thrown otherwise.

       @param img  image to rotate.
       @param angle  rotation angle, in degrees. Positive values rotate
       counter-clockwise, around the center of the image.
       @param inpaintingRatio  part of @a img used for the inpainting, in
       [0; 1], clamped to [0.001; 0.999]: the lower it is, the closer to its
       borders the pixels used.
       @return the rotated image, of the type and size of @a img.
    */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillInpaint1(const cv::Mat &img,
						       float angle,
						       float inpaintingRatio = 0.05f);

    
    /**
       Rotate an image, and fill the uncovered pixels by inpainting from a
       background image.

       The background image is rotated too, and the rotated image is copied
       onto it. If @a backgroundImg is empty, the background is extracted
       from @a img.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4, and the
       background images too. A std::invalid_argument exception is thrown
       otherwise. A background image is converted to the type of @a img (to
       gray for a gray image) and resized to its size.

       @param img  image to rotate.
       @param angle  rotation angle, in degrees. Positive values rotate
       counter-clockwise, around the center of the image.
       @param backgroundImg  background image used for the inpainting.
       @return the rotated image, of the type and size of @a img.
    */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillInpaint2(const cv::Mat &img,
						       float angle,
						       const cv::Mat &backgroundImg = cv::Mat());

    /**
       Rotate an image, and fill the uncovered pixels by inpainting from a
       background image, as rotateFillInpaint2() does, but the rotated image
       is inserted into the background with
       dc::GradientDomainDegradation::copyOnto().

       If @a backgroundImg is empty, the background is extracted from
       @a img.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4, and the
       background images too. A std::invalid_argument exception is thrown
       otherwise. A background image is converted to the type of @a img (to
       gray for a gray image) and resized to its size.

       @param img  image to rotate.
       @param angle  rotation angle, in degrees. Positive values rotate
       counter-clockwise, around the center of the image.
       @param backgroundImg  background image used for the inpainting.
       @return the rotated image, of the type and size of @a img.
    */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillInpaint3(const cv::Mat &img,
						       float angle,
						       const cv::Mat &backgroundImg = cv::Mat());

    
  } //namespace RotationDegradation
} //namespace dc


#endif /* ! ROTATIONDEGRADATION_HPP */    
