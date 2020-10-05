

SET(PYBIND11_ARCHIVE_VERSION 2.5.0)
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
#target_include_directories
target_link_libraries(DocCreatorPy PRIVATE DocCreatorDegradations)
