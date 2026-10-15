/*
  Tests for the BRS mesh reader, readBRS(), and the decoding of its
  decompressed data, decodeBRSData().

  readBRS() used to trust every count and size of the file: truncated
  files, sizes that did not match, a compressed index buffer shorter than
  its triangles, or a size near SIZE_MAX, were reads and writes out of
  bounds. A file the reader refuses must give false and an empty mesh; a
  file it accepts must give a mesh whose indices are all vertices.

  Run under AddressSanitizer, cf cmake/sanitizers/README.txt.
*/

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

#include "Mesh.hpp"
#include "brs.hpp"

#include "IndexBufferCompression/indexbuffercompression.h"
#include "IndexBufferCompression/writebitstream.h"

namespace {

  const std::string PLANE = std::string(MESH_DATA_DIR) + "/PlaneRegular.brs";

  //numbers of PlaneRegular.brs
  const uint32_t PLANE_NUM_VERTICES = 29640;
  const uint32_t PLANE_NUM_TRIANGLES = 58548;

  //the types of the header, cf brs.cpp
  const int VERTICES = 1;
  const int TEXCOORDS = 2;
  const int NORMALS = 4;

  std::vector<unsigned char>
  readBytes(const std::string &filename)
  {
    std::ifstream in(filename, std::ios::binary);
    return std::vector<unsigned char>(std::istreambuf_iterator<char>(in), {});
  }

  void
  writeBytes(const std::string &filename, const std::vector<unsigned char> &bytes)
  {
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    REQUIRE(out);
  }

  //@return true if every index of @a m is one of its vertices
  bool
  indicesAreVertices(const Mesh &m)
  {
    return std::all_of(m.triangles.begin(), m.triangles.end(),
                       [&](uint32_t i) { return i < m.numVertices(); });
  }

  bool
  isEmpty(const Mesh &m)
  {
    return m.numVertices() == 0 && m.numTriangles() == 0
      && m.normals.empty() && m.texCoords.empty();
  }

  //@return a mesh that is not empty, to check that a refused file empties it
  Mesh
  someMesh()
  {
    Mesh m;
    m.allocateVertices(3);
    m.allocateTriangles(1);
    m.triangles = { 0, 1, 2 };
    return m;
  }

  //@return a filename for this test, in the current directory
  std::string
  tmpFilename(const std::string &name)
  {
    return "tmp_testBRS_" + name + ".brs";
  }

  /*
    @return the decompressed data of a BRS file, as writeBRS() lays it out:
    the size of the compressed index buffer, the compressed index buffer of
    @a triangles, then @a numVertices vertices, and the texture coordinates
    and the normals if @a type has them.
  */
  std::vector<unsigned char>
  makeData(int type, uint32_t numVertices, const std::vector<uint32_t> &triangles,
           uint32_t indicesVertexCount)
  {
    WriteBitstream wb(4096);
    std::vector<uint32_t> vertexRemap(indicesVertexCount);
    CompressIndexBuffer(triangles.data(),
                        static_cast<uint32_t>(triangles.size() / 3),
                        vertexRemap.data(),
                        indicesVertexCount,
                        IBCF_PER_TRIANGLE_PREFIX_ENTROPY,
                        wb);
    wb.Finish();

    const size_t indicesSize = wb.ByteSize();
    size_t floats = size_t(numVertices) * 3;
    if (type & TEXCOORDS)
      floats += size_t(numVertices) * 2;
    if (type & NORMALS)
      floats += size_t(numVertices) * 3;

    std::vector<unsigned char> data(sizeof(size_t) + indicesSize + floats * sizeof(float));
    memcpy(data.data(), &indicesSize, sizeof(size_t));
    memcpy(data.data() + sizeof(size_t), wb.RawData(), indicesSize);
    for (size_t i = 0; i < floats; ++i) {
      const float f = static_cast<float>(i);
      memcpy(data.data() + sizeof(size_t) + indicesSize + i * sizeof(float), &f, sizeof(float));
    }
    return data;
  }

  //a strip of @a numTriangles triangles over @a numTriangles + 2 vertices
  std::vector<uint32_t>
  strip(uint32_t numTriangles)
  {
    std::vector<uint32_t> t;
    for (uint32_t i = 0; i < numTriangles; ++i) {
      t.push_back(i);
      t.push_back(i + 1);
      t.push_back(i + 2);
    }
    return t;
  }

} //anonymous namespace


TEST_CASE("readBRS reads the plane of data/Mesh")
{
  Mesh m;
  REQUIRE(readBRS(PLANE, m));
  REQUIRE(m.numVertices() == PLANE_NUM_VERTICES);
  REQUIRE(m.numTriangles() == PLANE_NUM_TRIANGLES);
  REQUIRE(m.hasNormals());
  REQUIRE(m.hasTexCoords());
  REQUIRE(m.isValid());
  REQUIRE(indicesAreVertices(m));
}

