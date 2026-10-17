#The list of the files the uninstaller removes, made by uninstall_list.cmake
#when DocCreator.nsi is compiled. !include of NSIS needs backslashes.
FILE(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/uninstall_files.nsh" UNINSTALL_FILES_NSH)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/DocCreator.nsi.in ${CMAKE_CURRENT_BINARY_DIR}/DocCreator.nsi @ONLY)