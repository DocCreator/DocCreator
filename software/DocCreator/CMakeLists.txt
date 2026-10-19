cmake_minimum_required(VERSION 3.18)

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

  #The versions name the libraries on Windows: leptonica-1.87.0.lib and
  #tesseract55.lib (major and minor only). They must be those of the
  #GIT_TAGs below.
  SET(LEPTONICA_VERSION 1.87.0)
  SET(TESSERACT_VERSION 55)

  
  # download tessdata

  INCLUDE(DownloadVerified)

  SET(TESSDATA_DST "${CMAKE_SOURCE_DIR}/thirdparty/tessdata")
  #The commit is pinned, and not "main", so that the models we check the
  #SHA256 of are the ones that are downloaded.
  SET(TESSDATA_COMMIT "e12c65a915945e4c28e237a9b52bc4a8f39a0cec")
  SET(TESSDATA_URL https://github.com/tesseract-ocr/tessdata_best/raw/${TESSDATA_COMMIT})

  SET(langname "eng")
  SET(langmodel "eng.traineddata")
  SET(langmodel_sha256 "8280aed0782fe27257a68ea10fe7ef324ca0f8d85bd2fd145d1c2b560bcb66ba")
  DOWNLOAD_VERIFIED("${TESSDATA_URL}/${langmodel}"
                    "${TESSDATA_DST}/${langmodel}"
                    ${langmodel_sha256}
                    "tesseract ${langname} language model")

  SET(langname "fra")
  SET(langmodel "fra.traineddata")
  SET(langmodel_sha256 "907743d98915c91a3906dfbf6e48b97598346698fe53aaa797e1a064ffcac913")
  DOWNLOAD_VERIFIED("${TESSDATA_URL}/${langmodel}"
                    "${TESSDATA_DST}/${langmodel}"
                    ${langmodel_sha256}
                    "tesseract ${langname} language model")



  #Names of the libraries, as leptonica and tesseract set them (their
  #OUTPUT_NAME): leptonica-<version> with MSVC (and MinGW, cf its SUFFIX),
  #tesseract<major><minor> on Windows, and a "d" in Debug, for leptonica
  #with MSVC only, for tesseract on Windows.
  #B: the name was chosen once, from CMAKE_BUILD_TYPE, which a multi-config
  # generator (Visual Studio, Ninja Multi-Config) leaves empty: the Debug
  # configuration, which builds leptonica-1.87.0d.lib and tesseract55d.lib,
  # was linked with the names of the Release libraries, which it does not
  # build (LNK1181). Each configuration now has its name, on the imported
  # targets leptonicaLib and tesseractLib below, which the program links.
  IF(MSVC OR MINGW)
    SET(LEPTONICA_BASENAME "leptonica-${LEPTONICA_VERSION}")
  ELSE()
    SET(LEPTONICA_BASENAME "leptonica")
  ENDIF()
  IF(MSVC)
    SET(LEPTONICA_DEBUG_SUFFIX "d")
  ELSE()
    SET(LEPTONICA_DEBUG_SUFFIX "")
  ENDIF()
  IF(WIN32)
    SET(TESSERACT_BASENAME "tesseract${TESSERACT_VERSION}")
    SET(TESSERACT_DEBUG_SUFFIX "d")
  ELSE()
    SET(TESSERACT_BASENAME "tesseract")
    SET(TESSERACT_DEBUG_SUFFIX "")
  ENDIF()
  SET(LEPTONICA_LIBRARY "${PROJECT_BINARY_DIR}/leptonica/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${LEPTONICA_BASENAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  SET(LEPTONICA_LIBRARY_DEBUG "${PROJECT_BINARY_DIR}/leptonica/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${LEPTONICA_BASENAME}${LEPTONICA_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  SET(TESSERACT_LIBRARY "${PROJECT_BINARY_DIR}/tesseract/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${TESSERACT_BASENAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  SET(TESSERACT_LIBRARY_DEBUG "${PROJECT_BINARY_DIR}/tesseract/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${TESSERACT_BASENAME}${TESSERACT_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

  #The library each build produces, for the generators that need to know
  #it (Ninja, Makefiles): of CMAKE_BUILD_TYPE with a single-config
  #generator; of the configuration being built with a multi-config one,
  #which needs a generator expression in the byproducts (CMake 3.20).
  IF(NOT CMAKE_CONFIGURATION_TYPES)
    IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
      SET(LEPTONICA_BYPRODUCT "${LEPTONICA_LIBRARY_DEBUG}")
      SET(TESSERACT_BYPRODUCT "${TESSERACT_LIBRARY_DEBUG}")
    ELSE()
      SET(LEPTONICA_BYPRODUCT "${LEPTONICA_LIBRARY}")
      SET(TESSERACT_BYPRODUCT "${TESSERACT_LIBRARY}")
    ENDIF()
  ELSEIF(NOT CMAKE_VERSION VERSION_LESS 3.20)
    SET(LEPTONICA_BYPRODUCT "$<IF:$<CONFIG:Debug>,${LEPTONICA_LIBRARY_DEBUG},${LEPTONICA_LIBRARY}>")
    SET(TESSERACT_BYPRODUCT "$<IF:$<CONFIG:Debug>,${TESSERACT_LIBRARY_DEBUG},${TESSERACT_LIBRARY}>")
  ELSE()
    SET(LEPTONICA_BYPRODUCT "")
    SET(TESSERACT_BYPRODUCT "")
  ENDIF()


  include(ExternalProject)

  #static libraries, with -DBUILD_SHARED_LIBS=OFF (-DSTATIC=ON before 1.84)
  ExternalProject_Add(leptonica
    # URL ${LEPTONICA_SRC_DIR}
    GIT_REPOSITORY https://github.com/DanBloomberg/leptonica.git
    #GIT_TAG 5ad7a0a3adcfa8944c082732b69b733917a54242
    #GIT_TAG f4138265b390f1921b9891d6669674d3157887d8  # 1.82.0
    #GIT_TAG b667978e86c4bf74f7fdd75f833127d2de327550  # 1.83.1
    GIT_TAG 13275a278eb55b5746e33f95fbf5a2c8f604b3ab  # 1.87.0
    PREFIX ${PROJECT_BINARY_DIR}/leptonica
    # PATCH_COMMAND patch -p0 < ${PROJECT_SOURCE_DIR}/thirdparty/leptonica-1.80.0.patch
    #Without any image format: DocCreator gives tesseract pixels, not
    # files, and does not need leptonica to read or write images. Each
    # format found on the system would otherwise be one more library for
    # the program, and the application bundles, to carry. Moreover, 1.87.0
    # links WebP and OpenJPEG as the imported targets WebP::webp and
    # openjp2, which its installed LeptonicaConfig.cmake never looks for
    # (its find_dependency() calls are under an empty if()), so configuring
    # tesseract against it fails.
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/leptonica -DBUILD_SHARED_LIBS=OFF -DSW_BUILD=OFF -DENABLE_ZLIB=OFF -DENABLE_PNG=OFF -DENABLE_GIF=OFF -DENABLE_JPEG=OFF -DENABLE_TIFF=OFF -DENABLE_WEBP=OFF -DENABLE_OPENJPEG=OFF -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    BUILD_BYPRODUCTS ${LEPTONICA_BYPRODUCT}
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
  SET_TARGET_PROPERTIES(leptonicaLib PROPERTIES
    IMPORTED_LOCATION ${LEPTONICA_LIBRARY}
    IMPORTED_LOCATION_DEBUG ${LEPTONICA_LIBRARY_DEBUG})
  ADD_DEPENDENCIES(leptonicaLib leptonica)

  ExternalProject_Get_Property(leptonica install_dir)
  #message("leptonica install_dir=${install_dir}")
  #message("Leptonica_INCLUDE_DIRS=${Leptonica_INCLUDE_DIRS}")
  set(Lept_INCLUDE_DIRS "${install_dir}/include;${install_dir}/include/leptonica")
  #message("moi Lept_INCLUDE_DIRS=${Lept_INCLUDE_DIRS}")
  #message("Leptonica_LIBRARIES=${Leptonica_LIBRARIES}")

  #Without curl and libarchive, which tesseract links when it finds them
  #and which DocCreator does not link: it reads plain .traineddata files.
  #Tesseract is built with the compiler DocCreator is built with, and its
  #default standard library: it is linked into the same executable.
  #B: Clang used to be given -stdlib=libc++ here. On Linux, where Clang uses
  # libstdc++ by default, that built tesseract against another standard
  # library than DocCreator, and failed outright where libc++ is not
  # installed. On macOS, libc++ is already the default.

  ExternalProject_Add(tesseract
    DEPENDS leptonica
    #URL ${TESSERACT_SRC_DIR}
    GIT_REPOSITORY https://github.com/tesseract-ocr/tesseract.git
    #GIT_TAG 8081cae82ee5649cf28a61533aeedee2b4ce4964
    #GIT_TAG 5ad5325a0aa8effc47ca033625b6a51682f82767 #5.2.0
    #GIT_TAG 71af4542990adae6d9f31d6657068bc0139c6ec4 #5.3.1
    GIT_TAG db0ec62f81b0737fbbe184d8fea40af5738f8eef #5.5.3
    PREFIX ${PROJECT_BINARY_DIR}/tesseract
    ##CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DLeptonica_DIR=${install_dir}/cmake -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/tesseract -DSTATIC=ON -DBUILD_TRAINING_TOOLS=OFF -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    #CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DLeptonica_DIR=${install_dir}/lib/cmake -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/tesseract -DSTATIC=ON -DSW_BUILD=OFF -DBUILD_TRAINING_TOOLS=OFF -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DLeptonica_DIR=${install_dir}/lib/cmake/leptonica -DPKG_CONFIG_EXECUTABLE="" -DLeptonica_FOUND=TRUE -DDISABLE_CURL=ON -DDISABLE_ARCHIVE=ON -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/tesseract -DBUILD_SHARED_LIBS=OFF -DSW_BUILD=OFF -DBUILD_TRAINING_TOOLS=OFF -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    BUILD_BYPRODUCTS ${TESSERACT_BYPRODUCT}
    )

  ExternalProject_Get_Property(tesseract binary_dir)
  ExternalProject_Get_Property(tesseract source_dir)
  ExternalProject_Get_Property(tesseract install_dir)
  set(tesseract_prefix "${install_dir}")
  set(Tesseract_INCLUDE_DIRS "${install_dir}/include;${install_dir}/include/tesseract") 
  set(tess_lib_dirs "${install_dir}/lib") 

  ADD_LIBRARY(tesseractLib STATIC IMPORTED GLOBAL)
  SET_TARGET_PROPERTIES(tesseractLib PROPERTIES
    IMPORTED_LOCATION ${TESSERACT_LIBRARY}
    IMPORTED_LOCATION_DEBUG ${TESSERACT_LIBRARY_DEBUG})
  ADD_DEPENDENCIES(tesseractLib tesseract)

  #leptonica is built without the image formats (cf. above): the static
  #library needs no other library. The libraries of the formats that
  #were searched here and linked to the program were those found on the
  #system, WebP and OpenJPEG included, which leptonica did not use.
  SET(OTHER_LIBS "")

    
  #the imported targets, which have the library of each configuration
  SET(Lept_LIBRARIES "leptonicaLib;${OTHER_LIBS}")
  SET(Tesseract_LIBRARIES tesseractLib)
  IF(MSVC OR MINGW)
    SET(Tesseract_LIBRARIES "tesseractLib;Ws2_32")
  ENDIF()

  #message("Lept_LIBRARIES=${Lept_LIBRARIES}")
  #message("Tesseract_LIBRARIES=${Tesseract_LIBRARIES}")


  #install tessdata
  install(DIRECTORY ${TESSDATA_DST} DESTINATION share/DocCreator)

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
  #B: were 0.2 and 0.7-beta2: the version of the project
  SET( MACOSX_BUNDLE_SHORT_VERSION_STRING ${DocCreator_VERSION} )
  SET( MACOSX_BUNDLE_VERSION ${DocCreator_VERSION} )
  SET( MACOSX_BUNDLE_LONG_VERSION_STRING "Version ${DocCreator_VERSION}" )
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
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )

ADD_EXECUTABLE(${PROGNAME} ${DocCreator_sources} ${DocCreator_headers} ${DocCreator_rcs} ${WIN_RESOURCE_FILE})
TARGET_INCLUDE_DIRECTORIES(${PROGNAME} PRIVATE ${FRAMEWORK_INCLUDE_DIRS} ${DOC3D_INCLUDE_DIRS} $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/GenerateDocument> ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Lept_INCLUDE_DIRS} ${Tesseract_INCLUDE_DIRS})
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES INSTALL_RPATH "${PROG_RPATH}")
TARGET_COMPILE_OPTIONS(${PROGNAME} PRIVATE ${WARNING_FLAGS})
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES
  CXX_STANDARD 17
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


set(PROG_RPATH "@loader_path/../lib") #B: necessary as we install libs (LipsumQt, framework, doc3D) in ../lib directory [Non-bundle case]
set(PROG_RPATH "${PROG_RPATH};${LEPTONICA_INSTALL_PATH};${TESSERACT_INSTALL_PATH}") #B: added to be able to launch app without install
#message("PROG_RPATH=${PROG_RPATH}")
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES INSTALL_RPATH "${PROG_RPATH}")
#Qt and OpenCV libs are marked @rpath in exe: the directories of the
#linked libraries are added to the install RPATH. (The Qt directory was
#computed from the location of QtCore, which is two directories deeper in
#a Qt 6 framework than in a Qt 5 one.)
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)


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
  #B: a target of ALL, copy_qt_dlls, ran windeployqt and copied the OpenCV
  # DLLs into CMAKE_INSTALL_PREFIX at build time: with the default prefix,
  # into Program Files, which fails without elevation; "cmake --install
  # --prefix X" got no DLL; they were not in the install manifest; and the
  # DLLs of an older build stayed in the prefix, then in the installer.
  # They are now installed by "cmake --install", in the prefix it is given:
  # windeployqt deploys into an empty directory of the build tree, which is
  # then installed (and recorded in the manifest).
  set(_opencv_dlls "")
  foreach(LIB ${OpenCV_LIBS})
    if(TARGET ${LIB})
      get_target_property(_type ${LIB} TYPE)
      if(_type STREQUAL "SHARED_LIBRARY")
        list(APPEND _opencv_dlls "$<TARGET_FILE:${LIB}>")
      endif()
    endif()
  endforeach()
  if(_opencv_dlls)
    install(FILES ${_opencv_dlls} DESTINATION programs COMPONENT applications)
  endif()

  if(TARGET Qt${QT_VERSION_MAJOR}::windeployqt)
    get_target_property(QT_WINDEPLOYQT_EXECUTABLE Qt${QT_VERSION_MAJOR}::windeployqt IMPORTED_LOCATION)
  else()
    #B: the hint was the path of qmake, a file, not its directory
    get_target_property(_qmake_location Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_path "${_qmake_location}" DIRECTORY)
    find_program(QT_WINDEPLOYQT_EXECUTABLE windeployqt
      HINTS "${_qt_bin_path}"
      DOC "Path to the windeployqt utility.")
  endif()
  if(QT_WINDEPLOYQT_EXECUTABLE)
    set(_qt_deploy_dir "${CMAKE_CURRENT_BINARY_DIR}/qt_deploy")
    install(CODE "
      file(REMOVE_RECURSE \"${_qt_deploy_dir}\")
      file(MAKE_DIRECTORY \"${_qt_deploy_dir}\")
      execute_process(
        COMMAND \"${QT_WINDEPLOYQT_EXECUTABLE}\"
          $<IF:$<CONFIG:Debug>,--debug,--release>
          --opengl --no-compiler-runtime --verbose 1
          --dir \"${_qt_deploy_dir}\"
          \"$<TARGET_FILE:${PROGNAME}>\"
        RESULT_VARIABLE _result)
      if(_result)
        message(FATAL_ERROR \"windeployqt failed: \${_result}\")
      endif()
      file(INSTALL \"${_qt_deploy_dir}/\"
        DESTINATION \"\${CMAKE_INSTALL_PREFIX}/programs\"
        USE_SOURCE_PERMISSIONS)
      " COMPONENT applications)
  else()
    message(WARNING "windeployqt not found: the Qt DLLs will not be installed")
  endif()

ENDIF(WIN32)

#B: a block of CPack settings was left here, with paths that do not exist
#and version 1.0.0: the installer is made by bundlers/win and bundlers/mac.
