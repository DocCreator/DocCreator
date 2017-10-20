# - Try to find Leptonica
# Once done, it will define
#
# Lept_FOUND - system has Lept
# Lept_INCLUDE_DIRS - Lept include directories
# Lept_LIBRARIES - link these to use Lept

FIND_PACKAGE(ZLIB) 

IF (NOT WIN32)
  FIND_PACKAGE(PkgConfig QUIET)
  IF (PKGCONFIG_FOUND)
    pkg_check_modules(Lept_PKGCONF QUIET lept)
  ENDIF(PKGCONFIG_FOUND)
ENDIF (NOT WIN32)

# Include dir
find_path(Lept_INCLUDE_DIR
  NAMES leptonica/allheaders.h
  PATHS ${Lept_PKGCONF_INCLUDE_DIRS}
  /usr/include/
  /usr/local/include
  )

# Library
find_library(Lept_LIBRARY
  NAMES lept
  PATHS ${Lept_PKGCONF_LIBRARY_DIRS}
  /usr/lib64
  /usr/lib
  /usr/local/lib
  )

INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set GIRL_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lept DEFAULT_MSG Lept_LIBRARY Lept_INCLUDE_DIR)

IF(LEPT_FOUND)
  SET(Lept_FOUND TRUE)
ENDIF(LEPT_FOUND)

IF(Lept_FOUND)
  set(Lept_INCLUDE_DIRS ${Lept_INCLUDE_DIR})
  set(Lept_LIBRARIES ${Lept_LIBRARY} ${ZLIB_LIBRARIES})
ENDIF(Lept_FOUND)
