
# CMake>=3.10: CMake processes **regular** and ``GENERATED`` source files in ``AUTOMOC`` and ``AUTOUIC``
IF(POLICY CMP0071)
  CMAKE_POLICY(SET CMP0071 NEW)
ENDIF(POLICY CMP0071)

# CMake>=3.13: ``UseSWIG`` generates now standard target names. 
IF(POLICY CMP0078)
  CMAKE_POLICY(SET CMP0078 NEW)
ENDIF(POLICY CMP0078)

# CMake>=3.14: ``UseSWIG`` honors ``SWIG_MODULE_NAME`` via ``-module`` flag.
IF(POLICY CMP0086)
  CMAKE_POLICY(SET CMP0086 NEW)
ENDIF(POLICY CMP0086)


FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

# Find Python Interpreter
# prefer Python 3.7 over 3.6 over ...
# user can overwrite it e.g.:
# cmake -H. -Bbuild -DBUILD_PYTHON=ON -DPython_ADDITIONAL_VERSIONS="2.7"
set(Python_ADDITIONAL_VERSIONS "3.8;3.7;3.6;3.5;2.7" CACHE STRING "Python to use for binding")
##set(Python_ADDITIONAL_VERSIONS "2.7" CACHE STRING "Python to use for binding")
find_package(PythonInterp REQUIRED)
#find_package(Python REQUIRED COMPONENTS Interpreter Development)
message(STATUS "Found Python: ${PYTHON_EXECUTABLE} (found version \"${PYTHON_VERSION_STRING}\")")
message("BORIS Python_Development_FOUND: ${Python_Development_FOUND}")
message("BORIS PYTHON_INCLUDE_DIRS: ${PYTHON_INCLUDE_DIRS}")
message("BORIS Python_INCLUDE_DIRS: ${Python_INCLUDE_DIRS}")
message("BORIS Python_LIBRARIES: ${Python_LIBRARIES}")
# Find Python Library
# Force PythonLibs to find the same version than the python interpreter (or nothing).
##set(Python_ADDITIONAL_VERSIONS "${PYTHON_VERSION_STRING}")
#set(Python_ADDITIONAL_VERSIONS "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
#find_package(PythonLibs REQUIRED)
#message(STATUS "Found Python Include: ${PYTHON_INCLUDE_DIRS} (found version \"${PYTHONLIBS_VERSION_STRING}\")")

#DEBUG
find_package(PythonLibs)
message("BORIS PYTHONLIBS_FOUND=${PYTHONLIBS_FOUND}")
message("BORIS PYTHON_INCLUDE_PATH=${PYTHON_INCLUDE_PATH}")
message("BORIS PYTHON_INCLUDE_DIR=${PYTHON_INCLUDE_DIR}")
message("BORIS PYTHON_INCLUDE_DIRS=${PYTHON_INCLUDE_DIRS}")
message("BORIS PYTHON_LIBRARIES=${PYTHON_LIBRARIES}")


#Dans cmake-swig :
#find_package(Python REQUIRED COMPONENTS Interpreter Development)
#if(Python_VERSION VERSION_GREATER_EQUAL 3)
#  list(APPEND CMAKE_SWIG_FLAGS "-py3;-DPY3")
#endif()







SET_SOURCE_FILES_PROPERTIES(Degradations.i PROPERTIES CPLUSPLUS ON)

#SET(EXTRA_LIBS DocCreatorDegradations ${OpenCV_LIBS})
SET(EXTRA_LIBS DocCreatorDegradations)

swig_add_library(Degradations TYPE MODULE LANGUAGE python OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/DocCreatorPython SOURCES Degradations.i Degradations.cpp)
#swig_add_module(Degradations python Degradations.i Degradations.cpp)

#Dans cmake-swig :
#set_property(SOURCE foo.i PROPERTY CPLUSPLUS ON)
#set_property(SOURCE foo.i PROPERTY SWIG_MODULE_NAME pyFoo)
#swig_add_library(pyFoo TYPE SHARED LANGUAGE python OUTPUT_DIR ${CMAKE_BINARY_DIR}/python/${PROJECT_NAME}/Foo SOURCES foo.i)
# Pour macosx :
#set_property(TARGET pyFoo APPEND PROPERTY LINK_FLAGS "-flat_namespace -undefined suppress") !!!!!
#Il change aussi les RPATH

message("CMAKE_CURRENT_BINARY_DIR/DocCreatorPython=${CMAKE_CURRENT_BINARY_DIR}/DocCreatorPython")

get_target_property(DEGINCLUDES DocCreatorDegradations INCLUDE_DIRECTORIES)

message("DEGRADATIONS_INCLUDE_DIRS=${DEGRADATIONS_INCLUDE_DIRS}")
message("DEGINCLUDES=${DEGINCLUDES}")


target_include_directories(${SWIG_MODULE_Degradations_REAL_NAME}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${PYTHON_INCLUDE_DIRS}
	#${Python_INCLUDE_DIRS} #cmake>=3.12.1 si find_package(Python REQUIRED COMPONENTS Interpreter Development)

	${CMAKE_CURRENT_BINARY_DIR}/DocCreatorPython
	
	${DEGRADATIONS_INCLUDE_DIRS}
	)
#set_property(TARGET Degradations PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)

