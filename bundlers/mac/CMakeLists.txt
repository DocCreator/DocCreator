
IF(APPLE)
  #Qt5::qmake or Qt6::qmake, according to the Qt found
  IF(TARGET Qt${QT_VERSION_MAJOR}::qmake)
    get_target_property(_qmake_location Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_path "${_qmake_location}" DIRECTORY)
  ENDIF()
  find_program(QT_MACDEPLOYQT_EXECUTABLE macdeployqt
      HINTS "${_qt_bin_path}"
      DOC "Path to the macdeployqt utility."
      )
  if (NOT "${QT_MACDEPLOYQT_EXECUTABLE}" STREQUAL "QT_MACDEPLOYQT_EXECUTABLE-NOTFOUND")
    set(QT_BIN_PATH "${_qt_bin_path}" CACHE INTERNAL
	"Path to qmake, windeployqt, and other Qt utilities."
	)
  endif()	

  #CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/bundlers/mac/mac_bundle.sh.in" "${CMAKE_SOURCE_DIR}/bundlers/mac/mac_bundle.sh" @ONLY) 

  #In order to set the executable bit on produced file,		    
  #we first write a temporary file then change its rights when we copy it to the final destination
  CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/bundlers/mac/mac_bundle.sh.in" "${CMAKE_BINARY_DIR}/bundlers/mac/mac_bundle.sh" @ONLY) 
  FILE(COPY "${CMAKE_BINARY_DIR}/bundlers/mac/mac_bundle.sh" DESTINATION "${CMAKE_SOURCE_DIR}/bundlers/mac" 
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ) 
ENDIF(APPLE)


