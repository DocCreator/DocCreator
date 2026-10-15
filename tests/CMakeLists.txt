
add_subdirectory(Degradations)

IF(NOT BUILD_ONLY_DEGRADATIONS)
  add_subdirectory(IO)
  add_subdirectory(Layout)
  add_subdirectory(Models)
  add_subdirectory(Utils)
ENDIF()

#Distortion3DMesh is only built when the 3D distortion is
IF(TARGET Distortion3DMesh)
  add_subdirectory(Mesh)
ENDIF()

#DocCreatorPy is only built when the python wrapper is
IF(TARGET DocCreatorPy)
  add_subdirectory(python)
ENDIF()

#The installed CMake package of the degradation library, used by a project
#outside the tree (cf. tests/package). It is not made with OSMesa.
IF(NOT BUILD_WITH_OSMESA)
  #the consumer is compiled and linked as this build is: a library built
  #with a sanitiser, coverage or fuzzing instrumentation needs its runtime
  SET(_package_test_flags ${CMAKE_CXX_FLAGS})
  IF(NOT SANITIZER STREQUAL "none")
    LIST(APPEND _package_test_flags ${SANITIZER_FLAGS})
  ENDIF()
  IF(ENABLE_COVERAGE)
    LIST(APPEND _package_test_flags --coverage)
  ENDIF()
  IF(BUILD_FUZZERS)
    LIST(APPEND _package_test_flags -fsanitize=fuzzer-no-link)
  ENDIF()
  STRING(REPLACE ";" " " _package_test_flags "${_package_test_flags}")

  ADD_TEST(NAME testInstalledPackage
    COMMAND ${CMAKE_COMMAND}
            -DBINARY_DIR=${PROJECT_BINARY_DIR}
            -DCONSUMER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/package
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/package
            -DCXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCXX_FLAGS=${_package_test_flags}
            -DBUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DOPENCV_DIR=${OpenCV_DIR}
            -DCONFIG=$<CONFIG>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/package/RunPackageTest.cmake)
ENDIF()
