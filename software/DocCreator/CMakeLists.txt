cmake_minimum_required(VERSION 2.8.12)

# Set version variables to empty if not specified in project() call.
IF(POLICY CMP0048)
  CMAKE_POLICY(SET CMP0048 NEW)
ENDIF(POLICY CMP0048)

# Only interpret if() arguments as variables or keywords when unquoted. [Policy introduced in cmake 3.1]
IF(POLICY CMP0054)
  CMAKE_POLICY(SET CMP0054 NEW)
ENDIF(POLICY CMP0054)

# Honor visibility properties for all target types. [Policy introduced in cmake 3.3]
IF(POLICY CMP0063)
  CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

# FindOpenGL prefers GLVND by default when available. [Policy introduced in cmake 3.11]
IF(POLICY CMP0072)
  CMAKE_POLICY(SET CMP0072 NEW)
ENDIF(POLICY CMP0072)

PROJECT(DocCreatorSoftware)
SET( CMAKE_COLOR_MAKEFILE ON )
SET( CMAKE_VERBOSE_MAKEFILE ON )


FIND_PACKAGE(OpenGL REQUIRED)  #required ???


OPTION(BUILD_TESSERACT "Build tesseract from sources" OFF)

IF(NOT BUILD_TESSERACT)
  FIND_PACKAGE(Tesseract QUIET)
  #if Tesseract is found, Tesseract_TESSDATA_PARENT_DIR should be set
  IF(NOT Tesseract_FOUND)
    SET(BUILD_TESSERACT ON CACHE STRING "Build Leptonica/Tesseract from sources." FORCE)
  else()
    message(STATUS "Found Tesseract.")
  ENDIF(NOT Tesseract_FOUND)
ENDIF(NOT BUILD_TESSERACT)


######################################## TESSERACT

