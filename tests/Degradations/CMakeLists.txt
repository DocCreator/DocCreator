

SET(STAINIMAGES_DATA_SRC_DIRECTORY ${CMAKE_SOURCE_DIR}/data/Image/stainImages/images)
SET(PHANTOMPATTERNS_DATA_SRC_DIRECTORY ${CMAKE_SOURCE_DIR}/data/Image/stainImages/images)
SET(BLEEDTHROUGH_TEST_IMAGES_SRC_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/testBleedThroughData)
configure_file(paths.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/paths.hpp @ONLY)



SET(TESTS_DEGRADATIONS_SRC
  testBleedThrough
  testBlurDegradation
  testCharactersDegradation
  testHoleDegradation
  testGradientDomainDegradation
  testPhantomCharacter
  testShadowBinding
  )

SET(LIBS  ${QT_LIBRARIES} ${DocCreator_LIBRARIES} ${OpenCV_LIBS})

include_directories(${FRAMEWORK_INCLUDE_DIRS})
include_directories(${CATCH_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

MESSAGE(CATCH_INCLUDE_DIRS=${CATCH_INCLUDE_DIRS})


FOREACH(FILE ${TESTS_DEGRADATIONS_SRC})
  add_executable(${FILE} ${FILE})
  target_link_libraries (${FILE} ${LIBS})
  add_test(${FILE} ${FILE})
ENDFOREACH(FILE)
