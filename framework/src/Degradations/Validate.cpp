#include "Validate.hpp"

#include <sstream>
#include <stdexcept>

namespace dc {

  static std::string
  describe(const cv::Mat &img)
  {
    std::ostringstream os;
    if (img.dims > 2) {
      os << img.dims << "-dimensional, ";
    }
    os << img.cols << 'x' << img.rows << ' ' << img.channels()
       << "-channel, depth " << img.depth();
    return os.str();
  }

  void
  throwIfInvalid(ImgError err,
		 const cv::Mat &img,
		 const char *argName,
		 const char *funcName)
  {
    if (err == ImgError::Ok)
      return;

    std::ostringstream os;
    os << funcName << ": " << argName << ' ';
    switch (err) {
      case ImgError::Empty:
	os << "is empty";
	break;
      case ImgError::BadDimensions:
	os << "must be a two-dimensional image";
	break;
      case ImgError::BadDepth:
	os << "must be 8 bits unsigned (CV_8U)";
	break;
      case ImgError::BadChannels:
	os << "has an unsupported number of channels";
	break;
      case ImgError::Ok:
	break;
    }
    os << " (got " << describe(img) << ')';
    throw std::invalid_argument(os.str());
  }

  void
  requireSameType(const cv::Mat &img1,
		  const cv::Mat &img2,
		  const char *argName1,
		  const char *argName2,
		  const char *funcName)
  {
    if (img1.type() == img2.type())
      return;

    std::ostringstream os;
    os << funcName << ": " << argName1 << " and " << argName2
       << " must have the same type (got " << describe(img1) << " and "
       << describe(img2) << ')';
    throw std::invalid_argument(os.str());
  }

  void
  requireSameTypeAndSize(const cv::Mat &img1,
			 const cv::Mat &img2,
			 const char *argName1,
			 const char *argName2,
			 const char *funcName)
  {
    requireSameType(img1, img2, argName1, argName2, funcName);

    if (img1.size() == img2.size())
      return;

    std::ostringstream os;
    os << funcName << ": " << argName1 << " and " << argName2
       << " must have the same size (got " << img1.cols << 'x' << img1.rows
       << " and " << img2.cols << 'x' << img2.rows << ')';
    throw std::invalid_argument(os.str());
  }

  void
  requireInRange(double value,
		 double minValue,
		 double maxValue,
		 const char *argName,
		 const char *funcName)
  {
    if (minValue <= value && value <= maxValue)
      return;

    std::ostringstream os;
    os << funcName << ": " << argName << " must be in [" << minValue << "; "
       << maxValue << "] (got " << value << ')';
    throw std::invalid_argument(os.str());
  }

  void
  requireArgument(bool condition, const char *what, const char *funcName)
  {
    if (condition)
      return;

    std::ostringstream os;
    os << funcName << ": " << what;
    throw std::invalid_argument(os.str());
  }

} //namespace dc
