#include "GLCamera.hpp"

#include <Eigen/Geometry> //cross

#include <iostream> //DEBUG

GLCamera::GLCamera()
  : m_position(Eigen::Vector3f::Zero())
  , m_viewMatrix(Eigen::Matrix4f::Identity())
  , m_projectionMatrix(Eigen::Matrix4f::Identity())
{}

void
GLCamera::lookAt(const Eigen::Vector3f &pos,
                 const Eigen::Vector3f &target,
                 const Eigen::Vector3f &up)
{
  //std::cerr<<"Camera::lookAt\n";
  //std::cerr<<"position=\n"<<pos<<"\n";
  //std::cerr<<"target=\n"<<target<<"\n";
  //std::cerr<<"up=\n"<<up<<"\n";

  m_position = pos;

  Eigen::Matrix3f L;
  L.row(2) = (pos - target).normalized();
  L.row(0) = up.cross(L.row(2)).normalized();
  L.row(1) = L.row(2).cross(L.row(0)).normalized();

  m_viewMatrix.block<3, 3>(0, 0) = L;
  m_viewMatrix.block<3, 1>(0, 3) = L * (-pos);
}

void
GLCamera::setPerspective(float left,
                         float right,
                         float top,
                         float bottom,
                         float near,
                         float far,
                         float focus)
{
  assert(right != left);
  assert(top != bottom);
  assert(focus != 0.f);
  //std::cerr<<"left="<<left<<" right="<<right<<" top="<<top<<" bottom="<<bottom<<" near="<<near<<" far="<<far<<" focus="<<focus<<"\n";

  m_projectionMatrix << 2.f / (right - left), 0,
    (right + left) / (focus * (right - left)), 0, 0, 2.f / (top - bottom),
    (top + bottom) / (focus * (top - bottom)), 0, 0, 0,
    -(far + near) / (focus * (far - near)),
    -2.f * far * near / (focus * (far - near)), 0, 0, -1.f / focus, 0;

  //std::cerr<<"mProjectionMatrix=\n"<<m_projectionMatrix<<"\n";
}

void
GLCamera::setPerspective(float fovX,
                         float aspect,
                         float near,
                         float far,
                         float focus)
{
  //std::cerr<<"setPerspective fovX="<<fovX<<" aspect="<<aspect<<" near="<<near<<" far="<<far<<" focus="<<focus<<"\n";

  const float r = std::tan(fovX * 0.5f) * focus;
  const float t = r / aspect;

  setPerspective(-r, r, t, -t, near, far, focus);
}

void
GLCamera::setOrthogonal(float left,
                        float right,
                        float top,
                        float bottom,
                        float near,
                        float far)
{
  assert(right != left);
  assert(top != bottom);
  assert(far != near);

  const float rml = 1.f / (right - left);
  const float tmb = 1.f / (top - bottom);
  const float fmn = 1.f / (far - near);

  m_projectionMatrix << 2.f * rml, 0, 0, -(right + left) * rml, 0, 2.f * tmb, 0,
    -(top + bottom) * tmb, 0, 0, -2.f * fmn, -(far + near) * fmn, 0, 0, 0, 1.f;
}
