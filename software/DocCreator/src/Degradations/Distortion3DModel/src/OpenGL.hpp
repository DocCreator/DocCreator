#ifndef OPENGL_HPP
#define OPENGL_HPP

#if defined(_WIN32) || defined(_WIN64)
#include <glad/glad.h>
#else

#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#endif //_WIN32 || _WIN64

//#define GL_GLEXT_PROTOTYPES
//#ifdef __APPLE__
//
//#include <OpenGL/gl3.h>
////#include <OpenGL/glu.h>
//
//#else
//
//#if defined(_WIN32) || defined(_WIN64)
//#include <windows.h>
//#endif //_WIN32 || _WIN64
//
//
////#include <GL/gl.h>
////#include <GL/glu.h>
//#include <GL/gl.h>
//
//#endif

#ifndef MAX_NB_GL_ERRORS
#define MAX_NB_GL_ERRORS 200
#endif

#if defined(NDEBUG)
#define GL_CHECK_ERROR() ;
#else
#define GL_CHECK_ERROR() GL_CHECK_ERROR_ALWAYS()
#endif

#ifdef GL_TABLE_TOO_LARGE
#define GL_PRINT_ERROR(error)                                                  \
  do {                                                                         \
    switch ((error)) {                                                         \
      case GL_INVALID_ENUM:                                                    \
        fprintf(stderr, "GL_INVALID_ENUM\n");                                  \
        break;                                                                 \
      case GL_INVALID_VALUE:                                                   \
        fprintf(stderr, "GL_INVALID_VALUE\n");                                 \
        break;                                                                 \
      case GL_INVALID_OPERATION:                                               \
        fprintf(stderr, "GL_INVALID_OPERATION\n");                             \
        break;                                                                 \
      case GL_OUT_OF_MEMORY:                                                   \
        fprintf(stderr, "GL_OUT_OF_MEMORY\n");                                 \
        break;                                                                 \
      case GL_TABLE_TOO_LARGE:                                                 \
        fprintf(stderr, "GL_TABLE_TOO_LARGE\n");                               \
        break;                                                                 \
      default:                                                                 \
        fprintf(stderr, "0x%X\n", (error));                                    \
        break;                                                                 \
    }                                                                          \
  } while (0)
#else
#define GL_PRINT_ERROR(error)                                                  \
  do {                                                                         \
    switch ((error)) {                                                         \
      case GL_INVALID_ENUM:                                                    \
        fprintf(stderr, "GL_INVALID_ENUM\n");                                  \
        break;                                                                 \
      case GL_INVALID_VALUE:                                                   \
        fprintf(stderr, "GL_INVALID_VALUE\n");                                 \
        break;                                                                 \
      case GL_INVALID_OPERATION:                                               \
        fprintf(stderr, "GL_INVALID_OPERATION\n");                             \
        break;                                                                 \
      case GL_OUT_OF_MEMORY:                                                   \
        fprintf(stderr, "GL_OUT_OF_MEMORY\n");                                 \
        break;                                                                 \
      default:                                                                 \
        fprintf(stderr, "0x%X\n", (error));                                    \
        break;                                                                 \
    }                                                                          \
  } while (0)

#endif

#if defined(__GNUC__)
#define GL_CHECK_ERROR_ALWAYS()                                                \
  do {                                                                         \
    unsigned int i = 0;                                                        \
    GLenum error = glGetError();                                               \
    while ((error != GL_NO_ERROR) && (i < MAX_NB_GL_ERRORS)) {                 \
      fprintf(                                                                 \
        stderr,                                                                \
        "OpenGL: Error:  **********************************************\n");   \
      fprintf(stderr,                                                          \
              "OpenGL: Error: %s(%d): %s: ",                                   \
              __FILE__,                                                        \
              __LINE__,                                                        \
              __PRETTY_FUNCTION__);                                            \
      GL_PRINT_ERROR(error);                                                   \
      fprintf(                                                                 \
        stderr,                                                                \
        "OpenGL: Error:  **********************************************\n");   \
      error = glGetError();                                                    \
      ++i;                                                                     \
    }                                                                          \
  } while (0)
#else
#define GL_CHECK_ERROR_ALWAYS()                                                \
  do {                                                                         \
    unsigned int i = 0;                                                        \
    GLenum error = glGetError();                                               \
    while ((error != GL_NO_ERROR) && (i < MAX_NB_GL_ERRORS)) {                 \
      fprintf(stderr, "OpenGL: Error: %s(%d): ", __FILE__, __LINE__);          \
      GL_PRINT_ERROR(error);                                                   \
      error = glGetError();                                                    \
      ++i;                                                                     \
    }                                                                          \
  } while (0)
#endif

#endif /* ! OPENGL_HPP */