TEST_CASE("readBRS reads back what writeBRS wrote")
{
  Mesh a;
  REQUIRE(readBRS(PLANE, a));
  const std::string filename = tmpFilename("roundtrip");
  REQUIRE(writeBRS(filename, a));

  Mesh b;
  REQUIRE(readBRS(filename, b));
  REQUIRE(b.numVertices() == a.numVertices());
  REQUIRE(b.numTriangles() == a.numTriangles());
  REQUIRE(b.hasNormals() == a.hasNormals());
  REQUIRE(b.hasTexCoords() == a.hasTexCoords());
  REQUIRE(indicesAreVertices(b));

  //vertices may be reordered: the same positions, in some order
  auto positions = [](const Mesh &m) {
    std::vector<std::array<float, 3>> p(m.numVertices());
    for (uint32_t i = 0; i < m.numVertices(); ++i)
      p[i] = { m.vertices[3 * i], m.vertices[3 * i + 1], m.vertices[3 * i + 2] };
    std::sort(p.begin(), p.end());
    return p;
  };
  REQUIRE(positions(a) == positions(b));

  std::remove(filename.c_str());
}

TEST_CASE("readBRS refuses a damaged file and leaves the mesh empty")
{
  const std::vector<unsigned char> plane = readBytes(PLANE);
  REQUIRE(plane.size() > 100);

  //offsets of the header, cf brs.hpp: type, numVertices, numTriangles,
  // then the size of the decompressed data
  const size_t OFFSET_TYPE = 0;
  const size_t OFFSET_NUM_VERTICES = 4;
  const size_t OFFSET_NUM_TRIANGLES = 8;
  const size_t OFFSET_TOTAL_SIZE = 12;

  auto refused = [](const std::vector<unsigned char> &bytes) {
    const std::string filename = tmpFilename("damaged");
    writeBytes(filename, bytes);
    Mesh m = someMesh();
    const bool ok = readBRS(filename, m);
    std::remove(filename.c_str());
    return ! ok && isEmpty(m);
  };
  auto with32 = [&](size_t offset, uint32_t value) {
    std::vector<unsigned char> b = plane;
    memcpy(b.data() + offset, &value, sizeof(value));
    return b;
  };
  auto with64 = [&](size_t offset, uint64_t value) {
    std::vector<unsigned char> b = plane;
    memcpy(b.data() + offset, &value, sizeof(value));
    return b;
  };

  SECTION("truncated")
  {
    for (size_t n : { size_t(0), size_t(3), size_t(4), size_t(11), size_t(12),
                      size_t(19), size_t(20), size_t(27), size_t(28),
                      plane.size() / 2, plane.size() - 1 }) {
      INFO("truncated at " << n);
      REQUIRE(refused(std::vector<unsigned char>(plane.begin(), plane.begin() + n)));
    }
  }

  SECTION("counts that do not match the data")
  {
    REQUIRE(refused(with32(OFFSET_NUM_VERTICES, 0)));
    REQUIRE(refused(with32(OFFSET_NUM_VERTICES, PLANE_NUM_VERTICES + 1)));
    REQUIRE(refused(with32(OFFSET_NUM_VERTICES, PLANE_NUM_VERTICES - 1)));
    REQUIRE(refused(with32(OFFSET_NUM_VERTICES, std::numeric_limits<uint32_t>::max())));
    REQUIRE(refused(with32(OFFSET_NUM_TRIANGLES, 0)));
    REQUIRE(refused(with32(OFFSET_NUM_TRIANGLES, PLANE_NUM_TRIANGLES + 1)));
    //2^32 - 1 triangles used to be a 48 GB allocation
    REQUIRE(refused(with32(OFFSET_NUM_TRIANGLES, std::numeric_limits<uint32_t>::max())));
  }

  SECTION("a decompressed size that does not match")
  {
    uint64_t totalSize = 0;
    memcpy(&totalSize, plane.data() + OFFSET_TOTAL_SIZE, sizeof(totalSize));
    REQUIRE(refused(with64(OFFSET_TOTAL_SIZE, 0)));
    REQUIRE(refused(with64(OFFSET_TOTAL_SIZE, totalSize - 1)));
    REQUIRE(refused(with64(OFFSET_TOTAL_SIZE, totalSize + 1)));
    REQUIRE(refused(with64(OFFSET_TOTAL_SIZE, std::numeric_limits<uint64_t>::max())));
  }

  SECTION("a type that is not one, or not this file's")
  {
    for (unsigned char type : { 0, 1, 2, 3, 5, 6, 8, 255 }) {
      std::vector<unsigned char> b = plane;
      b[OFFSET_TYPE] = type;
      INFO("type " << int(type));
      REQUIRE(refused(b));
    }
  }

  SECTION("damaged compressed data: refused, or a mesh whose indices are vertices")
  {
    for (size_t offset : { size_t(28), size_t(29), size_t(40), size_t(100),
                           size_t(1000), plane.size() / 2, plane.size() - 10 }) {
      std::vector<unsigned char> b = plane;
      b[offset] ^= 0xFF;
      const std::string filename = tmpFilename("flipped");
      writeBytes(filename, b);
      Mesh m = someMesh();
      const bool ok = readBRS(filename, m);
      std::remove(filename.c_str());
      INFO("byte flipped at " << offset);
      if (ok)
        REQUIRE((m.isValid() && indicesAreVertices(m)));
      else
        REQUIRE(isEmpty(m));
    }
  }

  SECTION("not a file")
  {
    Mesh m = someMesh();
    REQUIRE_FALSE(readBRS(tmpFilename("does_not_exist"), m));
    REQUIRE(isEmpty(m));
  }
}