if(APPLE)
    swig_link_libraries(Degradations ${EXTRA_LIBS})
    SET_TARGET_PROPERTIES(${SWIG_MODULE_Degradations_REAL_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
else()
  #    swig_link_libraries(Degradations ${EXTRA_LIBS} ${PYTHON_LIBRARIES})
  swig_link_libraries(Degradations ${EXTRA_LIBS})
endif()

SET_TARGET_PROPERTIES(${SWIG_MODULE_Degradations_REAL_NAME} PROPERTIES CXX_STANDARD 11)
SET_TARGET_PROPERTIES(${SWIG_MODULE_Degradations_REAL_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)

message("SWIG_MODULE_Degradations_REAL_NAME=${SWIG_MODULE_Degradations_REAL_NAME}") # == Degradations

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DocCreator.py ${CMAKE_CURRENT_BINARY_DIR}/DocCreatorPython/DocCreator.py COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/__init__.py ${CMAKE_CURRENT_BINARY_DIR}/DocCreatorPython/__init__.py COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_CURRENT_BINARY_DIR}/DocCreatorPython/README.md COPYONLY)

set(DOCCREATOR_PYTHON_NAME "DocCreator") #B:TODO
set(DOCCREATOR_PYTHON_VERSION "0.0") #${DocCreatorMajorVersion}.${DocCreatorMinorVersion}") #B:TODO
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in ${CMAKE_CURRENT_BINARY_DIR}/DocCreatorPython/setup.py)


MESSAGE("PYTHON OSMesa_LIBRARIES=${OSMesa_LIBRARIES}")
IF(BUILD_WITH_OSMESA)
MESSAGE("PYTHON BUILD_WITH_OSMESA OSMesa_LIBRARIES=${OSMesa_LIBRARIES}")
ENDIF(BUILD_WITH_OSMESA)
#BORIS: can we get it with get_property ???


#BORIS:TODO: only if python enabled

MESSAGE("PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")

# Find if python module MODULE_NAME is available,
# if not install it to the Python user install directory.
function(search_python_module MODULE_NAME)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "import ${MODULE_NAME}; print(${MODULE_NAME}.__version__)"
    RESULT_VARIABLE _RESULT
    OUTPUT_VARIABLE MODULE_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(${_RESULT} STREQUAL "0")
    message(STATUS "Found python module: ${MODULE_NAME} (found version \"${MODULE_VERSION}\")")
  else()
    message(WARNING "Can't find python module \"${MODULE_NAME}\". It will be user-installed using pip...")
    execute_process(
      COMMAND ${PYTHON_EXECUTABLE} -m pip install --upgrade --user ${MODULE_NAME}
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
  endif()
endfunction()

# Look for required python modules
search_python_module(setuptools)
search_python_module(wheel)

add_custom_target(python_package ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${DOCCREATOR_PYTHON_NAME}
  #COMMAND ${CMAKE_COMMAND} -E copy setup.py setup.py
  COMMAND ${CMAKE_COMMAND} -E copy README.md ${DOCCREATOR_PYTHON_NAME}/
  COMMAND ${CMAKE_COMMAND} -E copy __init__.py ${DOCCREATOR_PYTHON_NAME}/
  COMMAND ${CMAKE_COMMAND} -E copy DocCreator.py ${DOCCREATOR_PYTHON_NAME}/
  COMMAND ${CMAKE_COMMAND} -E copy Degradations.py ${DOCCREATOR_PYTHON_NAME}/
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Degradations> ${DOCCREATOR_PYTHON_NAME}/
  COMMAND ${CMAKE_COMMAND} -E copy ${OSMesa_LIBRARIES} ${DOCCREATOR_PYTHON_NAME}/libOSMesa.so.8 #Force this name
  #TODO: use ${SWIG_MODULE_Degradations_REAL_NAME}
  #TODO: copy license !!!
  COMMAND ${PYTHON_EXECUTABLE} setup.py clean --all bdist_wheel
  BYPRODUCTS
  DocCreatorPython/${DOCCREATOR_PYTHON_NAME}
  DocCreatorPython/build
  DocCreatorPython/dist
  DocCreatorPython/${DOCCREATOR_PYTHON_NAME}.egg-info
  WORKING_DIRECTORY DocCreatorPython
  )

message(" ${VENV_PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/wrapper/python/test.py")
# Test
if(BUILD_TESTING)
  #copy test.py in binary_dir to not have DocCreator.py in the same directory 
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test.py ${CMAKE_CURRENT_BINARY_DIR}/test.py COPYONLY)
 
  
  search_python_module(numpy) #required to run test
  # Look for python module virtualenv
  search_python_module(virtualenv)
  # Testing using a vitual environment
  set(VENV_EXECUTABLE ${PYTHON_EXECUTABLE} -m virtualenv)
  set(VENV_DIR ${CMAKE_CURRENT_BINARY_DIR}/venv)
  if(WIN32)
    set(VENV_PYTHON_EXECUTABLE "${VENV_DIR}\\Scripts\\python.exe")
  else()
    set(VENV_PYTHON_EXECUTABLE ${VENV_DIR}/bin/python)
  endif()
  # make a virtualenv to install our python package in it
  add_custom_command(TARGET python_package POST_BUILD
    COMMAND ${VENV_EXECUTABLE} -p ${PYTHON_EXECUTABLE} ${VENV_DIR}
    # Must not call it in a folder containing the setup.py otherwise pip call it
    # (i.e. "python setup.py bdist") while we want to consume the wheel package
    COMMAND ${VENV_PYTHON_EXECUTABLE} -m pip install --find-links=${CMAKE_CURRENT_BINARY_DIR}/DocCreatorPython/dist ${DOCCREATOR_PYTHON_NAME}
    COMMAND ${VENV_PYTHON_EXECUTABLE} -m pip install numpy
    BYPRODUCTS ${VENV_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  # run the tests within the virtualenv
  add_test(NAME pytest_venv
    COMMAND ${VENV_PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/test.py)
    #COMMAND ${VENV_PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/wrapper/python/test.py)
endif()
