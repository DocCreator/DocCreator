

SET(STAINIMAGES_DATA_SRC_DIRECTORY ${CMAKE_SOURCE_DIR}/data/Image/stainImages/images)
SET(PHANTOMPATTERNS_DATA_SRC_DIRECTORY ${CMAKE_SOURCE_DIR}/data/Image/phantomPatterns)
SET(BLEEDTHROUGH_TEST_IMAGES_SRC_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/testBleedThroughData)
SET(MESHES_DATA_SRC_DIRECTORY ${CMAKE_SOURCE_DIR}/data/Mesh)
CONFIGURE_FILE(paths.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/paths.hpp @ONLY)



SET(TESTS_DEGRADATIONS_SRC
  testCommon
  testBleedThrough
  testBlurDegradation
  testCharactersDegradation
  testHoleDegradation
  testGradientDomainDegradation
  testPhantomCharacter
  testShadowBinding
  )

IF(BUILD_WITH_OSMESA)
  SET(TESTS_DEGRADATIONS_SRC ${TESTS_DEGRADATIONS_SRC} testDistortion3D)
ENDIF()



FOREACH(FILE ${TESTS_DEGRADATIONS_SRC})
  add_executable(${FILE} ${FILE})
  target_include_directories(${FILE} PRIVATE ${FRAMEWORK_INCLUDE_DIRS} ${CATCH_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries (${FILE} DocCreatorDegradations)
  set_target_properties(${FILE} PROPERTIES CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )
  add_test(${FILE} ${FILE})
ENDFOREACH(FILE)
