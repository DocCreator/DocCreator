#include "RotationDegradation.hpp"

#include <cassert>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/photo/photo.hpp> //cv::inpaint

#include "GradientDomainDegradation.hpp" //copyOnto

#include <iostream>//DEBUG

namespace dc {
  namespace RotationDegradation {

    
    cv::Mat rotateFillColor(const cv::Mat &img,
			    float angle,
			    const cv::Scalar &color)
    {
      cv::Mat rotatedImg;
      const cv::Point2f centerPoint(img.cols/2.F, img.rows/2.F);
      const float scale = 1.F;
      const cv::Mat rotationMatrix = cv::getRotationMatrix2D(centerPoint, angle, scale);
      const cv::Size dSize = img.size();
      const int flags = cv::INTER_LINEAR;
      const int borderMode = cv::BORDER_CONSTANT;
      const cv::Scalar &borderValue = color;
      cv::warpAffine(img, rotatedImg, rotationMatrix, dSize, flags, borderMode, borderValue);

      assert(rotatedImg.type() == img.type());
      assert(rotatedImg.size() == img.size());
      
      return rotatedImg;
    }

    //return @a img2 converted to same type than @a img1.
    cv::Mat convertSameType(const cv::Mat &img2,
			    const cv::Mat &img1)
    {
      if (img2.type() != img1.type()) {
	cv::Mat img2b;
	if (img1.type() == CV_8UC1 && img2.type() == CV_8UC3) {
	  cv::cvtColor(img2, img2b, cv::COLOR_BGR2GRAY);
	}
	else if (img1.type() == CV_8UC1 && img2.type() == CV_8UC4) {
	  cv::cvtColor(img2, img2b, cv::COLOR_BGRA2GRAY);
	}
	else if (img1.type() == CV_8UC3 && img2.type() == CV_8UC1) {
	  cv::cvtColor(img2, img2b, cv::COLOR_GRAY2BGR);
	}
	else if (img1.type() == CV_8UC3 && img2.type() == CV_8UC4) {
	  cv::cvtColor(img2, img2b, cv::COLOR_BGRA2BGR);
	}
	else if (img1.type() == CV_8UC4 && img2.type() == CV_8UC1) {
	  cv::cvtColor(img2, img2b, cv::COLOR_GRAY2BGRA);
	}
	else if (img1.type() == CV_8UC4 && img2.type() == CV_8UC3) {
	  cv::cvtColor(img2, img2b, cv::COLOR_BGR2BGRA);
	}
	else {
	  std::cerr<<"img1.type()="<<img1.type()<<" CV_8UC1="<<CV_8UC1<<" CV_8UC3="<<CV_8UC3<<" CV_8UC4="<<CV_8UC4<<"  img2.type()="<<img2.type()<<"\n";
#if CV_MAJOR_VERSION < 3
	  const int code = CV_StsUnsupportedFormat;
#else
	  const int code = cv::Error::StsUnsupportedFormat;
#endif
	  CV_Error(code, "RotationDegradation: unhandled format");
	}
	return img2b;
      }
      else {
	return img2;
      }
    }


    
    cv::Mat rotateFillImage(const cv::Mat &img,
			    float angle,
			    const cv::Mat &backgroundImg)
    {
      if (backgroundImg.empty())
	return rotateFillColor(img, angle, cv::Scalar(0, 0, 0));

      cv::Mat rotatedImg;
      cv::Mat backgroundImgS = convertSameType(backgroundImg, img);
      
      const cv::Size dSize = img.size();
      cv::resize(backgroundImgS, rotatedImg, dSize, 0, 0, cv::INTER_LINEAR);
        
      const cv::Point2f centerPoint(img.cols/2.F, img.rows/2.F);
      const float scale = 1.F;
      const cv::Mat rotationMatrix = cv::getRotationMatrix2D(centerPoint, angle, scale);
      const int flags = cv::INTER_LINEAR;
      const int borderMode = cv::BORDER_TRANSPARENT;
      const cv::Scalar borderValue = 0;
      cv::warpAffine(img, rotatedImg, rotationMatrix, dSize, flags, borderMode, borderValue);
      
      assert(rotatedImg.type() == img.type());
      assert(rotatedImg.size() == img.size());

      return rotatedImg;
    }

