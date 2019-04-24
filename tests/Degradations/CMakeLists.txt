
SET(TESTS_DEGRADATIONS_SRC
  testHoleDegradation
  )

SET(LIBS  ${QT_LIBRARIES} ${DocCreator_LIBRARIES} ${OpenCV_LIBS})

include_directories(${FRAMEWORK_INCLUDE_DIRS})
include_directories(${CATCH_INCLUDE_DIRS})

MESSAGE(CATCH_INCLUDE_DIRS=${CATCH_INCLUDE_DIRS})


FOREACH(FILE ${TESTS_DEGRADATIONS_SRC})
  add_executable(${FILE} ${FILE})
  target_link_libraries (${FILE} ${LIBS})
  add_test(${FILE} ${FILE})
ENDFOREACH(FILE)
