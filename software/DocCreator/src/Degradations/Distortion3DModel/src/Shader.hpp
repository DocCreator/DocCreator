#ifndef SHADER_HPP
#define SHADER_HPP

#include "OpenGL.hpp"
#include <string>

class Shader
{
public:
  Shader()
    : m_programId(0)
    , m_isValid(false)
  {}

  bool isValid() const { return m_isValid; }

  bool loadFromFiles(const std::string &vertexFilename,
                     const std::string &fragmentFilename);

  bool loadFromStrings(const std::string &vertexString,
                       const std::string &fragmentString);

  //TODO: handle geometric shaders

  GLuint id() const { return m_programId; }

  void activate() const;

  GLint getUniformLocation(const char *name) const;
  GLint getAttribLocation(const char *name) const;

  void setSamplerUnit(const char *sampler, int unit) const;

  void dumpInfos() const;

protected:
  GLuint m_programId;
  bool m_isValid;
};

#endif /* ! SHADER_HPP */
