#ifndef MESH_TO_SURFACE_MESH_HPP
#define MESH_TO_SURFACE_MESH_HPP

#include <surface_mesh/Surface_mesh.h>
class Mesh;

void
mesh2surfaceMesh(const Mesh &m, surface_mesh::Surface_mesh &sm);

void
surfaceMesh2Mesh(const surface_mesh::Surface_mesh &sm, Mesh &m);

#endif /* ! MESH_TO_SURFACE_MESH_HPP */