    cv::Mat rotateFillImageN(const cv::Mat &img,
			    float angle,
			    const cv::Mat &backgroundImg,
			    int repeats)
    {
      if (backgroundImg.empty())
	return rotateFillColor(img, angle, cv::Scalar(0, 0, 0));

      if (repeats <= 0) {
	return rotateFillImage(img, angle, backgroundImg);
      }
      else {

	const float incr = angle / (repeats+1);
	float angle0 = incr;
	
	cv::Mat background = backgroundImg.clone();
	for (int r=0; r<repeats; ++r) {
	  background = rotateFillImage(background, angle0, background);
	  angle0 += incr;	
	}
	background = rotateFillImage(img, angle, background);
	return background;
      }
    }

    cv::Mat rotateFillImages(const cv::Mat &img,
			     float angle,
			     const std::vector<cv::Mat> &backgroundImgs)
    {
      const size_t sz = backgroundImgs.size();
      if (sz == 0) {
	return rotateFillColor(img, angle, cv::Scalar(0,0,0,0));
      }
      else {
	const float incr = angle / sz;
	float angle0 = incr;
	
	cv::Mat background = backgroundImgs[0].clone();
	for (size_t r=1; r<sz; ++r) {
	  background = rotateFillImage(backgroundImgs[r], angle0, background);
	  angle0 += incr;	
	}
	background = rotateFillImage(img, angle, background);
	return background;
      }
    }

    

    cv::Mat rotateFillBorder(const cv::Mat &img,
			     float angle,
			     BorderReplication borderMode)
    {
      cv::Mat rotatedImg;
      const cv::Point2f centerPoint(img.cols/2.F, img.rows/2.F);
      const float scale = 1.F;
      const cv::Mat rotationMatrix = cv::getRotationMatrix2D(centerPoint, angle, scale);
      const cv::Size dSize = img.size();
      const int flags = cv::INTER_LINEAR;
      int borderModeCV = cv::BORDER_REPLICATE;
      switch (borderMode) {
      default:
      case BorderReplication::REPLICATE:
	borderModeCV = cv::BORDER_REPLICATE;
	break;
      case BorderReplication::REFLECT:
	borderModeCV = cv::BORDER_REFLECT;
	break;
      case BorderReplication::WRAP:
	borderModeCV = cv::BORDER_WRAP;
	break;
      case BorderReplication::REFLECT101:
	borderModeCV = cv::BORDER_REFLECT101;
	break;
      }
      
      const cv::Scalar borderValue = 0;
      cv::warpAffine(img, rotatedImg, rotationMatrix, dSize, flags, borderModeCV, borderValue);

      assert(rotatedImg.type() == img.type());
      assert(rotatedImg.size() == img.size());
      
      return rotatedImg;
    }

    cv::Mat getRotatedImage(const cv::Mat &img,
			    const cv::Point2f &centerPoint,
			    float angle)
    {
      cv::Mat rotatedImg;
      const float scale = 1.F;
      const cv::Mat rotationMatrix = cv::getRotationMatrix2D(centerPoint, angle, scale);
      cv::warpAffine(img, rotatedImg, rotationMatrix, img.size());
      return rotatedImg;
    }
    
    /*
      maskScale must be in ]0; 1[
      
      

     */
    cv::Mat getRotatedMask(const cv::Mat &img,
			   const cv::Point2f &centerPoint,
			   float angle,
			   float maskScale = 0.9)
    {
      cv::Mat mask;
      assert(maskScale>0.F && maskScale<1.F);
      const cv::Mat rotationMatrix = cv::getRotationMatrix2D(centerPoint, angle, maskScale);
      const cv::Mat whiteImg(img.rows, img.cols, CV_8UC1, cv::Scalar(255));
      const int flags = cv::INTER_NEAREST;
      cv::warpAffine(whiteImg, mask, rotationMatrix, img.size(), flags);
      return 255-mask;
    }
    
