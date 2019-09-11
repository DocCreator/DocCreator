cmake_minimum_required(VERSION 2.8.12)

# Only interpret if() arguments as variables or keywords when unquoted. [Policy introduced in cmake 3.1]
IF(POLICY CMP0054)
  CMAKE_POLICY(SET CMP0054 NEW)
ENDIF(POLICY CMP0054)

# Honor visibility properties for all target types. [Policy introduced in cmake 3.3]
IF(POLICY CMP0063)
  CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

PROJECT(DocCreatorSoftware)
SET( CMAKE_COLOR_MAKEFILE ON )
SET( CMAKE_VERBOSE_MAKEFILE ON )

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" 
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" 
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
ENDIF()


#INCLUDE( ${QT_USE_FILE} )

SET(
  DocCreatorQ_headers
  src/Degradations/BleedThroughQ.hpp
  src/Degradations/BlurFilterQ.hpp
  src/Degradations/DocumentDegradation.hpp
  src/Degradations/GradientDomainDegradationQ.hpp
  src/Degradations/GrayCharacterDegradationModelQ.cpp
  src/Degradations/HoleDegradationQ.hpp
  src/Degradations/PhantomCharacterQ.hpp
  src/Degradations/ShadowBindingQ.hpp
)

SET(
  DocCreatorQ_sources
  src/Degradations/BleedThroughQ.cpp
  src/Degradations/BlurFilterQ.cpp
  src/Degradations/GradientDomainDegradationQ.cpp
  src/Degradations/GrayCharacterDegradationModelQ.cpp
  src/Degradations/HoleDegradationQ.cpp
  src/Degradations/PhantomCharacterQ.cpp
  src/Degradations/ShadowBindingQ.cpp
)

SET(
  DocCreator_headers
  src/DocCreator.hpp
  src/Document/DocumentPropertiesView.hpp
  src/Document/GraphicsBlockItem.hpp
  src/Document/GraphicsCharacterItem.hpp
  src/Document/GraphicsImageBlockItem.hpp
  src/Document/GraphicsMovableButton.hpp
  src/Document/GraphicsTextBlockItem.hpp
  src/Document/GraphicsComponentBlockItem.hpp
  src/Document/GraphicsTestBlockItem.hpp
  src/Document/GraphicView.hpp
  #  src/Document/lipsum.h
  src/Document/TextView.hpp
  src/FontEditor/CharEditCursorItem.hpp
  src/FontEditor/CharEditLineItem.hpp
  src/FontEditor/CharEditScene.hpp
  src/FontEditor/CharEditView.hpp
  src/FontEditor/FontEditorView.hpp
  src/VirtualKeyboard/CharacterButtonView.hpp
  src/VirtualKeyboard/ControlButtonView.hpp
  src/VirtualKeyboard/KeyboardView.hpp
  src/VirtualKeyboard/KeyboardViewXmlBuilder.hpp
  src/RandomDocument/RandomDocumentCreator.hpp
  src/Utils/CharacterListView.hpp
  src/Utils/CharacterListWidgetItem.hpp
  src/Utils/MessageHandler.hpp
  src/Utils/RandomElement.hpp
  src/Utils/FontUtils.hpp
  src/Degradations/BleedThroughParameters.hpp
  src/Degradations/ImageGenerationFromDirDialog.hpp
  src/Degradations/GrayCharacterDegradationParameter.hpp
  src/Degradations/GrayCharacterDegradationDialog.hpp
  src/Degradations/VersoImageChangerParameters.hpp
  src/Degradations/BleedThroughParametersDialog.hpp
  src/Degradations/BlurFilterDialog.hpp
  src/Degradations/ShadowBindingDialog.hpp
  src/Degradations/HoleDegradationDialog.hpp
  src/Degradations/PhantomCharacterDialog.hpp
  src/Degradations/GradientDomainDegradationDialog.hpp
  src/Degradations/VersoImageChanger.hpp
  src/RandomDocument/RandomDocumentParameters.hpp
  src/RandomDocument/RandomDocumentExporter.hpp
  src/RandomDocument/RandomDocumentParametersDialog.hpp
  src/RandomDocument/RandomBackGroundChanger.hpp
  src/Document/GridPageLayout.hpp
  src/Document/PageLayout.hpp
  src/Document/ChooseLabelForComponentForm.hpp
  src/Document/DocumentToXMLExporter.hpp
  src/Document/BackGroundChanger.hpp
  src/Document/LabelingComponentDialog.hpp
  src/Document/DocRenderFlags.hpp
  src/Document/ImageExporter.hpp
  src/GenerateDocument/Assistant.hpp
  src/FontGenerator/BinarizationDialog.hpp
  src/FontGenerator/Binarization.hpp
  src/FontGenerator/LineDetectionDialog.hpp
  src/FontGenerator/OCRSettingsDialog.hpp
  src/FontGenerator/OCRDialog.hpp
  src/FontGenerator/Baseline.hpp
  src/FontGenerator/BackgroundReconstructionDialog.hpp
  src/FontGenerator/FontExtractionDialog.hpp
  src/FontGenerator/TextInpainting.hpp
  src/FontGenerator/StructureDialog.hpp
  src/FontGenerator/StructureDetection.hpp
  src/FontGenerator/FontDocumentGenerator.hpp
  )

