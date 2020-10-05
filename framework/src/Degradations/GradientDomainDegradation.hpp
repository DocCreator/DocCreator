#ifndef GRADIENTDOMAINDEGRADATION_HPP
#define GRADIENTDOMAINDEGRADATION_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {
  namespace GradientDomainDegradation {

    /**
       Tells how the stain image must be inserted.
       Stain images, may be transformed in gray if the destination image is gray for example.
    */
    enum class InsertType {INSERT_AS_IS=0, /** stain image is not transformed before insertion */
			    INSERT_AS_GRAY, /** stain image is transformed to gray before insertion */
			    INSERT_AS_GRAY_IF_GRAY /** stain image is transformed to gray before insertion only if the destination image is gray */
    };

    /**
       Implements gradient-domain degradation from [1] using cv::seamlessClone().
       Several stain images are copied at random positions onto input image.

       [1]
       SEURET, Mathias, CHEN, Kai, EICHENBERGERY, Nicole, LIWICKI, Markus, INGOLD, Rolf. 
       Gradient-domain degradations for improving historical documents images layout analysis. 
       In : 2015 13th International Conference on Document Analysis and Recognition (ICDAR). IEEE, 2015. p. 1006-1010. 

       @warning This implementation is rather slow.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
       Output image will be of the same type and size than the input img.

       @param img input image to degrade.
       @param stainImageDir directory to load stain images from.
       @param numStainsToInsert number of stain images to insert. 
       @param insertType tells whether stain images must be converted to gray.
       @param doRotations indicates whether stain images are rotated before insertion.
       @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat degradation(const cv::Mat &img,
						const std::string &stainImageDir,
						size_t numStainsToInsert = 20,
						InsertType insertType = InsertType::INSERT_AS_GRAY_IF_GRAY,
						bool doRotations = true);

    /*
      Copy stain image @a stainImg onto destination image @a dstImg at position @a pos.
      
      @a img and @stainImg must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
      Output image will be of the same type and size than the input img.

      If there is no overlap between images, with given position, there is no copy.
      Otherwise, only the overlappong part of @a stainImg is copied.

      @stainImg is not converted to gray, even if @a dstImg is a grayscale image.
      @stainImg is not rotated before insertion.
      
      @param dstImg destination image to copy onto.
      @param stainImg stain image to insert.
      @param posCenter position of center of @a stainImg on destination image @a dstImg.
      @return modified image.
    */
    extern FRAMEWORK_EXPORT cv::Mat copyOnto(const cv::Mat &dstImg,
					     const cv::Mat &stainImg,
					     const cv::Point &posCenter);

    
    
  } //namespace GradientDomainDegradation
} //namespace dc


#endif /* ! GRADIENTDOMAINDEGRADATION_HPP */
