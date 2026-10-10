
SET(TESTS_MESH_SRC
  testMesh.cpp
  )

FOREACH(FILE ${TESTS_MESH_SRC})
  get_filename_component(TRGT ${FILE} NAME_WE)
  add_executable(${TRGT} ${FILE})
  #Distortion3DMesh exports its src/ directory, for Mesh.hpp
  target_link_libraries (${TRGT} Distortion3DMesh Catch2::Catch2WithMain)
  set_target_properties(${TRGT} PROPERTIES CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )
  add_test(${TRGT} ${TRGT})
ENDFOREACH(FILE)