SET(
  DocCreator_sources
  src/Utils/RandomElement.cpp
  src/Utils/FontUtils.cpp
  src/Utils/CharacterListWidgetItem.cpp
  src/Utils/CharacterListView.cpp
  src/VirtualKeyboard/CharacterButtonView.cpp
  src/VirtualKeyboard/KeyboardController.cpp
  src/VirtualKeyboard/KeyboardViewXmlBuilder.cpp
  src/VirtualKeyboard/KeyboardView.cpp
  src/VirtualKeyboard/ControlButtonView.cpp
  src/DocCreator.cpp
  src/Degradations/GrayCharacterDegradationDialog.cpp
  src/Degradations/VersoImageChanger.cpp
  src/Degradations/BleedThroughParameters.cpp
  src/Degradations/GrayCharacterDegradationParameter.cpp
  src/Degradations/VersoImageChangerParameters.cpp
  src/Degradations/BleedThroughParametersDialog.cpp
  src/Degradations/BlurFilterDialog.cpp
  src/Degradations/ShadowBindingDialog.cpp
  src/Degradations/HoleDegradationDialog.cpp
  src/Degradations/PhantomCharacterDialog.cpp
  src/Degradations/GradientDomainDegradationDialog.cpp
  src/Degradations/ImageGenerationFromDirDialog.cpp
  src/RandomDocument/RandomDocumentCreator.cpp
  src/RandomDocument/RandomBackGroundChanger.cpp
  src/RandomDocument/RandomDocumentExporter.cpp
  src/RandomDocument/RandomDocumentParametersDialog.cpp
  src/RandomDocument/RandomDocumentParameters.cpp
  src/FontEditor/CharEditScene.cpp
  src/FontEditor/FontEditorController.cpp
  src/FontEditor/CharEditCursorItem.cpp
  src/FontEditor/CharEditLineItem.cpp
  src/FontEditor/FontEditorView.cpp
  src/FontEditor/CharEditView.cpp
  src/main.cpp
  src/Document/GraphicsBlockItem.cpp
  src/Document/GraphicsTestBlockItem.cpp
  src/Document/GraphicsImageBlockItem.cpp
  src/Document/TextView.cpp
  src/Document/GraphicsMovableButton.cpp
  src/Document/PageLayout.cpp
  src/Document/GraphicsCharacterItem.cpp
  src/Document/DocumentPropertiesView.cpp
  src/Document/GraphicsPageItem.cpp
  src/Document/DocumentController.cpp
  src/Document/ChooseLabelForComponentForm.cpp
  src/Document/GraphicsComponentBlockItem.cpp
  src/Document/DocumentToXMLExporter.cpp
  #  src/Document/lipsum.cpp
  src/Document/GridPageLayout.cpp
  src/Document/GraphicView.cpp
  src/Document/DocumentView.cpp
  src/Document/ImageExporter.cpp
  src/Document/GraphicsTextBlockItem.cpp
  src/Document/LabelingComponentDialog.cpp
  src/Document/BackGroundChanger.cpp
  src/Document/GraphicsCursorItem.cpp
  src/GenerateDocument/Assistant.cpp
  src/FontGenerator/BinarizationDialog.cpp
  src/FontGenerator/Binarization.cpp
  src/FontGenerator/LineDetectionDialog.cpp
  src/FontGenerator/OCRSettingsDialog.cpp
  src/FontGenerator/OCRDialog.cpp
  src/FontGenerator/Baseline.cpp
  src/FontGenerator/BackgroundReconstructionDialog.cpp
  src/FontGenerator/FontExtractionDialog.cpp
  src/FontGenerator/TextInpainting.cpp
  src/FontGenerator/StructureDialog.cpp
  src/FontGenerator/StructureDetection.cpp
  src/FontGenerator/FontDocumentGenerator.cpp
  )

