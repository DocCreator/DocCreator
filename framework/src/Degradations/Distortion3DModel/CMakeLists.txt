
#Download eigen library if not done already
SET(EIGEN_VERSION "3.3.7")
SET(EIGEN_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eigen-${EIGEN_VERSION}")
IF (NOT EXISTS ${EIGEN_SRC_DIR})

   SET(EIGEN_ARCHIVE "eigen-${EIGEN_VERSION}.tar.gz")
   SET(EIGEN_URL "https://gitlab.com/libeigen/eigen/-/archive/${EIGEN_VERSION}/${EIGEN_ARCHIVE}")
   IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${EIGEN_ARCHIVE})
      MESSAGE(STATUS "downloading eigen")
      FILE(DOWNLOAD ${EIGEN_URL} ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${EIGEN_ARCHIVE})
   ENDIF()

   EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${EIGEN_ARCHIVE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty )

ENDIF()

SET(EIGEN_INCLUDE_DIR ${EIGEN_SRC_DIR})


#IndexBufferCompression library
SET(IndexBufferCompression_HDRS
  thirdparty/IndexBufferCompression/indexbuffercompressionformat.h
  thirdparty/IndexBufferCompression/indexbuffercompression.h
  thirdparty/IndexBufferCompression/indexbufferdecodetables.h
  thirdparty/IndexBufferCompression/indexbufferdecompression.h
  thirdparty/IndexBufferCompression/indexcompressionconstants.h
  thirdparty/IndexBufferCompression/readbitstream.h
  thirdparty/IndexBufferCompression/writebitstream.h
  )
SET(IndexBufferCompression_SRCS
  thirdparty/IndexBufferCompression/indexbuffercompression.cpp
  thirdparty/IndexBufferCompression/indexbufferdecompression.cpp
  )
ADD_LIBRARY(IndexBufferCompression STATIC ${IndexBufferCompression_SRCS} ${IndexBufferCompression_HDRS})
SET_TARGET_PROPERTIES(IndexBufferCompression PROPERTIES COMPILE_FLAGS "-fPIC")
#Build as static library to avoid visibility problems 


#VertexCacheOptimizer library
ADD_LIBRARY(VertexCacheOptimizer STATIC thirdparty/VertexCacheOptimizer/VertexCacheOptimizer.hpp thirdparty/VertexCacheOptimizer/VertexCacheOptimizer.cpp)
SET_TARGET_PROPERTIES(VertexCacheOptimizer PROPERTIES COMPILE_FLAGS "-fPIC")
#Build as static library to avoid visibility problems 



#lzham library
#Build as static library to avoid visibility problems 
SET(BUILD_SHARED_LIBS_SAVE ${BUILD_SHARED_LIBS})
SET(CMAKE_CXX_FLAGS_SAVE "${CMAKE_CXX_FLAGS}")
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "build shared libs?" FORCE)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC")
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lzham_codec)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVE}")
SET(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_SAVE} CACHE BOOL "build shared libs?" FORCE)
FIND_PATH(LZHAM_INCLUDE_DIR lzham.h ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lzham_codec/include NO_CMAKE_PATH NO_CMAKE_SYSTEM_PATH)
SET(LZHAM_LIBRARY lzhamdll)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/thirdparty/lzham_codec/lzhamdll)



IF(BUILD_WITH_OSMESA)

