cmake_minimum_required(VERSION 2.8.12)

# Honor visibility properties for all target types. [Policy introduced in cmake 3.3]
IF(POLICY CMP0063)
  CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

PROJECT(DocCreatorDegradator)
SET( CMAKE_COLOR_MAKEFILE ON )
SET( CMAKE_VERBOSE_MAKEFILE ON )

include_directories(${LIPSUM4QT_INCLUDE_DIRS})
include_directories(${FRAMEWORK_INCLUDE_DIRS})

configure_file(paths.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/paths.hpp @ONLY)

SET(sources
  main.cpp
  )

SET( PROGNAME DocCreatorDegradator )

SET(EXTRA_LIBS  ${QT_LIBRARIES} ${Lipsum4Qt_LIBRARIES} ${DocCreator_LIBRARIES} ${OpenCV_LIBS} ${LIBNAME})

ADD_EXECUTABLE(${PROGNAME} ${sources} )
TARGET_INCLUDE_DIRECTORIES(${PROGNAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
TARGET_LINK_LIBRARIES(${PROGNAME} ${EXTRA_LIBS})

# the install RPATH for bar to find foo in the install tree.
# if the install RPATH is not provided, the install bar will have none
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES INSTALL_RPATH "@loader_path/../lib")

#C++11 support
set_property(TARGET ${PROGNAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROGNAME} PROPERTY CXX_STANDARD_REQUIRED ON)


IF(UNIX)
  install(TARGETS ${PROGNAME} RUNTIME DESTINATION bin)
ENDIF(UNIX)
IF(WIN32)
  INSTALL(TARGETS ${PROGNAME} RUNTIME DESTINATION programs COMPONENT applications)
ENDIF(WIN32)

