# - Try to find Tesseract 
# Once done, it will define
#
# Tesseract_FOUND - system has Tesseract
# Tesseract_INCLUDE_DIRS - Tesseract include directories
# Tesseract_LIBRARIES - link these to use Tesseract
# Tesseract_TESSDATA_PARENT_DIR - tessdata parent dir if found

FIND_PACKAGE(Lept QUIET) #REQUIRED)


IF (NOT WIN32)
  FIND_PACKAGE(PkgConfig QUIET)
  IF (PKGCONFIG_FOUND)
    pkg_check_modules(Tesseract_PKGCONF QUIET tesseract)
  ENDIF(PKGCONFIG_FOUND)
ENDIF (NOT WIN32)

# Include dir
#B: Homebrew (/opt/homebrew on Apple silicon) was not searched
find_path(Tesseract_INCLUDE_DIR
  NAMES tesseract/baseapi.h
  PATHS ${Tesseract_PKGCONF_INCLUDE_DIRS}
  /usr/include/
  /usr/local/include
  /opt/homebrew/include
  )

# Library
find_library(Tesseract_LIBRARY
  NAMES tesseract
  PATHS ${Tesseract_PKGCONF_LIBRARY_DIRS}
  /usr/lib64
  /usr/lib
  /usr/local/lib
  /opt/homebrew/lib
  )

#tessdata
#B: the models of tesseract 5 (Ubuntu 24.04: tesseract-ocr/5/tessdata), and
# of Homebrew (share/tessdata), were not found: tesseract was then built
# from sources at each configuration. The share directory of the prefix
# pkg-config gives is searched too.
SET(_tessdata_prefixes "")
FOREACH(_prefix ${Tesseract_PKGCONF_PREFIX} /usr /usr/local /opt/homebrew)
  LIST(APPEND _tessdata_prefixes
    ${_prefix}/share
    ${_prefix}/share/tesseract
    ${_prefix}/share/tesseract-ocr
    ${_prefix}/share/tesseract-ocr/5
    ${_prefix}/share/tesseract-ocr/4.00)
ENDFOREACH()
find_path(Tesseract_TESSDATA_PARENT_DIR
  NAMES tessdata/eng.traineddata
  PATHS ${Tesseract_PKGCONF_DATAROOTDIR}
  ${_tessdata_prefixes}
  NO_DEFAULT_PATH
  )

INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set Tesseract_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Tesseract DEFAULT_MSG Tesseract_LIBRARY Tesseract_INCLUDE_DIR Lept_FOUND Tesseract_TESSDATA_PARENT_DIR)

IF(TESSERACT_FOUND)
  SET(Tesseract_FOUND TRUE)
ENDIF(TESSERACT_FOUND)

IF(Tesseract_FOUND)
  set(Tesseract_INCLUDE_DIRS ${Tesseract_INCLUDE_DIR} ${Lept_INCLUDE_DIRS})
  set(Tesseract_LIBRARIES ${Tesseract_LIBRARY} ${Lept_LIBRARIES})
ENDIF(Tesseract_FOUND)
