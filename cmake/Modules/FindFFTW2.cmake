# - Try to find FFTW2 
# Once done, it will define
#
# FFTW2_FOUND - system has FFTW2
# FFTW2_INCLUDE_DIRS - FFTW2 include directories
# FFTW2_DEFINITIONS - Compiler switches required for using FFTW2
# FFTW2_LIBRARIES - link these to use FFTW2

#include(LibFindMacros)


IF (FFTW2_INCLUDE_DIRS AND FFTW2_LIBRARIES)
   # in cache already
   SET(FFTW2_FIND_QUIETLY TRUE)
ENDIF (FFTW2_INCLUDE_DIRS AND FFTW2_LIBRARIES)


#libfind_pkg_check_modules(FFTW2_PKGCONF fftw3)
IF (NOT WIN32)
  include(FindPkgConfig)
  pkg_check_modules(FFTW2_PKGCONF fftw3)
  SET(FFTW2_DEFINITIONS ${FFTW2_PKGCONF_CFLAGS_OTHER})
ENDIF (NOT WIN32)


# Include dir
find_path(FFTW2_INCLUDE_DIR
  NAMES fftw.h
  PATHS ${FFTW2_PKGCONF_INCLUDE_DIRS} 
    $ENV{FFTWDIR} 
    ${INCLUDE_INSTALL_DIR} 
    ${FFTW2_DIR}/include
    ${FFTW2_DIR}
    ${FFTW2_HOME}/include
    ${FFTW2_HOME}
    $ENV{FFTW2_DIR}/include
    $ENV{FFTW2_DIR}
    $ENV{FFTW2_HOME}/include
    $ENV{FFTW2_HOME}
    /usr/include
    /usr/local/include )

# Library
find_library(FFTW2_LIBRARY
  NAMES 
  	fftw fftw2 libfftw libfftw2 
  PATHS 
  	${FFTW2_PKGCONF_LIBRARY_DIRS} 
  	$ENV{FFTWDIR} 
	${LIB_INSTALL_DIR} 
	${FFTW2_DIR}/lib
        ${FFTW2_DIR}
	${FFTW2_HOME}/lib
    ${FFTW2_HOME}
    $ENV{FFTW2_DIR}/lib
    $ENV{FFTW2_DIR}
    $ENV{FFTW2_HOME}/lib
    $ENV{FFTW2_HOME}
    /usr/lib
    /usr/local/lib )	



INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set FFTW2_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFTW2 DEFAULT_MSG FFTW2_LIBRARY FFTW2_INCLUDE_DIR)

IF (FFTW2_FOUND)
  set(FFTW2_INCLUDE_DIRS ${FFTW2_INCLUDE_DIR})
  set(FFTW2_LIBRARIES ${FFTW2_LIBRARY})
ELSE (FFTW2_FOUND)
   MESSAGE("FFTW2 not found.")  
ENDIF (FFTW2_FOUND)


MARK_AS_ADVANCED(FFTW2_INCLUDE_DIRS FFTW2_LIBRARIES)

