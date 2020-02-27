cmake_minimum_required(VERSION 2.8.12)

# Set version variables to empty if not specified in project() call.
IF(POLICY CMP0048)
  CMAKE_POLICY(SET CMP0048 NEW)
ENDIF(POLICY CMP0048)

# Honor visibility properties for all target types. [Policy introduced in cmake 3.3]
IF(POLICY CMP0063)
  CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

PROJECT(GrayscaleDegradator)
SET( CMAKE_COLOR_MAKEFILE ON )
SET( CMAKE_VERBOSE_MAKEFILE ON )



SET(sources
  main.cpp
  )

SET( PROGNAME GrayscaleDegradator )

SET(EXTRA_LIBS  ${QT_LIBRARIES} ${Lipsum4Qt_LIBRARIES} ${DocCreator_LIBRARIES} ${OpenCV_LIBS})

ADD_EXECUTABLE(${PROGNAME} ${sources} )
TARGET_INCLUDE_DIRECTORIES(${PROGNAME} PRIVATE ${LIPSUM4QT_INCLUDE_DIRS} ${FRAMEWORK_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROGNAME} ${EXTRA_LIBS})

# the install RPATH for bar to find foo in the install tree.
# if the install RPATH is not provided, the install bar will have none
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES INSTALL_RPATH "@loader_path/../lib")

#C++11 support
set_property(TARGET ${PROGNAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROGNAME} PROPERTY CXX_STANDARD_REQUIRED ON)

