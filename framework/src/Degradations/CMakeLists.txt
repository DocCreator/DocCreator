

IF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
  # Force to always compile with W3
  add_definitions(/W3)
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" 
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" 
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
ENDIF()


#C++11 support
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  EXECUTE_PROCESS(COMMAND "${CMAKE_CXX_COMPILER} -dumpversion" OUTPUT_VARIABLE GCC_VERSION)
  IF (GCC_VERSION VERSION_LESS 4.7)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  ENDIF()
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
ENDIF()
#VS2013 supports c++11 without any additional flags





OPTION(BUILD_WITH_OSMESA "Build Distortion3D with OSMesa for offscreen OpenGL rendering" OFF)

IF(BUILD_WITH_OSMESA OR NOT BUILD_ONLY_DEGRADATIONS)
  ADD_SUBDIRECTORY(Distortion3DModel)
ENDIF()

SET(
  Degradations_headers
  BleedThrough.hpp
  BlurFilter.hpp
  Distortion3D.hpp
  GradientDomainDegradation.hpp
  GrayscaleCharsDegradationModel.hpp
  HoleDegradation.hpp
  PhantomCharacter.hpp
  ShadowBinding.hpp
  ConnectedComponent.hpp
  FileUtils.hpp
  )

SET(
  Degradations_sources
  BleedThrough.cpp
  BlurFilter.cpp
  Distortion3D.cpp
  GradientDomainDegradation.cpp
  GrayscaleCharsDegradationModel.cpp
  HoleDegradation.cpp
  PhantomCharacter.cpp
  ShadowBinding.cpp
  ConnectedComponent.cpp
  FileUtils.cpp
  )

IF(BUILD_WITH_OSMESA)
  list( APPEND ThirdParty_LIBS Distortion3D)
ENDIF()


ADD_LIBRARY(DocCreatorDegradations STATIC ${Degradations_sources} ${Degradations_headers})
TARGET_INCLUDE_DIRECTORIES(DocCreatorDegradations PUBLIC ${OpenCV_INCLUDE_DIRS} ${FRAMEWORK_INCLUDE_DIRS}) #${FRAMEWORK_INCLUDE_DIRS} just for framework_global.h
TARGET_LINK_LIBRARIES(DocCreatorDegradations ${ThirdParty_LIBS})
SET_TARGET_PROPERTIES(DocCreatorDegradations PROPERTIES POSITION_INDEPENDENT_CODE ON) #ugly #for wrapper


INSTALL(TARGETS DocCreatorDegradations
        #FRAMEWORK DESTINATION /Library/Frameworks
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