SET(
  DocCreator_ui
  src/Degradations/GrayCharacterDegradationDialog.ui
  src/Degradations/ImageGenerationFromDirDialog.ui
  src/Degradations/GrayCharacterDegradationParameter.ui
  src/Degradations/BleedThroughParametersDialog.ui
  src/Degradations/BlurFilterDialog.ui
  src/Degradations/ShadowBindingDialog.ui
  src/Degradations/HoleDegradationDialog.ui
  src/Degradations/PhantomCharacterDialog.ui
  src/Degradations/GradientDomainDegradationDialog.ui
  src/RandomDocument/RandomDocumentParametersDialog.ui
  src/FontEditor/fonteditorview.ui
  src/Document/LabelingComponentDialog.ui
  src/Document/ChooseLabelForComponentForm.ui
  src/GenerateDocument/Assistant.ui
  src/FontGenerator/BinarizationDialog.ui
  src/FontGenerator/LineDetectionDialog.ui
  src/FontGenerator/OCRDialog.ui
  src/FontGenerator/BackgroundReconstructionDialog.ui
  src/FontGenerator/FontExtractionDialog.ui
  src/FontGenerator/StructureDialog.ui
  )


#Generate application.ini
SET(SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/DocCreator) #REQUIRES "make install" !!
SET(DATA_DIR ${SHARE_DIR}/data)
SET(FONT_DATA_DIRECTORY ${DATA_DIR}/font/)
SET(BACKGROUND_DATA_DIRECTORY ${DATA_DIR}/background/)
SET(IMAGE_DATA_DIRECTORY ${DATA_DIR}/Image/)
SET(MESH_DATA_DIRECTORY ${DATA_DIR}/Mesh/)
SET(HOLEPATTERNS_DATA_DIRECTORY ${DATA_DIR}/Image/holePatterns/)
SET(PHANTOMPATTERNS_DATA_DIRECTORY ${DATA_DIR}/Image/phantomPatterns/)
SET(BLURIMAGES_DATA_DIRECTORY ${DATA_DIR}/Image/blurImages/)
SET(STAINIMAGES_DATA_DIRECTORY ${DATA_DIR}/Image/stainImages/images/)
SET(XMLCHECKER_DIRECTORY ${SHARE_DIR}/xmlchecker/)
SET(KEYBOARD_DATA_DIRECTORY ${DATA_DIR}/keyboard/)
CONFIGURE_FILE(application.ini.in ${CMAKE_BINARY_DIR}/software/DocCreator/application.ini)
#Generate doccreator_app.qrc (that includes application.ini)
SET(APPLICATION_INI ${CMAKE_BINARY_DIR}/software/DocCreator/application.ini)
CONFIGURE_FILE(src/doccreator_app.qrc.in doccreator_app.qrc)

SET(
  DocCreator_rcs
  src/doccreator.qrc
  ${CMAKE_BINARY_DIR}/software/DocCreator/doccreator_app.qrc
  )


SET( LIBNAME DocCreatorQ )
SET( PROGNAME DocCreator )

IF( APPLE )
  SET( MACOSX_BUNDLE_ICON_FILE adoc.icns )
  SET( MACOSX_BUNDLE_SHORT_VERSION_STRING 0.2 )
  SET( MACOSX_BUNDLE_VERSION 0.7-beta2 )
  SET( MACOSX_BUNDLE_LONG_VERSION_STRING Version 0.2 )
  #SET( CMAKE_OSX_ARCHITECTURES i686 ) #Comment out if not universal binary