IF(BUILD_TESSERACT)
  
  message(STATUS "Will download and build leptonica and tesseract from sources")

  SET(LEPTONICA_VERSION 1.81.0)
  SET(TESSERACT_VERSION 50)
  #SET(TESSERACT_ARCHIVE_VERSION 4.1.1)
  #SET(LEPTONICA_SRC_DIR "${PROJECT_SOURCE_DIR}/thirdparty/leptonica-${LEPTONICA_VERSION}")
  #SET(TESSERACT_SRC_DIR "${PROJECT_SOURCE_DIR}/thirdparty/tesseract-${TESSERACT_ARCHIVE_VERSION}")
  #TODO: check that directories exist

  #IF (NOT EXISTS ${LEPTONICA_SRC_DIR} OR NOT EXISTS ${TESSERACT_SRC_DIR} OR NOT EXISTS ${PROJECT_SOURCE_DIR}/thirdparty/tessdata/fra.traineddata)
  IF (NOT EXISTS ${PROJECT_SOURCE_DIR}/thirdparty/tessdata/fra.traineddata)

    #SET(LEPTONICA_URL "https://github.com/DanBloomberg/leptonica/archive/${LEPTONICA_VERSION}.tar.gz")
    ##SET(LEPTONICA_URL "http://dept-info.labri.fr/~mansenca/leptonica-${LEPTONICA_VERSION}.tar.gz")
    #SET(TESSERACT_URL "https://github.com/tesseract-ocr/tesseract/archive/${TESSERACT_ARCHIVE_VERSION}.tar.gz")
    #SET(LEPTONICA_ARCHIVE "leptonica.tar.gz")
    #SET(TESSERACT_ARCHIVE "tesseract.tar.gz")
    #IF(NOT EXISTS ${PROJECT_SOURCE_DIR}/thirdparty/${TESSERACT_ARCHIVE})
      # MESSAGE(STATUS "  downloading leptonica & tesseract")
      # FILE(DOWNLOAD ${LEPTONICA_URL} ${PROJECT_SOURCE_DIR}/thirdparty/${LEPTONICA_ARCHIVE} STATUS status LOG log)
      # LIST(GET status 0 status_code)
      # LIST(GET status 1 status_string)
      # IF(NOT status_code EQUAL 0)
      #   MESSAGE(FATAL_ERROR "error: downloading ${LEPTONICA_URL} failed: status code=${status_code} string=${status_string}, log=${log}")
      # ENDIF()
      
      # FILE(DOWNLOAD ${TESSERACT_URL} ${PROJECT_SOURCE_DIR}/thirdparty/${TESSERACT_ARCHIVE} STATUS status LOG log)
      # LIST(GET status 0 status_code)
      # LIST(GET status 1 status_string)
      # IF(NOT status_code EQUAL 0)
      #   MESSAGE(FATAL_ERROR "error: downloading ${TESSERACT_URL} failed: status code=${status_code} string=${status_string}, log=${log}")
      # ENDIF()
      # MESSAGE(STATUS "  downloading done.")
    #ENDIF()

    #download tessdata
    SET(TESSDATA_URL https://github.com/tesseract-ocr/tessdata_best/raw/main)
    IF(NOT EXISTS ${PROJECT_SOURCE_DIR}/thirdparty/tessdata/eng.traineddata)
      MESSAGE(STATUS "  downloading tesseract language models")
      SET(TESSDATA_ENG_URL "${TESSDATA_URL}/eng.traineddata")
      FILE(DOWNLOAD ${TESSDATA_ENG_URL} ${PROJECT_SOURCE_DIR}/thirdparty/tessdata/eng.traineddata STATUS status LOG log)
      LIST(GET status 0 status_code)
      LIST(GET status 1 status_string)
      IF(NOT status_code EQUAL 0)
        MESSAGE(FATAL_ERROR "error: downloading ${TESSDATA_ENG_URL} failed: status code=${status_code} string=${status_string}, log=${log}")
      ENDIF()

      SET(TESSDATA_FRA_URL "${TESSDATA_URL}/fra.traineddata")
      FILE(DOWNLOAD ${TESSDATA_FRA_URL} ${PROJECT_SOURCE_DIR}/thirdparty/tessdata/fra.traineddata STATUS status LOG log)
      LIST(GET status 0 status_code)
      LIST(GET status 1 status_string)
      IF(NOT status_code EQUAL 0)
        MESSAGE(FATAL_ERROR "error: downloading ${TESSDATA_FRA_URL} failed: status code=${status_code} string=${status_string}, log=${log}")
      ENDIF()
      MESSAGE(STATUS "  downloading done.")
    ENDIF()

    # EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf ${PROJECT_SOURCE_DIR}/thirdparty/${LEPTONICA_ARCHIVE} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirdparty )
    # EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf ${PROJECT_SOURCE_DIR}/thirdparty/${TESSERACT_ARCHIVE} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirdparty )
    # MESSAGE(STATUS "  decompression done.")
  ENDIF()



  IF(NOT MSVC AND NOT MINGW)
    SET(LEPTONICA_LIBRARY "${PROJECT_BINARY_DIR}/leptonica/lib/${CMAKE_STATIC_LIBRARY_PREFIX}leptonica${CMAKE_STATIC_LIBRARY_SUFFIX}")
    SET(TESSERACT_LIBRARY "${PROJECT_BINARY_DIR}/tesseract/lib/${CMAKE_STATIC_LIBRARY_PREFIX}tesseract${CMAKE_STATIC_LIBRARY_SUFFIX}")
  ELSE(NOT MSVC AND NOT MINGW)

    #set(MY_DEFINITION 
    #  $<$<CONFIG:Debug>:definition_for_debug>
    #  $<$<CONFIG:RelWithDebInfo>:definition_for_rel_with_debug>
    #  $<$<CONFIG:Release>:definition_for_release> 
    #  $<$<CONFIG:MinSizeRel>:definition_for_tight_release>
    #  )
    
    #set(DEBUG_EXT "$<$<CONFIG:Debug>:d>")
    set(DEBUG_EXT "")
    IF(NOT CMAKE_CONFIGURATION_TYPES)
      #mono-config, e.g., ninja
      IF("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR
         "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebugInfo")
	       set(DEBUG_EXT "d")
      ENDIF()
    ENDIF()
    

    SET(LEPTONICA_LIBRARY "${PROJECT_BINARY_DIR}/leptonica/lib/${CMAKE_STATIC_LIBRARY_PREFIX}leptonica-${LEPTONICA_VERSION}${DEBUG_EXT}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    SET(TESSERACT_LIBRARY "${PROJECT_BINARY_DIR}/tesseract/lib/${CMAKE_STATIC_LIBRARY_PREFIX}tesseract${TESSERACT_VERSION}${DEBUG_EXT}${CMAKE_STATIC_LIBRARY_SUFFIX}")

  ENDIF(NOT MSVC AND NOT MINGW)


  include(ExternalProject)

  #build static library with -DSTATIC=ON
  ExternalProject_Add(leptonica
    # URL ${LEPTONICA_SRC_DIR}
    GIT_REPOSITORY https://github.com/DanBloomberg/leptonica.git
    GIT_TAG 5ad7a0a3adcfa8944c082732b69b733917a54242
    PREFIX ${PROJECT_BINARY_DIR}/leptonica
    # PATCH_COMMAND patch -p0 < ${PROJECT_SOURCE_DIR}/thirdparty/leptonica-1.80.0.patch
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/leptonica -DSTATIC=ON -DSW_BUILD=OFF -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    BUILD_BYPRODUCTS ${LEPTONICA_LIBRARY}
    )

  #-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} is to ensure that leptonica&tesseract are built with the same compiler than DocCreator
  # It is for example useful on Linux, when compiled with a non-default compiler.

  ExternalProject_Get_Property(leptonica binary_dir)
  ExternalProject_Get_Property(leptonica source_dir)
  ExternalProject_Get_Property(leptonica install_dir)
  set(leptonica_prefix "${install_dir}")

  set(Lept_INCLUDE_DIRS "${install_dir}/include;${install_dir}/include/leptonica") #(not needed to install tesseract), just later
  set(lept_lib_dirs "${install_dir}/lib")

  #library is in ${binary_dir}/src
  #headers are in ${source_dir}/src

  ADD_LIBRARY(leptonicaLib STATIC IMPORTED GLOBAL)
  SET_TARGET_PROPERTIES(leptonicaLib PROPERTIES IMPORTED_LOCATION ${LEPTONICA_LIBRARY})
  ADD_DEPENDENCIES(leptonicaLib leptonica)

  ExternalProject_Get_Property(leptonica install_dir)
  #message("leptonica install_dir=${install_dir}")
  #message("Leptonica_INCLUDE_DIRS=${Leptonica_INCLUDE_DIRS}")
  set(Lept_INCLUDE_DIRS "${install_dir}/include;${install_dir}/include/leptonica")
  #message("moi Lept_INCLUDE_DIRS=${Lept_INCLUDE_DIRS}")
  #message("Leptonica_LIBRARIES=${Leptonica_LIBRARIES}")

  #Tesseract is built with -std=c++11
  #If clang is used, we ensure that libc++ is also used.
  SET(ADDITIONAL_FLAG "")
  IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    SET(ADDITIONAL_FLAG "-DCMAKE_CXX_FLAGS=-stdlib=libc++")
  ENDIF()

  ExternalProject_Add(tesseract
    DEPENDS leptonica
    #URL ${TESSERACT_SRC_DIR}
    GIT_REPOSITORY https://github.com/tesseract-ocr/tesseract.git
    GIT_TAG 8081cae82ee5649cf28a61533aeedee2b4ce4964
    PREFIX ${PROJECT_BINARY_DIR}/tesseract
    #CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DLeptonica_DIR=${install_dir}/cmake -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/tesseract -DSTATIC=ON -DBUILD_TRAINING_TOOLS=OFF -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} ${ADDITIONAL_FLAG}
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DLeptonica_DIR=${install_dir}/lib/cmake -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/tesseract -DSTATIC=ON -DSW_BUILD=OFF -DBUILD_TRAINING_TOOLS=OFF -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} ${ADDITIONAL_FLAG}
    BUILD_BYPRODUCTS ${TESSERACT_LIBRARY}
    )

  ExternalProject_Get_Property(tesseract binary_dir)
  ExternalProject_Get_Property(tesseract source_dir)
  ExternalProject_Get_Property(tesseract install_dir)
  set(tesseract_prefix "${install_dir}")
  set(Tesseract_INCLUDE_DIRS "${install_dir}/include;${install_dir}/include/tesseract") 
  set(tess_lib_dirs "${install_dir}/lib") 

  ADD_LIBRARY(tesseractLib STATIC IMPORTED GLOBAL)
  SET_TARGET_PROPERTIES(tesseractLib PROPERTIES IMPORTED_LOCATION ${TESSERACT_LIBRARY})
  ADD_DEPENDENCIES(tesseractLib tesseract)

  #TODO: the following libraries are already searched by leptonica.
  #they are in ${install_dir}/cmake/LeptonicaTargets-release.cmake
  # but I don't know how to retrieve them.
  # As we use ExternalProject_Add(),
  # LeptonicaTargets-release.cmake is only created during the build phase (and not the configuration phase)
  # We cannot use find_package(leptonica) to get them.

  FIND_PACKAGE(ZLIB QUIET)  #ZLIB brought by PNG on linux but not on OSX 
  IF(ZLIB_FOUND)
    SET(OTHER_LIBS ${ZLIB_LIBRARIES})
  ENDIF()

  FIND_PACKAGE(PNG QUIET)
  IF(PNG_FOUND)
    SET(OTHER_LIBS "${PNG_LIBRARIES};${OTHER_LIBS}")
  ENDIF()

  FIND_PACKAGE(JPEG QUIET)
  IF(JPEG_FOUND)
    SET(OTHER_LIBS "${JPEG_LIBRARIES};${OTHER_LIBS}")
  ENDIF()

  FIND_PACKAGE(TIFF QUIET)
  IF(TIFF_FOUND)
    SET(OTHER_LIBS "${TIFF_LIBRARIES};${OTHER_LIBS}")
  ENDIF()

  FIND_PACKAGE(GIF QUIET)
  IF(GIF_FOUND)
    SET(OTHER_LIBS "${GIF_LIBRARIES};${OTHER_LIBS}")
  ENDIF()

  #message("OTHER_LIBS=${OTHER_LIBS}")
  
  FIND_PACKAGE(PkgConfig QUIET)
  IF (PKG_CONFIG_FOUND)
    #message("search with pkg config")
    pkg_check_modules(WEBP QUIET libwebp)
    IF(WEBP_FOUND)
      #SET(OTHER_LIBS "${WEBP_LIBRARIES};${OTHER_LIBS}")
      message("WEBP_FOUND=${WEBP_FOUND} by pkg: $WEBP_LIBRARY=${WEBP_LIBRARY} $WEBP_LIBRARIES=${WEBP_LIBRARIES} WEBP=${WEBP}")
    ENDIF()
    pkg_check_modules(JP2K QUIET libopenjp2)
    IF(JP2K_FOUND)
      SET(OTHER_LIBS "${JP2K_LIBRARIES};${OTHER_LIBS}")
      message("JP2K_FOUND=${JP2K_FOUND} by pkg: $JP2K_LIBRARY=${JP2K_LIBRARY} $JP2K_LIBRARIES=${JP2K_LIBRARIES} JP2K=${JP2K}")
    ENDIF()
  ENDIF()
  if(NOT WEBP)
    #message("search webp directly")
    find_path(WEBP_INCLUDE_DIR /webp/decode.h)
    find_library(WEBP_LIBRARY NAMES webp)
    if (WEBP_INCLUDE_DIR AND WEBP_LIBRARY)
      # message("WEBP_FOUND2=${WEBP_FOUND}: $WEBP_LIBRARY=${WEBP_LIBRARY} $WEBP_LIBRARIES=${WEBP_LIBRARIES} WEBP=${WEBP}")
      set(OTHER_LIBS "${WEBP_LIBRARY};${OTHER_LIBS}")
    endif()
  endif()
  if(NOT JP2K)
    #message("search jp2k directly")
    find_path(JP2K_INCLUDE_DIR /openjpeg-2.3/openjpeg.h)
    find_library(JP2K_LIBRARY NAMES openjp2)
    if (JP2K_INCLUDE_DIR AND JP2K_LIBRARY)
      message("JP2K_FOUND2=${JP2K_FOUND}: $JP2K_LIBRARY=${JP2K_LIBRARY} $JP2K_LIBRARIES=${JP2K_LIBRARIES} JP2K=${JP2K}")
      set(OTHER_LIBS "${JP2K_LIBRARY};${OTHER_LIBS}")
    endif()
  endif()

  message("OTHER_LIBS=${OTHER_LIBS}")
    
  SET(Lept_LIBRARIES "${LEPTONICA_LIBRARY};${OTHER_LIBS}")
  SET(Tesseract_LIBRARIES ${TESSERACT_LIBRARY})
  IF(MSVC OR MINGW)
    SET(Tesseract_LIBRARIES "${TESSERACT_LIBRARY};Ws2_32")
  ENDIF()

  #message("Lept_LIBRARIES=${Lept_LIBRARIES}")
  #message("Tesseract_LIBRARIES=${Tesseract_LIBRARIES}")


  #install tessdata
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/thirdparty/tessdata DESTINATION share/DocCreator)

  set(Tesseract_TESSDATA_PARENT_DIR "") #empty to be found dynamically

