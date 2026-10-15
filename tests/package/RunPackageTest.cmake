# Run by testInstalledPackage (cf. tests/CMakeLists.txt), as "cmake -P",
# with: BINARY_DIR CONSUMER_SOURCE_DIR WORK_DIR CXX_COMPILER CXX_FLAGS
# BUILD_TYPE OPENCV_DIR CONFIG
#
# 1. installs the component Degradations of the build in BINARY_DIR into
#    WORK_DIR/prefix: the library, its headers and its CMake package;
# 2. configures and builds the project of CONSUMER_SOURCE_DIR against that
#    prefix, with find_package() only;
# 3. runs the program it builds.

foreach(var BINARY_DIR CONSUMER_SOURCE_DIR WORK_DIR CXX_COMPILER)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "RunPackageTest.cmake: ${var} is not set")
  endif()
endforeach()

set(prefix "${WORK_DIR}/prefix")
set(build "${WORK_DIR}/build")
file(REMOVE_RECURSE "${WORK_DIR}")

function(run what)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "testInstalledPackage: ${what} failed")
  endif()
endfunction()

set(configArgs)
if(CONFIG)
  set(configArgs --config "${CONFIG}")
endif()

run("installing the component Degradations"
  "${CMAKE_COMMAND}" --install "${BINARY_DIR}" --prefix "${prefix}"
  --component Degradations ${configArgs})

run("configuring the consumer project"
  "${CMAKE_COMMAND}" -S "${CONSUMER_SOURCE_DIR}" -B "${build}"
  "-DCMAKE_PREFIX_PATH=${prefix}"
  "-DOpenCV_DIR=${OPENCV_DIR}"
  "-DCMAKE_CXX_COMPILER=${CXX_COMPILER}"
  "-DCMAKE_CXX_FLAGS=${CXX_FLAGS}"
  "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}")

run("building the consumer project"
  "${CMAKE_COMMAND}" --build "${build}" ${configArgs})

#in the build directory with one configuration, in the directory of the
#configuration with several (Visual Studio, Xcode)
set(program "")
foreach(candidate "${build}/consumer" "${build}/consumer.exe"
                  "${build}/${CONFIG}/consumer" "${build}/${CONFIG}/consumer.exe")
  if(NOT program AND EXISTS "${candidate}" AND NOT IS_DIRECTORY "${candidate}")
    set(program "${candidate}")
  endif()
endforeach()
if(NOT program)
  message(FATAL_ERROR "testInstalledPackage: the consumer program was not found in ${build}")
endif()
run("running the consumer program" "${program}")
