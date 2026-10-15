
SET(TESTS_MESH_SRC
  testMesh.cpp
  testBRS.cpp
  )

FOREACH(FILE ${TESTS_MESH_SRC})
  get_filename_component(TRGT ${FILE} NAME_WE)
  add_executable(${TRGT} ${FILE})
  #Distortion3DMesh exports its src/ directory, for Mesh.hpp
  target_link_libraries (${TRGT} Distortion3DMesh IndexBufferCompression Catch2::Catch2WithMain)
  #the index buffer compressor, to build BRS data no writer would
  target_include_directories(${TRGT} PRIVATE ${CMAKE_SOURCE_DIR}/framework/src/Degradations/Distortion3DModel/thirdparty)
  target_compile_definitions(${TRGT} PRIVATE MESH_DATA_DIR="${CMAKE_SOURCE_DIR}/data/Mesh")
  set_target_properties(${TRGT} PROPERTIES CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )
  add_test(${TRGT} ${TRGT})
ENDFOREACH(FILE)