#REM: must be available before Distortion3DCommon
#because OpenGL.hpp will include glad/glad.h

  #Download GLFW library if not done already
  SET(GLFW_VERSION "3.3.2")
  SET(GLFW_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw-${GLFW_VERSION}")
  IF (NOT EXISTS ${GLFW_SRC_DIR})

     SET(GLFW_ARCHIVE "${GLFW_VERSION}.tar.gz")
     SET(GLFW_URL "https://github.com/glfw/glfw/archive/${GLFW_ARCHIVE}")
     IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${GLFW_ARCHIVE})
        MESSAGE(STATUS "downloading eigen")
        FILE(DOWNLOAD ${GLFW_URL} ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${GLFW_ARCHIVE})
     ENDIF()

     EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${GLFW_ARCHIVE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty )

  ENDIF()

  ADD_DEFINITIONS(-DUSE_NATIVE_OSMESA=1)

  SET(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
  SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
  SET(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
  SET(GLFW_USE_OSMESA ON CACHE BOOL "Use OSMesa for offscreeen context creation")

  SET(GLFW_DIR ${GLFW_SRC_DIR})

  ADD_SUBDIRECTORY(${GLFW_DIR})


  set(GLAD_GL_SRCS "${GLFW_DIR}/deps/glad/gl.h"
            "${GLFW_DIR}/deps/glad_gl.c")

  ADD_LIBRARY(GLAD_GL STATIC ${GLFW_DIR}/deps/glad/gl.h ${GLFW_DIR}/deps/glad_gl.c)
  TARGET_INCLUDE_DIRECTORIES(GLAD_GL PUBLIC ${GLFW_DIR}/deps)
  
ENDIF()





SET(DISTORTION3D_COMMON_HEADERS
  ./src/GLRenderer.hpp
  ./src/GLMesh.hpp
  ./src/GLCamera.hpp
  ./src/GLObject.hpp
  ./src/OpenGL.hpp
  ./src/Shader.hpp
  ./src/Mesh.hpp
  ./src/brs.hpp
  ./src/obj.hpp
  ./src/shader_simple.hpp
  ./src/shader_background.hpp
)

SET(DISTORTION3D_COMMON_SOURCES
  ./src/GLCamera.cpp
  ./src/GLObject.cpp
  ./src/GLMesh.cpp
  ./src/Shader.cpp
  ./src/Mesh.cpp
  ./src/brs.cpp
  ./src/obj.cpp
)

SET(SRCS_COMMON ${DISTORTION3D_COMMON_HEADERS} ${DISTORTION3D_COMMON_SOURCES} )

SET(DISTORTION3D_COMMON_INCLUDE_DIRS "${EIGEN_INCLUDE_DIR};${LZHAM_INCLUDE_DIR};${OpenCV_INCLUDE_DIRS};${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
IF(BUILD_WITH_OSMESA)
  SET(DISTORTION3D_COMMON_INCLUDE_DIRS "${DISTORTION3D_COMMON_INCLUDE_DIRS};${GLFW_DIR}/deps")
ENDIF()

ADD_LIBRARY(Distortion3DCommon STATIC ${SRCS_COMMON})
TARGET_INCLUDE_DIRECTORIES(Distortion3DCommon PUBLIC ${DISTORTION3D_COMMON_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(Distortion3DCommon IndexBufferCompression VertexCacheOptimizer ${LZHAM_LIBRARY} ${OpenCV_LIBS})
SET_TARGET_PROPERTIES(Distortion3DCommon PROPERTIES COMPILE_FLAGS "-fPIC")
IF(BUILD_WITH_OSMESA)
TARGET_LINK_LIBRARIES(Distortion3DCommon IndexBufferCompression VertexCacheOptimizer ${LZHAM_LIBRARY} ${OpenCV_LIBS})
  TARGET_COMPILE_DEFINITIONS(Distortion3DCommon PUBLIC USE_NATIVE_OSMESA=1)
  TARGET_LINK_LIBRARIES(Distortion3DCommon GLAD_GL)
ENDIF()




IF(BUILD_WITH_OSMESA)

  SET(DISTORTION3D_HEADERS
      ./src/GLRenderer.hpp
      )

  SET(DISTORTION3D_SOURCES
  ./src/GLRenderer.cpp
  )

  SET(SRCS ${DISTORTION3D_HEADERS} ${DISTORTION3D_SOURCES} )

  SET(DISTORTION3D_INCLUDE_DIRS "${DISTORTION3D_COMMON_INCLUDE_DIRS}" ${GLFW_DIR}/deps/)

  ADD_LIBRARY(Distortion3D STATIC ${SRCS})
  TARGET_INCLUDE_DIRECTORIES(Distortion3D PUBLIC ${DISTORTION3D_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(Distortion3D glfw GLAD_GL Distortion3DCommon)
  SET_TARGET_PROPERTIES(Distortion3D PROPERTIES COMPILE_FLAGS "-fPIC")

ENDIF()
