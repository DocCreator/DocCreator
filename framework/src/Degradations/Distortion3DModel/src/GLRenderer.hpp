#ifndef GLRENDERER_HPP
#define GLRENDERER_HPP

#include <opencv2/core/core.hpp>

#include "OpenGL.hpp"

#include "GLCamera.hpp"
#include "GLObject.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"

class Mesh;

class GLRenderer
{

public:
  explicit GLRenderer(int width, int height);
  
  ~GLRenderer();

  void update();
  
  bool loadMesh(const std::string &meshFilename);
  
  bool loadTexture(const std::string &imageFilename);

  void setTexture(const cv::Mat &image);


  void setLightPos(float x, float y, float z);
  void setAmbientDiffuseSpecular(Eigen::Vector3f a, Eigen::Vector3f d, Eigen::Vector3f s);
  void setSpecularExponent(float se);
  void setUseTexture(bool onoff);
  void setTextureMatrix(const Eigen::Matrix3f &texMat);
  void applyTextureMatrix();
  void applyViewMatrix();
  void resetViewMatrix();

  void setUseBackgroundTexture(bool onoff);
  void setBackgroundTexture(const cv::Mat &img);
  void setBackgroundTextureMatrix(const Eigen::Matrix3f &texMat);


  cv::Mat render(bool random);

  
  void resizeGL(int w, int h);

 
protected:

  void initializeGL();
  
  
  void paintGL(bool random=false);

  
  void printGLInfos();

  void initGLForMesh();

  void updateCameraLookAt();


  void updateMeshGL();

  
private:
  GLRenderer(const GLRenderer &);
  GLRenderer &operator=(const GLRenderer &);
  

private:

  static unsigned int ms_initialized;
  
  
  GLFWwindow *m_window;


  GLCamera m_camera;
  float m_camPhy;
  float m_camTheta;
  float m_camDist;
  Eigen::Vector3f m_camLookAt;
  float m_camFov;
  
  Eigen::Vector4f m_lightPos;
  Eigen::Vector3f m_ambient;
  Eigen::Vector3f m_diffuse;
  Eigen::Vector3f m_specular;
  float m_specularExponent;
  bool m_useTexture;
  Eigen::Matrix3f m_texMat;

  GLObject *m_object;
  GLMesh *m_meshGL;
  Shader m_program;

  GLuint m_textureId;
  GLuint m_samplerId;
  GLenum m_minFilter;
  GLenum m_magFilter;
  

  int m_tex_width;
  int m_tex_height;
  Mesh m_mesh;

  bool m_useBackground;
  bool m_useBackgroundTexture;
  Mesh m_backgroundMesh;
  GLObject *m_backgroundObject;
  GLMesh *m_backgroundMeshGL;
  Shader m_backgroundProgram;
  GLuint m_backgroundTextureId;
  GLuint m_backgroundSamplerId;
  GLenum m_backgroundMinFilter;
  GLenum m_backgroundMagFilter;
  Eigen::Matrix3f m_backgroundTexMat;

  int m_channels;
  
};

#endif /* ! GLRENDERER_HPP */
