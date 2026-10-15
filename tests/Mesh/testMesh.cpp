/*
  Tests for Mesh's memory management: the rule of five, the
  "a null buffer always goes with a zero count" invariant, and
  removeDegenerateTrianglesIndices().

  Mesh owns four malloc'ed buffers, so these tests are most useful when run
  with a sanitizer:
    -fsanitize=address,undefined
  and, to make reads of uninitialised members deterministic:
    -ftrivial-auto-var-init=pattern
*/

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

#include "Mesh.hpp"

namespace {

  /*
    @return a mesh with @a numVerts vertices and @a numTris triangles,
    filled with recognizable values. All triangle indices are < @a numVerts.
  */
  Mesh
  makeMesh(uint32_t numVerts, uint32_t numTris, bool withNormals, bool withTexCoords)
  {
    REQUIRE(numVerts > 0);

    Mesh m;
    m.allocateVertices(numVerts);
    m.allocateTriangles(numTris);
    if (withNormals)
      m.allocateNormals();
    if (withTexCoords)
      m.allocateTexCoords();

    REQUIRE(! m.vertices.empty());
    REQUIRE((numTris == 0 || ! m.triangles.empty()));

    for (uint32_t i = 0; i < 3 * numVerts; ++i)
      m.vertices[i] = static_cast<float>(i);
    for (uint32_t i = 0; i < 3 * numTris; ++i)
      m.triangles[i] = i % numVerts;
    if (withNormals)
      for (uint32_t i = 0; i < 3 * numVerts; ++i)
        m.normals[i] = static_cast<float>(numVerts - i);
    if (withTexCoords)
      for (uint32_t i = 0; i < 2 * numVerts; ++i)
        m.texCoords[i] = 0.5f * static_cast<float>(i);

    return m;
  }

  //@return true if @a a and @a b hold the same mesh in distinct buffers.
  bool
  isDistinctEqualCopy(const Mesh &a, const Mesh &b)
  {
    if (a.numVertices() != b.numVertices() || a.numTriangles() != b.numTriangles())
      return false;
    if (a.hasNormals() != b.hasNormals() ||
        a.hasTexCoords() != b.hasTexCoords())
      return false;

    //buffers must not be shared
    //distinct buffers: the two must not share their storage
    if (a.numVertices() > 0 && a.vertices.data() == b.vertices.data())
      return false;
    if (a.numTriangles() > 0 && a.triangles.data() == b.triangles.data())
      return false;

    return a.vertices == b.vertices && a.triangles == b.triangles &&
           a.normals == b.normals && a.texCoords == b.texCoords;
  }

  bool
  isEmpty(const Mesh &m)
  {
    return m.numVertices() == 0 && m.vertices.empty() &&
           m.normals.empty() && m.texCoords.empty() &&
           m.numTriangles() == 0 && m.triangles.empty();
  }

} //anonymous namespace


TEST_CASE("Mesh is empty when default constructed")
{
  //This is what makes the copy constructor safe: copyFrom() starts by
  //comparing numVertices and calling free() on the buffers.
  const Mesh m;

  REQUIRE(isEmpty(m));
  REQUIRE_FALSE(m.isValid());
  REQUIRE_FALSE(m.hasNormals());
  REQUIRE_FALSE(m.hasTexCoords());
}

TEST_CASE("Mesh copy construction produces an independent copy")
{
  const Mesh a = makeMesh(100, 50, true, true);
  const Mesh b(a);

  REQUIRE(isDistinctEqualCopy(a, b));
}

