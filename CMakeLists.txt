cmake_minimum_required(VERSION 2.8.12)


FIND_PROGRAM(CCACHE_PROGRAM ccache)
IF(CCACHE_PROGRAM)
  SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
ENDIF(CCACHE_PROGRAM)


PROJECT(DocCreator)
SET( CMAKE_COLOR_MAKEFILE ON )
#SET( CMAKE_VERBOSE_MAKEFILE ON )

IF(${CMAKE_VERSION} VERSION_GREATER 3.0.0)
  cmake_policy(SET CMP0063 NEW)
ENDIF()

#CMake 2.8.12 & up. 
# Cf http://www.kitware.com/blog/home/post/510
# enable @rpath in the install name for any shared library being built
# note: it is planned that a future version of CMake will enable this by default
SET(CMAKE_MACOSX_RPATH 1) 



SET(DocCreatorMajorVersion 1)
SET(DocCreatorMinorVersion 0)
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


FIND_PACKAGE(QtX REQUIRED)

FIND_PACKAGE(OpenCV REQUIRED)
if( OpenCV_FOUND )
  list( APPEND ThirdParty_LIBS ${OpenCV_LIBS} )
  include_directories( ${OpenCV_INCLUDE_DIRS} )
endif( OpenCV_FOUND )

FIND_PACKAGE(OpenGL REQUIRED)



########################################################################
### Hide some variables
IF(APPLE)
  MARK_AS_ADVANCED(CMAKE_OSX_ARCHITECTURES)
  MARK_AS_ADVANCED(CMAKE_OSX_DEPLOYMENT_TARGET)
  MARK_AS_ADVANCED(CMAKE_OSX_SYSROOT)
ENDIF(APPLE)

########################################################################

SET(CMAKE_CXX_VISIBILITY_PRESET hidden)

OPTION(BUILD_OTHER_PROGS "Build other programs than DocCreator" OFF)

OPTION(BUILD_TESSERACT "Build tesseract from sources" OFF)

IF(NOT BUILD_TESSERACT)
  FIND_PACKAGE(Tesseract QUIET)
  #if Tesseract is found, Tesseract_TESSDATA_PARENT_DIR should be set
  IF(NOT Tesseract_FOUND)
    SET(BUILD_TESSERACT ON)
  ENDIF(NOT Tesseract_FOUND)
ENDIF(NOT BUILD_TESSERACT)


