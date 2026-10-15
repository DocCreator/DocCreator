#ifndef HOLEDEGRADATION_HPP
#define HOLEDEGRADATION_HPP

#include <cstdint>
#include <optional>

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  /**
     Holes: parts of the page missing, in its middle, on a border or in a
     corner, through which the page below or a color is seen.
  */
  namespace HoleDegradation {

    /**
       The border or corner where a hole is.

       The border and the corner values overlap (BORDER_TOP and
       CORNER_TOPLEFT are both 0): a value is to be read with the HoleType it
       goes with.
    */
    enum class HoleSide {
      BORDER_TOP=0,       /**< top border */
      BORDER_RIGHT,       /**< right border */
      BORDER_BOTTOM,      /**< bottom border */
      BORDER_LEFT,        /**< left border */
      CORNER_TOPLEFT=0,   /**< top left corner */
      CORNER_TOPRIGHT,    /**< top right corner */
      CORNER_BOTTOMRIGHT, /**< bottom right corner */
      CORNER_BOTTOMLEFT   /**< bottom left corner */
    };

    /** Where a hole is on the page. */
    enum class HoleType {
      CENTER=0,      /**< anywhere in the page */
      BORDER,        /**< on a border, cf. HoleSide */
      CORNER,        /**< in a corner, cf. HoleSide */
      NUM_HOLE_TYPES /**< the number of hole types, not a type */
    };

    /**
      Add a hole to an image, at a given position.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4, @a holePattern
      too, and @a belowImg, if not empty, of the type of @a img. A
      std::invalid_argument exception is thrown otherwise.

      @param img  image to add a hole to.
      @param holePattern  pattern of the hole: its black (0) pixels are the
      hole. It may be of type CV_8UC1, CV_8UC3 or CV_8UC4, and is converted
      to gray.
      @param size  number of pixels added to the width and to the height of
      the pattern, which is resized accordingly. 0 keeps its size.
      @param type  whether the hole is in the middle of the page, on a
      border or in a corner.
      @param pos  position of the top left corner of the pattern in
      @a img. The pattern may extend outside the image.
      @param side  border or corner of the hole, for a border or a corner
      hole; not used for a center hole. Border patterns are drawn for the top
      border, and corner patterns for the top left corner: they are turned
      to @a side.
      @param color  color of the hole where @a belowImg does not cover it:
      everywhere if @a belowImg is empty.
      @param belowImg  image seen through the hole: the page below. It must
      be empty, or of the type of @a img. Where it does not cover the hole,
      @a color is used.
      @param shadowBorderWidth  width, in pixels, of the shadow along the
      edge of the hole. The pixels of the hole closer than this to its edge
      are multiplied by @a shadowBorderIntensity. 0 adds no shadow.
      @param shadowBorderIntensity  factor of the shadow, in [0; 1]: the
      closer to 0, the darker the shadow. Values above 1 lighten the edge
      instead; the result is saturated to [0; 255]. The alpha channel is
      not changed.
      @return the image with the hole, of the type and size of @a img.
    */
    extern FRAMEWORK_EXPORT cv::Mat addHole(const cv::Mat &img, const cv::Mat &holePattern, cv::Point pos, int size, HoleType type, HoleSide side, const cv::Scalar &color, const cv::Mat &belowImg=cv::Mat(), int shadowBorderWidth=0, float shadowBorderIntensity=0.1f);

    /**
      Draw a random position for a hole: a position of the top left corner
      of its pattern, as addHole() takes it.

      A center hole is anywhere in the image. A border or corner hole is on
      its border or corner, and may extend outside the image by up to
      @a ratioOutside of its width and height. If the image is too small for
      the pattern, the center of the image is returned.

      @param imgSize  size of the image.
      @param holePatternSize  size of the hole pattern, as drawn: for the top
      border, or for the top left corner.
      @param type  whether the hole is in the middle of the page, on a
      border or in a corner.
      @param ratioOutside  part, in [0; 1], of the pattern that may be
      outside the image: 0.6 means 60 % of it. Not used for a center hole.
      @param side  border or corner of the hole. Not used for a center hole.
      @param seed  seed of the random generator. If not provided, a
      non-reproducible seed is used. Two calls with the same seed and the
      same arguments give the same result.
      @return the position of the top left corner of the pattern.
     */
    extern FRAMEWORK_EXPORT cv::Point getRandomPosition(const cv::Size &imgSize,
							const cv::Size &holePatternSize,
							HoleType type,
							float ratioOutside,
							HoleSide side,
							std::optional<uint32_t> seed = std::nullopt);

    /**
      Add a hole to an image, at a random position.

      The position is drawn by getRandomPosition(), then the hole is added
      by addHole().

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4, @a holePattern
      too, and @a belowImg, if not empty, of the type of @a img. A
      std::invalid_argument exception is thrown otherwise.

      @param img  image to add a hole to.
      @param holePattern  pattern of the hole: its black (0) pixels are the
      hole. It may be of type CV_8UC1, CV_8UC3 or CV_8UC4, and is converted
      to gray.
      @param size  number of pixels added to the width and to the height of
      the pattern, which is resized accordingly. 0 keeps its size.
      @param type  whether the hole is in the middle of the page, on a
      border or in a corner.
      @param ratioOutside  part, in [0; 1], of the pattern that may be
      outside the image. Not used for a center hole.
      @param side  border or corner of the hole, for a border or a corner
      hole; not used for a center hole. Border patterns are drawn for the top
      border, and corner patterns for the top left corner: they are turned
      to @a side.
      @param color  color of the hole where @a belowImg does not cover it:
      everywhere if @a belowImg is empty.
      @param belowImg  image seen through the hole: the page below. It must
      be empty, or of the type of @a img. Where it does not cover the hole,
      @a color is used.
      @param shadowBorderWidth  width, in pixels, of the shadow along the
      edge of the hole. The pixels of the hole closer than this to its edge
      are multiplied by @a shadowBorderIntensity. 0 adds no shadow.
      @param shadowBorderIntensity  factor of the shadow, in [0; 1]: the
      closer to 0, the darker the shadow. Values above 1 lighten the edge
      instead; the result is saturated to [0; 255]. The alpha channel is
      not changed.
      @param seed  seed of the random generator. If not provided, a
      non-reproducible seed is used. Two calls with the same seed and the
      same arguments give the same result.
      @return the image with the hole, of the type and size of @a img.
     */
    extern FRAMEWORK_EXPORT cv::Mat addHoleAtRandom(const cv::Mat &img, const cv::Mat &holePattern, int size, HoleType type, float ratioOutside, HoleSide side, const cv::Scalar &color = cv::Scalar(0, 0, 0, 255), const cv::Mat &belowImg=cv::Mat(), int shadowBorderWidth=0, float shadowBorderIntensity=1000.f, std::optional<uint32_t> seed = std::nullopt);


    
  } //namespace HoleDegradation
    
} //namespace dc
  
#endif //HOLEDEGRADATION_HPP
