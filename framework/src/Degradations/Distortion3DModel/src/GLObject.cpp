#include "GLObject.hpp"

#include <cassert>

#include <Eigen/Dense>

#include "GLCamera.hpp"
#include "GLMesh.hpp"
#include "Shader.hpp"



//#include <iostream> //DEBUG

GLObject::GLObject()
  : m_shader(nullptr)
  , m_mesh(nullptr)
  , m_transformation(Eigen::Matrix4f::Identity())
{}

void
GLObject::attachMesh(const GLMesh *mesh)
{
  m_mesh = mesh;
}

void
GLObject::attachShader(const Shader *shader)
{
  m_shader = shader;
}

void
GLObject::setTransformation(const Eigen::Matrix4f &t)
{
  m_transformation = t;
}

void
GLObject::draw(const GLCamera &camera)
{
  assert(m_shader);
  assert(m_mesh);

  m_shader->activate();

  const GLint viewMatrixLoc = m_shader->getUniformLocation("view_matrix");
  Eigen::Matrix4f objectViewMatrix = camera.getViewMatrix() * m_transformation;
  if (viewMatrixLoc >= 0) {
    glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, objectViewMatrix.data());
    //std::cerr<<"viewMatrix:\n"<<viewMatrix<<"\n";
  }

  const GLint projectionMatrixLoc =
    m_shader->getUniformLocation("projection_matrix");
  if (projectionMatrixLoc >= 0) {
    glUniformMatrix4fv(
      projectionMatrixLoc, 1, GL_FALSE, camera.getProjectionMatrix().data());
    //std::cerr<<"projectionMatrix:\n"<<camera.getProjectionMatrix()<<"\n";
  }

  const GLint normalMatrixLoc = m_shader->getUniformLocation("normal_matrix");
  if (normalMatrixLoc >= 0) {
    //normal matrix: N=(L^-1)^t
    Eigen::Matrix3f normalMatrix = objectViewMatrix.topLeftCorner(3, 3);
    normalMatrix = normalMatrix.inverse().transpose();
    //TODO:OPTIM: we should recompute normalMatrix only when viewMatrix changed.
    // It could be a member of GLCamera...

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, normalMatrix.data());
    //std::cerr<<"normalMatrix:\n"<<normalMatrix<<"\n";
  }

  //std::cerr<<"viewMatrixLoc="<<viewMatrixLoc<<" projectionMatrixLoc="<<projectionMatrixLoc<<" normalMatrixLoc="<<normalMatrixLoc<<"\n";

  m_mesh->draw(m_shader->id());
}
