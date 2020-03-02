
#Download eigen library if not done already
SET(EIGEN_VERSION "3.3.7")
SET(EIGEN_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eigen-${EIGEN_VERSION}")
IF (NOT EXISTS ${EIGEN_SRC_DIR})

   SET(EIGEN_ARCHIVE "eigen-${EIGEN_VERSION}.tar.gz")
   SET(EIGEN_URL "https://gitlab.com/libeigen/eigen/-/archive/${EIGEN_VERSION}/${EIGEN_ARCHIVE}")
   IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${EIGEN_ARCHIVE})
      MESSAGE(STATUS "downloading eigen")
      FILE(DOWNLOAD ${EIGEN_URL} ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${EIGEN_ARCHIVE} STATUS status LOG log)
      LIST(GET status 0 status_code)
      LIST(GET status 1 status_string)
      IF(NOT status_code EQUAL 0)
	MESSAGE(FATAL_ERROR "error: downloading ${EIGEN_URL} failed: status code=${status_code} string=${status_string}, log=${log}")
      ENDIF()
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
SET_TARGET_PROPERTIES(IndexBufferCompression
  PROPERTIES CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )
SET_TARGET_PROPERTIES(IndexBufferCompression PROPERTIES POSITION_INDEPENDENT_CODE ON) #COMPILE_FLAGS "-fPIC")
#Build as static library to avoid visibility problems 


#VertexCacheOptimizer library
ADD_LIBRARY(VertexCacheOptimizer STATIC thirdparty/VertexCacheOptimizer/VertexCacheOptimizer.hpp thirdparty/VertexCacheOptimizer/VertexCacheOptimizer.cpp)
SET_TARGET_PROPERTIES(VertexCacheOptimizer
  PROPERTIES CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )
SET_TARGET_PROPERTIES(VertexCacheOptimizer PROPERTIES POSITION_INDEPENDENT_CODE ON) #COMPILE_FLAGS "-fPIC")
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

  FIND_PACKAGE(OSMesa REQUIRED)

  #Download GLFW library if not done already
  SET(GLFW_VERSION "3.3.2")
  SET(GLFW_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw-${GLFW_VERSION}")
  IF (NOT EXISTS ${GLFW_SRC_DIR})

     SET(GLFW_ARCHIVE "${GLFW_VERSION}.tar.gz")
     SET(GLFW_URL "https://github.com/glfw/glfw/archive/${GLFW_ARCHIVE}")
     IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${GLFW_ARCHIVE})
        MESSAGE(STATUS "downloading GLFW")
        FILE(DOWNLOAD ${GLFW_URL} ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${GLFW_ARCHIVE} STATUS status LOG log)
        LIST(GET status 0 status_code)
        LIST(GET status 1 status_string)
        IF(NOT status_code EQUAL 0)
	  MESSAGE(FATAL_ERROR "error: downloading ${GLFW_URL} failed: status code=${status_code} string=${status_string}, log=${log}")
        ENDIF()
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

  MESSAGE("DEBUUUUUGGGG2")

  SET(GLAD_GL_SRCS "${GLFW_DIR}/deps/glad/gl.h"
            "${GLFW_DIR}/deps/glad_gl.c")

  ADD_LIBRARY(GLAD_GL STATIC ${GLFW_DIR}/deps/glad/gl.h ${GLFW_DIR}/deps/glad_gl.c)
  TARGET_INCLUDE_DIRECTORIES(GLAD_GL PUBLIC ${GLFW_DIR}/deps)
  
ELSE()

  #glad library
  IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    SET(glad_SRCS thirdparty/glad/src/glad.c)
    ADD_LIBRARY(GLAD_GL STATIC ${glad_SRCS} ${glad_HDRS})
    TARGET_INCLUDE_DIRECTORIES(GLAD_GL PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/include)
    SET_TARGET_PROPERTIES(GLAD_GL PROPERTIES POSITION_INDEPENDENT_CODE ON) #COMPILE_FLAGS "-fPIC")
    #Build as static library to avoid visibility problems 
    SET(GLAD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/include)
    SET(GLAD_LIBRARIES "glad")
    #SET(GLAD_LIBRARIES "${GLAD_LIBRARIES};dl") #On Linux, to use glad, we need to link to dl
  ENDIF()

