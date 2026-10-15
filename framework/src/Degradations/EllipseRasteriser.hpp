#ifndef ELLIPSERASTERISER_HPP
#define ELLIPSERASTERISER_HPP

#include <cstddef>
#include <vector>

#include <opencv2/core/core.hpp>

namespace dc {

  /**
    @internal
    The pixels of one segment, from the centre of a noise region to a point
    of its elliptical contour, with what the degradation needs to shade
    them.
  */
  struct NoiseLine
  {
    float distanceToCentre = 0.f; ///< distance from the centre of the region to the contour point
    int borderGrayLevel = 0; ///< gray level of the image at the contour point
    std::vector<cv::Point> points; ///< the pixels of the segment
  };


  /**
    @internal
    Cuts the elliptical noise region of a seed point into the segments that
    go from its centre to each point of its contour, for the character
    degradation.

    It reads the image and writes nothing but a scratch buffer of its own,
    so one instance may be reused for a whole image but not shared between
    threads.
  */
  class EllipseRasteriser
  {
  public:

    /**
      Fill the first elements of @a lines with one NoiseLine per point of the
      contour of the ellipse of centre @a centre, of semi axes @a
      semiMinorAxis and @a semiMajorAxis, rotated by @a theta degrees, and
      @return how many.

      @a lines is an in/out parameter, and the count is returned instead of
      the vector being resized, so that the point vectors of one seed point
      are reused by the next one instead of being allocated again. It only
      ever grows, to the largest contour and the longest segment met.

      @a rec receives the bounding box of the contour, and @a maxGray and
      @a minGray the extreme gray levels of @a gray at its points. Only a
      window of 2 * @a localZone pixels around the centre is looked at.
    */
    size_t rasterise(const cv::Mat &gray,
		     int localZone,
		     std::vector<NoiseLine> &lines,
		     cv::Rect &rec,
		     int &maxGray,
		     int &minGray,
		     cv::Point centre,
		     double semiMinorAxis,
		     double semiMajorAxis,
		     double theta);

  private:
    //the ellipse is drawn here, kept from one call to the next
    cv::Mat m_ellipse;
  };

} //namespace dc

#endif /* ! ELLIPSERASTERISER_HPP */
