

SET(STAINIMAGES_DATA_SRC_DIRECTORY ${CMAKE_SOURCE_DIR}/data/Image/stainImages/images)
SET(PHANTOMPATTERNS_DATA_SRC_DIRECTORY ${CMAKE_SOURCE_DIR}/data/Image/phantomPatterns)
SET(BLEEDTHROUGH_TEST_IMAGES_SRC_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/testBleedThroughData)
SET(MESHES_DATA_SRC_DIRECTORY ${CMAKE_SOURCE_DIR}/data/Mesh)
CONFIGURE_FILE(paths.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/paths.hpp @ONLY)



SET(TESTS_DEGRADATIONS_SRC
  testCommon.cpp
  testBleedThrough.cpp
  testBlurDegradation.cpp
  testCharactersDegradation.cpp
  testElasticDeformation.cpp
  testHoleDegradation.cpp
  testGradientDomainDegradation.cpp
  testNoiseDegradation.cpp
  testPhantomCharacter.cpp
  testRotationDegradation.cpp
  testShadowBinding.cpp
  )

IF(BUILD_WITH_OSMESA)
  SET(TESTS_DEGRADATIONS_SRC ${TESTS_DEGRADATIONS_SRC} testDistortion3D)
ENDIF()



FOREACH(FILE ${TESTS_DEGRADATIONS_SRC})
  get_filename_component(TRGT ${FILE} NAME_WE)
  add_executable(${TRGT} ${FILE})
  target_include_directories(${TRGT} PRIVATE ${FRAMEWORK_INCLUDE_DIRS} ${CATCH_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries (${TRGT} DocCreatorDegradations)
  set_target_properties(${TRGT} PROPERTIES CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )
  add_test(${TRGT} ${TRGT})
ENDFOREACH(FILE)