ENDIF()


SET(DISTORTION3D_MESH_HEADERS
  ./src/Mesh.hpp
  ./src/brs.hpp
  ./src/obj.hpp
)
SET(DISTORTION3D_MESH_SOURCES
  ./src/Mesh.cpp
  ./src/brs.cpp
  ./src/obj.cpp
)

SET(DISTORTION3D_MESH_INCLUDE_DIRS "${LZHAM_INCLUDE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
ADD_LIBRARY(Distortion3DMesh STATIC ${DISTORTION3D_MESH_HEADERS} ${DISTORTION3D_MESH_SOURCES})
TARGET_INCLUDE_DIRECTORIES(Distortion3DMesh PUBLIC ${DISTORTION3D_MESH_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(Distortion3DMesh IndexBufferCompression VertexCacheOptimizer ${LZHAM_LIBRARY})
TARGET_COMPILE_OPTIONS(Distortion3DMesh PRIVATE ${WARNING_FLAGS})
SET_TARGET_PROPERTIES(Distortion3DMesh PROPERTIES POSITION_INDEPENDENT_CODE ON) #COMPILE_FLAGS "-fPIC")
SET_TARGET_PROPERTIES(Distortion3DMesh PROPERTIES CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )


SET(DISTORTION3D_COMMON_HEADERS
  ./src/GLRenderer.hpp
  ./src/GLMesh.hpp
  ./src/GLCamera.hpp
  ./src/GLObject.hpp
  ./src/OpenGL.hpp
  ./src/Shader.hpp
  ./src/shader_simple.hpp
  ./src/shader_background.hpp
)

SET(DISTORTION3D_COMMON_SOURCES
  ./src/GLCamera.cpp
  ./src/GLObject.cpp
  ./src/GLMesh.cpp
  ./src/Shader.cpp
)



SET(DISTORTION3D_COMMON_INCLUDE_DIRS "${EIGEN_INCLUDE_DIR};${LZHAM_INCLUDE_DIR};${OpenCV_INCLUDE_DIRS};${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
IF(BUILD_WITH_OSMESA)
  SET(DISTORTION3D_COMMON_INCLUDE_DIRS "${DISTORTION3D_COMMON_INCLUDE_DIRS};${GLFW_DIR}/include;${GLFW_DIR}/deps")
ENDIF()

ADD_LIBRARY(Distortion3DCommon STATIC ${DISTORTION3D_COMMON_HEADERS} ${DISTORTION3D_COMMON_SOURCES})
TARGET_INCLUDE_DIRECTORIES(Distortion3DCommon PUBLIC ${DISTORTION3D_COMMON_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(Distortion3DCommon Distortion3DMesh ${OpenCV_LIBS})
TARGET_COMPILE_OPTIONS(Distortion3DCommon PRIVATE ${WARNING_FLAGS})
SET_TARGET_PROPERTIES(Distortion3DCommon PROPERTIES POSITION_INDEPENDENT_CODE ON) #COMPILE_FLAGS "-fPIC")
SET_TARGET_PROPERTIES(Distortion3DCommon PROPERTIES CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )
IF(BUILD_WITH_OSMESA)
  TARGET_LINK_LIBRARIES(Distortion3DCommon Distortion3DMesh VertexCacheOptimizer ${OpenCV_LIBS})
  TARGET_COMPILE_DEFINITIONS(Distortion3DCommon PUBLIC USE_NATIVE_OSMESA=1)
  TARGET_LINK_LIBRARIES(Distortion3DCommon GLAD_GL)
ELSE()
  IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    TARGET_LINK_LIBRARIES(Distortion3DCommon GLAD_GL)
  ENDIF()

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
  TARGET_COMPILE_OPTIONS(Distortion3D PRIVATE ${WARNING_FLAGS})
  TARGET_LINK_LIBRARIES(Distortion3D glfw GLAD_GL Distortion3DCommon)
  SET_TARGET_PROPERTIES(Distortion3D PROPERTIES CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )
  SET_TARGET_PROPERTIES(Distortion3D PROPERTIES POSITION_INDEPENDENT_CODE ON) #COMPILE_FLAGS "-fPIC")

ENDIF()
