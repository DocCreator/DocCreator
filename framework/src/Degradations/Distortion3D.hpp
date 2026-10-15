#ifndef DISTORTION3D_HPP
#define DISTORTION3D_HPP

#include <cstdint>
#include <optional>
#include <string>

#include <framework_global.h>
#include <opencv2/core/core.hpp>

namespace dc {

  /**
     3D distortion: the image is mapped as a texture onto a 3D mesh of a
     scanned document, which is rendered with a lighting, as described in:

     Kieu, Van Cuong; Journet, Nicholas; Visani, Muriel; Mullot, Rémy;
     Domenger, Jean-Philippe. Semi-synthetic Document Image Generation
     Using Texture Mapping on Scanned 3D Document Shapes. In: 2013 12th
     International Conference on Document Analysis and Recognition (ICDAR).
     IEEE, 2013, p. 489-493.

     The rendering needs OSMesa: without it (cf. USE_NATIVE_OSMESA), these
     functions do nothing but print a warning.

     @warning unlike the other degradations, these four functions are not
     reentrant: they drive a single OpenGL context, through GLFW, whose
     documentation requires that it be used by one thread at a time. They
     take a lock, so that calling them from several threads is safe, but
     they do not run in parallel.
  */
  namespace Distortion3D {

    /**
      Apply a 3D distortion to an image.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
      std::invalid_argument exception is thrown otherwise.

      The background, where the mesh does not cover the image, is black.

      Without OSMesa (cf. USE_NATIVE_OSMESA), the 3D distortion is not
      available: a warning is printed on the standard error, and the
      function returns a copy of @a img.

      @param img  image to distort.
      @param meshFilename  mesh file, in the .obj or the .brs format. The
      mesh must have texture coordinates.
      @param random  whether the lighting and the orientation of the mesh
      are random.
      @param seed  seed of the random lighting and orientation. If not
      provided, a non-reproducible seed is used. Two calls with the same
      seed and the same arguments give the same result.
      @return the distorted image, of the type and size of @a img.
     */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat degrade3D(const cv::Mat &img, const std::string &meshFilename, bool random=true, std::optional<uint32_t> seed = std::nullopt);

    
    /**
      Apply @a numOutputImages 3D distortions to an image, and save them as
      PNG images.

      The images are saved as @a outputPrefix + "_" + their index, padded
      with zeros to the width of the largest one, + ".png". They have the
      type and size of @a img, and a black background. The series is
      reproducible from one seed, and its images differ from each other.

      @a img must be of type CV_8UC1, CV_8UC3 or CV_8UC4. A
      std::invalid_argument exception is thrown otherwise.

      Without OSMesa (cf. USE_NATIVE_OSMESA), the 3D distortion is not
      available: a warning is printed on the standard error, and no
      image is saved.

      @param img  image to distort.
      @param numOutputImages  number of images to save.
      @param meshFilename  mesh file, in the .obj or the .brs format. The
      mesh must have texture coordinates.
      @param outputPrefix  prefix of the file names. It may contain the
      directory to save the images into.
      @param random  whether the lighting and the orientation of the mesh
      are random.
      @param seed  seed of the random lighting and orientation. If not
      provided, a non-reproducible seed is used. Two calls with the same
      seed and the same arguments give the same result.
     */
    extern FRAMEWORK_EXPORT void degrade3DN(const cv::Mat &img, size_t numOutputImages, const std::string &meshFilename, const std::string &outputPrefix, bool random=true, std::optional<uint32_t> seed = std::nullopt);

    //extern FRAMEWORK_EXPORT void degrade3D(cv::Mat &img, const std::string &meshDir, size_t numOutputImages, const std::string &prefix);

    
    /**
      Apply a 3D distortion to an image, with another image in the
      background.

      @a img and @a backgroundImg must be of type CV_8UC1, CV_8UC3 or
      CV_8UC4. A std::invalid_argument exception is thrown otherwise.

      Without OSMesa (cf. USE_NATIVE_OSMESA), the 3D distortion is not
      available: a warning is printed on the standard error, and the
      function returns a copy of @a img.

      @param img  image to distort.
      @param meshFilename  mesh file, in the .obj or the .brs format. The
      mesh must have texture coordinates.
      @param backgroundImg  image shown where the mesh does not cover the
      image.
      @param random  whether the lighting and the orientation of the mesh
      are random.
      @param seed  seed of the random lighting and orientation. If not
      provided, a non-reproducible seed is used. Two calls with the same
      seed and the same arguments give the same result.
      @return the distorted image, of the size of @a img and of the type
      of the input image that has the most channels.
     */
    [[nodiscard]] extern FRAMEWORK_EXPORT cv::Mat degrade3DWithBackground(const cv::Mat &img, const std::string &meshFilename, const cv::Mat &backgroundImg, bool random=true, std::optional<uint32_t> seed = std::nullopt);
    
    /**
      Apply @a numOutputImages 3D distortions to an image, with another image
      in the background, and save them as PNG images.

      The images are named as with degrade3DN(). They have the size of
      @a img and the type of the input image that has the most channels.

      @a img and @a backgroundImg must be of type CV_8UC1, CV_8UC3 or
      CV_8UC4. A std::invalid_argument exception is thrown otherwise.

      Without OSMesa (cf. USE_NATIVE_OSMESA), the 3D distortion is not
      available: a warning is printed on the standard error, and no
      image is saved.

      @param img  image to distort.
      @param numOutputImages  number of images to save.
      @param meshFilename  mesh file, in the .obj or the .brs format. The
      mesh must have texture coordinates.
      @param backgroundImg  image shown where the mesh does not cover the
      image.
      @param outputPrefix  prefix of the file names. It may contain the
      directory to save the images into.
      @param random  whether the lighting and the orientation of the mesh
      are random.
      @param seed  seed of the random lighting and orientation. If not
      provided, a non-reproducible seed is used. Two calls with the same
      seed and the same arguments give the same result.
    */
    extern FRAMEWORK_EXPORT void degrade3DWithBackgroundN(const cv::Mat &img, size_t numOutputImages, const std::string &meshFilename, const cv::Mat &backgroundImg, const std::string &outputPrefix, bool random=true, std::optional<uint32_t> seed = std::nullopt);


    //extern FRAMEWORK_EXPORT void degrade3DWithBackground(cv::Mat &img, const std::string &meshDir, const std::string &backgroundImageDir, size_t numOutputImages, const std::string &prefix);
    
    

  } //namespace Distortion3D 

} //namespace dc



#endif /* ! DISTORTION3D_HPP */
