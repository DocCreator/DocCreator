
#include "Mesh.hpp"
#include "brs.hpp"
#include "obj.hpp"
#include <cassert>
#include <iostream>

#include <algorithm>
#include <cctype>
#include <string>

static std::string
getExtension(const std::string &filename)
{
  std::string::size_type pos = filename.rfind('.');
  if (pos != std::string::npos)
    return std::string(filename, pos + 1);
  return std::string();
}

static std::string
toupper(const std::string &s)
{
  //std::transform(s.begin(), s.end(), s.begin(), (int(*)(int))std::toupper);

  std::string so = s;
  for (auto &c : so)
    c = toupper(c);
  return so;
}

int
main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << "USage: " << argv[0]
              << " inputMeshFilename outputMeshFilename\n";
    exit(10);
  }

  const char *inputMeshFilename = argv[1];
  const char *outputMeshFilename = argv[2];

  Mesh mesh;

  if (isBRSFile(inputMeshFilename)) {
    const bool readOk = readBRS(inputMeshFilename, mesh);
    if (!readOk) {
      std::cerr << "ERROR: unable to read BRS file: " << inputMeshFilename
                << "\n";
      exit(10);
    }
  } else if (isOBJFile(inputMeshFilename)) {
    const bool readOk = readOBJ(inputMeshFilename, mesh);
    if (!readOk) {
      std::cerr << "ERROR: unable to read OBJ file: " << inputMeshFilename
                << "\n";
      exit(10);
    }
  } else {
    std::cerr << "ERROR: unhandled file format for input file: "
              << inputMeshFilename << "\n";
    exit(10);
  }

  if (!mesh.isValid()) {
    std::cerr << "Mesh is not valid !\n";
    exit(10);
  }
  if (!check_EdgesH(mesh)) {
    std::cerr << "Unable to handle this mesh \n";
    exit(10);
  }

  mesh.removeDegenerateTrianglesIndices();
  mesh.removeNonReferencedVertices();

  //remove tex coords
  mesh.freeTexCoords();

  assert(mesh.isValid());

  const std::string ext = toupper(getExtension(outputMeshFilename));

  if (ext == "OBJ") {
    writeOBJ(outputMeshFilename, mesh);
    std::cout << "wrote " << outputMeshFilename << "\n";
  } else if (ext == "BRS") {
    writeBRS(outputMeshFilename, mesh);
    std::cout << "wrote " << outputMeshFilename << "\n";
  } else {
    std::cerr << "ERROR: unhandled file format for output file: "
              << outputMeshFilename << "\n";
  }

  return 0;
}
