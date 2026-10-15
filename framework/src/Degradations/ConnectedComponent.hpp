#ifndef CONNECTEDCOMPONENT_HPP
#define CONNECTEDCOMPONENT_HPP

#include <vector>

#include <opencv2/core/core.hpp>

#include <framework_global.h>

namespace dc {

  /** A connected component: the list of its pixels. */
  using CC = std::vector<cv::Point>;
  /** A list of connected components. */
  using CCs = std::vector<CC>;


  /** Extraction of the connected components of a binary image. */
  namespace ConnectedComponent
  {

    /**
       Extract all the connected components of a binary image.

       @a input is considered binarized: its white pixels (255) are the
       background, every other pixel belongs to a component.

       @a input must be of type CV_8UC1, and @a connectivity 4 or 8. A
       std::invalid_argument exception is thrown otherwise.

       @param input  binary image.
       @param ccs  receives the components, in the order a scan by rows
       meets them. It is cleared first.
       @param connectivity  4 or 8.
    */
    extern FRAMEWORK_EXPORT void extractAllConnectedComponents(
					      const cv::Mat &input,
					      CCs &ccs,
					      int connectivity = 8);

  } //namespace ConnectedComponent

} //namespace dc
  
#endif // CONNECTEDCOMPONENT_HPP
