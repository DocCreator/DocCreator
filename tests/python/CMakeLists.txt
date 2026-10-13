

# The python bindings are tested through pytest, on the module built in
# wrapper/python: it is put on PYTHONPATH below rather than installed.
FIND_PACKAGE(Python QUIET COMPONENTS Interpreter)

IF(NOT Python_Interpreter_FOUND)
  MESSAGE(STATUS "Python tests DISABLED (no python interpreter found)")
  RETURN()
ENDIF()

EXECUTE_PROCESS(COMMAND ${Python_EXECUTABLE} -c "import pytest, numpy"
  RESULT_VARIABLE _pytest_result
  OUTPUT_QUIET ERROR_QUIET)
IF(NOT _pytest_result EQUAL 0)
  MESSAGE(STATUS "Python tests DISABLED (pytest and numpy are needed for ${Python_EXECUTABLE})")
  RETURN()
ENDIF()

ADD_TEST(NAME testDegradationsPython
  COMMAND ${Python_EXECUTABLE} -m pytest -q --no-header
          ${CMAKE_CURRENT_SOURCE_DIR}/test_degradations.py)

#the module is named DocCreatorPy.cpython-*.so and sits next to its target
SET_TESTS_PROPERTIES(testDegradationsPython PROPERTIES
  ENVIRONMENT "PYTHONPATH=$<TARGET_FILE_DIR:DocCreatorPy>"
  )

MESSAGE(STATUS "Python tests ENABLED")
