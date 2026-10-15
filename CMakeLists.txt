cmake_minimum_required(VERSION 3.16)

# CMP0048 (project() manages the VERSION variables), CMP0054 (if() only
# interprets unquoted arguments as variables) and CMP0063 (the visibility
# properties are honoured for every target type) were set to NEW here, each
# guarded by IF(POLICY ...). They were introduced in CMake 3.0, 3.1 and 3.3,
# so the minimum above already gives all three.

FIND_PROGRAM(CCACHE_PROGRAM ccache)
IF(CCACHE_PROGRAM)
  SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  message(STATUS "Using ccache for building")
ENDIF(CCACHE_PROGRAM)


PROJECT(DocCreator)
SET( CMAKE_COLOR_MAKEFILE ON )
#SET( CMAKE_VERBOSE_MAKEFILE ON )

# Baseline C++ standard for every target in the project (Qt6 requires at
# least C++17; we align the rest of the tree on the same standard).
IF(NOT DEFINED CMAKE_CXX_STANDARD)
  SET(CMAKE_CXX_STANDARD 17)
ENDIF()
SET(CMAKE_CXX_STANDARD_REQUIRED YES)
SET(CMAKE_CXX_EXTENSIONS OFF)


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


########################################################################
## Sanitisers

SET(SANITIZER "none" CACHE STRING
  "Build with a sanitiser: none, address, undefined, address+undefined, thread, leak")
SET_PROPERTY(CACHE SANITIZER PROPERTY STRINGS
  none address undefined address+undefined thread leak)

