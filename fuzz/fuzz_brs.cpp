/*
  libFuzzer target for the BRS mesh reader.

  readBRS() reads a header of counts and sizes, decompresses LZHAM data of
  the size it gives, and decodes from it a compressed index buffer and the
  vertex arrays. Every one of those numbers comes from the file, and before
  they were checked a truncated file, a size near SIZE_MAX or an index
  buffer shorter than its triangles were reads and writes out of bounds.

  Random bytes rarely get through LZHAM, so each input is also given
  directly to decodeBRSData(), which decodes what LZHAM gives: the first
  9 bytes as the type and the two counts, the rest as the data. That is
  where the index decoder and its bit reader are reached.

  Whatever the input, a mesh that is accepted must have every index be a
  vertex.

  Cf fuzz/README.txt for how to build and run this.
*/

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

#include "Mesh.hpp"
#include "brs.hpp"

namespace {

  void
  checkMesh(bool ok, const Mesh &mesh)
  {
    if (! ok) {
      if (mesh.numVertices() != 0 || mesh.numTriangles() != 0)
        abort(); //a refused input leaves the mesh empty
      return;
    }
    for (const uint32_t index : mesh.triangles) {
      if (index >= mesh.numVertices())
        abort();
    }
  }

} //namespace

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  //readBRS() takes a path, so the input has to reach the disk
  {
    char filename[] = "/tmp/fuzz_brs_XXXXXX";
    const int fd = mkstemp(filename);
    if (fd >= 0) {
      const ssize_t written = write(fd, data, size);
      close(fd);
      if (written == static_cast<ssize_t>(size)) {
        Mesh mesh;
        checkMesh(readBRS(filename, mesh), mesh);
      }
      unlink(filename);
    }
  }

  if (size >= 9) {
    const int type = data[0];
    uint32_t numVertices = 0;
    uint32_t numTriangles = 0;
    memcpy(&numVertices, data + 1, sizeof(numVertices));
    memcpy(&numTriangles, data + 5, sizeof(numTriangles));
    Mesh mesh;
    checkMesh(decodeBRSData(type, numVertices, numTriangles, data + 9, size - 9, mesh), mesh);
  }

  return 0;
}