TEST_CASE("Mesh copy assignment")
{
  SECTION("onto an empty mesh")
  {
    const Mesh a = makeMesh(100, 50, true, false);
    Mesh b;
    b = a;

    REQUIRE(isDistinctEqualCopy(a, b));
    REQUIRE_FALSE(b.hasNormals() == false); //a has normals
    REQUIRE_FALSE(b.hasTexCoords());
  }

  SECTION("drops normals and texCoords the source does not have")
  {
    //Same vertex count, so allocateVertices() does not reallocate and used
    //to leave the destination's own normals/texCoords in place.
    const Mesh src = makeMesh(100, 50, false, false);
    Mesh dst = makeMesh(100, 50, true, true);
    dst = src;

    REQUIRE_FALSE(dst.hasNormals());
    REQUIRE_FALSE(dst.hasTexCoords());
    REQUIRE(isDistinctEqualCopy(src, dst));
  }

  SECTION("after clear(), with the same counts")
  {
    //clear() must leave counts at 0, otherwise allocateVertices()/
    //allocateTriangles() take their early-return branch and nothing is
    //allocated before the copy writes.
    const Mesh a = makeMesh(100, 50, true, true);
    Mesh b = makeMesh(100, 50, true, true);
    b.clear();
    REQUIRE(isEmpty(b));

    b = a;
    REQUIRE(isDistinctEqualCopy(a, b));
  }

  SECTION("onto a mesh with different counts")
  {
    const Mesh a = makeMesh(100, 50, true, true);
    Mesh b = makeMesh(7, 3, false, false);
    b = a;

    REQUIRE(isDistinctEqualCopy(a, b));
  }
}

TEST_CASE("Mesh move construction steals the buffers")
{
  Mesh a = makeMesh(100, 50, true, true);
  const float *const verticesBefore = a.vertices.data();
  const uint32_t *const trianglesBefore = a.triangles.data();

  const Mesh b(std::move(a));

  //the move takes the very buffers, it does not copy them
  REQUIRE(b.vertices.data() == verticesBefore);
  REQUIRE(b.triangles.data() == trianglesBefore);
  REQUIRE(b.numVertices() == 100);
  REQUIRE(b.numTriangles() == 50);
  REQUIRE(b.hasNormals());
  REQUIRE(b.hasTexCoords());

  //a moved-from mesh is empty, and thus still usable
  REQUIRE(isEmpty(a));
  a.clear();
  REQUIRE(isEmpty(a));
  a = makeMesh(7, 3, false, false);
  REQUIRE(a.numVertices() == 7);
}

TEST_CASE("Mesh move assignment steals the buffers and frees its own")
{
  Mesh a = makeMesh(100, 50, true, true);
  const float *const verticesBefore = a.vertices.data();

  Mesh b = makeMesh(40, 20, false, false);
  b = std::move(a);

  //the move takes the very buffer, it does not copy it
  REQUIRE(b.vertices.data() == verticesBefore);
  REQUIRE(b.numVertices() == 100);
  REQUIRE(b.numTriangles() == 50);
  REQUIRE(b.hasNormals());
  REQUIRE(b.hasTexCoords());
  REQUIRE(isEmpty(a));
}

TEST_CASE("Mesh self assignment is harmless")
{
  Mesh a = makeMesh(10, 5, true, true);
  const float *const verticesBefore = a.vertices.data();

  Mesh &aRef = a; //through a reference, to keep the compiler quiet

  a = aRef;
  REQUIRE(a.vertices.data() == verticesBefore);
  REQUIRE(a.numVertices() == 10);
  REQUIRE(a.hasNormals());
  REQUIRE(a.hasTexCoords());

  /*
    B: self move assignment used to leave the mesh untouched, because the
    move assignment operator was written by hand and began with
    "if (this != &m)". The compiler written one moves each vector into
    itself, which the standard leaves valid but unspecified, and which
    empties them in practice.

    What is required of it is that the object stays usable, which is what
    is checked here. No code of this project moves a mesh onto itself.
  */
  a = std::move(aRef);
  REQUIRE(a.numVertices() * 3 == a.vertices.size());
  REQUIRE(a.numTriangles() * 3 == a.triangles.size());
  a.clear();
  REQUIRE(isEmpty(a));
}

