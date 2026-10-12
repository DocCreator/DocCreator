#include "GradientDomainDegradation.hpp"

#include <cstdlib>
#include <iostream>
#include <random>

//#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/photo/photo.hpp> 
#include <opencv2/highgui/highgui.hpp>

#include "FileUtils.hpp"
#include "ColorUtils.hpp" //dc::isGray()
#include "Random.hpp"
#include "Validate.hpp"

namespace dc {
  namespace GradientDomainDegradation {

/*
  draw @a numToDraw indices in the range [rangeMin, rangeMax[
 */
static std::vector<size_t>
drawIndices(RandomEngine &rng, size_t numToDraw, size_t rangeMin, size_t rangeMax)
{
  std::vector<size_t> indices;
  assert(rangeMin < rangeMax);

  const size_t reserve = ((numToDraw + (rangeMax-rangeMin - 1)) / (rangeMax-rangeMin))*(rangeMax-rangeMin);
  indices.reserve(reserve);
  while (indices.size() != reserve) {
    for (size_t i=rangeMin; i<rangeMax; ++i) {
      indices.push_back(i);
    }
  }
  assert(indices.size() >= numToDraw);
    
  std::shuffle(indices.begin(), indices.end(), RandomBitGenerator(rng));

  indices.resize(numToDraw);

#ifndef NDEBUG
  for (size_t i=0; i<indices.size(); ++i) {
    if (! (indices[i]>= rangeMin && indices[i]< rangeMax))
      std::cerr<<"ERROR: not in range ["<<rangeMin<<"; "<<rangeMax<<"[\n";
  }
#endif //NDEBUG

  assert(indices.size() == numToDraw);
  
  return indices;
}


/*
  compute crop of @a stainImg copied onto @a dstImg at position @a pos.
  If there is intersection between @a stainImg and @a dstImg, update @a roi and @a newPos.
  
  @param[in] stainImg stain image to insert. It must be of type CV_8UC3.
  @param[in] dstImg destination image to copy onto. It must be of type 8UC3.
  @param[in] posCenter position of center of @a stainImg on destination image @a dstImg.

  @param[out] roi  produced region-of-interest of @a stainImg if intersection.
  @param[out] newPos  produced new center position of @a stainImg in @a dstImg if intersection.
  @return true if intersection, false otherwise
 */
bool
testCrop(const cv::Mat &stainImg,
	 const cv::Mat &dstImg,
	 const cv::Point &pos,
	 cv::Rect &roi,
	 cv::Point &newPos)
{
  //dstImg in stainImg coordinate system
  const int xi0 = stainImg.cols/2 - pos.x;
  const int yi0 = stainImg.rows/2 - pos.y;
  const int xi1 = xi0 + dstImg.cols;
  const int yi1 = yi0 + dstImg.rows;

  const int xc0 = std::max(xi0, 0);
  const int yc0 = std::max(yi0, 0);
  const int xc1 = std::min(xi1, stainImg.cols);
  const int yc1 = std::min(yi1, stainImg.rows);

  if (xc1<=xc0 || yc1<=yc0) {
    return false;
  }
  if (xc0 != 0 || yc0 != 0 || (xc1-xc0)!=stainImg.cols || (yc1-yc0)!=stainImg.rows) {

    roi = cv::Rect(xc0, yc0, xc1-xc0, yc1-yc0);

    //coords new center in stainImg coordinate system
    const int ncx = xc0 + (xc1-xc0)/2;
    const int ncy = yc0 + (yc1-yc0)/2;
    //coords new center in dstImg coordinate system
    newPos = cv::Point(ncx-xi0, ncy-yi0);
  }
  else {
    roi = cv::Rect(0, 0, stainImg.cols, stainImg.rows);
    newPos = pos;
  }
  return true;
}




/*
  Try to copy stain image @a stainImg onto destination image @a dstImg at position @a pos.

  If there is no overlap between images, with given position, there is no copy.
  Otherwise, only the overlappong part of @a stainImg is copied.

  @param stainImg stain image to insert. It must be of type CV_8UC3.
  @param dstImg destination image to copy onto. It must be of type 8UC3.
  @param posCenter position of center of @a stainImg on destination image @a dstImg.
  @return true on success, false otherwise.
 */
bool
copyOntoAux(const cv::Mat &stainImg,
	    cv::Mat &dstImg,
	    const cv::Point &posCenter)
{
  bool inserted = false;

#if CV_MAJOR_VERSION >= 3
  //cv::seamlessClone() was introduced in OpenCV 3.0

  cv::Rect roi;
  cv::Point newPosCenter;
  const bool intersection = testCrop(stainImg,
				     dstImg,
				     posCenter,
				     roi,
				     newPosCenter);
  if (intersection) {

    //cv::seamlessClone() needs 3-channel src and dst images
    assert(dstImg.type() == CV_8UC3);
    assert(stainImg.type() == CV_8UC3);

    cv::Mat stainImgCrop = stainImg(roi);
    
    cv::Mat mask;
    mask.create(stainImgCrop.size(), stainImgCrop.type()); //B:could be CV_8UC1...
    mask = cv::Scalar(255, 255, 255); //set white everywhere

    cv::Mat output2;
    constexpr int flags2 = cv::MIXED_CLONE;
    cv::seamlessClone(stainImgCrop, dstImg, mask, newPosCenter, output2, flags2);
    dstImg = output2;
    
    inserted = true;
  }

#else

#ifdef _MSC_VER
#pragma message ( "Warning: GradientDomainDegradation not supported on OpenCV < 3.0" )
#else
#pragma message "Warning: GradientDomainDegradation not supported on OpenCV < 3.0"
#endif
  (void)stainImg;
  (void)dstImg;
  (void)posCenter;
#endif //CV_MAJOR_VERSION >= 3
  
  return inserted;
}