IF(BUILD_TESSERACT)
  
  message("will build leptonica and tesseract from sources")

  IF(NOT MSVC)
    SET(LEPTONICA_LIBRARY "${PROJECT_BINARY_DIR}/leptonica/lib/${CMAKE_STATIC_LIBRARY_PREFIX}leptonica${CMAKE_STATIC_LIBRARY_SUFFIX}")
    SET(TESSERACT_LIBRARY "${PROJECT_BINARY_DIR}/tesseract/lib/${CMAKE_STATIC_LIBRARY_PREFIX}libtesseract${CMAKE_STATIC_LIBRARY_SUFFIX}")
  ELSE(NOT MSVC)

    SET(LEPTONICA_VERSION 1.74.4)
    SET(TESSERACT_VERSION 305)
    #TODO: these version numbers should be extracted automatically...

    #set(MY_DEFINITION 
    #  $<$<CONFIG:Debug>:definition_for_debug>
    #  $<$<CONFIG:RelWithDebInfo>:definition_for_rel_with_debug>
    #  $<$<CONFIG:Release>:definition_for_release> 
    #  $<$<CONFIG:MinSizeRel>:definition_for_tight_release>
    #  )
    
    #set(DEBUG_EXT "$<$<CONFIG:Debug>:d>")
    set(DEBUG_EXT "")
    IF(NOT CMAKE_CONFIGURATION_TYPES)
      #mono-config, e.g., ninja
      IF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebugInfo")
	set(DEBUG_EXT "d")
      ENDIF()
    ENDIF()
    
    

    #SET(LEPTONICA_LIBRARY "${PROJECT_BINARY_DIR}/leptonica/lib/${CMAKE_STATIC_LIBRARY_PREFIX}lept${LEPTONICA_VERSION}${DEBUG_EXT}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    SET(LEPTONICA_LIBRARY "${PROJECT_BINARY_DIR}/leptonica/lib/${CMAKE_STATIC_LIBRARY_PREFIX}leptonica-${LEPTONICA_VERSION}${DEBUG_EXT}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    #SET(TESSERACT_LIBRARY "${PROJECT_BINARY_DIR}/tesseract/lib/${CMAKE_STATIC_LIBRARY_PREFIX}tesseract${TESSERACT_VERSION}${DEBUG_EXT}${CMAKE_STATIC_LIBRARY_SUFFIX};${PROJECT_BINARY_DIR}/tesseract/lib/${CMAKE_STATIC_LIBRARY_PREFIX}tessopt${CMAKE_STATIC_LIBRARY_SUFFIX};Ws2_32")
    SET(TESSERACT_LIBRARY "${PROJECT_BINARY_DIR}/tesseract/lib/${CMAKE_STATIC_LIBRARY_PREFIX}tesseract${TESSERACT_VERSION}${DEBUG_EXT}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  ENDIF(NOT MSVC)
  
  include(ExternalProject)

  #build static library with -DSTATIC=ON
  ExternalProject_Add(leptonica
    URL ${PROJECT_SOURCE_DIR}/thirdparty/leptonica
    PREFIX ${PROJECT_BINARY_DIR}/leptonica
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/leptonica -DSTATIC=ON
    BUILD_BYPRODUCTS ${LEPTONICA_LIBRARY}
    )
  #The client does: #include<leptonica/allheaders.h>
  #I had to modify leptonica CMakeLists.txt to correctly install headers, in leptonica directory

  ExternalProject_Get_Property(leptonica binary_dir)
  ExternalProject_Get_Property(leptonica source_dir)
  ExternalProject_Get_Property(leptonica install_dir)
  set(leptonica_prefix "${install_dir}")

  set(Lept_INCLUDE_DIRS "${install_dir}/include;${install_dir}/include/leptonica") #(not needed to install tesseract), just later
  set(lept_lib_dirs "${install_dir}/lib")

  #library is in ${binary_dir}/src
  #headers are in ${source_dir}/src

  ADD_LIBRARY(leptonicaLib STATIC IMPORTED GLOBAL)
  SET_TARGET_PROPERTIES(leptonicaLib PROPERTIES IMPORTED_LOCATION ${LEPTONICA_LIBRARY})
  ADD_DEPENDENCIES(leptonicaLib leptonica)


  ExternalProject_Add(tesseract
    DEPENDS leptonica
    URL ${PROJECT_SOURCE_DIR}/thirdparty/tesseract
    PREFIX ${PROJECT_BINARY_DIR}/tesseract
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DLeptonica_DIR=${install_dir}/cmake -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/tesseract -DSTATIC=ON
    BUILD_BYPRODUCTS ${TESSERACT_LIBRARY}
    )

  ExternalProject_Get_Property(tesseract binary_dir)
  ExternalProject_Get_Property(tesseract source_dir)
  ExternalProject_Get_Property(tesseract install_dir)
  set(tesseract_prefix "${install_dir}")
  set(Tesseract_INCLUDE_DIRS "${install_dir}/include;${install_dir}/include/tesseract") 
  set(tess_lib_dirs "${install_dir}/lib") 

  ADD_LIBRARY(tesseractLib STATIC IMPORTED GLOBAL)
  SET_TARGET_PROPERTIES(tesseractLib PROPERTIES IMPORTED_LOCATION ${TESSERACT_LIBRARY})
  ADD_DEPENDENCIES(tesseractLib tesseract)

  #TODO: the following libraries are already searched by leptonica.
  # CMakeLists for leptonica should provide a way to retrieve them !

  FIND_PACKAGE(ZLIB QUIET)  #ZLIB brought by PNG on linux but not on OSX 
  IF(ZLIB_FOUND)
    SET(OTHER_LIBS ${ZLIB_LIBRARIES})
  ENDIF()

  FIND_PACKAGE(PNG QUIET)
  IF(PNG_FOUND)
    SET(OTHER_LIBS "${PNG_LIBRARIES};${OTHER_LIBS}")
  ENDIF()

  FIND_PACKAGE(JPEG QUIET)
  IF(JPEG_FOUND)
    SET(OTHER_LIBS "${JPEG_LIBRARIES};${OTHER_LIBS}")
  ENDIF()

  FIND_PACKAGE(TIFF QUIET)
  IF(TIFF_FOUND)
    SET(OTHER_LIBS "${TIFF_LIBRARIES};${OTHER_LIBS}")
  ENDIF()

  FIND_PACKAGE(GIF QUIET)
  IF(GIF_FOUND)
    SET(OTHER_LIBS "${GIF_LIBRARIES};${OTHER_LIBS}")
  ENDIF()
  
  FIND_PACKAGE(PkgConfig QUIET)
  IF (PKG_CONFIG_FOUND)
    pkg_check_modules(WEBP QUIET libwebp)
    IF(WEBP_FOUND)
      SET(OTHER_LIBS "${WEBP_LIBRARIES};${OTHER_LIBS}")
    ENDIF()
    pkg_check_modules(JP2K QUIET libopenjp2)
    IF(JP2K_FOUND)
      SET(OTHER_LIBS "${JP2K_LIBRARIES};${OTHER_LIBS}")
    ENDIF()
  ENDIF()

  SET(Lept_LIBRARIES "${LEPTONICA_LIBRARY};${OTHER_LIBS}")
  SET(Tesseract_LIBRARIES ${TESSERACT_LIBRARY})
  IF(MSVC)
    SET(Tesseract_LIBRARIES "${TESSERACT_LIBRARY};Ws2_32")
  ENDIF()

  #download tessdata
  SET(TESSDATA_URL http://github.com/tesseract-ocr/tessdata/raw/master)
  IF(NOT EXISTS ${PROJECT_SOURCE_DIR}/thirdparty/tessdata/eng.traineddata)
    FILE(DOWNLOAD ${TESSDATA_URL}/eng.traineddata ${PROJECT_SOURCE_DIR}/thirdparty/tessdata/eng.traineddata)
    FILE(DOWNLOAD ${TESSDATA_URL}/fra.traineddata ${PROJECT_SOURCE_DIR}/thirdparty/tessdata/fra.traineddata)
  ENDIF()
  
  #install tessdata
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/thirdparty/tessdata DESTINATION share/DocCreator)

  set(Tesseract_TESSDATA_PARENT_DIR "") #empty to be found dynamically

ENDIF()

SET(Tesseract_LIBRARIES "${Tesseract_LIBRARIES};${Lept_LIBRARIES}")

include_directories(${Lept_INCLUDE_DIRS})
include_directories(${Tesseract_INCLUDE_DIRS})



SET(LIPSUM4QT_PATH ${PROJECT_SOURCE_DIR}/libs/Lipsum4Qt)
SET(LIPSUM4QT_INCLUDE_DIRS ${LIPSUM4QT_PATH}/src)
SET(Lipsum4Qt_LIBRARIES Lipsum4Qt)

SET(FRAMEWORK_PATH ${PROJECT_SOURCE_DIR}/framework)
SET(FRAMEWORK_INCLUDE_DIRS ${FRAMEWORK_PATH}/src)
SET(FRAMEWORK_LIBRARIES DocCreatorFramework)
SET(DocCreator_LIBRARIES DocCreatorFramework)

SET(SOFTWARE_PATH ${PROJECT_SOURCE_DIR}/software)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ADD_SUBDIRECTORY(${LIPSUM4QT_PATH})
ADD_SUBDIRECTORY(${FRAMEWORK_PATH})
ADD_SUBDIRECTORY(${SOFTWARE_PATH})

ADD_SUBDIRECTORY(bundlers)