ENDIF( APPLE )


if( UNIX AND NOT APPLE )
  if( NOT DESKTOP_ENTRY )
    set( DESKTOP_ENTRY DocCreator.desktop )
  endif( NOT DESKTOP_ENTRY )
  
  #set(CMAKE_INSTALL_PREFIX /usr/local/) ###B: UGLY ????
  
  add_custom_command( OUTPUT ${DESKTOP_ENTRY}
    COMMAND touch ${DESKTOP_ENTRY}
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/adoc-desktop.sh ${CMAKE_INSTALL_PREFIX} >${DESKTOP_ENTRY}
    DEPENDS adoc-desktop.sh
    COMMENT "Generating desktop entry file"
    )
  add_custom_target( DESKTOP_ENTRY_FILE ALL
    DEPENDS ${DESKTOP_ENTRY}
    )
  
  set( APP_ICON ${CMAKE_SOURCE_DIR}/adoc.icns )
endif( UNIX AND NOT APPLE )

IF(WIN32) 
  set(WIN_RESOURCE_FILE adoc.rc) #for icon
ENDIF()

QTX_WRAP_CPP(MOC_LIB ${DocCreatorQ_headers})

QTX_ADD_RESOURCES(RCS ${DocCreator_rcs} )
QTX_WRAP_UI(UI ${DocCreator_ui})
QTX_WRAP_CPP(MOC ${DocCreator_headers})


ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/Degradations/Distortion3DModel)


SET(EXTRA_LIBS ${QT_LIBRARIES} ${Lipsum4Qt_LIBRARIES} ${DocCreator_LIBRARIES} ${OpenCV_LIBS} ${DOC3D_LIBRARIES} ${Tesseract_LIBRARIES})

#message("EXTRA_LIBS=${EXTRA_LIBS}")

#IF( APPLE )
#   ADD_EXECUTABLE(${PROGNAME} MACOSX_BUNDLE ${DocCreator_sources}  ${MOC} ${RCS} ${UI})
#   ADD_CUSTOM_COMMAND( TARGET ${PROGNAME} POST_BUILD
#     #COMMAND mkdir ARGS ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
#     COMMAND cp ARGS ${CMAKE_SOURCE_DIR}/${MACOSX_BUNDLE_ICON_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
#      #COMMAND cp -r src/images ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
#      #COMMAND cp  application.ini ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
#    )
#  ELSE( APPLE )
#    ADD_EXECUTABLE(${PROGNAME} ${DocCreator_sources} ${DocCreator_headers} ${MOC} ${RCS} ${UI} ${WIN_RESOURCE_FILE})
#    if(UNIX AND NOT APPLE)
#       INSTALL( TARGETS ${PROGNAME} DESTINATION bin )
#       install( FILES ${APP_ICON} DESTINATION share/icons )
#	INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_ENTRY} DESTINATION share/applications)
#   endif(UNIX AND NOT APPLE)
# ENDIF( APPLE )


ADD_LIBRARY(${LIBNAME} STATIC ${DocCreatorQ_sources} ${DocCreatorQ_headers} ${MOC_LIB})
TARGET_INCLUDE_DIRECTORIES(${LIBNAME} PUBLIC ${FRAMEWORK_INCLUDE_DIRS} $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
TARGET_LINK_LIBRARIES(${LIBNAME} ${EXTRA_LIBS})

ADD_EXECUTABLE(${PROGNAME} ${DocCreator_sources} ${DocCreator_headers} ${MOC} ${RCS} ${UI} ${WIN_RESOURCE_FILE})
TARGET_INCLUDE_DIRECTORIES(${PROGNAME} PRIVATE ${FRAMEWORK_INCLUDE_DIRS} ${DOC3D_INCLUDE_DIRS} $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Lept_INCLUDE_DIRS} ${Tesseract_INCLUDE_DIRS})
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES INSTALL_RPATH "${PROG_RPATH}")


# Cf http://www.kitware.com/blog/home/post/510
#As CMAKE_MACOSX_RPATH is defined to 1,
# Cmake  will automatically add an RPATH to this executable which is the absolute location of libDocCreator in the build tree.
TARGET_LINK_LIBRARIES(${PROGNAME} ${EXTRA_LIBS} ${LIBNAME})
#TARGET_LINK_LIBRARIES(${PROGNAME} ${ALL_LIBS})


