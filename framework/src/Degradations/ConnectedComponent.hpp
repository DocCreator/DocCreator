#ifndef CONNECTEDCOMPONENT_HPP
#define CONNECTEDCOMPONENT_HPP

#include <vector>

#include <opencv2/core/core.hpp>

#include <framework_global.h>

namespace dc {

  typedef std::vector<cv::Point> CC;
  typedef std::vector<CC> CCs;


  namespace ConnectedComponent
  {

    /**
       @brief extract all connected componenents from image @a input 
       and fill @a ccs.

       @a is considered binarized. All non-white pixels may serve as a 
       region growing algorithm seed.

       @a input must be of type CV_8UC1.
       @a connectivity must be 4 or 8.
    */
    extern FRAMEWORK_EXPORT void extractAllConnectedComponents(
					      const cv::Mat &input,
					      CCs &ccs,
					      int connectivity = 8);

  } //namespace ConnectedComponent

} //namespace dc
  
#endif // CONNECTEDCOMPONENT_HPP
