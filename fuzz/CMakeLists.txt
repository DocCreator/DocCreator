

# libFuzzer is a clang feature: -fsanitize=fuzzer brings in both the driver
# (which provides main()) and the coverage instrumentation. There is no
# equivalent in gcc.
IF(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"
      OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
  MESSAGE(FATAL_ERROR "BUILD_FUZZERS=ON needs clang, not ${CMAKE_CXX_COMPILER_ID}. Cf fuzz/README.txt")
ENDIF()

# A fuzz target without a sanitiser finds only the crashes that happen to
# be fatal, which is most of the point missed.
IF(SANITIZER STREQUAL "none")
  MESSAGE(WARNING "BUILD_FUZZERS=ON without a sanitiser: add -DSANITIZER=address+undefined")
ENDIF()

SET(FUZZ_TARGETS
  fuzz_font.cpp
  fuzz_xmldocument.cpp
  )

FOREACH(FILE ${FUZZ_TARGETS})
  get_filename_component(TRGT ${FILE} NAME_WE)
  ADD_EXECUTABLE(${TRGT} ${FILE})
  TARGET_INCLUDE_DIRECTORIES(${TRGT} PRIVATE ${FRAMEWORK_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(${TRGT} DocCreatorFramework ${QT_LIBRARIES})
  TARGET_COMPILE_OPTIONS(${TRGT} PRIVATE -fsanitize=fuzzer)
  TARGET_LINK_OPTIONS(${TRGT} PRIVATE -fsanitize=fuzzer)
  SET_TARGET_PROPERTIES(${TRGT} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )
ENDFOREACH(FILE)

# Seed corpora, so that a run starts from something the readers accept
# instead of from nothing.
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/corpus_font)
FILE(GLOB _seed_fonts ${CMAKE_SOURCE_DIR}/data/font/*.bof ${CMAKE_SOURCE_DIR}/data/font/*.of)
IF(_seed_fonts)
  FILE(COPY ${_seed_fonts} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/corpus_font)
ENDIF()

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/corpus_xmldocument)
FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/corpus_xmldocument
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

MESSAGE(STATUS "Fuzz targets ENABLED: ${FUZZ_TARGETS}")
