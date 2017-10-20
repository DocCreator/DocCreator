#ifndef TEXCOORDCOMPUTATIONCOMMON_HPP
#define TEXCOORDCOMPUTATIONCOMMON_HPP

#include <stdint.h> //uint32_t
#include <vector>
class Mesh;

void
checkTexCoords(const Mesh &mesh);

float
moveTo_X_min(Mesh &mesh);

void
moveTo_X_min(Mesh &mesh, float min_x);

void
moveTo_XZ_min(Mesh &mesh);

void
normalizeTexCoords(Mesh &mesh);
void
normalizeTexCoordsB(Mesh &mesh);

void
getIntersectionPlanesVertices(const Mesh &mesh,
                              std::vector<uint32_t> &verticesIndices,
                              std::vector<uint32_t> &startYIndices);

#endif /* ! TEXCOORDCOMPUTATIONCOMMON_HPP */
