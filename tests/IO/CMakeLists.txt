

SET(FONT_DATA_SRC_DIRECTORY ${CMAKE_SOURCE_DIR}/data/font)

CONFIGURE_FILE(paths.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/paths.hpp @ONLY)


SET(TESTS_IO_SRC
  testFontIO.cpp
  )

FOREACH(FILE ${TESTS_IO_SRC})
  get_filename_component(TRGT ${FILE} NAME_WE)
  add_executable(${TRGT} ${FILE})
  target_include_directories(${TRGT} PRIVATE ${FRAMEWORK_INCLUDE_DIRS} ${CATCH_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries (${TRGT} DocCreatorFramework)
  set_target_properties(${TRGT} PROPERTIES CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )
  add_test(${TRGT} ${TRGT})
ENDFOREACH(FILE)
