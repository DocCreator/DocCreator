
IF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
  # Force to always compile with W3
  add_definitions(/W3)
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" 
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" 
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
ENDIF()


INCLUDE_DIRECTORIES(src)


FILE(
  GLOB_RECURSE
  DocCreatorFramework_sources
  src/*
  )

FILE(
  GLOB_RECURSE
  DocCreatorFramework_headers
  src/*.hpp
  )
FILE(
  GLOB
  headers_context
  src/context/*.h
  )
FILE(
  GLOB
  headers_core
  src/core/*.h
  )

FILE(
  GLOB
  headers_iomanager
  src/iomanager/*.h
  src/iomanager/*.hpp
  )
FILE(
  GLOB
  headers_models
  src/models/*.h
  )
FILE(
  GLOB
  headers_global
  src/*.h
  )
FILE(
  GLOB
  headers_models_doc
  src/models/doc/*.h
  )
FILE(
  GLOB
  headers_mvc
  src/mvc/*.h
  )
FILE(
  GLOB
  headers_patterns
  src/patterns/*.h
  )

FILE(
  GLOB
  Degradations_headers
  src/Degradations/*.h*
  )

FILE(
  GLOB
  Utils_headers
  src/Utils/*.h*
  )



# Variable to store output files
SET(outFiles)

# Find all the input files

FOREACH(infileName ${DocCreatorFramework_headers})
  SET(outFiles ${outFiles} "${infileName}")
ENDFOREACH(infileName)


##qt4_automoc(${DocCreatorFramework_sources})

QTX_WRAP_CPP(MOC ${DocCreatorFramework_headers})


#ADD_LIBRARY(DocCreatorFramework SHARED ${DocCreatorFramework_sources} ${MOC})
#SET_TARGET_PROPERTIES(DocCreatorFramework PROPERTIES COMPILE_FLAGS "-DFRAMEWORK_LIB")

ADD_LIBRARY(DocCreatorFramework STATIC ${DocCreatorFramework_sources} ${MOC})



#SET_TARGET_PROPERTIES( DocCreatorFramework
#      PROPERTIES
#      FRAMEWORK FALSE
#      FRAMEWORK_VERSION 1.0
#      #	PRIVATE_HEADER ${DocCreatorFramework_sources}
#      PUBLIC_HEADER ${outFiles}
#      MACOSX_FRAMEWORK_BUNDLE_VERSION "1.0"
#      MACOSX_FRAMEWORK_SHORT_VERSION_STRING "1.0"
#      MACOSX_FRAMEWORK_IDENTIFIER "identifier"
# )

TARGET_LINK_LIBRARIES(DocCreatorFramework ${QT_LIBRARIES} ${ThirdParty_LIBS})

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





INSTALL(FILES ${headers_global} DESTINATION include/DocCreatorFramework)
INSTALL(FILES ${headers_context} DESTINATION include/DocCreatorFramework/context)
INSTALL(FILES ${headers_core} DESTINATION include/DocCreatorFramework/core)
INSTALL(FILES ${headers_iomanager} DESTINATION include/DocCreatorFramework/iomanager)
INSTALL(FILES ${headers_models} DESTINATION include/DocCreatorFramework/models)
INSTALL(FILES ${headers_models_doc} DESTINATION include/DocCreatorFramework/models/doc)
INSTALL(FILES ${headers_mvc} DESTINATION include/DocCreatorFramework/mvc)
INSTALL(FILES ${headers_patterns} DESTINATION include/DocCreatorFramework/patterns)
INSTALL(FILES ${Degradations_headers} DESTINATION include/DocCreatorFramework/Degradations)
INSTALL(FILES ${Utils_headers} DESTINATION include/DocCreatorFramework/Utils)

INSTALL(TARGETS DocCreatorFramework
        #FRAMEWORK DESTINATION /Library/Frameworks
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

#B: useless ???
#INSTALL(FILES DocCreatorFramework.pc DESTINATION lib/pkgconfig)
