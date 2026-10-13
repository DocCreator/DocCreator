
add_subdirectory(Degradations)

IF(NOT BUILD_ONLY_DEGRADATIONS)
  add_subdirectory(IO)
  add_subdirectory(Layout)
  add_subdirectory(Models)
  add_subdirectory(Utils)
ENDIF()

#Distortion3DMesh is only built when the 3D distortion is
IF(TARGET Distortion3DMesh)
  add_subdirectory(Mesh)
ENDIF()

#DocCreatorPy is only built when the python wrapper is
IF(TARGET DocCreatorPy)
  add_subdirectory(python)
ENDIF()

