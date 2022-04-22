
add_subdirectory(Degradations)

IF(NOT BUILD_ONLY_DEGRADATIONS)
  add_subdirectory(IO)
ENDIF()

