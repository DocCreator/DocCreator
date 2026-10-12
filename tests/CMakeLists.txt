
add_subdirectory(Degradations)

IF(NOT BUILD_ONLY_DEGRADATIONS)
  add_subdirectory(IO)
  add_subdirectory(Models)
  add_subdirectory(Utils)
ENDIF()

#Distortion3DMesh is only built when the 3D distortion is
IF(TARGET Distortion3DMesh)
  add_subdirectory(Mesh)
ENDIF()

