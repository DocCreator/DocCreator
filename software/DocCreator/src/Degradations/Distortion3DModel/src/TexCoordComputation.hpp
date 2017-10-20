#ifndef TEXCOORDCOMPUTATION_HPP
#define TEXCOORDCOMPUTATION_HPP

#include <stdint.h> //uint32_t
#include <vector>
class Mesh;

void
computeTexCoords0(Mesh &mesh);

void
computeTexCoords1(Mesh &mesh);

void
DEBUG_checkEdges(const Mesh &mesh);

//DEBUG
void
getIntersectionPlanesVertices(const Mesh &mesh,
                              std::vector<uint32_t> &verticesIndices,
                              std::vector<uint32_t> &startYIndices);

#endif /* ! TEXCOORDCOMPUTATION_HPP */