ENDIF()

SET(Tesseract_LIBRARIES "${Tesseract_LIBRARIES};${Lept_LIBRARIES}")
#message("Tesseract_LIBRARIES=${Tesseract_LIBRARIES}")


######################################## TESSERACT





SET(
  DocCreatorQ_headers
  src/Degradations/BleedThroughQ.hpp
  src/Degradations/BlurFilterQ.hpp
  src/Degradations/DocumentDegradation.hpp
  src/Degradations/ElasticDeformationQ.hpp
  src/Degradations/GradientDomainDegradationQ.hpp
  src/Degradations/GrayCharacterDegradationModelQ.cpp
  src/Degradations/HoleDegradationQ.hpp
  src/Degradations/NoiseDegradationQ.hpp
  src/Degradations/PhantomCharacterQ.hpp
  src/Degradations/RotationDegradationQ.hpp
  src/Degradations/ShadowBindingQ.hpp
)

SET(
  DocCreatorQ_sources
  src/Degradations/BleedThroughQ.cpp
  src/Degradations/BlurFilterQ.cpp
  src/Degradations/ElasticDeformationQ.cpp
  src/Degradations/GradientDomainDegradationQ.cpp
  src/Degradations/GrayCharacterDegradationModelQ.cpp
  src/Degradations/HoleDegradationQ.cpp
  src/Degradations/NoiseDegradationQ.cpp
  src/Degradations/PhantomCharacterQ.cpp
  src/Degradations/RotationDegradationQ.cpp
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
  #src/Degradations/ImageGenerationFromDirDialog.hpp
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
  src/GenerateDocument/ColorButton.hpp
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
  #src/Degradations/ImageGenerationFromDirDialog.cpp
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
  src/GenerateDocument/ColorButton.cpp
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
  #src/Degradations/ImageGenerationFromDirDialog.ui
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
TARGET_COMPILE_OPTIONS(${LIBNAME} PRIVATE ${WARNING_FLAGS})
SET_TARGET_PROPERTIES(${LIBNAME} PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )

ADD_EXECUTABLE(${PROGNAME} ${DocCreator_sources} ${DocCreator_headers} ${DocCreator_rcs} ${WIN_RESOURCE_FILE})
TARGET_INCLUDE_DIRECTORIES(${PROGNAME} PRIVATE ${FRAMEWORK_INCLUDE_DIRS} ${DOC3D_INCLUDE_DIRS} $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/GenerateDocument> ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Lept_INCLUDE_DIRS} ${Tesseract_INCLUDE_DIRS})
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES INSTALL_RPATH "${PROG_RPATH}")
TARGET_COMPILE_OPTIONS(${PROGNAME} PRIVATE ${WARNING_FLAGS})
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )
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
