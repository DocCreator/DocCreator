#ifndef ROTATIONDEGRADATION_HPP
#define ROTATIONDEGRADATION_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {
  namespace RotationDegradation {

    /**
       Rotate in image and fill appearing background pixels with uniform color.
       
       Output image will be of the same type and size than the input img.

       @param img input image to degrade
       @param angle rotation angle in degrees.
       @param color fill color (BGR).
     */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillColor(const cv::Mat &img,
						    float angle,
						    const cv::Scalar &color);

    /**
       Rotate in image and fill appearing background pixels with given background image.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
       Background image may be converted to the same type. 
       In particular if input image is grayscale, background image will be converted to gray.
       Background image may also be resized to the size of input img.
       Output image will be of the same type and size than the input img.
       
       @param img input image to degrade
       @param angle rotation angle in degrees.
       @param backgroundImage background image (rescaled if necessary).
     */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillImage(const cv::Mat &img,
						    float angle,
						    const cv::Mat &backgroundImg);


    /**
       Rotate in image and fill appearing background pixels with given background image.

       The background image is inserted (@a repeats + 1) times. First at angle 0, then at angle (@a angle/(@a repeast + 1)), ...

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
       Background image may be converted to the same type. 
       In particular if input image is grayscale, background image will be converted to gray.
       Background image may also be resized to the size of input img.
       Output image will be of the same type and size than the input img.
       
       @param img input image to degrade
       @param angle rotation angle in degrees.
       @param backgroundImage background image (rescaled if necessary).
       @param repeats number of intermediary images between the background image and the foreground image.
     */    
    extern FRAMEWORK_EXPORT cv::Mat rotateFillImage(const cv::Mat &img,
						    float angle,
						    const cv::Mat &backgroundImg,
						    int repeats);
    
    /**
       Rotate in image and fill appearing background pixels with given background images.

       The background images are inserted once each. @a backgroundImgs[0] is inserted at angle 0, then @a backgroundImgs[1] at angle (@a angle/@a backgroundImgs.size()), ...
       If @a backgroundImgs is empty, black background pixels are used.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
       Background image may be converted to the same type. 
       In particular if input image is grayscale, background image will be converted to gray.
       Background image may also be resized to the size of input img.
       Output image will be of the same type and size than the input img.
       
       @param img input image to degrade
       @param angle rotation angle in degrees.
       @param backgroundImage background image (rescaled if necessary).
     */    
    extern FRAMEWORK_EXPORT cv::Mat rotateFillImage(const cv::Mat &img,
						    float angle,
						    const std::vector<cv::Mat> &backgroundImgs);
    
    

    enum class BorderReplication {REPLICATE=0, // aaaaaa|abcdefgh|hhhhhhh
				  REFLECT, // fedcba|abcdefgh|hgfedcb
				  WRAP, // cdefgh|abcdefgh|abcdefg
				  REFLECT101}; // gfedcb|abcdefgh|gfedcba

    
    /**
       Rotate in image and fill appearing background pixels replicating input image.
       
       Output image will be of the same type and size than the input img.

       @param img input image to degrade
       @param angle rotation angle in degrees.
       @param borderMode mode of replication of pixels.
    */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillBorder(const cv::Mat &img,
						     float angle,
						     BorderReplication borderMode = BorderReplication::WRAP);
    

    /**
       Rotate in image and fill appearing background pixels with inpainting.

       @a img pixels are used for inpainting, according to @a inpaintingRatio.

       Output image will be of the same type and size than the input img.

       @param img input image to degrade
       @param angle rotation angle in degrees.
       @param inpaintingRatio ratio of pixels of @a img to use for inpainting. Must be in [0; 1]. For a low value, only the pixels from the borders of @a img will be used.
    */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillInpaint1(const cv::Mat &img,
						       float angle,
						       float inpaintingRatio = 0.05);

    
    /**
       Rotate in image and fill appearing background pixels with inpainting.

       @a backgroundImg pixels are used for inpainting.
       If @backgroundImg is empty, the background of @img is extracted and used for inpainting.

       @img is rotated and copied onto rotated background.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
       Background image may be converted to the same type. 
       In particular if input image is grayscale, background image will be converted to gray.
       Background image may also be resized to the size of input img.
       Output image will be of the same type and size than the input img.

       @param img input image to degrade
       @param angle rotation angle in degrees.
       @param backgroundImg background image used for inpainting.

    */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillInpaint2(const cv::Mat &img,
						       float angle,
						       const cv::Mat &backgroundImg = cv::Mat());

    /**
       Rotate in image and fill appearing background pixels with inpainting.

       @a backgroundImg pixels are used for inpainting.
       If @backgroundImg is empty, the background of @img is extracted and used for inpainting.

       @img is rotated and copied onto rotated background with dc::GradientDomainDegradation::copyOnto().

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
       Background image may be converted to the same type. 
       In particular if input image is grayscale, background image will be converted to gray.
       Background image may also be resized to the size of input img.
       Output image will be of the same type and size than the input img.

       @param img input image to degrade
       @param angle rotation angle in degrees.
       @param backgroundImg background image used for inpainting.

    */
    extern FRAMEWORK_EXPORT cv::Mat rotateFillInpaint3(const cv::Mat &img,
						       float angle,
						       const cv::Mat &backgroundImg = cv::Mat());

    
  } //namespace RotationDegradation
} //namespace dc


#endif /* ! ROTATIONDEGRADATION_HPP */    
