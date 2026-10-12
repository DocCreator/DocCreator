#ifndef DISTORTION3D_HPP
#define DISTORTION3D_HPP

#include <cstdint>
#include <optional>
#include <string>

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  /**
     @warning unlike the other degradations, these four functions are not
     reentrant: they drive a single OpenGL context, through GLFW, whose
     documentation requires that it be used by one thread at a time. They
     take a lock, so that calling them from several threads is safe, but
     they do not run in parallel.
  */
  namespace Distortion3D {

    /*
      Apply 3D distortion to an image.

      Described in [1]

      [1]
      Semi-synthetic Document Image Generation Using Texture Mapping on Scanned 3D Document Shapes
      KIEU, Van Cuaong, JOURNET, Nicholas, VISANI, Muriel, MULLOT, Rémy, DOMENGER, Jean-Philippe
      In : 2013 12th International Conference on Document Analysis and Recognition (ICDAR). IEEE, 2013. p. 489-493.


      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. An std::invalid_argument exception is thrown otherwise. 
      Output image will be of the same type and size than @a img.

      The background will be black.
      
      @prama[in] img  input original image.
      @prama[in] meshFilename  input mesh filename. Only .obj and .brs mesh file format is supported. Mesh must have texture coordinates.
      @prama[in] random  input boolean value indicating if a random lighting and orientation must be used.
      @prama[in] seed  seed of the random lighting and orientation. If not provided, a non reproducible seed is used. Two calls with the same seed and the same parameters give the same result.

      @return modified image.

     */
    extern FRAMEWORK_EXPORT cv::Mat degrade3D(const cv::Mat &img, const std::string &meshFilename, bool random=true, std::optional<uint32_t> seed = std::nullopt);

    
    /*
      Apply different 3D distortions to an image and save them as png images.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. An std::invalid_argument exception is thrown otherwise. 
      Output image will be of the same type and size than @a img.

      The background will be black.
      
      @prama[in] img  input original image.
      @prama[in] numOutputImages  number of output images to save.
      @prama[in] meshFilename  input mesh filename. Only .obj and .brs mesh file format is supported. Mesh must have texture coordinates.
      @prama[in] outputPrefix  prefix of output images. It may contain the path where to save images.
      @prama[in] random  input boolean value indicating if a random lighting and orientation must be used.
      @prama[in] seed  seed of the random lighting and orientation. If not provided, a non reproducible seed is used. Two calls with the same seed and the same parameters give the same result.

    */
    extern FRAMEWORK_EXPORT void degrade3DN(const cv::Mat &img, size_t numOutputImages, const std::string &meshFilename, const std::string &outputPrefix, bool random=true, std::optional<uint32_t> seed = std::nullopt);

    //extern FRAMEWORK_EXPORT void degrade3D(cv::Mat &img, const std::string &meshDir, size_t numOutputImages, const std::string &prefix);

    
    /*
      Apply 3D distortion to an image, with another image in the background.

      @a img and @a backgroundImg must be of type CV_8UC1, CV_8UC3 or CV_8UC4. An std::invalid_argument exception is thrown otherwise. 
      Output image will have the largest of the two input images types.
      Output image will be of the same type and size than @a img.
      
      @prama[in] img  input original image.
      @prama[in] meshFilename  input mesh filename. Only .obj and .brs mesh file format is supported. Mesh must have texture coordinates.
      @prama[in] backgroundImg  input background image.
      @prama[in] random  input boolean value indicating if a random lighting and orientation must be used.
      @prama[in] seed  seed of the random lighting and orientation. If not provided, a non reproducible seed is used. Two calls with the same seed and the same parameters give the same result.

      @return modified image.

     */
    extern FRAMEWORK_EXPORT cv::Mat degrade3DWithBackground(const cv::Mat &img, const std::string &meshFilename, const cv::Mat &backgroundImg, bool random=true, std::optional<uint32_t> seed = std::nullopt);
    
    /*
      Apply different 3D distortions to an image, with another image in the background, and save them as png images.

      @a img and @a backgroundImg must be of type CV_8UC1, CV_8UC3 or CV_8UC4. An std::invalid_argument exception is thrown otherwise. 
      Output image will have the largest of the two input images types.
      Output image will be of the same type and size than @a img.

      @prama[in] img  input original image.
      @prama[in] numOutputImages  number of output images to save.
      @prama[in] meshFilename  input mesh filename. Only .obj and .brs mesh file format is supported. Mesh must have texture coordinates.
      @prama[in] outputPrefix  prefix of output images. It may contain the path where to save images.
      @prama[in] random  input boolean value indicating if a random lighting and orientation must be used.
      @prama[in] seed  seed of the random lighting and orientation. If not provided, a non reproducible seed is used. Two calls with the same seed and the same parameters give the same result.

    */
    extern FRAMEWORK_EXPORT void degrade3DWithBackgroundN(const cv::Mat &img, size_t numOutputImages, const std::string &meshFilename, const cv::Mat &backgroundImg, const std::string &outputPrefix, bool random=true, std::optional<uint32_t> seed = std::nullopt);


    //extern FRAMEWORK_EXPORT void degrade3DWithBackground(cv::Mat &img, const std::string &meshDir, const std::string &backgroundImageDir, size_t numOutputImages, const std::string &prefix);
    
    

  } //namespace Distortion3D 

} //namespace dc



#endif /* ! DISTORTION3D_HPP */