TEST_CASE("Mesh swap exchanges meshes of different shapes")
{
  //swap() used to assert that both meshes had the same shape *after* the
  //swap, which fired as soon as they differed.
  SECTION("member swap")
  {
    Mesh a = makeMesh(100, 50, true, true);
    Mesh b = makeMesh(30, 10, false, false);
    const float *const aVertices = a.vertices.data();
    const float *const bVertices = b.vertices.data();

    a.swap(b);

    REQUIRE(a.vertices.data() == bVertices);
    REQUIRE(b.vertices.data() == aVertices);
    REQUIRE(a.numVertices() == 30);
    REQUIRE(b.numVertices() == 100);
    REQUIRE_FALSE(a.hasNormals());
    REQUIRE(b.hasNormals());
  }

  SECTION("swap with an empty mesh")
  {
    Mesh a = makeMesh(100, 50, true, true);
    Mesh b;

    a.swap(b);

    REQUIRE(isEmpty(a));
    REQUIRE(b.numVertices() == 100);
    REQUIRE(b.isValid());
  }

  SECTION("std::swap, as keepOnlyLargestCC() uses it")
  {
    //With no move operations this went through the copy constructor.
    Mesh a = makeMesh(100, 50, true, true);
    Mesh b = makeMesh(30, 10, false, false);
    const float *const aVertices = a.vertices.data();
    const float *const bVertices = b.vertices.data();

    std::swap(a, b);

    REQUIRE(a.vertices.data() == bVertices);
    REQUIRE(b.vertices.data() == aVertices);
    REQUIRE(a.numVertices() == 30);
    REQUIRE(b.numVertices() == 100);
  }
}

TEST_CASE("Mesh copyTo produces an independent copy")
{
  const Mesh a = makeMesh(50, 20, true, true);

  SECTION("onto an empty mesh")
  {
    Mesh b;
    a.copyTo(b);
    REQUIRE(isDistinctEqualCopy(a, b));
  }

  SECTION("onto a mesh that has normals the source lacks")
  {
    const Mesh src = makeMesh(50, 20, false, false);
    Mesh dst = makeMesh(50, 20, true, true);
    src.copyTo(dst);

    REQUIRE_FALSE(dst.hasNormals());
    REQUIRE_FALSE(dst.hasTexCoords());
    REQUIRE(isDistinctEqualCopy(src, dst));
  }
}

TEST_CASE("Mesh handles empty meshes")
{
  const Mesh empty;

  Mesh copied(empty);
  REQUIRE(isEmpty(copied));

  Mesh assigned = makeMesh(10, 5, true, true);
  assigned = empty;
  REQUIRE(isEmpty(assigned));

  Mesh moved(std::move(copied));
  REQUIRE(isEmpty(moved));

  moved.clear();
  REQUIRE(isEmpty(moved));
}

TEST_CASE("getAABB does not dereference the vertices of an empty mesh")
{
  const Mesh empty;
  REQUIRE(empty.vertices.empty());

  float minB[3] = { 1.f, 2.f, 3.f };
  float maxB[3] = { 4.f, 5.f, 6.f };
  empty.getAABB(minB, maxB);

  for (int k = 0; k < 3; ++k) {
    REQUIRE(minB[k] == 0.f);
    REQUIRE(maxB[k] == 0.f);
  }
}

TEST_CASE("unitize keeps the vertices of a degenerate mesh finite")
{
  SECTION("an empty mesh")
  {
    Mesh m;
    m.unitize();
    REQUIRE(m.numVertices() == 0);
  }

  SECTION("a single vertex")
  {
    Mesh m;
    m.allocateVertices(1);
    m.vertices[0] = 10.f;
    m.vertices[1] = -20.f;
    m.vertices[2] = 30.f;

    m.unitize();

    //the vertex is moved to the origin, and not to NaN by a division by the
    //null largest dimension
    for (int k = 0; k < 3; ++k) {
      REQUIRE(std::isfinite(m.vertices[k]));
      REQUIRE(m.vertices[k] == 0.f);
    }
  }

  SECTION("a planar mesh")
  {
    //all the vertices in the z=5 plane
    Mesh m;
    m.allocateVertices(3);
    const float verts[9] = { 0.f, 0.f, 5.f,
			     2.f, 0.f, 5.f,
			     0.f, 4.f, 5.f };
    std::copy(verts, verts + 9, m.vertices.begin());

    m.unitize();

    for (uint32_t i = 0; i < 3 * m.numVertices(); ++i) {
      REQUIRE(std::isfinite(m.vertices[i]));
    }
    //the largest dimension is 4, so the mesh fits in [-1; 1]
    for (uint32_t i = 0; i < 3 * m.numVertices(); ++i) {
      REQUIRE(std::abs(m.vertices[i]) <= 1.f);
    }
  }
}

