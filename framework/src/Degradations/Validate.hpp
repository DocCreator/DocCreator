#ifndef VALIDATE_HPP
#define VALIDATE_HPP

#include <framework_global.h>

#include <opencv2/core/core.hpp>

namespace dc {

  /**
    @file
    Validation of the arguments of the exported degradations.

    The preconditions documented in the headers ("@a img must be of type
    CV_8UC1, CV_8UC3 or CV_8UC4") used to be checked with assert() only.
    As the default build type is RelWithDebInfo, which defines NDEBUG, they
    were not checked at all in practice, and an image of another type was
    reinterpreted byte per byte: garbage output at best, an out-of-bounds
    access at worst (reading a CV_8UC1 image with at<cv::Vec3b>() walks three
    times past the end of each row).

    The checks below are compiled in every build type. They are meant to be
    called at the exported entry points only: inside the library, where the
    types have already been checked, assert() remains the right tool.
  */

  /**
    Why an image argument was rejected.
  */
  enum class ImgError
  {
    Ok = 0,       /**< the image is accepted */
    Empty,        /**< the image has no pixel */
    BadDimensions,/**< the image is not two-dimensional */
    BadDepth,     /**< the image is not of 8 bit unsigned values */
    BadChannels   /**< the image has an unsupported number of channels */
  };

  /**
    @param img  image to check.
    @return ImgError::Ok if @a img is a non-empty CV_8UC1, CV_8UC3 or
    CV_8UC4 image, why it is not otherwise.
  */
  [[nodiscard]] inline ImgError
  check8UC134(const cv::Mat &img) noexcept
  {
    if (img.empty())
      return ImgError::Empty;
    //B: the python layer accepts an array of any dimensionality: a 4D array
    // gives a cv::Mat whose rows and cols are both -1.
    if (img.dims != 2)
      return ImgError::BadDimensions;
    if (img.depth() != CV_8U)
      return ImgError::BadDepth;
    const int c = img.channels();
    if (c != 1 && c != 3 && c != 4)
      return ImgError::BadChannels;
    return ImgError::Ok;
  }

  /**
    @param img  image to check.
    @return ImgError::Ok if @a img is a non-empty CV_8UC1 image, why it is
    not otherwise.
  */
  [[nodiscard]] inline ImgError
  check8UC1(const cv::Mat &img) noexcept
  {
    if (img.empty())
      return ImgError::Empty;
    if (img.dims != 2)
      return ImgError::BadDimensions;
    if (img.depth() != CV_8U)
      return ImgError::BadDepth;
    if (img.channels() != 1)
      return ImgError::BadChannels;
    return ImgError::Ok;
  }

  /**
    Throw a std::invalid_argument exception describing @a img if @a err is
    not ImgError::Ok.

    pybind11 translates std::invalid_argument into a Python ValueError.

    @param err  result of a check of @a img.
    @param img  image checked.
    @param argName  name of the argument, for the message.
    @param funcName  name of the function, for the message.
    @throw std::invalid_argument if @a err is not ImgError::Ok.
  */
  extern FRAMEWORK_EXPORT void
  throwIfInvalid(ImgError err,
		 const cv::Mat &img,
		 const char *argName,
		 const char *funcName);

  /**
    Check that @a img is a non-empty CV_8UC1, CV_8UC3 or CV_8UC4 image.

    @param img  image to check.
    @param argName  name of the argument, for the message.
    @param funcName  name of the function, for the message.

    @throw std::invalid_argument if it is not.
  */
  inline void
  require8UC134(const cv::Mat &img, const char *argName, const char *funcName)
  {
    throwIfInvalid(check8UC134(img), img, argName, funcName);
  }

  /**
    Check that @a img is a non-empty CV_8UC1 image.

    @param img  image to check.
    @param argName  name of the argument, for the message.
    @param funcName  name of the function, for the message.

    @throw std::invalid_argument if it is not.
  */
  inline void
  require8UC1(const cv::Mat &img, const char *argName, const char *funcName)
  {
    throwIfInvalid(check8UC1(img), img, argName, funcName);
  }

  /**
    Check that @a img1 and @a img2 have the same type.

    @param img1, img2  images to check.
    @param argName1, argName2  names of the arguments, for the message.
    @param funcName  name of the function, for the message.

    @throw std::invalid_argument if they do not.
  */
  extern FRAMEWORK_EXPORT void
  requireSameType(const cv::Mat &img1,
		  const cv::Mat &img2,
		  const char *argName1,
		  const char *argName2,
		  const char *funcName);

  /**
    Check that @a img1 and @a img2 have the same type and the same size.

    @param img1, img2  images to check.
    @param argName1, argName2  names of the arguments, for the message.
    @param funcName  name of the function, for the message.

    @throw std::invalid_argument if they do not.
  */
  extern FRAMEWORK_EXPORT void
  requireSameTypeAndSize(const cv::Mat &img1,
			 const cv::Mat &img2,
			 const char *argName1,
			 const char *argName2,
			 const char *funcName);

  /**
    Check that @a value is in [@a minValue; @a maxValue].

    @param value  value to check.
    @param minValue, maxValue  bounds of the range, included.
    @param argName  name of the argument, for the message.
    @param funcName  name of the function, for the message.

    @throw std::invalid_argument if it is not.
  */
  extern FRAMEWORK_EXPORT void
  requireInRange(double value,
		 double minValue,
		 double maxValue,
		 const char *argName,
		 const char *funcName);

  /**
    Check an arbitrary precondition on the arguments.

    @param condition  the precondition.
    @param what  what is expected, for example "kernelSize must be positive
    and odd", for the message.
    @param funcName  name of the function, for the message.

    @throw std::invalid_argument if @a condition is false.
  */
  extern FRAMEWORK_EXPORT void
  requireArgument(bool condition, const char *what, const char *funcName);

} //namespace dc

#endif /* ! VALIDATE_HPP */
