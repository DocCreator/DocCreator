
set (CMAKE_CXX_STANDARD 11)

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  #libc++ is required both on mac and linux
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
ENDIF()

add_subdirectory(Degradations)

