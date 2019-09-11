
FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

# Find Python Interpreter
# prefer Python 3.7 over 3.6 over ...
# user can overwrite it e.g.:
# cmake -H. -Bbuild -DBUILD_PYTHON=ON -DPython_ADDITIONAL_VERSIONS="2.7"
#set(Python_ADDITIONAL_VERSIONS "3.7;3.6;3.5;2.7" CACHE STRING "Python to use for binding")
set(Python_ADDITIONAL_VERSIONS "2.7" CACHE STRING "Python to use for binding")
find_package(PythonInterp REQUIRED)
message(STATUS "Found Python: ${PYTHON_EXECUTABLE} (found version \"${PYTHON_VERSION_STRING}\")")

# Find Python Library
# Force PythonLibs to find the same version than the python interpreter (or nothing).
set(Python_ADDITIONAL_VERSIONS "${PYTHON_VERSION_STRING}")
find_package(PythonLibs REQUIRED)
message(STATUS "Found Python Include: ${PYTHON_INCLUDE_DIRS} (found version \"${PYTHONLIBS_VERSION_STRING}\")")



SET_SOURCE_FILES_PROPERTIES(Degradations.i PROPERTIES CPLUSPLUS ON)

#SET(EXTRA_LIBS DocCreatorDegradations ${OpenCV_LIBS})
SET(EXTRA_LIBS DocCreatorDegradations)

swig_add_library(Degradations
	TYPE MODULE
	LANGUAGE python
	SOURCES Degradations.i Degradations.cpp)

message("BORIS:  ${SWIG_MODULE_Degradations_REAL_NAME}")
message("BORIS: DEGRADATIONS_INCLUDE_DIRS=${DEGRADATIONS_INCLUDE_DIRS}")
message("BORIS: DEGRADATIONS_LIBRARIES=${DEGRADATIONS_LIBRARIES}")
message("BORIS: EXTRA_LIBS=${EXTRA_LIBS}")
get_target_property(DEGINCLUDES DocCreatorDegradations INCLUDE_DIRECTORIES)
message("BORIS: DEGINCLUDES=${DEGINCLUDES}")


target_include_directories(_Degradations
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${PYTHON_INCLUDE_DIRS}
	${DEGRADATIONS_INCLUDE_DIRS}
	)
#set_property(TARGET Degradations PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)

if(APPLE)
    swig_link_libraries(Degradations ${EXTRA_LIBS})
    SET_TARGET_PROPERTIES(_Degradations PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
else()
    swig_link_libraries(Degradations ${EXTRA_LIBS} ${PYTHON_LIBRARIES})
endif()

SET_TARGET_PROPERTIES(_Degradations PROPERTIES CXX_STANDARD 11)
SET_TARGET_PROPERTIES(_Degradations PROPERTIES CXX_STANDARD_REQUIRED ON)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DocCreator.py ${CMAKE_CURRENT_BINARY_DIR}/DocCreator.py COPYONLY)
