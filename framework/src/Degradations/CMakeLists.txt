

OPTION(BUILD_WITH_OSMESA "Build Distortion3D with OSMesa for offscreen OpenGL rendering" OFF)
if (NOT BUILD_WITH_OSMESA)
  message(STATUS "Build with OSMesa DISABLED (you can activate build with OSMesa for offscreen OpenGL rendering for 3D distortion with '-DBUILD_WITH_OSMESA=ON' cmake option)")
endif (NOT BUILD_WITH_OSMESA)

IF(BUILD_WITH_OSMESA OR NOT BUILD_ONLY_DEGRADATIONS OR BUILD_OTHER_PROGS_3D)
  ADD_SUBDIRECTORY(Distortion3DModel)
ENDIF()

SET(
  Degradations_headers
  Degradations.hpp
  BleedThrough.hpp
  BlurFilter.hpp
  Distortion3D.hpp
  ElasticDeformation.hpp
  GradientDomainDegradation.hpp
  GrayscaleCharsDegradationModel.hpp
  HoleDegradation.hpp
  NoiseDegradation.hpp
  PhantomCharacter.hpp
  RotationDegradation.hpp
  ShadowBinding.hpp
  ConnectedComponent.hpp
  Random.hpp
  Validate.hpp
  ColorUtils.hpp
  FileUtils.hpp
  )

SET(
  Degradations_sources
  BleedThrough.cpp
  EllipseRasteriser.cpp
  BlurFilter.cpp
  Distortion3D.cpp
  ElasticDeformation.cpp
  GradientDomainDegradation.cpp
  GrayscaleCharsDegradationModel.cpp
  HoleDegradation.cpp
  NoiseDegradation.cpp
  PhantomCharacter.cpp
  RotationDegradation.cpp
  ShadowBinding.cpp
  ConnectedComponent.cpp
  Random.cpp
  Validate.cpp
  ColorUtils.cpp
  FileUtils.cpp
  )

IF(BUILD_WITH_OSMESA)
  list( APPEND ThirdParty_LIBS Distortion3D)
ENDIF()


ADD_LIBRARY(DocCreatorDegradations STATIC ${Degradations_sources} ${Degradations_headers})
#the name a project that uses the installed package links to
ADD_LIBRARY(DocCreator::Degradations ALIAS DocCreatorDegradations)
#The headers are included as <Degradations/...hpp>, and they include
#<framework_global.h>: framework/src in the build tree, include/DocCreatorFramework
#once installed. The include directories of OpenCV come with its imported
#targets; they are given explicitly only to the build tree, for an OpenCV too
#old to have them, and not to the installed package, where they would be the
#paths of this machine.
TARGET_INCLUDE_DIRECTORIES(DocCreatorDegradations PUBLIC
  $<BUILD_INTERFACE:${OpenCV_INCLUDE_DIRS}>
  $<BUILD_INTERFACE:${FRAMEWORK_INCLUDE_DIRS}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/DocCreatorFramework>)
TARGET_LINK_LIBRARIES(DocCreatorDegradations PUBLIC ${ThirdParty_LIBS})
#the headers use C++17: a user of the library must compile with it too
TARGET_COMPILE_FEATURES(DocCreatorDegradations PUBLIC cxx_std_17)
SET_TARGET_PROPERTIES(DocCreatorDegradations 
  PROPERTIES CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  EXPORT_NAME Degradations
  )
TARGET_COMPILE_OPTIONS(DocCreatorDegradations PRIVATE ${WARNING_FLAGS})
SET_TARGET_PROPERTIES(DocCreatorDegradations PROPERTIES POSITION_INDEPENDENT_CODE ON) #ugly #for wrapper


#Installed in the component Degradations: "cmake --install . --component
#Degradations" installs the library and its package alone. framework_global.h
#is installed here too, as every header of the library includes it, and the
#framework, which installs it as well, is not built with the degradations only.
INSTALL(FILES ${Degradations_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/DocCreatorFramework/Degradations
  COMPONENT Degradations)
INSTALL(FILES ${FRAMEWORK_INCLUDE_DIRS}/framework_global.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/DocCreatorFramework
  COMPONENT Degradations)

INSTALL(TARGETS DocCreatorDegradations
        EXPORT DocCreatorTargets
        #FRAMEWORK DESTINATION /Library/Frameworks
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Degradations
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Degradations
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Degradations
)

