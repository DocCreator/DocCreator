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



FIND_PROGRAM(CCACHE_PROGRAM ccache)
IF(CCACHE_PROGRAM)
  SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  message(STATUS "Using ccache for building")
ENDIF(CCACHE_PROGRAM)


PROJECT(DocCreator)
SET( CMAKE_COLOR_MAKEFILE ON )
#SET( CMAKE_VERBOSE_MAKEFILE ON )


#CMake 2.8.12 & up. 
# Cf http://www.kitware.com/blog/home/post/510
# enable @rpath in the install name for any shared library being built
# note: it is planned that a future version of CMake will enable this by default
SET(CMAKE_MACOSX_RPATH 1) 



SET(DocCreatorMajorVersion 1)
SET(DocCreatorMinorVersion 2)
SET(DocCreatorVersion "${DocCreatorMajorVersion}.${DocCreatorMinorVersion}")


####################################################################
## Set build_type if not set


IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF()



########################################################################

SET(CMAKE_MODULE_PATH  "${CMAKE_SOURCE_DIR}/cmake/Modules;${CMAKE_MODULE_PATH}" )

########################################################################

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake VERBATIM)


########################################################################



FIND_PACKAGE(OpenCV REQUIRED core imgproc highgui photo)
if( OpenCV_FOUND )
  if (OpenCV_VERSION VERSION_GREATER 3.0)
    FIND_PACKAGE(OpenCV QUIET REQUIRED core imgproc highgui imgcodecs photo)
  else()
    message(STATUS "Found OpenCV: ${OpenCV_DIR} (found version \"${OpenCV_VERSION}\")")
  endif()
  list( APPEND ThirdParty_LIBS ${OpenCV_LIBS} )
endif( OpenCV_FOUND )



########################################################################
### Hide some variables
IF(APPLE)
  MARK_AS_ADVANCED(CMAKE_OSX_ARCHITECTURES)
  MARK_AS_ADVANCED(CMAKE_OSX_DEPLOYMENT_TARGET)
  MARK_AS_ADVANCED(CMAKE_OSX_SYSROOT)
ENDIF(APPLE)

########################################################################

SET(CMAKE_CXX_VISIBILITY_PRESET hidden)


IF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
  # Force to always compile with W3
  SET(WARNING_FLAGS "/W3")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" 
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" 
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  SET(WARNING_FLAGS -Wall -Wextra)
ENDIF()



OPTION(BUILD_ONLY_DEGRADATIONS "Build only degradations library" OFF)
if (NOT BUILD_ONLY_DEGRADATIONS)
  message(STATUS "Build only degradations DISABLED (you can build only degradations library with '-DBUILD_ONLY_DEGRADATIONS=ON' cmake option)")
endif (NOT BUILD_ONLY_DEGRADATIONS)


IF(NOT BUILD_ONLY_DEGRADATIONS)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOUIC ON)
  FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Widgets Xml XmlPatterns PrintSupport OpenGL Network)
  SET(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Xml Qt5::XmlPatterns Qt5::PrintSupport Qt5::OpenGL Qt5::Network)
  SET(USE_QT5 TRUE)
ENDIF()

OPTION(BUILD_OTHER_PROGS "Build other programs than DocCreator" OFF)
if (NOT BUILD_OTHER_PROGS)
  message(STATUS "Build other programs DISABLED (you can activate other programs build with '-DBUILD_OTHER_PROGS=ON' cmake option)")
endif (NOT BUILD_OTHER_PROGS)

OPTION(BUILD_OTHER_PROGS_3D "Build 3D programs" OFF)
if (NOT BUILD_OTHER_PROGS_3D)
  message(STATUS "Build other 3D programs DISABLED (you can activate other 3D programs build with '-DBUILD_OTHER_PROGS_3D=ON' cmake option)")
endif (NOT BUILD_OTHER_PROGS_3D)


OPTION(BUILD_TESTING "Build tests" OFF)

if (BUILD_TESTING)
  ENABLE_TESTING()
  include(CTest)
  message(STATUS "Build test files ENABLED")
  if (CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wextra -pedantic -W -Wshadow -Wunused-variable  -Wunused-parameter -Wunused-function        -Wunused  -Wno-long-long -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
  endif(CMAKE_COMPILER_IS_GNUCXX)
  if (CMAKE_COMPILER_IS_GNUCC)
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -Wall -Wextra -W -Wno-long-long -pedantic -fprofile-arcs -ftest-coverage")
  endif (CMAKE_COMPILER_IS_GNUCC)
else (BUILD_TESTING)
  message(STATUS "Build test files DISABLED (you can activate unit tests with '-DBUILD_TESTING=ON' cmake option)")
endif (BUILD_TESTING)


SET(CATCH_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/thirdparty/catch)

SET(FRAMEWORK_PATH ${PROJECT_SOURCE_DIR}/framework)
SET(FRAMEWORK_INCLUDE_DIRS ${FRAMEWORK_PATH}/src) #for framwork_global.h

SET(DEGRADATIONS_PATH ${PROJECT_SOURCE_DIR}/framework/src/Degradations)

ADD_SUBDIRECTORY(${DEGRADATIONS_PATH})

ADD_SUBDIRECTORY(tools)

IF(NOT BUILD_ONLY_DEGRADATIONS)

  SET(LIPSUM4QT_PATH ${PROJECT_SOURCE_DIR}/libs/Lipsum4Qt)
  SET(LIPSUM4QT_INCLUDE_DIRS ${LIPSUM4QT_PATH}/src)
  SET(Lipsum4Qt_LIBRARIES Lipsum4Qt)

  SET(FRAMEWORK_LIBRARIES DocCreatorFramework)
  SET(DocCreator_LIBRARIES DocCreatorFramework)

  SET(SOFTWARE_PATH ${PROJECT_SOURCE_DIR}/software)


  ADD_SUBDIRECTORY(${LIPSUM4QT_PATH})
  ADD_SUBDIRECTORY(${FRAMEWORK_PATH})

  ADD_SUBDIRECTORY(${SOFTWARE_PATH})

  ADD_SUBDIRECTORY(bundlers)

ENDIF(NOT BUILD_ONLY_DEGRADATIONS)


if (BUILD_TESTING)
  add_subdirectory (${PROJECT_SOURCE_DIR}/tests)
endif (BUILD_TESTING)

