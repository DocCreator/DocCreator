

SET(TESTS_LAYOUT_SRC
  testDocumentTextLayout.cpp
  )

FOREACH(FILE ${TESTS_LAYOUT_SRC})
  get_filename_component(TRGT ${FILE} NAME_WE)
  add_executable(${TRGT} ${FILE} ${CMAKE_SOURCE_DIR}/software/DocCreator/src/Degradations/DocumentTextLayout.cpp)
  target_include_directories(${TRGT} PRIVATE ${FRAMEWORK_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/software/DocCreator/src/Degradations)
  target_link_libraries (${TRGT} DocCreatorFramework Catch2::Catch2WithMain)
  set_target_properties(${TRGT} PROPERTIES CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )
  add_test(${TRGT} ${TRGT})
ENDFOREACH(FILE)
