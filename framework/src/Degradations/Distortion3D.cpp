#include "Distortion3D.hpp"

#include <iostream>
#include <mutex>

#include "Random.hpp"
#include "Validate.hpp"

#ifdef USE_NATIVE_OSMESA

#include <iomanip>
#include <sstream>

#include <opencv2/highgui/highgui.hpp>

#include "GLRenderer.hpp"

#endif //USE_NATIVE_OSMESA

namespace dc {
  namespace Distortion3D {

#ifdef USE_NATIVE_OSMESA
    /*
      B: GLFW requires that its windows and its contexts be used by one
      thread at a time, and GLRenderer counts its instances in a static to
      know when to call glfwInit() and glfwTerminate(). Two threads
      rendering at the same time corrupted that count and shared one
      context. The four functions below take this lock, so that a library
      used from several threads, as the python module is, is safe; they do
      not run in parallel.
    */
    static std::mutex rendererMutex;
#endif //USE_NATIVE_OSMESA

    cv::Mat degrade3D(const cv::Mat &img, const std::string &meshFilename, bool random, std::optional<uint32_t> seed)
    {
      require8UC134(img, "img", "dc::Distortion3D::degrade3D");

#ifdef USE_NATIVE_OSMESA
      const std::lock_guard<std::mutex> lock(rendererMutex);

      GLRenderer w(img.cols, img.rows);
      if (! w.isValid()) {
	std::cerr<<"ERROR: unable to create the OpenGL renderer\n";
	return cv::Mat();
      }
      const bool loadMeshOk = w.loadMesh(meshFilename);
      if (! loadMeshOk) {
	return cv::Mat();
      }
      w.setTexture(img);
      RandomEngine rng = makeRandomEngine(seed);
      const cv::Mat out = w.render(random, rng);
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
      (void)seed;
      
      std::cerr<<"Warning: Distortion3D not supported when not compiled with OSMesa\n";
      //B: was img itself, which shares its pixels with the caller's image
      return img.clone();
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

    
    void degrade3DN(const cv::Mat &img, size_t numOutputImages, const std::string &meshFilename, const std::string &outputPrefix, bool random, std::optional<uint32_t> seed)
    {
      require8UC134(img, "img", "dc::Distortion3D::degrade3DN");

#ifdef USE_NATIVE_OSMESA
      const std::lock_guard<std::mutex> lock(rendererMutex);

      GLRenderer w(img.cols, img.rows);
      if (! w.isValid()) {
	std::cerr<<"ERROR: unable to create the OpenGL renderer\n";
	return;
      }
      const bool loadMeshOk = w.loadMesh(meshFilename);
      if (! loadMeshOk) {
	return;
      }
      w.setTexture(img);

      /*
	B: one engine for the whole series, so that the N images differ from
	each other while the series stays reproducible from one seed.
      */
      RandomEngine rng = makeRandomEngine(seed);

      const std::string ext=".png";
      const int numberWidth = computeNumberWidth(numOutputImages);
      for (size_t i=0; i<numOutputImages; ++i) {
	const cv::Mat out = w.render(random, rng);
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
      (void)seed;
      
      std::cerr<<"Warning: Distortion3D not supported when not compiled with OSMesa\n";
#endif //USE_NATIVE_OSMESA
      
    }

    cv::Mat degrade3DWithBackground(const cv::Mat &img, const std::string &meshFilename, const cv::Mat &backgroundImg, bool random, std::optional<uint32_t> seed)
    {
      require8UC134(img, "img", "dc::Distortion3D::degrade3DWithBackground");
      if (! backgroundImg.empty()) {
	require8UC134(backgroundImg, "backgroundImg", "dc::Distortion3D::degrade3DWithBackground");
      }

#ifdef USE_NATIVE_OSMESA
      const std::lock_guard<std::mutex> lock(rendererMutex);

      GLRenderer w(img.cols, img.rows);
      if (! w.isValid()) {
	std::cerr<<"ERROR: unable to create the OpenGL renderer\n";
	return cv::Mat();
      }
      const bool loadMeshOk = w.loadMesh(meshFilename);
      if (! loadMeshOk) {
	return cv::Mat();
      }
      w.setTexture(img);
      w.setBackgroundTexture(backgroundImg);
      RandomEngine rng = makeRandomEngine(seed);
      const cv::Mat out = w.render(random, rng);
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
      (void)seed;

      std::cerr<<"Warning: Distortion3D not supported when not compiled with OSMesa\n";
      //B: was img itself, which shares its pixels with the caller's image
      return img.clone();
#endif //USE_NATIVE_OSMESA
      

    }
    
    void degrade3DWithBackgroundN(const cv::Mat &img, size_t numOutputImages, const std::string &meshFilename, const cv::Mat &backgroundImg, const std::string &outputPrefix, bool random, std::optional<uint32_t> seed)
    {
      require8UC134(img, "img", "dc::Distortion3D::degrade3DWithBackgroundN");
      if (! backgroundImg.empty()) {
	require8UC134(backgroundImg, "backgroundImg", "dc::Distortion3D::degrade3DWithBackgroundN");
      }

#ifdef USE_NATIVE_OSMESA
      const std::lock_guard<std::mutex> lock(rendererMutex);

      GLRenderer w(img.cols, img.rows);
      if (! w.isValid()) {
	std::cerr<<"ERROR: unable to create the OpenGL renderer\n";
	return;
      }
      const bool loadMeshOk = w.loadMesh(meshFilename);
      if (! loadMeshOk) {
	return;
      }
      w.setTexture(img);
      w.setBackgroundTexture(backgroundImg);

      /*
	B: one engine for the whole series, so that the N images differ from
	each other while the series stays reproducible from one seed.
      */
      RandomEngine rng = makeRandomEngine(seed);

      const std::string ext=".png";
      const int numberWidth = computeNumberWidth(numOutputImages);
      for (size_t i=0; i<numOutputImages; ++i) {
	const cv::Mat out = w.render(random, rng);
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
      (void)seed;
      
      std::cerr<<"Warning: Distortion3D not supported when not compiled with OSMesa\n";
#endif //USE_NATIVE_OSMESA
    }
    

  } //namespace Distortion3D
} //namespace dc