    cv::Mat
    copyOnto(const cv::Mat &dstImg,
	     const cv::Mat &stainImg,
	     const cv::Point &posCenter)
    {
#if CV_MAJOR_VERSION >= 3
      //cv::seamlessClone() was introduced in OpenCV 3.0

      //cv::seamlessClone() needs 3-channel src and dst images

      static const char *func = "dc::GradientDomainDegradation::copyOnto";
      require8UC134(dstImg, "img", func);
      require8UC134(stainImg, "stainImg", func);

      cv::Mat dstImg2;
      if (dstImg.channels() == 3) {
	dstImg2 = dstImg.clone();
      }
      else if (dstImg.channels() == 4) {
	cv::cvtColor(dstImg, dstImg2, cv::COLOR_BGRA2BGR); //remove alpha channel
      }
      else if (dstImg.channels() == 1) {
	cv::cvtColor(dstImg, dstImg2, cv::COLOR_GRAY2BGR);
      }

      cv::Mat stainImg2 = stainImg;
      if (stainImg.channels() == 4) {
	cv::cvtColor(stainImg, stainImg2, cv::COLOR_BGRA2BGR); //remove alpha channel
      }
      else if (stainImg.channels() == 1) {
	cv::cvtColor(stainImg, stainImg2, cv::COLOR_GRAY2BGR);
      }

      copyOntoAux(stainImg2, dstImg2, posCenter);

      if (dstImg.channels() == 1) {
	cv::cvtColor(dstImg2, dstImg2, cv::COLOR_BGR2GRAY);
      }
      else if (dstImg.channels() == 4) {
	//Keep alpha channel from @a in.
	cv::Mat from[] = {dstImg2, dstImg2};
	cv::Mat dstImg2B(dstImg.rows, dstImg.cols, dstImg.type());
	const int from_to[] = {0,0, 1,1, 2,2, 6,3};
	cv::mixChannels(from, 2, &dstImg2B, 1, from_to, 4);
	dstImg2 = dstImg2B;
      }
      assert(dstImg2.type() == dstImg.type());
      return dstImg2;

#else
#pragma message "Warning: GradientDomainDegradation not supported on OpenCV < 3.0"
      return dstImg;
#endif

    }


cv::Mat
degradation(const cv::Mat &img,
	    const std::string &stainDirName,
	    const size_t numStainsToInsert,
	    InsertType insertType,
	    bool doRotations,
	    std::optional<uint32_t> seed)
{
  require8UC134(img, "img", "dc::GradientDomainDegradation::degradation");

  cv::Mat out = img.clone();

#if CV_MAJOR_VERSION < 3

#ifdef _MSC_VER
#pragma message ( "Warning: GradientDomainDegradation not supported on OpenCV < 3.0" )
#else
#pragma message "Warning: GradientDomainDegradation not supported on OpenCV < 3.0"
#endif
  std::cerr<<"Warning: GradientDomainDegradation not supported on OpenCV < 3.0\n";
  return out;
#endif //CV_MAJOR_VERSION >= 3


  
  std::vector<std::string> stainFiles = dc::listDirectory(stainDirName);
  if (stainFiles.empty()) {
    std::cerr<<"Warning: No file found in stain directory: "<<stainDirName<<"\n";
    return out;
  }
  
  //B: one generator for the whole degradation
  RandomEngine rng = makeRandomEngine(seed);

  std::vector<size_t> indices =
    drawIndices(rng, numStainsToInsert, 0, stainFiles.size());
  assert(indices.size() == numStainsToInsert);

  //cv::seamlessClone() needs 3-channel image
  if (out.channels() == 1) {
    cv::cvtColor(out, out, cv::COLOR_GRAY2BGR); 
  }
  else if (out.channels() == 4) {
    cv::cvtColor(out, out, cv::COLOR_BGRA2BGR);
  }
  


  bool isImgGray = false;
  if (insertType == InsertType::INSERT_AS_GRAY_IF_GRAY) {
    isImgGray = dc::isGray(img);
  }
  const bool haveToConvert = (insertType == InsertType::INSERT_AS_GRAY
			      || (insertType == InsertType::INSERT_AS_GRAY_IF_GRAY && isImgGray));

  
  const size_t numIndices = indices.size();
  for (size_t i=0; i<numIndices; ++i) {

    const size_t ind = indices[i];
    assert(ind < stainFiles.size());
    const std::string stainFilename = dc::makePath(stainDirName, stainFiles[ind]);

    cv::Mat stain = cv::imread(stainFilename);
    if (stain.empty()) {
      std::cerr<<"Warning: unable to read stain image: "<<stainFilename<<"\n";
      continue;
    }
    if (haveToConvert) {
      cv::cvtColor(stain, stain, cv::COLOR_BGR2GRAY);
      cv::cvtColor(stain, stain, cv::COLOR_GRAY2BGR); //because cv::seamlessClone() needs 3-channel image
    }
    if (doRotations) {
      const int indAngle = rng.uniform(0, 4); //angles [0, 90, 180, 270]
      if (indAngle == 1) { //90° clockwise
	cv::transpose(stain, stain);
	cv::flip(stain, stain, 1);
      }
      else if (indAngle == 2) {	//180° clockwise
	cv::flip(stain, stain, -1);    
      }
      else if (indAngle == 3) {	//270° clockwise == -90° counter clockwise
	cv::transpose(stain, stain);
	cv::flip(stain, stain, 0);    
      }
    }

    
    //cv::RNG::uniform() is [a; b[, so the position stays inside the image
    const cv::Point pos(rng.uniform(0, img.cols), rng.uniform(0, img.rows));

    assert(out.type() == CV_8UC3);
    assert(stain.type() == CV_8UC3);
    
    const bool insertOk = copyOntoAux(stain, out, pos);
    if (! insertOk) {
      std::cerr<<"Warning: unable to insert image "<<stainFilename<<"\n";
    }

  }

  assert(out.type() == CV_8UC3);
  if (img.channels() == 1) {
    cv::cvtColor(out, out, cv::COLOR_BGR2GRAY);
  }
  else if (img.channels() == 4) {
    //Keep alpha channel from @a img.
    //cv::extractChannel(img, alpha, 4); //suppose @a in in BGRA
    cv::Mat from[] = {out, img};
    cv::Mat out2(img.rows, img.cols, img.type());
    const int from_to[] = {0,0, 1,1, 2,2, 6,3};
    cv::mixChannels(from, 2, &out2, 1, from_to, 4);
    out = out2;
  }
  assert(out.type() == img.type());
  
  return out;
}


} //namespace GradientDomainDegradation
} //namespace dc



