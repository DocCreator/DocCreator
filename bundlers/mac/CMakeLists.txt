
IF(APPLE)
  if(USE_QT5)
    get_target_property(_qmake_location Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_path "${_qmake_location}" DIRECTORY)
    find_program(QT_MACDEPLOYQT_EXECUTABLE macdeployqt
      HINTS "${_qmake_location}"
      DOC "Path to the macdeployqt utility."
      )
    if (NOT "${QT_MACDEPLOYQT_EXECUTABLE}" STREQUAL "QT_MACDEPLOYQT_EXECUTABLE-NOTFOUND")
      set(QT_BIN_PATH "${_qt_bin_path}" CACHE INTERNAL
	"Path to qmake, windeployqt, and other Qt utilities."
	)
    endif()	
  endif()

  #CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/bundlers/mac/mac_bundle.sh.in" "${CMAKE_SOURCE_DIR}/bundlers/mac/mac_bundle.sh" @ONLY) 

  #In order to set the executable bit on produced file,		    
  #we first write a temporary file then change its rights when we copy it to the final destination
  CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/bundlers/mac/mac_bundle.sh.in" "${CMAKE_BINARY_DIR}/bundlers/mac/mac_bundle.sh" @ONLY) 
  FILE(COPY "${CMAKE_BINARY_DIR}/bundlers/mac/mac_bundle.sh" DESTINATION "${CMAKE_SOURCE_DIR}/bundlers/mac" 
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ) 
ENDIF(APPLE)


