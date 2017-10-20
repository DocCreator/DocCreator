cmake_minimum_required(VERSION 2.8.12)

PROJECT(GrayscaleDegradator)
SET( CMAKE_COLOR_MAKEFILE ON )
SET( CMAKE_VERBOSE_MAKEFILE ON )

include_directories(${LIPSUM4QT_INCLUDE_DIRS})
include_directories(${FRAMEWORK_INCLUDE_DIRS})


SET(sources
  main.cpp
  )

SET( PROGNAME GrayscaleDegradator )

SET(EXTRA_LIBS  ${QT_LIBRARIES} ${Lipsum4Qt_LIBRARIES} ${DocCreator_LIBRARIES} ${OpenCV_LIBS})

ADD_EXECUTABLE(${PROGNAME} ${sources} )
TARGET_LINK_LIBRARIES(${PROGNAME} ${EXTRA_LIBS})

# the install RPATH for bar to find foo in the install tree.
# if the install RPATH is not provided, the install bar will have none
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES INSTALL_RPATH "@loader_path/../lib")

#C++11 support
set_property(TARGET ${PROGNAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROGNAME} PROPERTY CXX_STANDARD_REQUIRED ON)

