

OPTION(BUILD_WITH_OSMESA "Build Distortion3D with OSMesa for offscreen OpenGL rendering" OFF)
if (NOT BUILD_WITH_OSMESA)
  message(STATUS "Build with OSMesa DISABLED (you can activate build with OSMesa for offscreen OpenGL rendering for 3D distortion with '-DBUILD_WITH_OSMESA=ON' cmake option)")
endif (NOT BUILD_WITH_OSMESA)

IF(BUILD_WITH_OSMESA OR NOT BUILD_ONLY_DEGRADATIONS OR BUILD_OTHER_PROGS_3D)
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
  RotationDegradation.hpp
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
  RotationDegradation.cpp
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
SET_TARGET_PROPERTIES(DocCreatorDegradations 
  PROPERTIES CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )
TARGET_COMPILE_OPTIONS(DocCreatorDegradations PRIVATE ${WARNING_FLAGS})
SET_TARGET_PROPERTIES(DocCreatorDegradations PROPERTIES POSITION_INDEPENDENT_CODE ON) #ugly #for wrapper


INSTALL(TARGETS DocCreatorDegradations
        #FRAMEWORK DESTINATION /Library/Frameworks
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

