

SET(TESTS_MODELS_SRC
  testCharacter.cpp
  testDocNode.cpp
  )

FOREACH(FILE ${TESTS_MODELS_SRC})
  get_filename_component(TRGT ${FILE} NAME_WE)
  add_executable(${TRGT} ${FILE})
  target_include_directories(${TRGT} PRIVATE ${FRAMEWORK_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries (${TRGT} DocCreatorFramework Catch2::Catch2WithMain)
  set_target_properties(${TRGT} PROPERTIES CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )
  add_test(${TRGT} ${TRGT})
ENDFOREACH(FILE)