TEST_CASE("Mesh never keeps a non-zero count with a null buffer")
{
  Mesh m = makeMesh(100, 50, true, true);

  m.freeVertices();
  REQUIRE(m.vertices.empty());
  REQUIRE(m.numVertices() == 0);

  m.freeTriangles();
  REQUIRE(m.triangles.empty());
  REQUIRE(m.numTriangles() == 0);

  //re-allocating the same counts must really allocate
  m.allocateVertices(100);
  m.allocateTriangles(50);
  REQUIRE(! m.vertices.empty());
  REQUIRE(! m.triangles.empty());
  REQUIRE(m.numVertices() == 100);
  REQUIRE(m.numTriangles() == 50);

  m.clear();
  REQUIRE(isEmpty(m));
}

TEST_CASE("removeDegenerateTrianglesIndices updates the triangle count")
{
  //4 vertices, 4 triangles, two of which have a repeated index.
  Mesh m;
  m.allocateVertices(4);
  m.allocateTriangles(4);
  REQUIRE(m.isValid());

  for (uint32_t i = 0; i < 12; ++i)
    m.vertices[i] = static_cast<float>(i);

  const uint32_t tris[12] = { 0, 1, 2,   //kept
                              0, 0, 1,   //degenerate
                              1, 2, 3,   //kept
                              2, 2, 2 }; //degenerate
  std::copy(tris, tris + 12, m.triangles.begin());

  m.removeDegenerateTrianglesIndices();

  REQUIRE(m.numTriangles() == 2);
  REQUIRE(m.isValid());

  const uint32_t expected[6] = { 0, 1, 2, 1, 2, 3 };
  REQUIRE(std::equal(expected, expected + 6, m.triangles.begin()));

  //Every remaining index must address an existing vertex. With a stale
  //count, the tail of the buffer was never written and was used as indices.
  for (uint32_t i = 0; i < 3 * m.numTriangles(); ++i) {
    REQUIRE(m.triangles[i] < m.numVertices());
  }

  //This reads vertices[] through triangles[]: out-of-range indices show up
  //as a heap overflow under ASan.
  m.computeNormals();
  REQUIRE(m.hasNormals());
}

TEST_CASE("removeDegenerateTrianglesIndices keeps a mesh without degenerates")
{
  Mesh m = makeMesh(4, 2, false, false);
  const uint32_t tris[6] = { 0, 1, 2, 1, 2, 3 };
  std::copy(tris, tris + 6, m.triangles.begin());

  m.removeDegenerateTrianglesIndices();

  REQUIRE(m.numTriangles() == 2);
  REQUIRE(std::equal(tris, tris + 6, m.triangles.begin()));
}

TEST_CASE("getLargestBorderEdges of a mesh with no border is empty")
{
  //No contour, so no largest one: the index of the largest stayed one past
  //the end of the contours, and was read there when asserts are off.
  std::vector<Edge> edges(3);

  SECTION("an empty mesh")
  {
    const Mesh m;
    getLargestBorderEdges(m, edges);
    REQUIRE(edges.empty());
  }

  SECTION("a closed mesh: a tetrahedron")
  {
    Mesh m = makeMesh(4, 4, false, false);
    const uint32_t tris[12] = { 0, 1, 2,  0, 3, 1,  1, 3, 2,  2, 3, 0 };
    std::copy(tris, tris + 12, m.triangles.begin());

    getLargestBorderEdges(m, edges);
    REQUIRE(edges.empty());
  }
}

TEST_CASE("getLargestBorderEdges of a quad is its four sides")
{
  Mesh m = makeMesh(4, 2, false, false);
  const uint32_t tris[6] = { 0, 1, 2,  0, 2, 3 };
  std::copy(tris, tris + 6, m.triangles.begin());

  std::vector<Edge> edges;
  getLargestBorderEdges(m, edges);
  REQUIRE(edges.size() == 4);
}
