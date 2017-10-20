# - Try to find GLEW
# Once done this will define
# 
# GLEW_FOUND        - system has GLEW
# GLEW_INCLUDE_DIRS - GLEW include directories
# GLEW_DEFINITIONS - Compiler switches required for using GLEW
# GLEW_LIBRARIES - link these to use GLEW
#   

IF (GLEW_INCLUDE_DIRS AND GLEW_LIBRARIES)
  # Already in cache, be silent
  SET(GLEW_FIND_QUIETLY TRUE)
ENDIF (GLEW_INCLUDE_DIRS AND GLEW_LIBRARIES)




find_path(GLEW_INCLUDE_DIR NAMES GL/glew.h)

# Library
find_library(GLEW_LIBRARY NAMES GLEW)


INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set GLEW_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW DEFAULT_MSG GLEW_LIBRARY GLEW_INCLUDE_DIR)

IF (GLEW_FOUND)
  set(GLEW_INCLUDE_DIRS  ${GLEW_INCLUDE_DIR})
  set(GLEW_LIBRARIES ${GLEW_LIBRARY})
ENDIF (GLEW_FOUND)



MARK_AS_ADVANCED(GLEW_INCLUDE_DIR GLEW_LIBRARIES )