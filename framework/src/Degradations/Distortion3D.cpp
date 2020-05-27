#include "Distortion3D.hpp"

#include <iostream>

#ifdef USE_NATIVE_OSMESA

#include <iomanip>
#include <sstream>

#include <opencv2/highgui/highgui.hpp>

#include "GLRenderer.hpp"

#endif //USE_NATIVE_OSMESA

namespace dc {
  namespace Distortion3D {

    cv::Mat degrade3D(const cv::Mat &img, const std::string &meshFilename, bool random)
    {
#ifdef USE_NATIVE_OSMESA
      GLRenderer w(img.cols, img.rows);
      const bool loadMeshOk = w.loadMesh(meshFilename);
      if (! loadMeshOk) {
	return cv::Mat();
      }
      w.setTexture(img);
      const cv::Mat out = w.render(random);
      return out;
#else

#ifdef _MSC_VER
#pragma message ( "Warning: Distortion3D not supported when not compiled with OSMesa" )
#else
#pragma message "Warning: Distortion3D not supported when not compiled with OSMesa"
#endif
      (void)img;
      (void)meshFilename;
      (void)random;
      
      std::cerr<<"Warning: Distortion3D not supported when not compiled with OSMesa\n";
      return img;
#endif //USE_NATIVE_OSMESA
    }

#ifdef USE_NATIVE_OSMESA
    static int
    computeNumberWidth(int n)
    {
      int i = 0;
      for (int m = n; m > 0; m /= 10, ++i)
	;
      return i;
    }
#endif //USE_NATIVE_OSMESA

    
    void degrade3D(const cv::Mat &img, size_t numOutputImages, const std::string &meshFilename, const std::string &outputPrefix, bool random)
    {
#ifdef USE_NATIVE_OSMESA
      GLRenderer w(img.cols, img.rows);
      const bool loadMeshOk = w.loadMesh(meshFilename);
      if (! loadMeshOk) {
	return;
      }
      w.setTexture(img);

      const std::string ext=".png";
      const int numberWidth = computeNumberWidth(numOutputImages);
      for (size_t i=0; i<numOutputImages; ++i) {
	const cv::Mat out = w.render(random);
	if (! out.empty()) {
	  std::stringstream ss;
	  ss << outputPrefix << "_" << std::setfill('0') << std::setw(numberWidth)<<i<<ext;
	  const std::string outputFilename = ss.str();
	  const bool writeOk = cv::imwrite(outputFilename, out);
	  if (! writeOk) {
	    std::cerr<<"ERROR: unable to save: "<<outputFilename<<"\n";
	  }
	}
      }
#else
#ifdef _MSC_VER
#pragma message ( "Warning: Distortion3D not supported when not compiled with OSMesa" )
#else
#pragma message "Warning: Distortion3D not supported when not compiled with OSMesa"
#endif
      (void)img;
      (void)numOutputImages;
      (void)meshFilename;
      (void)outputPrefix;
      (void)random;
      
      std::cerr<<"Warning: Distortion3D not supported when not compiled with OSMesa\n";
#endif //USE_NATIVE_OSMESA
      
    }

    cv::Mat degrade3DWithBackground(const cv::Mat &img, const std::string &meshFilename, cv::Mat &backgroundImg, bool random)
    {
#ifdef USE_NATIVE_OSMESA
      GLRenderer w(img.cols, img.rows);
      const bool loadMeshOk = w.loadMesh(meshFilename);
      if (! loadMeshOk) {
	return cv::Mat();
      }
      w.setTexture(img);
      w.setBackgroundTexture(backgroundImg);
      const cv::Mat out = w.render(random);
      return out;
#else
#ifdef _MSC_VER
#pragma message ( "Warning: Distortion3D not supported when not compiled with OSMesa" )
#else
#pragma message "Warning: Distortion3D not supported when not compiled with OSMesa"
#endif
      (void)img;
      (void)meshFilename;
      (void)backgroundImg;
      (void)random;

      std::cerr<<"Warning: Distortion3D not supported when not compiled with OSMesa\n";
      return img;
#endif //USE_NATIVE_OSMESA
      

    }
    
    void degrade3DWithBackground(const cv::Mat &img, size_t numOutputImages, const std::string &meshFilename, const cv::Mat &backgroundImg, const std::string &outputPrefix, bool random)
    {
#ifdef USE_NATIVE_OSMESA
      GLRenderer w(img.cols, img.rows);
      const bool loadMeshOk = w.loadMesh(meshFilename);
      if (! loadMeshOk) {
	return;
      }
      w.setTexture(img);
      w.setBackgroundTexture(backgroundImg);

      const std::string ext=".png";
      const int numberWidth = computeNumberWidth(numOutputImages);
      for (size_t i=0; i<numOutputImages; ++i) {
	const cv::Mat out = w.render(random);
	if (! out.empty()) {
	  std::stringstream ss;
	  ss << outputPrefix << "_" << std::setfill('0') << std::setw(numberWidth)<<i<<ext;
	  const std::string outputFilename = ss.str();
	  const bool writeOk = cv::imwrite(outputFilename, out);
	  if (! writeOk) {
	    std::cerr<<"ERROR: unable to save: "<<outputFilename<<"\n";
	  }
	}
      }
#else
#ifdef _MSC_VER
#pragma message ( "Warning: Distortion3D not supported when not compiled with OSMesa" )
#else
#pragma message "Warning: Distortion3D not supported when not compiled with OSMesa"
#endif
      (void)img;
      (void)numOutputImages;
      (void)meshFilename;
      (void)backgroundImg;
      (void)outputPrefix;
      (void)random;
      
      std::cerr<<"Warning: Distortion3D not supported when not compiled with OSMesa\n";
#endif //USE_NATIVE_OSMESA
    }
    

  } //namespace Distortion3D
} //namespace dc
