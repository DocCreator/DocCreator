
SET( LIB_NAME Lipsum4Qt )


SET(
  Lipsum4Qt_sources
  src/Lipsum4Qt.cpp
  )

SET(
  Lipsum4Qt_headers
  src/Lipsum4Qt_global.h
  src/Lipsum4Qt.hpp
  )

set_property(SOURCE src/Lipsum4Qt.hpp
  PROPERTY MACOSX_PACKAGE_LOCATION Headers
  )

#QTX_ADD_RESOURCES(RCS ${Lipsum4Qt_resources} )
#QTX_WRAP_UI(UI ${Lipsum4Qt_ui})
QTX_WRAP_CPP(MOC ${Lipsum4Qt_headers})


SET(EXTRA_LIBS  ${QT_LIBRARIES})


#C++11 support (for all executables)
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  EXECUTE_PROCESS(COMMAND "${CMAKE_CXX_COMPILER} -dumpversion" OUTPUT_VARIABLE GCC_VERSION)
  IF (GCC_VERSION VERSION_LESS 4.7)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  ENDIF()
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  #libc++ is required both on mac and linux
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
ENDIF()
#VS2013 supports c++11 without any additional flags


# ADD_LIBRARY(${LIB_NAME} SHARED ${Lipsum4Qt_sources}  ${MOC} ${RCS} ${UI} ${Lipsum4Qt_headers})
# SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES COMPILE_FLAGS "-DLIPSUM4QT_LIBRARY")

# IF( APPLE )
#     SET_TARGET_PROPERTIES(${LIB_NAME}
#         PROPERTIES
#         FRAMEWORK TRUE
#         FRAMEWORK_VERSION 1.0
#         #PRIVATE_HEADER Lipsum4Qt_global.h
#         PUBLIC_HEADER "src/Lipsum4Qt_global.h;src/Lipsum4Qt.hpp"
#         MACOSX_FRAMEWORK_BUNDLE_VERSION "1.0"
#         MACOSX_FRAMEWORK_SHORT_VERSION_STRING "1.0"
#         MACOSX_FRAMEWORK_IDENTIFIER "identifier"
#         #INSTALL_NAME_DIR "@executable_path/../../../Library/Frameworks" ##Boris: valid only if we make a bundle ???
# 	INSTALL_NAME_DIR "@executable_path/../lib"
#      )
#     #ADD_CUSTOM_COMMAND( TARGET ${PROGNAME} POST_BUILD
#       #COMMAND mkdir ARGS ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
#       #COMMAND cp ARGS ${MACOSX_BUNDLE_ICON_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
#       #COMMAND cp -r src/images ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
#       #COMMAND cp  application.ini ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
#     #)
# ENDIF( APPLE )
ADD_LIBRARY(${LIB_NAME} STATIC ${Lipsum4Qt_sources} ${Lipsum4Qt_headers} ${MOC} )
TARGET_INCLUDE_DIRECTORIES(${LIB_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> ${QT_QTCORE_INCLUDE_DIR} )


TARGET_LINK_LIBRARIES(${LIB_NAME} ${QT_LIBRARIES})


INSTALL(FILES ${Lipsum4Qt_headers} DESTINATION include/${LIB_NAME})


# INSTALL(TARGETS ${LIB_NAME}
#         #FRAMEWORK DESTINATION /Library/Frameworks  ##Boris: valid only if we make a bundle ???
# 	FRAMEWORK DESTINATION lib
#         RUNTIME DESTINATION bin
#         LIBRARY DESTINATION lib
#         ARCHIVE DESTINATION lib

#   #      PRIVATE_HEADER DESTINATION share/foo-${foo_ver}/PrivateHeaders
#    #     PUBLIC_HEADER DESTINATION include/foo-${foo_ver}
#     #    RESOURCE DESTINATION share/foo-${foo_ver}/Resources
# )

#INSTALL(FILES ${LIB_NAME}.pc DESTINATION lib/pkgconfig)
