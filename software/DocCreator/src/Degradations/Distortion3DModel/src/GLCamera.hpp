#ifndef GLCAMERA_HPP
#define GLCAMERA_HPP

#include <Eigen/Core>

class GLCamera
{
public:
  GLCamera();

  //change viewMatrix
  void lookAt(const Eigen::Vector3f &pos,
              const Eigen::Vector3f &target,
              const Eigen::Vector3f &up);

  //change projectionMatrix
  void setPerspective(float left,
                      float right,
                      float top,
                      float bottom,
                      float near,
                      float far,
                      float focus = 0.4);

  void setPerspective(float fovX,
                      float aspect,
                      float near,
                      float far,
                      float focus = 0.4);

  void setOrthogonal(float left,
                     float right,
                     float top,
                     float bottom,
                     float near,
                     float far);

  const Eigen::Vector3f &getPosition() const { return m_position; }
  const Eigen::Matrix4f &getViewMatrix() const { return m_viewMatrix; }
  const Eigen::Matrix4f &getProjectionMatrix() const
  {
    return m_projectionMatrix;
  }

protected:
  Eigen::Vector3f m_position;
  Eigen::Matrix4f m_viewMatrix;
  Eigen::Matrix4f m_projectionMatrix;
};

#endif /* ! GLCAMERA_HPP */
