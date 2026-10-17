/*
  readOBJ() refused a texture coordinate or normal index out of range only
  when there were as many of them as vertices: otherwise the index was kept,
  and the re-ordering of the texture coordinates or of the normals read
  them out of their bounds.
*/

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <string>

#include "obj.hpp"

namespace {

  std::string
  writeFile(const std::string &name, const std::string &content)
  {
    std::ofstream out(name.c_str(), std::ios::binary);
    out << content;
    return name;
  }

  //four vertices, two triangles; fewer texture coordinates than vertices,
  //which makes readOBJ() re-order them
  const char *VERTICES =
    "v 0 0 0\n"
    "v 1 0 0\n"
    "v 0 1 0\n"
    "v 1 1 0\n";

} //namespace

TEST_CASE( "Testing readOBJ" )
{
  SECTION("texture coordinates fewer than the vertices are re-ordered")
  {
    const std::string filename = writeFile(
      "tmp_testOBJ_valid.obj",
      std::string(VERTICES) +
      "vt 0 0\n"
      "vt 1 0\n"
      "vt 0 1\n"
      "f 1/1 2/2 3/3\n"
      "f 2/2 4/1 3/3\n");
    Mesh mesh;
    REQUIRE( readOBJ(filename, mesh) );
    REQUIRE( mesh.numVertices() == 4 );
    REQUIRE( mesh.hasTexCoords() );
    REQUIRE( mesh.texCoords.size() == 8 );
    //vertex 4 has the texture coordinate 1
    REQUIRE( mesh.texCoords[2 * 3 + 0] == 0.f );
    REQUIRE( mesh.texCoords[2 * 3 + 1] == 0.f );
    //vertex 2 has the texture coordinate 2
    REQUIRE( mesh.texCoords[2 * 1 + 0] == 1.f );
    REQUIRE( mesh.texCoords[2 * 1 + 1] == 0.f );
  }

  SECTION("a texture coordinate index out of range is refused")
  {
    //5 texture coordinates for 4 vertices would not be re-ordered: 3 here
    const std::string filename = writeFile(
      "tmp_testOBJ_bad_vt.obj",
      std::string(VERTICES) +
      "vt 0 0\n"
      "vt 1 0\n"
      "vt 0 1\n"
      "f 1/9 2/2 3/3\n"
      "f 2/2 4/1 3/3\n");
    Mesh mesh;
    REQUIRE_FALSE( readOBJ(filename, mesh) );
    REQUIRE_FALSE( mesh.isValid() );
  }

  SECTION("a normal index out of range is refused")
  {
    const std::string filename = writeFile(
      "tmp_testOBJ_bad_vn.obj",
      std::string(VERTICES) +
      "vn 0 0 1\n"
      "vn 0 1 0\n"
      "f 1//5 2//1 3//2\n"
      "f 2//1 4//2 3//1\n");
    Mesh mesh;
    REQUIRE_FALSE( readOBJ(filename, mesh) );
    REQUIRE_FALSE( mesh.isValid() );
  }
}
