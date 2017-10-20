#ifndef BRS_HPP
#define BRS_HPP

/*
  BRS mesh format
  - stores only triangles
  - stores vertices with, optionally, texcoords and/or normals [per vertex].
  - everything is stored in binary little endian.
  - triangles/vertices may be re-ordered, and data compressed.

  The header is defined this way:
  - 4 chars: 'B', 'R', 'S' and
    a char that is the type of data:
      1 for vertices only, 3 for vertices & texcoords,
      5 for vertices & normals, 7 for vertices, texcoords & normals.
  - an uint32 for nbVertices
  - an uint32 for nbTriangles

*/

#include <string>
class Mesh;

bool
isBRSFile(const std::string &filename);

bool
readBRS(const std::string &filename, Mesh &mesh);

/**
   @warning it is recommanded to call optimizeTriangleOrdering() on @a mesh before calling this function.

   @warning triangles or/and vertices may be re-ordered when @a mesh is saved.
 */
bool
writeBRS(const std::string &filename, const Mesh &mesh);

#endif /* ! BRS_HPP */
