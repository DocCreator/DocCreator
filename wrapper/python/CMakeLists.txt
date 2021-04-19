

SET(PYBIND11_ARCHIVE_VERSION 2.6.2)
SET(PYBIND11_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/pybind11-${PYBIND11_ARCHIVE_VERSION}")
IF (NOT EXISTS ${PYBIND11_SRC_DIR})
  SET(PYBIND11_URL "https://github.com/pybind/pybind11/archive/v${PYBIND11_ARCHIVE_VERSION}.tar.gz")
  SET(PYBIND11_ARCHIVE "v${PYBIND11_ARCHIVE_VERSION}.tar.gz")
  MESSAGE(STATUS "  downloading pybind11")
  message("debug ${PYBIND11_URL}")
  message("debug ${CMAKE_CURRENT_SOURCE_DIR}/${PYBIND11_ARCHIVE}")
  FILE(DOWNLOAD ${PYBIND11_URL} ${CMAKE_CURRENT_SOURCE_DIR}/${PYBIND11_ARCHIVE} STATUS status LOG log)
  LIST(GET status 0 status_code)
  LIST(GET status 1 status_string)
  IF(NOT status_code EQUAL 0)
    MESSAGE(FATAL_ERROR "error: downloading ${PYBIND11_URL} failed: status code=${status_code} string=${status_string}, log=${log}")
  ENDIF()
  message("debug ${CMAKE_CURRENT_SOURCE_DIR}/${PYBIND11_ARCHIVE}")
  MESSAGE(STATUS "  downloading done.")
  
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_SOURCE_DIR}/${PYBIND11_ARCHIVE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
  MESSAGE(STATUS "  decompression done.")
  
ENDIF()


add_subdirectory(pybind11-${PYBIND11_ARCHIVE_VERSION})
pybind11_add_module(DocCreatorPy Degradations.cpp ndarray_converter.cpp)
target_include_directories(DocCreatorPy PRIVATE ${PYTHON_INCLUDE_DIR})
target_link_libraries(DocCreatorPy PRIVATE DocCreatorDegradations)

MESSAGE("BORIS PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
message("BORIS PYTHONLIBS_FOUND=${PYTHONLIBS_FOUND}")
message("BORIS PYTHON_INCLUDE_PATH=${PYTHON_INCLUDE_PATH}")
message("BORIS PYTHON_INCLUDE_DIR=${PYTHON_INCLUDE_DIR}")
message("BORIS PYTHON_INCLUDE_DIRS=${PYTHON_INCLUDE_DIRS}")
message("BORIS PYTHON_LIBRARIES=${PYTHON_LIBRARIES}")
message("BORIS $<TARGET_FILE:DocCreatorPy>=$<TARGET_FILE:DocCreatorPy>")
message("BORIS $<TARGET_FILE_NAME:DocCreatorPy>=$<TARGET_FILE_NAME:DocCreatorPy>")

set(DOCCREATOR_PYTHON_NAME "DocCreatorPy") #B:TODO
set(DOCCREATOR_PYTHON_VERSION "0.0") #${DocCreatorMajorVersion}.${DocCreatorMinorVersion}") #B:TODO

set(PACKAGE_DIR "PACKAGE")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/__init__.py ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/__init__.py COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/README.md COPYONLY)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/setup.py)

IF(BUILD_WITH_OSMESA)
  FIND_PACKAGE(OSMesa REQUIRED) #ugly, second find
  message("BORIS OSMesa_LIBRARIES=${OSMesa_LIBRARIES}")
  #names dynamically searched/opened in glfw-3.3.2/src/osmesa_context.c
  IF (WIN32)
    set(DST_OSMESA_NAME "OSMesa.dll")
  ELSEIF(APPLE)
    set(DST_OSMESA_NAME "libOSMesa.8.dylib")
  ELSEIF(UNIX)
    set(DST_OSMESA_NAME "libOSMesa.so.8")
  ENDIF()
  set(LIBOSMESA ", '${DST_OSMESA_NAME}'") #will be used in setup.py.in
ENDIF()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in ${CMAKE_CURRENT_BINARY_DIR}/setup.py.in)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/setup.py INPUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py.in)


IF(BUILD_WITH_OSMESA)

add_custom_target(python_package ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${DOCCREATOR_PYTHON_NAME}
  #COMMAND ${CMAKE_COMMAND} -E copy README.md ${DOCCREATOR_PYTHON_NAME}/
  COMMAND ${CMAKE_COMMAND} -E copy __init__.py ${DOCCREATOR_PYTHON_NAME}/
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DocCreatorPy> ${DOCCREATOR_PYTHON_NAME}/
  COMMAND ${CMAKE_COMMAND} -E copy ${OSMesa_LIBRARIES} ${DOCCREATOR_PYTHON_NAME}/${DST_OSMESA_NAME} #Force this name
  COMMAND ${PYTHON_EXECUTABLE} setup.py clean --all bdist_wheel
  BYPRODUCTS
  ${PACKAGE_DIR}/${DOCCREATOR_PYTHON_NAME}
  ${PACKAGE_DIR}/build
  ${PACKAGE_DIR}/dist
  ${PACKAGE_DIR}/${DOCCREATOR_PYTHON_NAME}.egg-info
  WORKING_DIRECTORY ${PACKAGE_DIR}
  )

ELSE()

add_custom_target(python_package ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${DOCCREATOR_PYTHON_NAME}
  #COMMAND ${CMAKE_COMMAND} -E copy README.md ${DOCCREATOR_PYTHON_NAME}/
  COMMAND ${CMAKE_COMMAND} -E copy __init__.py ${DOCCREATOR_PYTHON_NAME}/
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DocCreatorPy> ${DOCCREATOR_PYTHON_NAME}/
  COMMAND ${PYTHON_EXECUTABLE} setup.py clean --all bdist_wheel
  BYPRODUCTS
  ${PACKAGE_DIR}/${DOCCREATOR_PYTHON_NAME}
  ${PACKAGE_DIR}/build
  ${PACKAGE_DIR}/dist
  ${PACKAGE_DIR}/${DOCCREATOR_PYTHON_NAME}.egg-info
  WORKING_DIRECTORY ${PACKAGE_DIR}
  )

ENDIF()
