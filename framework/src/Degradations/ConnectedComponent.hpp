#ifndef CONNECTEDCOMPONENT_HPP
#define CONNECTEDCOMPONENT_HPP

#include <vector>

#include <opencv2/core/core.hpp>

#include <framework_global.h>

namespace dc {

  typedef std::vector<cv::Point> CC;
  typedef std::vector<CC> CCs;


  class FRAMEWORK_EXPORT ConnectedComponent
  {
  public:

    /**
       @brief extract all connected componenents from image @a input 
       and fill @a ccs.

       @a connectivity must be 4 or 8.
    */
    static void extractAllConnectedComponents(const cv::Mat &input,
					      CCs &ccs,
					      int connectivity = 8);

  private:

    static void extractConnectedComponent(cv::Mat &input,
					  const cv::Point &seed,
					  CC &cc,
					  int connectivity);
  };

} //namespace dc
  
#endif // CONNECTEDCOMPONENT_HPP
