# - Try to find FFTW3 
# Once done, it will define
#
# FFTW3_FOUND - system has FFTW3
# FFTW3_INCLUDE_DIRS - FFTW3 include directories
# FFTW3_DEFINITIONS - Compiler switches required for using FFTW3
# FFTW3_LIBRARIES - link these to use FFTW3

#include(LibFindMacros)


IF (FFTW3_INCLUDE_DIRS AND FFTW3_LIBRARIES)
   # in cache already
   SET(FFTW3_FIND_QUIETLY TRUE)
ENDIF (FFTW3_INCLUDE_DIRS AND FFTW3_LIBRARIES)


#libfind_pkg_check_modules(FFTW3_PKGCONF fftw3)
IF (NOT WIN32)
  include(FindPkgConfig)
  pkg_check_modules(FFTW3_PKGCONF fftw3)
  SET(FFTW3_DEFINITIONS ${FFTW3_PKGCONF_CFLAGS_OTHER})
ENDIF (NOT WIN32)


# Include dir
find_path(FFTW3_INCLUDE_DIR
  NAMES fftw3.h
  PATHS ${FFTW3_PKGCONF_INCLUDE_DIRS} 
    $ENV{FFTWDIR} 
    ${INCLUDE_INSTALL_DIR} 
    ${FFTW3_DIR}/include
    ${FFTW3_DIR}
    ${FFTW3_HOME}/include
    ${FFTW3_HOME}
    $ENV{FFTW3_DIR}/include
    $ENV{FFTW3_DIR}
    $ENV{FFTW3_HOME}/include
    $ENV{FFTW3_HOME}
    /usr/include
    /usr/local/include )

# Library
find_library(FFTW3_LIBRARY
  NAMES 
  	fftw3 libfftw libfftw3 libfftw3-3
  PATHS 
  	${FFTW3_PKGCONF_LIBRARY_DIRS} 
  	$ENV{FFTWDIR} 
	${LIB_INSTALL_DIR} 
	${FFTW3_DIR}/lib
        ${FFTW3_DIR}
	${FFTW3_HOME}/lib
    ${FFTW3_HOME}
    $ENV{FFTW3_DIR}/lib
    $ENV{FFTW3_DIR}
    $ENV{FFTW3_HOME}/lib
    $ENV{FFTW3_HOME}
    /usr/lib
    /usr/local/lib )	



INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set FFTW3_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFTW3 DEFAULT_MSG FFTW3_LIBRARY FFTW3_INCLUDE_DIR)

IF (FFTW3_FOUND)
  set(FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR})
  set(FFTW3_LIBRARIES ${FFTW3_LIBRARY})
ELSE (FFTW3_FOUND)
   MESSAGE("FFTW3 not found.")  
ENDIF (FFTW3_FOUND)


MARK_AS_ADVANCED(FFTW3_INCLUDE_DIRS FFTW3_LIBRARIES)