# the install RPATH for bar to find foo in the install tree.
# if the install RPATH is not provided, the install bar will have none
#SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES INSTALL_RPATH "@loader_path/../lib")


#B: is there a better way to find the Qt installation path ?
get_target_property(QtCore_location Qt5::Core LOCATION)
#get_filename_component(QTCORE_INSTALL_PATH "${QtCore_location}" ABSOLUTE)
#get_filename_component(QT_INSTALL_PATH "${QTCORE_INSTALL_PATH}/.." ABSOLUTE)
get_filename_component(QT_INSTALL_PATH "${QtCore_location}/../.." ABSOLUTE)
#message("QT_INSTALL_PATH=${QT_INSTALL_PATH}")

set(PROG_RPATH "@loader_path/../lib") #B: necessary as we install libs (LipsumQt, framework, doc3D) in ../lib directory [Non-bundle case]
set(PROG_RPATH "${PROG_RPATH};${QT_INSTALL_PATH}") #B: necessary as Qt libs are marked @rpath in exe
set(PROG_RPATH "${PROG_RPATH};${LEPTONICA_INSTALL_PATH};${TESSERACT_INSTALL_PATH}") #B: added to be able to launch app without install
#message("PROG_RPATH=${PROG_RPATH}")
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES INSTALL_RPATH "${PROG_RPATH}")


#GET_TARGET_PROPERTY(CURRENT_INSTALL_RPATHS ${PROGNAME} INSTALL_RPATH)
#message("DEBUG ======= CURRENT_INSTALL_RPATHS=${CURRENT_INSTALL_RPATHS}")
#SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES INSTALL_RPATH "${CURRENT_INSTALL_RPATHS};${LEPTONICA_INSTALL_PATH};${TESSERACT_INSTALL_PATH}")
#GET_TARGET_PROPERTY(CURRENT_INSTALL_RPATHS2 ${PROGNAME} INSTALL_RPATH)
#message("DEBUG ======= CURRENT_INSTALL_RPATHS2=${CURRENT_INSTALL_RPATHS2}")

IF(BUILD_TESSERACT)
  add_dependencies(${PROGNAME} tesseractLib)  #B: to be able to compile with "make -j 9"
ENDIF(BUILD_TESSERACT)


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


IF(UNIX)
  install(TARGETS ${PROGNAME} RUNTIME DESTINATION bin)
ENDIF(UNIX)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/data DESTINATION share/DocCreator)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/xmlchecker DESTINATION share/DocCreator)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux") 
  # Linux specific code
  install(FILES fr.labri.doccreator.doccreator.desktop DESTINATION share/applications/)
  install(FILES fr.labri.doccreator.doccreator.appdata.xml DESTINATION share/appdata/)
  install(FILES adoc.png DESTINATION share/icons/hicolor/512x512/apps/ RENAME fr.labri.doccreator.doccreator.png)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")



IF(BUILD_OTHER_PROGS)

  ADD_SUBDIRECTORY(src/Degradator)
  
  # SET(SOURCES src/testBlurFilter.cpp src/Degradations/BlurFilterQ.cpp)
  # #src/Degradations/BlurFilterDialog.cpp src/Degradations/BlurFilterDialog.hpp)
  # #SET(MY_UI_FILES src/Degradations/BlurFilterDialog.ui)
  # #QTX_WRAP_UI(MY_UI ${MY_UI_FILES})
  # SET(MY_MOC_FILES src/Degradations/BlurFilterQ.hpp src/Degradations/DocumentDegradation.hpp )
  # QTX_WRAP_CPP(MY_MOC ${MY_MOC_FILES})
  # ADD_EXECUTABLE(testBlurFilter ${SOURCES} ${MY_MOC} ${MY_UI})
  # TARGET_LINK_LIBRARIES(testBlurFilter ${EXTRA_LIBS})
  # IF(BUILD_TESSERACT)
  #   add_dependencies(testBlurFilter tesseractLib)  #B: to be able to compile with "make -j 9"
  # ENDIF(BUILD_TESSERACT)


ENDIF(BUILD_OTHER_PROGS)



