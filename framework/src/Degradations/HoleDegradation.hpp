#ifndef HOLEDEGRADATION_HPP
#define HOLEDEGRADATION_HPP

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {
  namespace HoleDegradation {

    enum class Border {TOP=0, RIGHT, BOTTOM, LEFT};
    enum class Corner {TOPLEFT=0, TOPRIGHT, BOTTOMRIGHT, BOTTOMLEFT};
    enum class HoleType {CENTER=0, BORDER, CORNER, NUM_HOLE_TYPES};

    /*
      @a holePattern must be of type CV_8UC1.

      pixels inside hole are filled with pixels from @a matBelow if not empty and visible or color @a color otherwise.
      If not empty, @a matBelow must be of the same type than @a matOriginal.

      @a side is used only if @a type is BORDER or CORNER.
      @a holePattern for corner or border must be oriented for top left corner or top border. It will be rotated if necessary.

      @a holePattern is inserted at position (@a xOrigin, @a yOrigin).

    */
    extern FRAMEWORK_EXPORT cv::Mat holeDegradation(const cv::Mat &matOriginal, const cv::Mat &holePattern, int xOrigin, int yOrigin, int size, HoleType type, int side, const cv::Scalar &color, const cv::Mat &matBelow=cv::Mat(), int shadowBorderWidth=0, float shadowBorderIntensity=1000);

    /*
      Get random position for hole.

      @a ratioOutside value in [0; 1.0]. Indicates ratio of @a holePattern image that can be outside image.  0.6 means 60% of pattern can be outside image. 1.0 means pattern is completely outside.

     */
    extern FRAMEWORK_EXPORT cv::Point getRandomPosition(const cv::Size &matOriginalSize,
							const cv::Size &holePatternSize,
							HoleType type,
							float ratioOutside,
							int side);

    /*
      @a holePattern is inserted at random position.
      @a ratioOutside value in [0; 1.0]. Indicates ratio of @a holePattern image that can be outside image.  0.6 means 60% of pattern can be outside image. 1.0 means pattern is completely outside.

     */
    extern FRAMEWORK_EXPORT cv::Mat holeDegradation(const cv::Mat &matOriginal, const cv::Mat &holePattern, int size, HoleType type, float ratioOutside, int side, const cv::Scalar &color, const cv::Mat &matBelow=cv::Mat(), int shadowBorderWidth=0, float shadowBorderIntensity=1000);

    
    

    
  } //namespace HoleDegradation
    
} //namespace dc
  
#endif //HOLEDEGRADATION_HPP
