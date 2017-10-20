#ifndef GLMESH_HPP
#define GLMESH_HPP

#include "OpenGL.hpp"

#include "Mesh.hpp"

class GLMesh
{
public:
  GLMesh();

  ~GLMesh();

  void init(const Mesh &m);

  //@param progId  id of GLSL program used to draw geometry
  void draw(GLuint progId) const;

protected:
  GLuint m_vertexArrayId;
  GLuint m_vertexBufferId;
  GLuint m_indexBufferId;
  size_t m_numIndices;

  bool m_isInitialized;
};

#endif /* ! GLMESH_HPP */