IF(WIN32)

  INSTALL(TARGETS ${PROGNAME} RUNTIME DESTINATION programs COMPONENT applications)

  # Tell CMake to install the windows runtime libraries to the programs
  # directory and tell CPack that they belong to the applications component
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  include(InstallRequiredSystemLibraries)
  install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION programs
    COMPONENT applications)
  

  # install qt libraries & folders
  if(USE_QT5)
    get_target_property(_qmake_location Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_path "${_qmake_location}" DIRECTORY)
    find_program(QT_WINDEPLOYQT_EXECUTABLE windeployqt
      HINTS "${_qmake_location}"
      DOC "Path to the windeployqt utility."
      )
    if (NOT "${QT_WINDEPLOYQT_EXECUTABLE}" STREQUAL "QT_WINDEPLOYQT_EXECUTABLE-NOTFOUND")
      set(QT_BIN_PATH "${_qt_bin_path}" CACHE INTERNAL
	"Path to qmake, windeployqt, and other Qt utilities."
	)
      add_custom_target(copy_qt_dlls ALL)
      set_target_properties(copy_qt_dlls PROPERTIES FOLDER "Copy Tasks")
      add_custom_command(TARGET copy_qt_dlls
	POST_BUILD
	COMMAND set "PATH=${QT_BIN_PATH};%PATH%"
	COMMAND "${QT_WINDEPLOYQT_EXECUTABLE}"
	$<$<CONFIG:Debug>:--debug>
	$<$<NOT:$<CONFIG:Debug>>:--release>
	--opengl
	--no-compiler-runtime
	--verbose 10
	--dir "${CMAKE_INSTALL_PREFIX}/programs" #install Qt files in INSTALL_PREFIX directory and not build directory.
	"$<TARGET_FILE:${PROGNAME}>" 			
	)

      MACRO (COPY_DLL libname)
	ADD_CUSTOM_COMMAND (
	  TARGET copy_qt_dlls
	  POST_BUILD
	  COMMAND ${CMAKE_COMMAND}
	  #ARGS -E copy "${libname}" "${CMAKE_INSTALL_PREFIX}/programs"
	  ARGS -E copy $<TARGET_FILE:${libname}> "${CMAKE_INSTALL_PREFIX}/programs"
	  )
	ADD_CUSTOM_COMMAND (
	  TARGET copy_qt_dlls
	  POST_BUILD
	  COMMAND  ${CMAKE_COMMAND}
	  ARGS -E echo $<TARGET_FILE:${libname}>
	  )
      ENDMACRO (COPY_DLL)

      foreach(LIB ${OpenCV_LIBS})
	COPY_DLL(${LIB})
      endforeach()

      add_dependencies(copy_qt_dlls ${PROGNAME})

      #We always install ?
      #We would like to add a dependency on "install" target. Is it possible ???

    endif()
  endif(USE_QT5)

ENDIF(WIN32)



set(CPACK_PACKAGE_NAME ${PROGNAME})
set(CPACK_PACKAGE_VENDOR "LaBRI")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Document Creator")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CPackExample")

set(CPACK_COMPONENTS_ALL applications)
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "${PROGNAME}")
#example:
# set(CPACK_COMPONENTS_ALL applications CTData instructionData)
# set(CPACK_COMPONENT_CTDATA_GROUP "data")
# set(CPACK_COMPONENT_GROUP_DATA_DESCRIPTION "data (CT and instructions)")

SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")

set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\documentImageCreator\\src\\gui\\DocCreator\\adoc_package.bmp")

IF(WIN32 AND NOT UNIX)

  set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}\\documentImageCreator\\src\\gui\\DocCreator\\adoc_install.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}\\documentImageCreator\\src\\gui\\DocCreator\\adoc_uninstall.ico")
  
  set(CPACK_NSIS_MENU_LINKS "${PROGNAME}" "Document Creator")
  
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\bitbucket.org/BorisMansencal/doccreator")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\bitbucket.org/BorisMansencal/doccreator")
  SET(CPACK_NSIS_CONTACT "boris.mansencal@labri.fr")
  SET(CPACK_NSIS_MODIFY_PATH ON)
  
endif()

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_SOURCE_DIR}/CPack_install")


include(CPack)
