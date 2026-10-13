#ifndef MESH_HPP
#define MESH_HPP

#include <cmath>
#include <cstdint>
#include <vector>


class Mesh
{
public:
  /*
    B: these were six raw members: three float*, one uint32_t*, and the two
    counts, with malloc(), free(), a hand written copy, a hand written move
    and the invariant "a null buffer pointer always goes with a zero count"
    written in a comment. Two of the bugs of this file were that invariant
    broken: a count kept after its buffer was freed, and a count left
    unchanged while its buffer shrank.

    The counts are read from the buffers now, so they cannot disagree with
    them, and the four allocate/free pairs are resize() and clear(). What is
    left is the rule of zero: the destructor, the copy and the move are the
    ones the compiler writes.
  */
  std::vector<float> vertices;    //3 per vertex
  std::vector<float> normals;     //3 per vertex, empty if absent
  std::vector<float> texCoords;   //2 per vertex, empty if absent
  std::vector<uint32_t> triangles; //3 per triangle

public:
  uint32_t numVertices() const
  {
    return static_cast<uint32_t>(vertices.size() / 3);
  }

  uint32_t numTriangles() const
  {
    return static_cast<uint32_t>(triangles.size() / 3);
  }

  /*
    Give the mesh @a numVerts vertices. The normals and the texture
    coordinates are dropped, as they are indexed by vertex.

    B: this did nothing when the count was already @a numVerts, which kept
    the three buffers as they were. It still does.
  */
  void allocateVertices(uint32_t numVerts);
  void allocateNormals();
  void allocateTexCoords();
  void allocateTriangles(uint32_t numTris);

  void freeVertices();
  void freeNormals();
  void freeTexCoords();
  void freeTriangles();

  bool isValid() const { return !vertices.empty() && !triangles.empty(); }
  bool hasTexCoords() const { return !texCoords.empty(); }
  bool hasNormals() const { return !normals.empty(); }

  void copyTo(Mesh &m) const;
  void copyFrom(const Mesh &m);
  void swap(Mesh &m) noexcept;

  /*
    Free all memory and become invalid.
   */
  void clear();

  //TODO: void removeDegenerates();

  void removeDegenerateTrianglesIndices();
  void removeDuplicatedVertices();
  void removeNonReferencedVertices();

  /**
     Translate mesh to its origin and scale to fit in a unit cube.
   */
  void unitize();

  /**
     Allocate and compute (normalized) normals.
     After this call, hasNormals() is true.
   */
  void computeNormals();

  /**
     Normalize existing normals.
   */
  void normalizeNormals();

  /**
     Compute the axis aligned bounding box of the mesh.
     @a minB and @a maxB are left to 0 for an empty mesh.
   */
  void getAABB(float minB[3], float maxB[3]) const;

  /*
    Optimize indices of triangles to be more vertex cache friendly.

   */
  void optimizeTriangleOrdering();

protected:
  void copyToWithNewIndices(Mesh &m,
                            uint32_t *newVIndices,
                            uint32_t newNumVertices);
};

struct Edge
{
  uint32_t vertexIndex[2];

  bool operator==(Edge e2) const
  {
    return vertexIndex[0] == e2.vertexIndex[0] &&
           vertexIndex[1] == e2.vertexIndex[1];
  }
};

struct EdgeF
{
  uint32_t vertexIndex[2];
  uint32_t faceIndex[2];

  bool operator==(Edge e2) const
  {
    return vertexIndex[0] == e2.vertexIndex[0] &&
           vertexIndex[1] == e2.vertexIndex[1];
  }
};

//Make mesh of a sphere.
//Mesh will have (@a rings * @a sectors * 2) vertices.
Mesh
makeSphereMesh(float radius,
               unsigned int rings = 8,
               unsigned int sectors = 8,
               bool withTexCoords = false);

Mesh
makePlane();

//get edges from mesh @a mesh via HashMap [slow]
std::vector<Edge>
getEdgesH(const Mesh &mesh);

//get edges from mesh @a mesh
std::vector<Edge>
getEdges(const Mesh &mesh);

//get edgesF from @a mesh
// border edges have one of faceIndex[i] that is -1.
void
getEdgesF(const Mesh &mesh, std::vector<EdgeF> &edges);

//Get border edges (that is edges not shared by two triangles).
//Edges are in no particular order.
//Edges all have first index inferior to second index.
//Warning: it may be edges from outer border or holes.
std::vector<Edge>
getBorderEdges(const Mesh &mesh);

//Get border edges arranged by contours
//Warning edges in each contour are not sorted
void
getBorderEdgesAsContours(const Mesh &mesh,
                         std::vector<std::vector<Edge>> &outEdges);

//Get edges from largest border contour
void
getLargestBorderEdges(const Mesh &mesh, std::vector<Edge> &edges);

//get border points, that is points belonging to border edges
//Warning: they may belong to holes borders
std::vector<uint32_t>
getBorderPoints(const Mesh &mesh);

//get border points, that is points belonging to border edges [slow]
//Warning: they may belong to holes borders
std::vector<uint32_t>
getBorderPointsB(const Mesh &mesh);

//get border points of largest contour.
std::vector<uint32_t>
getLargestBorderPoints(const Mesh &mesh);

//check that edges of mesh are correct !
bool
check_EdgesH(const Mesh &mesh);

//Get adjancy triangles on each edge of each triangle.
//adjacentTris[3*i+j] (with i in [0; mesh.numTriangles[ & j in [0; 3[)
// is the adjacent triangle of triangle i on edge j.
// [for triangle abc, edge 0 is ca, edge 1 is ab, edge 2 is bc].
// For border triangles edges, the value -1 indicates no neighbour.
void
getAdjacentTriangles(const Mesh &mesh, std::vector<uint32_t> &adjacentTris);

//get connected components (of triangles) from mesh @a mesh.
//@a ccs is of size mesh.numTriangles and have for each triangle the id of its connected component.
//@a ccSizes contains the size for each connected component. ccsSizes[i] is the number of triangles with id i. ccSizes.size() is the number of connected components.
void
getConnectedComponents(const Mesh &mesh,
                       std::vector<uint32_t> &ccs,
                       std::vector<uint32_t> &ccSizes);

//Will only keep the largest connected component (of triangles) in mesh.
//Other connected components, if any, are removed.
void
keepOnlyLargestCC(Mesh &mesh);

#endif /* ! MESH_HPP */
