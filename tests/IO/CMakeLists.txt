

SET(FONT_DATA_SRC_DIRECTORY ${CMAKE_SOURCE_DIR}/data/font)

CONFIGURE_FILE(paths.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/paths.hpp @ONLY)


SET(TESTS_IO_SRC
  testFontIO
  )

FOREACH(FILE ${TESTS_IO_SRC})
  add_executable(${FILE} ${FILE})
  target_include_directories(${FILE} PRIVATE ${FRAMEWORK_INCLUDE_DIRS} ${CATCH_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries (${FILE} DocCreatorFramework)
  set_target_properties(${FILE} PROPERTIES CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )
  add_test(${FILE} ${FILE})
ENDFOREACH(FILE)