# The flags go on every target of the tree, compiling and linking, which is
# what the sanitisers require. AddressSanitizer and ThreadSanitizer cannot
# be combined, hence one value and not a list of options.
IF(NOT SANITIZER STREQUAL "none")
  IF(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"
        OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"
        OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
    MESSAGE(FATAL_ERROR "SANITIZER=${SANITIZER} needs gcc or clang, not ${CMAKE_CXX_COMPILER_ID}")
  ENDIF()

  IF(SANITIZER STREQUAL "address")
    SET(SANITIZER_FLAGS -fsanitize=address)
  ELSEIF(SANITIZER STREQUAL "undefined")
    SET(SANITIZER_FLAGS -fsanitize=undefined)
  ELSEIF(SANITIZER STREQUAL "address+undefined")
    SET(SANITIZER_FLAGS -fsanitize=address,undefined)
  ELSEIF(SANITIZER STREQUAL "thread")
    SET(SANITIZER_FLAGS -fsanitize=thread)
  ELSEIF(SANITIZER STREQUAL "leak")
    SET(SANITIZER_FLAGS -fsanitize=leak)
  ELSE()
    MESSAGE(FATAL_ERROR "SANITIZER=${SANITIZER} is not one of: none address undefined address+undefined thread leak")
  ENDIF()

  # -fno-omit-frame-pointer and -g for readable stacks, -O1 so that the
  # build stays usable; without -fno-optimize-sibling-calls the frames of
  # the tail calls are missing from them.
  LIST(APPEND SANITIZER_FLAGS
    -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O1)

  ADD_COMPILE_OPTIONS(${SANITIZER_FLAGS})
  ADD_LINK_OPTIONS(${SANITIZER_FLAGS})

  MESSAGE(STATUS "Building with sanitiser: ${SANITIZER}")
  MESSAGE(STATUS "  suppressions and recommended options: cmake/sanitizers/README.txt")
ENDIF()

########################################################################
## Coverage

# B: -fprofile-arcs -ftest-coverage used to be added to every gcc Debug
# build with tests, and nothing read what they produce. They are now asked
# for, and the "coverage" target turns them into a report.
OPTION(ENABLE_COVERAGE "Instrument the build for gcov, and add a 'coverage' target that runs the tests and writes a report (needs gcc, gcovr and BUILD_TESTING)" OFF)

IF(ENABLE_COVERAGE)
  IF(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    MESSAGE(FATAL_ERROR "ENABLE_COVERAGE needs gcc, whose gcov the report is made with, not ${CMAKE_CXX_COMPILER_ID}")
  ENDIF()
  IF(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    MESSAGE(WARNING "ENABLE_COVERAGE with CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}: optimisations inline and move code, and the lines counted are then not the lines written. Use -DCMAKE_BUILD_TYPE=Debug.")
  ENDIF()

  # --coverage is -fprofile-arcs -ftest-coverage when compiling, and links
  # the gcov runtime when linking. On every target of the tree; not on
  # leptonica and tesseract, which are separate projects.
  # -fprofile-update=atomic: the degradations run on several threads, and
  # counters updated without it race, which gcov then reads as negative
  # counts (gcc bug 68080) and gcovr refuses.
  ADD_COMPILE_OPTIONS(--coverage -fprofile-update=atomic)
  ADD_LINK_OPTIONS(--coverage)

  MESSAGE(STATUS "Building with coverage instrumentation: 'make coverage' writes the report")
ENDIF()

# Turns the misuse of QList::at() and std::vector::operator[] into a clean
# abort instead of an out-of-bounds read. Debug only: it costs a bounds
# check on every access.
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  ADD_COMPILE_DEFINITIONS($<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>)
ENDIF()



OPTION(BUILD_ONLY_DEGRADATIONS "Build only degradations library" OFF)
if (NOT BUILD_ONLY_DEGRADATIONS)
  message(STATUS "Build only degradations DISABLED (you can build only degradations library with '-DBUILD_ONLY_DEGRADATIONS=ON' cmake option)")
endif (NOT BUILD_ONLY_DEGRADATIONS)


IF(NOT BUILD_ONLY_DEGRADATIONS)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOUIC ON)
  FIND_PACKAGE(Qt6 COMPONENTS Widgets Xml PrintSupport OpenGL OpenGLWidgets Network Concurrent) #XmlPatterns
  if (NOT Qt6_FOUND)
    FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Widgets Xml XmlPatterns PrintSupport OpenGL Network Concurrent)
    SET(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Xml Qt5::XmlPatterns Qt5::PrintSupport Qt5::OpenGL Qt5::Network Qt5::Concurrent)
    SET(QT_VERSION_MAJOR 5)
  else()
    SET(QT_LIBRARIES Qt6::Core Qt6::Widgets Qt6::Xml Qt6::PrintSupport Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Network Qt6::Concurrent) #Qt6::XmlPatterns
    SET(QT_VERSION_MAJOR 6)
  endif()
  message(STATUS "Found Qt${QT_VERSION_MAJOR}")

ENDIF()

OPTION(BUILD_OTHER_PROGS "Build other programs than DocCreator" OFF)
if (NOT BUILD_OTHER_PROGS)
  message(STATUS "Build other programs DISABLED (you can activate other programs build with '-DBUILD_OTHER_PROGS=ON' cmake option)")
endif (NOT BUILD_OTHER_PROGS)

OPTION(BUILD_OTHER_PROGS_3D "Build 3D programs" OFF)
if (NOT BUILD_OTHER_PROGS_3D)
  message(STATUS "Build other 3D programs DISABLED (you can activate other 3D programs build with '-DBUILD_OTHER_PROGS_3D=ON' cmake option)")
endif (NOT BUILD_OTHER_PROGS_3D)


OPTION(BUILD_PYTHON_WRAPPER "Build python wrapper" OFF)

OPTION(BUILD_TESTING "Build tests" OFF)

OPTION(BUILD_FUZZERS "Build the libFuzzer targets of fuzz/ (needs clang)" OFF)
# The fuzz targets steer by the code coverage of what they call, which is
# in the libraries: those have to be instrumented too, without the
# libFuzzer main(), which only the targets link.
# B: without this, only the few lines of each fuzz_*.cpp were
# instrumented, and the fuzzers mutated blind -- fuzz_brs reached 32 edges.
IF(BUILD_FUZZERS)
  ADD_COMPILE_OPTIONS(-fsanitize=fuzzer-no-link)
ENDIF()

if (BUILD_TESTING)
  ENABLE_TESTING()
  include(CTest)
  message(STATUS "Build test files ENABLED")

  #Add a "runTests" target to print tests outputs on failure
  IF(CMAKE_CONFIGURATION_TYPES)
    ADD_CUSTOM_TARGET(runTests COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure --build-config "$<CONFIGURATION>")
  ELSE(CMAKE_CONFIGURATION_TYPES)
    ADD_CUSTOM_TARGET(runTests COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure)
  ENDIF(CMAKE_CONFIGURATION_TYPES)

  if (CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wextra -pedantic -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused  -Wno-long-long -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings")
  endif(CMAKE_COMPILER_IS_GNUCXX)
  if (CMAKE_COMPILER_IS_GNUCC)
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -Wall -Wextra -W -Wno-long-long -pedantic")
  endif (CMAKE_COMPILER_IS_GNUCC)
else (BUILD_TESTING)
  message(STATUS "Build test files DISABLED (you can activate unit tests with '-DBUILD_TESTING=ON' cmake option)")
endif (BUILD_TESTING)


IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/thirdparty/catch)
ENDIF()

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

if (BUILD_PYTHON_WRAPPER)
  add_subdirectory (wrapper/python)
endif(BUILD_PYTHON_WRAPPER)

if (BUILD_TESTING)
  add_subdirectory (${PROJECT_SOURCE_DIR}/tests)
endif (BUILD_TESTING)

IF(ENABLE_COVERAGE)
  IF(NOT BUILD_TESTING)
    MESSAGE(FATAL_ERROR "ENABLE_COVERAGE needs BUILD_TESTING=ON: the report is of what the tests run")
  ENDIF()
  FIND_PROGRAM(GCOVR_EXECUTABLE gcovr)
  IF(NOT GCOVR_EXECUTABLE)
    MESSAGE(FATAL_ERROR "ENABLE_COVERAGE needs gcovr (https://gcovr.com, 'pip install gcovr' or the distribution's package)")
  ENDIF()
  #the gcov of the compiler: another version's misreads the counters
  GET_FILENAME_COMPONENT(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
  STRING(REGEX MATCH "[0-9]+" COMPILER_MAJOR "${CMAKE_CXX_COMPILER_VERSION}")
  FIND_PROGRAM(GCOV_EXECUTABLE NAMES gcov-${COMPILER_MAJOR} gcov HINTS ${COMPILER_DIR})
  IF(NOT GCOV_EXECUTABLE)
    MESSAGE(FATAL_ERROR "ENABLE_COVERAGE needs gcov")
  ENDIF()
  CMAKE_HOST_SYSTEM_INFORMATION(RESULT COVERAGE_JOBS QUERY NUMBER_OF_LOGICAL_CORES)

  ADD_CUSTOM_TARGET(coverage
    COMMAND ${CMAKE_COMMAND}
            -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
            -DBINARY_DIR=${PROJECT_BINARY_DIR}
            -DGCOVR=${GCOVR_EXECUTABLE}
            -DGCOV=${GCOV_EXECUTABLE}
            -DCTEST=${CMAKE_CTEST_COMMAND}
            -DJOBS=${COVERAGE_JOBS}
            -P ${PROJECT_SOURCE_DIR}/cmake/coverage/Coverage.cmake
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    COMMENT "Running the tests and writing the coverage report"
    USES_TERMINAL)
ENDIF()

if (BUILD_FUZZERS)
  add_subdirectory (${PROJECT_SOURCE_DIR}/fuzz)
endif (BUILD_FUZZERS)

# The API documentation, with Doxygen when it is found: "make doc", then
# doc/html/index.html in the build directory. Cf doc/Doxyfile.in.
FIND_PACKAGE(Doxygen QUIET)
IF(DOXYGEN_FOUND)
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in
                 ${PROJECT_BINARY_DIR}/doc/Doxyfile @ONLY)
  ADD_CUSTOM_TARGET(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doc/Doxyfile
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generating the API documentation with Doxygen"
    VERBATIM)
ENDIF()

