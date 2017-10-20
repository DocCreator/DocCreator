#ifndef GLOBJECT_HPP
#define GLOBJECT_HPP

#include <Eigen/Core>

class GLMesh;
class Shader;
class GLCamera;

class GLObject
{
public:
  GLObject();

  void attachMesh(const GLMesh *mesh);
  void attachShader(const Shader *shader);

  void setTransformation(const Eigen::Matrix4f &t);

  void draw(const GLCamera &camera);

protected:
  const Shader *m_shader;
  const GLMesh *m_mesh;

  Eigen::Matrix4f m_transformation;
};

#endif /* ! GLOBJECT_HPP */
