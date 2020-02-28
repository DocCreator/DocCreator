cmake_minimum_required(VERSION 2.8.12)

# Set version variables to empty if not specified in project() call.
IF(POLICY CMP0048)
  CMAKE_POLICY(SET CMP0048 NEW)
ENDIF(POLICY CMP0048)

# Only interpret if() arguments as variables or keywords when unquoted. [Policy introduced in cmake 3.1]
IF(POLICY CMP0054)
  CMAKE_POLICY(SET CMP0054 NEW)
ENDIF(POLICY CMP0054)

# Honor visibility properties for all target types. [Policy introduced in cmake 3.3]
IF(POLICY CMP0063)
  CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)



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
  SET(glad_SRCS thirdparty/glad/src/glad.c)
  ADD_LIBRARY(glad STATIC ${glad_SRCS} ${glad_HDRS})
  TARGET_INCLUDE_DIRECTORIES(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/include)
  SET_TARGET_PROPERTIES(glad PROPERTIES POSITION_INDEPENDENT_CODE ON) #COMPILE_FLAGS "-fPIC")
  #Build as static library to avoid visibility problems 
  SET(GLAD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/include)
  SET(GLAD_LIBRARIES "glad")
  #SET(GLAD_LIBRARIES "${GLAD_LIBRARIES};dl") #On Linux, to use glad, we need to link to dl
ENDIF()

#Surface_mesh library
SET(surface_mesh_SRCS
    thirdparty/surface_mesh/IO.cpp
    thirdparty/surface_mesh/IO_obj.cpp
    thirdparty/surface_mesh/IO_off.cpp
    thirdparty/surface_mesh/IO_poly.cpp
    thirdparty/surface_mesh/IO_stl.cpp
    thirdparty/surface_mesh/Surface_mesh.cpp
    )
SET(surface_mesh_HDRS
    thirdparty/surface_mesh/IO.h
    thirdparty/surface_mesh/properties.h
    thirdparty/surface_mesh/Surface_mesh.h
    thirdparty/surface_mesh/types.h
    thirdparty/surface_mesh/Vector.h
)
#file(GLOB_RECURSE surface_mesh_HDRS thirdparty/surface_mesh/*.h)
ADD_LIBRARY(surface_mesh STATIC ${surface_mesh_SRCS} ${surface_mesh_HDRS})
TARGET_INCLUDE_DIRECTORIES(surface_mesh PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
SET_TARGET_PROPERTIES(surface_mesh PROPERTIES POSITION_INDEPENDENT_CODE ON) #COMPILE_FLAGS "-fPIC")
#Build as static library to avoid visibility problems 



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
  ./src/TexCoordComputation.hpp
  ./src/TexCoordComputationB.hpp
  ./src/TexCoordComputationCommon.hpp
  ./src/RandomScreenshotsParamsDialog.hpp
  ./src/Mesh2SurfaceMesh.hpp
  ./src/shader_sphere.hpp
  )

SET(DOC3D_SOURCES
  ./src/GLWidget.cpp
  ./src/MeshWindow.cpp
  ./src/TexCoordComputation.cpp
  ./src/TexCoordComputationB.cpp
  ./src/TexCoordComputationCommon.cpp
  ./src/RandomScreenshotsParamsDialog.cpp
  ./src/Mesh2SurfaceMesh.cpp
  )

QTX_WRAP_CPP(DOC3D_MOC_SOURCES ${DOC3D_HEADERS})

SET(SRCS ${DOC3D_HEADERS}  ${DOC3D_SOURCES} ${DOC3D_MOC_SOURCES} )



SET(DOC3D_INCLUDE_DIRS_LOCAL "${QT_QTCORE_INCLUDE_DIR};${QT_QTGUI_INCLUDE_DIR};${QT_QTOPENGL_INCLUDE_DIR};${OPENGL_INCLUDE_DIR};${GLAD_INCLUDE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/thirdparty;${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_BINARY_DIR}/src;${CMAKE_CURRENT_BINARY_DIR}/../..;${EIGEN_INCLUDE_DIR}")
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

SET(DOC3D_AUX_LIBRARIES "${QT_QTMAIN_LIBRARY};${QT_LIBRARIES};${OPENGL_LIBRARIES};${GLAD_LIBRARIES};surface_mesh;Distortion3DCommon;${DocCreator_LIBRARIES}")


# Cf http://www.kitware.com/blog/home/post/510
#As CMAKE_MACOSX_RPATH is defined to 1,
# Cmake  will automatically add an RPATH to this executable which is the absolute location of libDocCreator in the build tree.


ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRCS})
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${DOC3D_INCLUDE_DIRS_LOCAL})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${DOC3D_AUX_LIBRARIES})
TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )



# the install RPATH for bar to find foo in the install tree.
# if the install RPATH is not provided, the install bar will have none
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "@loader_path/../lib")


SET(DOC3D_LIBRARIES_LOCAL "${PROJECT_NAME};${DOC3D_AUX_LIBRARIES}")
SET(DOC3D_LIBRARIES "${DOC3D_LIBRARIES_LOCAL}" PARENT_SCOPE)

IF(BUILD_OTHER_PROGS_3D)

  ADD_EXECUTABLE(TestDoc3D WIN32 MACOSX_BUNDLE  ./test/testMeshWindow.cpp  ${SRCS})
  TARGET_INCLUDE_DIRECTORIES(TestDoc3D PRIVATE ${DOC3D_INCLUDE_DIRS_LOCAL} ${CMAKE_CURRENT_SOURCE_DIR}/src)
  TARGET_LINK_LIBRARIES(TestDoc3D ${DOC3D_LIBRARIES_LOCAL})
  TARGET_COMPILE_OPTIONS(TestDoc3D PRIVATE ${WARNING_FLAGS})
  SET_TARGET_PROPERTIES(TestDoc3D PROPERTIES COMPILE_FLAGS "${QTX_EXECUTABLE_COMPILE_FLAGS}")
  SET_TARGET_PROPERTIES(TestDoc3D PROPERTIES CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )

ENDIF(BUILD_OTHER_PROGS_3D)
