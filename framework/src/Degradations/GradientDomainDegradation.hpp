#ifndef GRADIENTDOMAINDEGRADATION_HPP
#define GRADIENTDOMAINDEGRADATION_HPP

#include <cstdint>
#include <optional>

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  /**
     Gradient-domain degradation: stain images inserted into a document
     image with cv::seamlessClone(), as described in:

     Seuret, Mathias; Chen, Kai; Eichenberger, Nicole; Liwicki, Marcus;
     Ingold, Rolf. Gradient-domain degradations for improving historical
     documents images layout analysis. In: 2015 13th International
     Conference on Document Analysis and Recognition (ICDAR). IEEE, 2015,
     p. 1006-1010.
  */
  namespace GradientDomainDegradation {

    /**
       How the stain images are inserted: in color, or in gray, for example
       into a gray document image.
    */
    enum class InsertType {
      INSERT_AS_IS=0,        /**< the stain images are inserted as they are */
      INSERT_AS_GRAY,        /**< the stain images are converted to gray before insertion */
      INSERT_AS_GRAY_IF_GRAY /**< the stain images are converted to gray before insertion if the image is gray */
    };

    /**
       Insert stain images at random positions into an image.

       The stain images are the image files of @a stainImageDir (cf.
       dc::listImageFiles()), chosen at random. If there is none, a warning
       is printed on the standard error and a copy of @a img is returned.

       @warning This implementation is rather slow.

       @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
       std::invalid_argument exception is thrown otherwise.

       @param img  image to degrade.
       @param stainImageDir  directory of the stain images.
       @param numStainsToInsert  number of stain images to insert.
       @param insertType  whether the stain images are converted to gray.
       @param doRotations  whether the stain images are, at random, mirrored
       horizontally, vertically, or turned by 180 degrees before insertion.
       @param seed  seed of the random generator. If not provided, a
       non-reproducible seed is used. Two calls with the same seed and the
       same arguments give the same result.
       @return the degraded image, of the type and size of @a img.
    */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat degradation(const cv::Mat &img,
						const std::string &stainImageDir,
						size_t numStainsToInsert = 20,
						InsertType insertType = InsertType::INSERT_AS_GRAY_IF_GRAY,
						bool doRotations = true,
						std::optional<uint32_t> seed = std::nullopt);

    /**
      Insert one stain image into an image, with cv::seamlessClone().

      Only the part of @a stainImg that overlaps @a img is inserted; if they
      do not overlap, or if that part is less than 5 pixels wide or high,
      which cv::seamlessClone() cannot insert, a copy of @a img is
      returned. @a stainImg is neither
      converted to gray nor turned, but the result has the type of @a img:
      into a gray image, the stain is inserted in gray. The alpha channel of
      @a img is kept.

      @a img and @a stainImg must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
      std::invalid_argument exception is thrown otherwise.

      @param img  image to insert into.
      @param stainImg  stain image to insert.
      @param posCenter  position, in @a img, of the center of @a stainImg.
      @return the image with the stain, of the type and size of @a img.
    */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat copyOnto(const cv::Mat &img,
					     const cv::Mat &stainImg,
					     const cv::Point &posCenter);

    
    
  } //namespace GradientDomainDegradation
} //namespace dc


#endif /* ! GRADIENTDOMAINDEGRADATION_HPP */