TEST_CASE("decodeBRSData")
{
  const uint32_t numTriangles = 50;
  const uint32_t numVertices = numTriangles + 2;
  const std::vector<uint32_t> triangles = strip(numTriangles);
  const int type = VERTICES | TEXCOORDS | NORMALS;

  const std::vector<unsigned char> data = makeData(type, numVertices, triangles, numVertices);
  size_t indicesSize = 0;
  memcpy(&indicesSize, data.data(), sizeof(size_t));

  auto refused = [](int t, uint32_t nv, uint32_t nt, const std::vector<unsigned char> &d) {
    Mesh m = someMesh();
    const bool ok = decodeBRSData(t, nv, nt, d.data(), d.size(), m);
    return ! ok && isEmpty(m);
  };
  auto withIndicesSize = [&](size_t size) {
    std::vector<unsigned char> d = data;
    memcpy(d.data(), &size, sizeof(size));
    return d;
  };

  SECTION("valid data")
  {
    Mesh m;
    REQUIRE(decodeBRSData(type, numVertices, numTriangles, data.data(), data.size(), m));
    REQUIRE(m.numVertices() == numVertices);
    REQUIRE(m.numTriangles() == numTriangles);
    REQUIRE(m.hasNormals());
    REQUIRE(m.hasTexCoords());
    REQUIRE(indicesAreVertices(m));
  }

  SECTION("an index buffer size near SIZE_MAX")
  {
    //(size + 7) & ~7 wrapped to 0: 8 bytes were allocated, size copied
    for (size_t size : { std::numeric_limits<size_t>::max(),
                         std::numeric_limits<size_t>::max() - 3,
                         std::numeric_limits<size_t>::max() - 7 }) {
      INFO("size " << size);
      REQUIRE(refused(type, numVertices, numTriangles, withIndicesSize(size)));
    }
  }

  SECTION("an index buffer size that does not match the data")
  {
    REQUIRE(refused(type, numVertices, numTriangles, withIndicesSize(0)));
    REQUIRE(refused(type, numVertices, numTriangles, withIndicesSize(indicesSize - 1)));
    REQUIRE(refused(type, numVertices, numTriangles, withIndicesSize(indicesSize + 1)));
  }

  SECTION("data shorter than its header says")
  {
    for (size_t n : { size_t(0), size_t(4), sizeof(size_t), data.size() - 1 }) {
      std::vector<unsigned char> d(data.begin(), data.begin() + n);
      INFO("data of " << n << " bytes");
      REQUIRE(refused(type, numVertices, numTriangles, d));
    }
  }

  SECTION("more triangles than the index buffer holds")
  {
    //the decoder asks the bit reader for more than the buffer holds: it
    // used to read past it
    const std::vector<unsigned char> few = makeData(type, numVertices, strip(2), numVertices);
    REQUIRE(refused(type, numVertices, numTriangles, few));
  }

  SECTION("indices that are not vertices")
  {
    //indices up to numVertices - 1, but data for 10 vertices only
    const std::vector<unsigned char> d = makeData(type, 10, triangles, numVertices);
    REQUIRE(refused(type, 10, numTriangles, d));
  }

  SECTION("types that are not ones")
  {
    for (int t : { 0, 2, 6, 8, -1 })
      REQUIRE(refused(t, numVertices, numTriangles, data));
  }

  SECTION("no data")
  {
    Mesh m = someMesh();
    REQUIRE_FALSE(decodeBRSData(type, numVertices, numTriangles, nullptr, 0, m));
    REQUIRE(isEmpty(m));
  }
}
