# Run by the "coverage" target (cf ENABLE_COVERAGE in the root
# CMakeLists.txt), as "cmake -P", with:
#   SOURCE_DIR  BINARY_DIR  GCOVR  GCOV  CTEST  JOBS
#
# 1. removes the counters (*.gcda) left by earlier runs, so that the report
#    is of this run only;
# 2. runs the tests. A failing test does not stop the report: it is said,
#    and the report is of what ran;
# 3. writes the report in BINARY_DIR/coverage/:
#      index.html     line and branch coverage, file by file, browsable
#      coverage.txt   the same, as text
#      coverage.xml   Cobertura, what CI services read
#    and prints the totals.

foreach(var SOURCE_DIR BINARY_DIR GCOVR GCOV CTEST JOBS)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Coverage.cmake: ${var} is not set")
  endif()
endforeach()

file(GLOB_RECURSE counters "${BINARY_DIR}/*.gcda")
list(LENGTH counters numCounters)
if(numCounters GREATER 0)
  file(REMOVE ${counters})
endif()
message(STATUS "coverage: ${numCounters} old counter files removed")

execute_process(COMMAND "${CTEST}" --output-on-failure -j ${JOBS}
                WORKING_DIRECTORY "${BINARY_DIR}"
                RESULT_VARIABLE testsResult)
if(NOT testsResult EQUAL 0)
  message(WARNING "coverage: some tests failed; the report is of what ran")
endif()

set(outDir "${BINARY_DIR}/coverage")
file(REMOVE_RECURSE "${outDir}")
file(MAKE_DIRECTORY "${outDir}")

# What is measured: the code of the project, not the libraries it
# downloads or carries (thirdparty/, pybind11), not the tests themselves,
# and not what the build generates (moc, uic), which is not under
# SOURCE_DIR. The counters under thirdparty/ and in the *_autogen
# directories of moc are not even read: gcov fails on them, on a working
# directory it cannot find for Catch2, built with the tests, and on the
# relative paths of the headers moc includes.
execute_process(
  COMMAND "${GCOVR}"
          --root "${SOURCE_DIR}"
          --object-directory "${BINARY_DIR}"
          --gcov-executable "${GCOV}"
          -j ${JOBS}
          --exclude-directories "(.*/)?thirdparty(/.*)?"
          --exclude-directories "(.*/)?[^/]*_autogen(/.*)?"
          --filter "${SOURCE_DIR}/framework/"
          --filter "${SOURCE_DIR}/software/"
          --filter "${SOURCE_DIR}/tools/"
          --filter "${SOURCE_DIR}/wrapper/python/"
          --exclude ".*/thirdparty/.*"
          --exclude ".*/pybind11-[^/]*/.*"
          --html-details "${outDir}/index.html"
          --txt "${outDir}/coverage.txt"
          --cobertura "${outDir}/coverage.xml"
          --print-summary
  WORKING_DIRECTORY "${BINARY_DIR}"
  RESULT_VARIABLE gcovrResult)
if(NOT gcovrResult EQUAL 0)
  message(FATAL_ERROR "coverage: gcovr failed")
endif()

message(STATUS "coverage: report in ${outDir}/index.html")
