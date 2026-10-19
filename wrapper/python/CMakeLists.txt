

INCLUDE(DownloadVerified)

SET(PYBIND11_ARCHIVE_VERSION 3.0.3)
#SHA256 of v${PYBIND11_ARCHIVE_VERSION}.tar.gz: change it when the version changes
SET(PYBIND11_SHA256 "787459e1e186ee82001759508fefa408373eae8a076ffe0078b126c6f8f0ec5e")
FETCH_ARCHIVE_VERIFIED("https://github.com/pybind/pybind11/archive/v${PYBIND11_ARCHIVE_VERSION}.tar.gz"
                       ${PYBIND11_SHA256}
                       "pybind11 v${PYBIND11_ARCHIVE_VERSION}"
                       "pybind11-${PYBIND11_ARCHIVE_VERSION}"
                       PYBIND11_SRC_DIR)


# Python is looked for here, and not left to pybind11: it is added with
# ADD_SUBDIRECTORY() below, so the variables its own FIND_PACKAGE() sets
# live in its directory scope and are empty here. PYTHON_EXECUTABLE was
# read from here anyway, which made the NumPy lookup below run "" -c ...
# and fail with a message naming no interpreter at all.
# NumPy is a component: numpy/ndarrayobject.h is included by
# ndarray_converter.cpp.
SET(PYBIND11_FINDPYTHON ON)
FIND_PACKAGE(Python REQUIRED COMPONENTS Interpreter Development.Module NumPy)
MESSAGE(STATUS "Using Python ${Python_VERSION}: ${Python_EXECUTABLE}")
MESSAGE(STATUS "NumPy include directory: ${Python_NumPy_INCLUDE_DIRS}")

add_subdirectory(${PYBIND11_SRC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/pybind11)
pybind11_add_module(DocCreatorPy Degradations.cpp ndarray_converter.cpp)
set_target_properties(DocCreatorPy PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS OFF)
target_include_directories(DocCreatorPy PRIVATE ${Python_NumPy_INCLUDE_DIRS})

target_link_libraries(DocCreatorPy PRIVATE DocCreatorDegradations)
#DocCreatorPy.__version__, which said "dev": nothing defined VERSION_INFO
target_compile_definitions(DocCreatorPy PRIVATE VERSION_INFO="${PROJECT_VERSION}")

set(DOCCREATOR_PYTHON_NAME "DocCreatorPy") #B:TODO
#the version of the project, cf. PROJECT() in the root CMakeLists.txt
set(DOCCREATOR_PYTHON_VERSION "${PROJECT_VERSION}")

set(PACKAGE_DIR "PACKAGE")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/__init__.py ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/__init__.py COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/README.md COPYONLY)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/setup.py)

IF(BUILD_WITH_OSMESA)
  FIND_PACKAGE(OSMesa REQUIRED) #ugly, second find
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
#B: the package did not depend on NumPy, which its module imports: it was
# installed without it, and failed on import. A module built with NumPy 1
# does not import with NumPy 2; one built with NumPy 2 imports with NumPy
# 1.19 and later (the API that NumPy 2 targets by default).
if(Python_NumPy_VERSION VERSION_LESS 2)
  string(REGEX MATCH "^[0-9]+\\.[0-9]+" _numpy_major_minor "${Python_NumPy_VERSION}")
  set(DOCCREATOR_NUMPY_REQUIREMENT "numpy>=${_numpy_major_minor},<2")
else()
  set(DOCCREATOR_NUMPY_REQUIREMENT "numpy>=1.19")
endif()
message(STATUS "NumPy ${Python_NumPy_VERSION}: the package requires ${DOCCREATOR_NUMPY_REQUIREMENT}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml.in ${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml.in)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/setup.py INPUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py.in)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/pyproject.toml INPUT ${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml.in)

# Build the wheel via the PEP 517 frontend (python -m build), avoiding
# direct invocation of setup.py which is deprecated by setuptools.
# We use --no-isolation so the build reuses the already-available
# setuptools/wheel from the current Python environment instead of
# creating an isolated venv and redownloading them.
set(_python_build_cmd ${Python_EXECUTABLE} -m build --wheel --no-isolation --outdir dist .)

#B: python_package was a target of ALL with no output: the wheel was made
# again at every build, even with nothing changed. It is now made when the
# module or the files of the package change, and leaves a stamp.
set(_copy_osmesa "")
IF(BUILD_WITH_OSMESA)
  set(_copy_osmesa COMMAND ${CMAKE_COMMAND} -E copy ${OSMesa_LIBRARIES} ${DOCCREATOR_PYTHON_NAME}/${DST_OSMESA_NAME}) #Force this name
ENDIF()

set(_wheel_stamp ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/wheel.stamp)
add_custom_command(
  OUTPUT ${_wheel_stamp}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${DOCCREATOR_PYTHON_NAME}
  COMMAND ${CMAKE_COMMAND} -E copy __init__.py ${DOCCREATOR_PYTHON_NAME}/
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DocCreatorPy> ${DOCCREATOR_PYTHON_NAME}/
  ${_copy_osmesa}
  COMMAND ${CMAKE_COMMAND} -E rm -rf build dist ${DOCCREATOR_PYTHON_NAME}.egg-info
  COMMAND ${_python_build_cmd}
  COMMAND ${CMAKE_COMMAND} -E touch ${_wheel_stamp}
  DEPENDS DocCreatorPy
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/__init__.py
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/README.md
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/setup.py
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_DIR}/pyproject.toml
  BYPRODUCTS
    ${PACKAGE_DIR}/${DOCCREATOR_PYTHON_NAME}
    ${PACKAGE_DIR}/build
    ${PACKAGE_DIR}/dist
    ${PACKAGE_DIR}/${DOCCREATOR_PYTHON_NAME}.egg-info
  WORKING_DIRECTORY ${PACKAGE_DIR}
  COMMENT "Making the wheel of the python module"
  VERBATIM
  )
add_custom_target(python_package ALL DEPENDS ${_wheel_stamp})
