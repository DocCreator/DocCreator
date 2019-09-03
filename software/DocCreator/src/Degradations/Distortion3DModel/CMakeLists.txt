cmake_minimum_required(VERSION 2.8.12)

SET(CMAKE_MODULE_PATH  "${CMAKE_SOURCE_DIR}/cmake/Modules;${CMAKE_MODULE_PATH}" )

SET(TARGET Doc3D)
project(${TARGET})



#CMake 2.8.12 & up. 
# Cf http://www.kitware.com/blog/home/post/510
# enable @rpath in the install name for any shared library being built
# note: it is planned that a future version of CMake will enable this by default
SET(CMAKE_MACOSX_RPATH 1) 


#glad library
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/include)
  file(GLOB_RECURSE glad_SRCS thirdparty/glad/src/*.c)
  ADD_LIBRARY(glad STATIC ${glad_SRCS} ${glad_HDRS})
  SET_TARGET_PROPERTIES(glad PROPERTIES COMPILE_FLAGS "-fPIC")
  #Build as static library to avoid visibility problems 
  SET(GLAD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/include)
  SET(GLAD_LIBRARIES "glad")
  #SET(GLAD_LIBRARIES "${GLAD_LIBRARIES};dl") #On Linux, to use glad, we need to link to dl
ENDIF()

#Surface_mesh library
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
file(GLOB_RECURSE surface_mesh_SRCS thirdparty/surface_mesh/*.cpp)
#file(GLOB_RECURSE surface_mesh_HDRS thirdparty/surface_mesh/*.h)
ADD_LIBRARY(surface_mesh STATIC ${surface_mesh_SRCS} ${surface_mesh_HDRS})
SET_TARGET_PROPERTIES(surface_mesh PROPERTIES COMPILE_FLAGS "-fPIC")
#Build as static library to avoid visibility problems 


#IndexBufferCompression library
FILE(GLOB_RECURSE
  IndexBufferCompression_HDRS
  thirdparty/IndexBufferCompression/*.h 
  )
FILE(GLOB_RECURSE
  IndexBufferCompression_SRCS
  thirdparty/IndexBufferCompression/*.cpp 
  )
ADD_LIBRARY(IndexBufferCompression STATIC ${IndexBufferCompression_SRCS} ${IndexBufferCompression_HDRS})
SET_TARGET_PROPERTIES(IndexBufferCompression PROPERTIES COMPILE_FLAGS "-fPIC")
#Build as static library to avoid visibility problems 


#lzham library
#Build as static library to avoid visibility problems 

SET(BUILD_SHARED_LIBS_SAVE ${BUILD_SHARED_LIBS})
SET(CMAKE_CXX_FLAGS_SAVE  "${CMAKE_CXX_FLAGS}")

SET(BUILD_SHARED_LIBS OFF CACHE BOOL "build shared libs?" FORCE)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC")

ADD_SUBDIRECTORY(thirdparty/lzham_codec)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS_SAVE}")
SET(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_SAVE} CACHE BOOL "build shared libs?" FORCE)

FIND_PATH(LZHAM_INCLUDE_DIR lzham.h ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lzham_codec/include NO_CMAKE_PATH NO_CMAKE_SYSTEM_PATH)
#FIND_LIBRARY(LZHAMCOMP_LIBRARY lzhamcomp ${CMAKE_BINARY_DIR}/thirdparty/lzham_codec/lzhamcomp)
#FIND_LIBRARY(LZHAMDECOMP_LIBRARY lzhamdecomp ${CMAKE_BINARY_DIR}/thirdparty/lzham_codec/lzhamdecomp)
#FIND_LIBRARY(LZHAM_LIBRARY lzhamdll ${CMAKE_BINARY_DIR}/thirdparty/lzham_codec/lzhamdll)
SET(LZHAM_LIBRARY lzhamdll)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/thirdparty/lzham_codec/lzhamdll)



set(${PROJECT_NAME}_MAJOR_VERSION 0)
set(${PROJECT_NAME}_MINOR_VERSION 0)
set(${PROJECT_NAME}_PATCH_VERSION 1)

#
IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF()



## Qt4 or Qt5
FIND_PACKAGE(QtX)

#OpenGL
#B: is it really necessary ? should be found by Qt !?
FIND_PACKAGE(OpenGL REQUIRED) 
IF(NOT OPENGL_FOUND)
  MESSAGE(SEND_ERROR "Failed to find OpenGL.")
ENDIF(NOT OPENGL_FOUND)

SET(EIGEN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eigen-3.3.7)


# Generate GLWidget.hpp from GLWidget.hpp.in according to Qt version.
# (Hack: see GLWidget.hpp.in for details)
if (Qt5Widgets_FOUND)
  if (Qt5Widgets_VERSION VERSION_LESS 5.4.0)
    set(QGL_CLASS QGLWidget)
    message(STATUS "Use QGLWidget as OpenGL widget.")
  else()
    set(QGL_CLASS QOpenGLWidget)
    message(STATUS "Use QOpenGLWidget as OpenGL widget.")
  endif()
else()
    set(QGL_CLASS QGLWidget)
    message(STATUS "Use QGLWidget as OpenGL widget.")
endif(Qt5Widgets_FOUND)
configure_file(src/GLWidget.hpp.in src/GLWidget.hpp @ONLY)


SET(DOC3D_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/src/GLWidget.hpp #./src/GLWidget.hpp
  ./src/MeshWindow.hpp
  ./src/GLMesh.hpp
  ./src/GLCamera.hpp
  ./src/GLObject.hpp
  ./src/OpenGL.hpp
  ./src/Shader.hpp
  ./src/Mesh.hpp
  ./src/obj.hpp
  ./src/brs.hpp
  ./src/TexCoordComputation.hpp
  ./src/TexCoordComputationB.hpp
  ./src/TexCoordComputationCommon.hpp
  ./src/RandomScreenshotsParamsDialog.hpp
  ./src/Mesh2SurfaceMesh.hpp
  ./thirdparty/VertexCacheOptimizer/VertexCacheOptimizer.hpp
  )

SET(DOC3D_SOURCES
  ./src/GLCamera.cpp
  ./src/GLObject.cpp
  ./src/GLMesh.cpp
  ./src/Shader.cpp
  ./src/GLWidget.cpp
  ./src/MeshWindow.cpp
  ./src/Mesh.cpp
  ./src/obj.cpp
  ./src/brs.cpp
  ./src/TexCoordComputation.cpp
  ./src/TexCoordComputationB.cpp
  ./src/TexCoordComputationCommon.cpp
  ./src/RandomScreenshotsParamsDialog.cpp
  ./src/Mesh2SurfaceMesh.cpp
  ./thirdparty/VertexCacheOptimizer/VertexCacheOptimizer.cpp
  )

QTX_WRAP_CPP(DOC3D_MOC_SOURCES ${DOC3D_HEADERS})

SET(SRCS ${DOC3D_HEADERS}  ${DOC3D_SOURCES} ${DOC3D_MOC_SOURCES} )

#INCLUDE_DIRECTORIES("${QT_QTCORE_INCLUDE_DIR};${QT_QTGUI_INCLUDE_DIR};${QT_QTOPENGL_INCLUDE_DIR};${OPENGL_INCLUDE_DIR};${GLEW_INCLUDE_DIRS}")

#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
#INCLUDE_DIRECTORIES(${LZHAM_INCLUDE_DIR})
#INCLUDE_DIRECTORIES(${EIGEN_INCLUDE_DIR})


SET(DOC3D_INCLUDE_DIRS_LOCAL "${QT_QTCORE_INCLUDE_DIR};${QT_QTGUI_INCLUDE_DIR};${QT_QTOPENGL_INCLUDE_DIR};${OPENGL_INCLUDE_DIR};${GLAD_INCLUDE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/thirdparty;${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_BINARY_DIR}/src;;${CMAKE_CURRENT_BINARY_DIR}/../..;${LZHAM_INCLUDE_DIR};${EIGEN_INCLUDE_DIR}")
#Added "${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_BINARY_DIR}/src" for generated GLWidget.hpp compilation
# and ";${CMAKE_CURRENT_BINARY_DIR}/../.." to be able to still do : #include "Degradations/Distortion3DModel/src/GLWidget.hpp"

SET(DOC3D_INCLUDE_DIRS "${DOC3D_INCLUDE_DIRS_LOCAL}" PARENT_SCOPE)

#${OPENGL_LIBRARIES} lists X11, Xext, ...
# In Tulip CMakeLists.txt :
#IF(WIN32)
#  SET(GL_LDFLAGS "-l${OPENGL_gl_LIBRARY} -l${OPENGL_glu_LIBRARY} ${GLEW_LIBRARY}")
#ELSE(WIN32)
#  SET(GL_LDFLAGS "${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${GLEW_LIBRARY}")
#ENDIF(WIN32)


ADD_DEFINITIONS(-DSRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}") #PROJECT_SOURCE_DIR}")

SET(DOC3D_AUX_LIBRARIES "${QT_QTMAIN_LIBRARY};${QT_LIBRARIES};${OPENGL_LIBRARIES};${GLAD_LIBRARIES};IndexBufferCompression;${LZHAM_LIBRARY};surface_mesh;${DocCreator_LIBRARIES}")


# Cf http://www.kitware.com/blog/home/post/510
#As CMAKE_MACOSX_RPATH is defined to 1,
# Cmake  will automatically add an RPATH to this executable which is the absolute location of libDocCreator in the build tree.

INCLUDE_DIRECTORIES(${DOC3D_INCLUDE_DIRS_LOCAL})

ADD_LIBRARY(${PROJECT_NAME} STATIC  ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${DOC3D_AUX_LIBRARIES})

#C++11 (for all executables)
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  EXECUTE_PROCESS(COMMAND "${CMAKE_CXX_COMPILER} -dumpversion" OUTPUT_VARIABLE GCC_VERSION)
  IF (GCC_VERSION VERSION_LESS 4.7)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  ENDIF()
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
ENDIF()
#VS2013 supports c++11 without any additional flags


# the install RPATH for bar to find foo in the install tree.
# if the install RPATH is not provided, the install bar will have none
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "@loader_path/../lib")


SET(DOC3D_LIBRARIES_LOCAL "${PROJECT_NAME};${DOC3D_AUX_LIBRARIES}")
SET(DOC3D_LIBRARIES "${DOC3D_LIBRARIES_LOCAL}" PARENT_SCOPE)

OPTION(BUILD_OTHER_PROGS_3D "Build 3D programs" OFF)

IF(BUILD_OTHER_PROGS_3D)

  #ADD_EXECUTABLE(testMeshAdjacentTriangles WIN32 MACOSX_BUNDLE src/testMeshAdjacentTriangles.cpp ${SRCS})
  #TARGET_LINK_LIBRARIES(testMeshAdjacentTriangles ${QT_QTMAIN_LIBRARY} ${QT_LIBRARIES} ${OPENGL_LIBRARIES} IndexBufferCompression ${LZHAM_LIBRARY})
  ##SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${QTX_EXECUTABLE_COMPILE_FLAGS}")
  #SET_PROPERTY(TARGET testMeshAdjacentTriangles PROPERTY CXX_STANDARD 11)
  #SET_PROPERTY(TARGET testMeshAdjacentTriangles PROPERTY CXX_STANDARD_REQUIRED ON)

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src)

  ADD_EXECUTABLE(TestDoc3D WIN32 MACOSX_BUNDLE  ./test/testMeshWindow.cpp  ${SRCS})
  TARGET_LINK_LIBRARIES(TestDoc3D ${DOC3D_LIBRARIES_LOCAL})
  SET_TARGET_PROPERTIES(TestDoc3D PROPERTIES COMPILE_FLAGS "${QTX_EXECUTABLE_COMPILE_FLAGS}")
  #SET_PROPERTY(TARGET TestDoc3D PROPERTY CXX_STANDARD 11)
  #SET_PROPERTY(TARGET TestDoc3D PROPERTY CXX_STANDARD_REQUIRED ON)

  ADD_EXECUTABLE(convertBRSOBJ WIN32 MACOSX_BUNDLE  src/convertBRSOBJ.cpp  ${SRCS})
  TARGET_LINK_LIBRARIES(convertBRSOBJ ${DOC3D_LIBRARIES_LOCAL})

  ADD_EXECUTABLE(removeTexCoords WIN32 MACOSX_BUNDLE src/removeTexCoords.cpp ${SRCS})
  TARGET_LINK_LIBRARIES(removeTexCoords ${DOC3D_LIBRARIES_LOCAL})
  #SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${QTX_EXECUTABLE_COMPILE_FLAGS}")
  #SET_PROPERTY(TARGET removeTexCoords PROPERTY CXX_STANDARD 11)
  #SET_PROPERTY(TARGET removeTexCoords PROPERTY CXX_STANDARD_REQUIRED ON)

ENDIF(BUILD_OTHER_PROGS_3D)