    cv::Mat rotateFillInpaint1(const cv::Mat &img,
			       float angle,
			       float inpaintingRatio)
    {      
      //cv::inpaint does not work on 4-channel images.
      cv::Mat img2 = img;
      cv::Mat alpha;
      if (img.channels() == 4) {
	img2 = cv::Mat( img.rows, img.cols, CV_8UC3 );
	alpha = cv::Mat( img.rows, img.cols, CV_8UC1 );
	cv::Mat out[] = { img2, alpha };
	const int from_to[] = { 0,0, 1,1, 2,2, 3,3 };
	cv::mixChannels( &img, 1, out, 2, from_to, 4 );
	alpha = rotateFillBorder(alpha, angle, BorderReplication::REPLICATE);
      }
      
      const cv::Point2f centerPoint(img2.cols/2.F, img2.rows/2.F);
      
      const cv::Mat rotatedImg = getRotatedImage(img2,
						 centerPoint,
						 angle);
      const float maskScale = 1.F - std::min(0.999F, std::max(0.001F, inpaintingRatio));
      //std::cerr<<"inpaintingRatio="<<inpaintingRatio<<" -> maskScale="<<maskScale<<"\n";
      const cv::Mat maskImg = getRotatedMask(img2, centerPoint, angle, maskScale);

      cv::Mat inpaintedImg;
      const float inpaintedRadius = 2;
      cv::inpaint(rotatedImg, maskImg, inpaintedImg,
		  inpaintedRadius,
		  cv::INPAINT_TELEA);

      if (img.channels() == 4) {
	assert(inpaintedImg.channels() == 3);
	cv::Mat mv[] = {inpaintedImg, alpha};
	cv::Mat dst;
	cv::merge(mv, 2, dst);
	inpaintedImg = dst;
	assert(inpaintedImg.channels() == 4);
      }
      
      assert(inpaintedImg.size() == img.size());
      assert(inpaintedImg.type() == img.type());
      
      return inpaintedImg;
    }

    cv::Mat getBinarizedImg(const cv::Mat &img)
    {
      cv::Mat bin;
      if (img.channels() == 3) {
	cv::cvtColor(img, bin, cv::COLOR_BGR2GRAY);
      }
      else if (img.channels() == 4) {
	cv::cvtColor(img, bin, cv::COLOR_BGRA2GRAY);
      }
      else {
	bin = img.clone();
      }

      if (bin.channels() != 1) {
#if CV_MAJOR_VERSION < 3
	  const int code = CV_StsUnsupportedFormat;
#else
	  const int code = cv::Error::StsUnsupportedFormat;
#endif
	  CV_Error(code, "RotationDegradation: unhandled format");
      }

      const int maxValue = 255;
      cv::threshold(bin, bin, 0, maxValue, cv::THRESH_BINARY_INV|cv::THRESH_OTSU);

      assert(bin.size() == img.size());
      assert(bin.type() == CV_8UC1);

      return bin;
    }
    
    cv::Mat getBackground(const cv::Mat &img)
    {
      cv::Mat background;

      cv::Mat bin = getBinarizedImg(img);

      const int dilation_type = cv::MORPH_ELLIPSE;
      const int dilation_size = 4;
      cv::Mat element = cv::getStructuringElement(
						  dilation_type,
						  cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
						  cv::Point(dilation_size, dilation_size));
      cv::dilate(bin, bin, element);
      
      cv::inpaint(img, bin, background, 1, cv::INPAINT_TELEA);

      assert(background.size() == img.size());
      assert(background.type() == img.type());
      return background;
    }

    cv::Mat getBackgroundSame(const cv::Mat &img, const cv::Mat &backgroundImg)
    {
      cv::Mat background = backgroundImg;
      if (backgroundImg.empty()) {
	background = getBackground(img);
      }
      else {
	background = convertSameType(backgroundImg, img);
	cv::resize(background, background, img.size(), 0, 0, cv::INTER_LINEAR);
      }
      assert(background.size() == img.size());
      assert(background.type() == img.type());
      return background;
    }

