#include "Shader.hpp"

#include <cassert>
#include <fstream>
#include <iostream>

static std::string
getStringFromFile(const std::string &filename)
{
  std::string s;

  if (filename == "") {
    //no shader specified
    return "";
  }

  std::ifstream in(filename.c_str());
  if (!in) {
    std::cerr << "ERROR: unable to read filename: " << filename << "\n";
    return s;
  }

  std::string line;
  while (std::getline(in, line)) {
    s += line + "\n";
  }

  return s;
}

static void
printInfoLog(GLuint object, const std::string &msg)
{
  GLint logLength = 0;
  if (glIsShader(object) != 0u) {
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &logLength);
  }
  else if (glIsProgram(object) != 0u) {
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &logLength);
  }
  else {
    std::cerr << "Error: Not a shader or a program\n";
    return;
  }

  if (logLength > 0) {

    auto log = new GLchar[logLength];
    if (glIsShader(object) != 0u) {
      glGetShaderInfoLog(object, logLength, nullptr, log);
    }
    else if (glIsProgram(object) != 0u) {
      glGetProgramInfoLog(object, logLength, nullptr, log);
    }

    std::cerr << msg << " info: \n" << log << "\n";

    delete[] log;
  }
}

static GLuint
createShader(const std::string &src, GLenum type, const std::string &msg)
{
  GLuint res = glCreateShader(type);

  std::string src2;
// #ifdef WITH_EMSCRIPTEN
//   //OpenGL ES 3.0 = WebGL 2.0
//   src2 = "#version 300 es\n";
//   if (type == GL_FRAGMENT_SHADER)
//     src2 += "precision highp float;\n";
// #else
  //OpenGL 3.3
  src2 = "#version 330 core\n";
//#endif //WITH_EMSCRIPTEN
  src2 += src;
  
  const GLchar *s_src = src2.c_str();


  GL_CHECK_ERROR_ALWAYS();

  glShaderSource(res, 1, static_cast<const GLchar **>(&s_src), nullptr);

  GL_CHECK_ERROR_ALWAYS();

  glCompileShader(res);

  GL_CHECK_ERROR_ALWAYS();

  GLint compile_ok = GL_FALSE;
  glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
  if (compile_ok == GL_FALSE) {
    std::cerr << "Error in " << msg << " shader\n";
    printInfoLog(res, msg);
    glDeleteShader(res);
    return 0;
  }

  return res;
}

bool
Shader::loadFromFiles(const std::string &vertexFilename,
                      const std::string &fragmentFilename)
{
  const std::string v = getStringFromFile(vertexFilename);
  const std::string s = getStringFromFile(fragmentFilename);

  return loadFromStrings(v, s);
}

bool
Shader::loadFromStrings(const std::string &vertexString,
                        const std::string &fragmentString)
{

  GLuint program = glCreateProgram();

  if (vertexString != "") {
    GLuint shader = createShader(vertexString, GL_VERTEX_SHADER, "vertex");
    if (shader == 0u) {
      return false;
    }
    glAttachShader(program, shader);
    //std::cerr << "attach vertex shader " << shader << "\n";
  }

  if (fragmentString != "") {
    GLuint shader =
      createShader(fragmentString, GL_FRAGMENT_SHADER, "fragment");
    if (shader == 0u) {
      return false;
    }
    glAttachShader(program, shader);
    //std::cerr << "attach fragment shader " << shader << "\n";
  }

  glLinkProgram(program);
  GLint link_ok = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
  if (link_ok == 0) {
    std::cerr << "ERROR: unable to link program\n";
    printInfoLog(program, "program");
    glDeleteProgram(program);
    return false;
  }

  m_programId = program;

  m_isValid = true;

  return true;
}

void
Shader::activate() const
{
  assert(m_isValid);

  glUseProgram(m_programId);
}

GLint
Shader::getUniformLocation(const char *name) const
{
  assert(m_isValid);

  return glGetUniformLocation(m_programId, name);
}

GLint
Shader::getAttribLocation(const char *name) const
{
  assert(m_isValid);

  return glGetAttribLocation(m_programId, name);
}

void
Shader::setSamplerUnit(const char *sampler, int unit) const
{
  assert(m_isValid);

  glUniform1i(getUniformLocation(sampler), unit);
}

void
Shader::dumpInfos() const
{
  GLint nbAttrs;
  glGetProgramiv(m_programId, GL_ACTIVE_ATTRIBUTES, &nbAttrs);

  const size_t SIZE = 1024;
  GLchar name[SIZE];

  GLint size;
  GLenum type;
  for (GLint i = 0; i < nbAttrs; ++i) {
    glGetActiveAttrib(m_programId, i, SIZE, nullptr, &size, &type, name);
    std::cerr << "Attrib " << glGetAttribLocation(m_programId, name) << ": "
              << name << ", " << size << ", " << type << "\n";
  }
}
