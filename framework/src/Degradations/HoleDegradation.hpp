#ifndef HOLEDEGRADATION_HPP
#define HOLEDEGRADATION_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {
  namespace HoleDegradation {

    enum class HoleSide {BORDER_TOP=0, BORDER_RIGHT, BORDER_BOTTOM, BORDER_LEFT, CORNER_TOPLEFT=0, CORNER_TOPRIGHT, CORNER_BOTTOMRIGHT, CORNER_BOTTOMLEFT};
    enum class HoleType {CENTER=0, BORDER, CORNER, NUM_HOLE_TYPES};

    /*
      Add hole to an image at a given position.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
      Output image will be of the same type.
      @a holePattern may be of type CV_8UC1, CV_8UC3 or CV_8UC4 and will be converted to grayscale and used as a binary mask.

      pixels inside hole are filled with pixels from @a belowImg if
      @a belowImg is not empty and visible, or with color @a color otherwise.
      If not empty, @a belowImg must be of the same type than @a img.

      @a side is used only if @a type is BORDER or CORNER.
      @a holePattern for corner or border must be oriented for top left corner or top border. It will be rotated if necessary.

      @a holePattern is inserted at position (@a xOrigin, @a yOrigin).

      @param img image to add holes onto.
      @param holePattern hole pattern image. It must be of type CV_8UC1.
      @param pos position of holePattern in @a img.
      @param size size increment (in each dimension) for hole pattern image.
      @param type type of hole on pattern image (center, border or corner).
      @param side side of hole. For a center hole, this value is not used. For a border or corner hole, the hole pattern image will be transformed. Border hole images are considered to be for the top border side. Corner hole images are considered to be for the top left border side.
      @param color color used to fill the applied hole pattern image, only if @a belowImg is not empty.
      @param belowImg below image used to fill the applied hole pattern image. In this case, @a color is not used.
      @param shadowBorderWidth width in pixels of shadow added around hole border. If zero, no shadow is added.
      @param shadowBorderIntensity intensity in [0; 1] by which the original below color/image is multiplied to obtain shadow value on hole border. The closer to 0, the darker the shadow.
      @return modified image
    */
    extern FRAMEWORK_EXPORT cv::Mat addHole(const cv::Mat &img, const cv::Mat &holePattern, cv::Point pos, int size, HoleType type, HoleSide side, const cv::Scalar &color, const cv::Mat &belowImg=cv::Mat(), int shadowBorderWidth=0, float shadowBorderIntensity=0.1f);

    /*
      Get random position for hole.

      @a ratioOutside value in [0; 1.0]. Indicates ratio of @a holePattern image that can be outside image.  0.6 means 60% of pattern can be outside image. 1.0 means pattern is completely outside.

     */
    extern FRAMEWORK_EXPORT cv::Point getRandomPosition(const cv::Size &imgSize,
							const cv::Size &holePatternSize,
							HoleType type,
							float ratioOutside,
							HoleSide side);

    /*
      Add hole to an image at a random position.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4.
      Output image will be of the same type.
      @a holePattern may be of type CV_8UC1, CV_8UC3 or CV_8UC4 and will be converted to grayscale and used as a binary mask.

      @a holePattern is inserted at random position according to @a holePattern size and @a ratioOutside value.

      @param img image to add holes onto.
      @param holePattern hole pattern image. It must be of type CV_8UC1.
      @param size size increment (in each dimension) for hole pattern image.
      @param type type of hole on pattern image (center, border or corner).
      @a ratioOutside value in [0; 1.0] that indicates the ratio of @a holePattern image that can be outside image. 0.6 means 60% of pattern can be outside image, 1.0 means pattern is completely outside.
      @param side side of hole. For a center hole, this value is not used. For a border or corner hole, the hole pattern image will be transformed. Border hole images are considered to be for the top border side. Corner hole images are considered to be for the top left border side.
      @param color color used to fill the applied hole pattern image, only if @a belowImg is not empty.
      @param belowImg below image used to fill the applied hole pattern image. In this case, @a color is not used.
      @param shadowBorderWidth width in pixels of shadow added around hole border. If zero, no shadow is added.
      @param shadowBorderIntensity intensity in [0; 1] by which the original below color/image is multiplied to obtain shadow value on hole border. The closer to 0, the darker the shadow.
      @return modified image
     */
    extern FRAMEWORK_EXPORT cv::Mat addHoleAtRandom(const cv::Mat &img, const cv::Mat &holePattern, int size, HoleType type, float ratioOutside, HoleSide side, const cv::Scalar &color = cv::Scalar(0, 0, 0, 255), const cv::Mat &belowImg=cv::Mat(), int shadowBorderWidth=0, float shadowBorderIntensity=1000.f);


    
  } //namespace HoleDegradation
    
} //namespace dc
  
#endif //HOLEDEGRADATION_HPP