    cv::Mat rotateFillInpaint2(const cv::Mat &img,
			       float angle,
			       const cv::Mat &backgroundImg)
    {
      //cv::inpaint does not work on 4-channel images.
      cv::Mat img2 = img;
      cv::Mat alpha;
      if (img.channels() == 4) {
	img2 = cv::Mat( img.rows, img.cols, CV_8UC3 );
	alpha = cv::Mat( img.rows, img.cols, CV_8UC1 );
	cv::Mat out[] = { img2, alpha };
	const int from_to[] = { 0,0, 1,1, 2,2, 3,3 };
	cv::mixChannels( &img, 1, out, 2, from_to, 4 );
	alpha = rotateFillBorder(alpha, angle, BorderReplication::REPLICATE);
      }

      const cv::Mat background = getBackgroundSame(img2, backgroundImg);

      const cv::Point2f centerPoint(img2.cols/2.F, img2.rows/2.F);
      const cv::Mat rotatedBackground = getRotatedImage(background, centerPoint, angle);
      const cv::Mat rotatedBackgroundMask = getRotatedMask(rotatedBackground, centerPoint, angle, 0.95F);

      cv::Mat rotatedBackgroundInpainted;
      const float inpaintedRadius = 2;
      cv::inpaint(rotatedBackground, rotatedBackgroundMask,
		  rotatedBackgroundInpainted, inpaintedRadius, cv::INPAINT_TELEA);

      const cv::Mat maskBef = getRotatedMask(img2, centerPoint, angle, 0.99F);
      cv::Mat mask;
      cv::threshold(maskBef, mask, 0, 255, cv::THRESH_BINARY_INV);

      const cv::Mat rotatedImgOriginal =
	getRotatedImage(img2, centerPoint, angle);
      
      rotatedImgOriginal.copyTo(rotatedBackgroundInpainted, mask);

      if (img.channels() == 4) {
	assert(rotatedBackgroundInpainted.channels() == 3);
	cv::Mat mv[] = {rotatedBackgroundInpainted, alpha};
	cv::Mat dst;
	cv::merge(mv, 2, dst);
	rotatedBackgroundInpainted = dst;
	assert(rotatedBackgroundInpainted.channels() == 4);
      }
      
      assert(rotatedBackgroundInpainted.size() == img.size());
      assert(rotatedBackgroundInpainted.type() == img.type());
      
      return rotatedBackgroundInpainted; 
    }

    cv::Mat rotateFillInpaint3(const cv::Mat &img,
			       float angle,
			       const cv::Mat &backgroundImg)
    {
      //cv::inpaint does not work on 4-channel images.
      //dc::GradientDomainDegradation::copyOnto() only works on 3-channel images
      cv::Mat img2 = img;
      cv::Mat alpha;
      if (img.channels() == 4) {
	img2 = cv::Mat( img.rows, img.cols, CV_8UC3 );
	alpha = cv::Mat( img.rows, img.cols, CV_8UC1 );
	cv::Mat out[] = { img2, alpha };
	const int from_to[] = { 0,0, 1,1, 2,2, 3,3 };
	cv::mixChannels( &img, 1, out, 2, from_to, 4 );
	alpha = rotateFillBorder(alpha, angle, BorderReplication::REPLICATE);
      }
      else if (img.channels() == 1) {
	cv::cvtColor(img, img2, cv::COLOR_GRAY2BGR);
      }
      
      cv::Mat background = getBackgroundSame(img2, backgroundImg);
      const cv::Mat rotatedAndInpaintedImg = rotateFillInpaint2(img2, angle, background);

      const cv::Point2f centerPoint(img2.cols/2.F, img2.rows/2.F);
      cv::Mat rotatedBackground = getRotatedImage(background, centerPoint, angle);
      const cv::Mat rotatedBackgroundMask = getRotatedMask(rotatedBackground, centerPoint, angle, 0.95F);
      
      cv::Mat rotatedBackgroundInpainted;
      const float inpaintedRadius = 2;
      cv::inpaint(rotatedBackground, rotatedBackgroundMask,
		  rotatedBackgroundInpainted, inpaintedRadius, cv::INPAINT_TELEA);

      dc::GradientDomainDegradation::copyOnto(rotatedAndInpaintedImg, rotatedBackgroundInpainted, cv::Point(img2.cols/2, img2.rows/2));

      if (img.channels() == 4) {
	assert(rotatedBackgroundInpainted.channels() == 3);
	cv::Mat mv[] = {rotatedBackgroundInpainted, alpha};
	cv::Mat dst;
	cv::merge(mv, 2, dst);
	rotatedBackgroundInpainted = dst;
	assert(rotatedBackgroundInpainted.channels() == 4);
      }
      else if (img.channels() == 1) {
	cv::cvtColor(rotatedBackgroundInpainted, rotatedBackgroundInpainted, cv::COLOR_BGR2GRAY);
      }
      
      assert(rotatedBackgroundInpainted.size() == img.size());
      assert(rotatedBackgroundInpainted.type() == img.type());
      
      return rotatedBackgroundInpainted;
    }      


  } //namespace RotationDegradation
} //namespace dc


    
