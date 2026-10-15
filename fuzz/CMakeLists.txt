

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

# The mesh reader is not Qt code: it links the mesh library only.
ADD_EXECUTABLE(fuzz_brs fuzz_brs.cpp)
TARGET_LINK_LIBRARIES(fuzz_brs Distortion3DMesh)
TARGET_COMPILE_OPTIONS(fuzz_brs PRIVATE -fsanitize=fuzzer)
TARGET_LINK_OPTIONS(fuzz_brs PRIVATE -fsanitize=fuzzer)
SET_TARGET_PROPERTIES(fuzz_brs PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )
LIST(APPEND FUZZ_TARGETS fuzz_brs.cpp)

# Seed corpora, so that a run starts from something the readers accept
# instead of from nothing.
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/corpus_font)
FILE(GLOB _seed_fonts ${CMAKE_SOURCE_DIR}/data/font/*.bof ${CMAKE_SOURCE_DIR}/data/font/*.of)
IF(_seed_fonts)
  FILE(COPY ${_seed_fonts} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/corpus_font)
ENDIF()

#the two planes: the other meshes of data/Mesh are a megabyte and more
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/corpus_brs)
FILE(COPY ${CMAKE_SOURCE_DIR}/data/Mesh/PlaneRegular.brs ${CMAKE_SOURCE_DIR}/data/Mesh/PlaneIrregular.brs
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/corpus_brs)

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/corpus_xmldocument)
FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/corpus_xmldocument
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

MESSAGE(STATUS "Fuzz targets ENABLED: ${FUZZ_TARGETS}")
