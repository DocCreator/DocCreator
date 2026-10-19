
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

  #B: the script was also copied into the source tree, which fails when it
  # is read-only: it is made in the build tree only, executable.
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/mac_bundle.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/mac_bundle.sh.tmp" @ONLY)
  FILE(COPY "${CMAKE_CURRENT_BINARY_DIR}/mac_bundle.sh.tmp" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/exec"
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
  FILE(RENAME "${CMAKE_CURRENT_BINARY_DIR}/exec/mac_bundle.sh.tmp" "${CMAKE_CURRENT_BINARY_DIR}/mac_bundle.sh")
  #B: the versions of Info.plist were 0.01 and 0.1: those of the project
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in" "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY)
ENDIF(APPLE)


