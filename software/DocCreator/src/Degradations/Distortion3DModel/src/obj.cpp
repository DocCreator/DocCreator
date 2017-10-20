#include "obj.hpp"

#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream> //DEBUG
#include <sstream>
#include <vector>

bool
firstPass(std::ifstream &in,
          size_t &numVertices,
          size_t &numTexCoords,
          size_t &numNormals,
          size_t &numFaces,
          std::string &line)
{
  if (in.bad())
    return false;

  numVertices = 0;
  numTexCoords = 0;
  numNormals = 0;
  numFaces = 0;

  while (!in.eof() && !in.bad()) {
    std::getline(in, line);

    if (!line.empty()) {

      if (line[0] == 'v') {
        if (line[1] == ' ') {
          ++numVertices;
        } else if (line[1] == 't') {
          ++numTexCoords;
        } else if (line[1] == 'n') {
          ++numNormals;
        }
      } else if (line[0] == 'f') {
        ++numFaces;
      }
    }
  }

  if (numVertices <= 1 || numFaces == 0) {
    return false;
  }
  return true;
}

bool
isOBJFile(const std::string &filename)
{
  std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
  if (!in) {
    return false;
  }

  //OBJ files do not have a magic number.
  //we have to parse them to check that they are OBJ files...

  size_t numVertices;
  size_t numTexCoords;
  size_t numNormals;
  size_t numFaces;
  std::string line;
  return firstPass(in, numVertices, numTexCoords, numNormals, numFaces, line);
}

bool
readOBJ(const std::string &filename, Mesh &mesh)
{
  std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
  if (!in) {
    return false;
  }

  size_t numVertices;
  size_t numTexCoords;
  size_t numNormals;
  size_t numFaces;
  std::string line;

  const bool firstPassOk =
    firstPass(in, numVertices, numTexCoords, numNormals, numFaces, line);
  if (!firstPassOk) {
    std::cerr << "OBJ: not a valid OBJ file\n";
    return false;
  }

  //std::cerr<<"numVertices="<<numVertices<<" numTexCoords="<<numTexCoords<<" numNormals="<<numNormals<<" numFaces="<<numFaces<<"\n";

  //REM: faces in OBJ format describe independent triangles.
  //For example a vertice, shared with 4 faces, could have a different texcoord (or normal) on each face.
  //But we could also have two independent triangles that have the same texcoord indice (it allows to 'compress' texcoords or normals)

  //REM: if numTexCoors <= numVertices, we keep texcoords (we may have to re-order them),
  // otherwise we remove texcoords.
  // Same thing for normals.

  mesh.allocateVertices(numVertices);

  std::vector<uint32_t> texCoordsIndices;
  std::vector<uint32_t> normalsIndices;
  bool parseTexCoords = false;
  bool parseNormals = false;

  normalsIndices.resize(3 * numFaces);
  std::vector<float> tmpNormals;
  float *normals = nullptr;
  if (numNormals != 0) {
    parseNormals = true;
    if (numNormals != numVertices)
      std::cerr << "OBJ: warning: numNormals=" << numNormals
                << " != numVertices=" << numVertices << "\n";

    if (numNormals != numVertices) {
      tmpNormals.resize(3 * numNormals);
      normals = &tmpNormals[0];
    } else {
      mesh.allocateNormals();
      normals = mesh.normals;
    }
    assert(normals != nullptr);
  }

  texCoordsIndices.resize(3 * numFaces);
  std::vector<float> tmpTexCoords;
  float *texCoords = nullptr;
  if (numTexCoords != 0) {
    parseTexCoords = true;
    if (numTexCoords != numVertices)
      std::cerr << "OBJ: warning: numTexCoords=" << numTexCoords
                << " != numVertices=" << numVertices << "\n";
    if (numTexCoords != numVertices) {
      tmpTexCoords.resize(2 * numTexCoords);
      texCoords = &tmpTexCoords[0];
    } else {
      mesh.allocateTexCoords();
      texCoords = mesh.texCoords;
    }
    assert(texCoords != nullptr);
  }

  mesh.allocateTriangles(numFaces);

  size_t expectNumSlashesForFaces = 0;
  if (parseNormals) {
    expectNumSlashesForFaces = 2;
  } else if (parseTexCoords) {
    expectNumSlashesForFaces = 1;
  }
  size_t expectNumSlashesForFaces3 = 3 * expectNumSlashesForFaces;

  //std::cerr<<"parseNormals="<<parseNormals<<" (normals="<<normals<<") parseTexCoords="<<parseTexCoords<<" (texCoords="<<texCoords<<") expectNumSlashesForFaces="<<expectNumSlashesForFaces<<"\n";

  in.clear();
  in.seekg(std::ios_base::beg);

  size_t vi = 0;
  size_t ni = 0;
  size_t ti = 0;
  size_t fi = 0;

  std::stringstream ss;
  char slash;
  while (!in.eof() && !in.bad()) {
    std::getline(in, line);

    if (!line.empty()) {

      if (line[0] == 'v') {
        if (line[1] == ' ') { //vertice

          ss.clear();
          ss.str(std::string(line, 2));
          assert(vi < mesh.numVertices);
          ss >> mesh.vertices[3 * vi + 0];
          ss >> mesh.vertices[3 * vi + 1];
          if (ss.fail() || ss.bad()) {
            mesh.clear();
            return false;
          }
          ss >> mesh.vertices[3 * vi + 2];
          ++vi;
          if (ss.fail() || ss.bad()) {
            std::cerr << "ERROR: readOBJ(): unable to parse line: " << line
                      << "\n";
            mesh.clear();
            return false;
          }

        } else if (line[1] == 't') { //texcoord
          ss.clear();
          ss.str(std::string(line, 2));
          assert(ti < numTexCoords);
          ss >> texCoords[2 * ti + 0];
          ss >> texCoords[2 * ti + 1];
          ++ti;
          if (ss.fail() || ss.bad()) {
            std::cerr << "ERROR: readOBJ(): unable to parse line: " << line
                      << "\n";
            mesh.clear();
            return false;
          }
        } else if (line[1] == 'n') { //normal
          ss.clear();
          ss.str(std::string(line, 2));
          assert(ni < numNormals);
          ss >> normals[3 * ni + 0];
          ss >> normals[3 * ni + 1];
          ss >> normals[3 * ni + 2];
          ++ni;
          if (ss.fail() || ss.bad()) {
            std::cerr << "ERROR: readOBJ(): unable to parse line: " << line
                      << "\n";
            mesh.clear();
            return false;
          }
        }
      } else if (line[0] == 'f') { //face

        size_t numSlashes = 0;
        for (int i = 1; line[i] != '\0'; ++i) { //start from 1;
          if (line[i] == '/')
            ++numSlashes;
        }

        //std::cerr<<"DEBUG face : line=+"<<line<<"+ numSlashes="<<numSlashes<<" expectNumSlashesForFaces3="<<expectNumSlashesForFaces3<<"\n";

        if (numSlashes != expectNumSlashesForFaces3) {
          //all faces does not have texcoords or normals
          // or are not triangles.

          std::cerr << "OBJ: for line=" << line << " the number of slashes ("
                    << numSlashes << ") is defferent than the expected number ("
                    << expectNumSlashesForFaces3 << ")\n";
          std::cerr << "OBJ: it means that all the faces have not the same "
                       "texcoords or normals, or some faces are not "
                       "triangles\n";

          mesh.clear();
          return false;
        }

        ss.clear();
        ss.str(std::string(line, 2));

        //std::cerr<<"DEBUG ss.str()=+"<<ss.str()<<"+  ss.good()="<<ss.good()<<" ss.bad()="<<ss.bad()<<"\n";

        for (int i = 0; i < 3; ++i) {
          uint32_t ind;
          ss >> ind;

          //std::cerr<<"ind="<<ind<<"\n";

          if (ss.fail() || ss.bad()) {
            std::cerr << "OBJ: parse error for vertex in faces\n";
            std::cerr << "line: " << line << "\n";
            return false;
          }
          ind -= 1; //indices start from 1 in OBJ format
          if (ind >= numVertices) {
            std::cerr << "OBJ: invalid vertex index " << ind + 1
                      << " in faces\n";
            std::cerr << "line: " << line << "\n";
            mesh.clear();
            return false;
          }
          assert(fi < numFaces);
          mesh.triangles[3 * fi + i] = ind;

          if (parseTexCoords) {
            ss >> slash;
            ss >> ind;
            //std::cerr<<"slash="<<slash<<" ind="<<ind<<"\n";

            if (ss.fail() || ss.bad()) {
              std::cerr << "OBJ: parse error for texcoord in faces\n";
              mesh.clear();
              return false;
            }
            ind -= 1;
            if (ind >= numTexCoords) {
              std::cerr << "OBJ: invalid texcoords index " << ind + 1
                        << " in faces\n";
              std::cerr << "line: " << line << "\n";
              if (numTexCoords == numVertices) {
                mesh.clear();
                return false;
              }
            }
            //if (numTexCoords == numVertices) {
            //if (! (fi < numFaces && 3*fi+i < texCoordsIndices.size()))
            //std::cerr<<"numTexCoords="<<numTexCoords<<", fi="<<fi<<" <? numFaces="<<numFaces<<", i="<<i<<", 3*fi+i="<<3*fi+i<<" <?  texCoordsIndices.size()="<<texCoordsIndices.size()<<"\n";
            assert(fi < numFaces && 3 * fi + i < texCoordsIndices.size());
            texCoordsIndices[3 * fi + i] = ind;
            //}
          } else if (parseNormals) {
            ss >> slash; //there are two slashes for vertex+normals
          }

          if (parseNormals) {
            ss >> slash;
            ss >> ind;
            if (ss.fail() || ss.bad()) {
              std::cerr << "OBJ: parse error for normals in faces\n";
              mesh.clear();
              return false;
            }
            ind -= 1;
            if (ind >= numNormals) {
              std::cerr << "OBJ: invalid normals index " << ind + 1
                        << " in faces\n";
              std::cerr << "line: " << line << "\n";
              if (numNormals == numVertices) {
                mesh.clear();
                return false;
              }
            }
            //if (numNormals == numVertices) {
            assert(fi < numFaces && 3 * fi + i < normalsIndices.size());
            normalsIndices[3 * fi + i] = ind;
            //}
          }
        }
        ++fi;
      }
    }
  }

  //std::cerr<<"second pass done\n";

  assert(!parseTexCoords || texCoordsIndices.size() == numFaces * 3);
  assert(!parseNormals || normalsIndices.size() == numFaces * 3);

  //Re-order

  std::vector<float> v;
  v.reserve(
    std::max(parseTexCoords * numTexCoords * 2, parseNormals * numNormals * 3));

  if (parseTexCoords && numTexCoords <= numVertices) {

    if (numTexCoords != numVertices) {
      mesh.allocateTexCoords();
    }

    const size_t faces3 = numFaces * 3;

    //check if we must re-order texcoords (and from which indice)
    assert(texCoordsIndices.size() == faces3);
    size_t k = 0;

    if (numTexCoords == numVertices) {
      //texcoords are alreay on mesh.texCoords
      // => no need to copy equal indices

      for (; k < faces3; ++k) {
        if (texCoordsIndices[k] != mesh.triangles[k])
          break;
      }
      k /= 3;
    }

    //std::cerr<<"must reorder texCoords ? "<<(k != numFaces)<<"\n";

    if (k != numFaces) { //we must re-order

      //std::cerr<<"must reorder texcoords k="<<k<<"\n";

      const size_t numTexCoords2 = 2 * numTexCoords;
      v.resize(numTexCoords2);
      for (size_t i = 0; i < numTexCoords2; ++i)
        v[i] = texCoords[i];

      for (size_t i = k; i < faces3; ++i) {
        const size_t vi = mesh.triangles[i];
        const size_t ti = texCoordsIndices[i];
        assert(2 * ti + 1 < v.size());
        mesh.texCoords[2 * vi + 0] = v[2 * ti + 0];
        mesh.texCoords[2 * vi + 1] = v[2 * ti + 1];
      }
    }
  }

  //code duplication

  if (parseNormals && numNormals <= numVertices) {

    if (numNormals != numVertices) {
      mesh.allocateNormals();
    }

    //check if we must re-order normals (and from which indice)
    const size_t faces3 = numFaces * 3;
    assert(normalsIndices.size() == faces3);
    size_t k = 0;

    if (numNormals == numVertices) {
      //normals are already in mesh.normals
      // => no need to copy equal indices
      for (; k < faces3; ++k) {
        if (normalsIndices[k] != mesh.triangles[k])
          break;
      }
      k /= 3;
    }

    //std::cerr<<"must reorder normals ? "<<(k != numFaces)<<"\n";

    if (k != numFaces) { //we must re-order

      //std::cerr<<"must reorder normals k="<<k<<"\n";

      const size_t numNormals3 = 3 * numNormals;
      v.resize(numNormals3);
      for (size_t i = 0; i < numNormals3; ++i)
        v[i] = normals[i];

      for (size_t i = k; i < faces3; ++i) {
        const size_t vi = mesh.triangles[i];
        const size_t ni = normalsIndices[i];
        assert(3 * ni + 2 < v.size());
        mesh.normals[3 * vi + 0] = v[3 * ni + 0];
        mesh.normals[3 * vi + 1] = v[3 * ni + 1];
        mesh.normals[3 * vi + 2] = v[3 * ni + 2];
      }
    }
  }

  //REM:
  //we do not check if a vertex indice is associated with
  // different texcoords (or normals) in different triangles.

  assert(numNormals == 0 || mesh.hasNormals());
  assert(numTexCoords == 0 || mesh.hasTexCoords());

  //assert((numNormals != 0 && mesh.numVertices == numNormals) == mesh.hasNormals());
  //assert((numTexCoords != 0 && mesh.numVertices == numTexCoords) == mesh.hasTexCoords());

  return true;
}

bool
writeOBJ(const std::string &filename, const Mesh &mesh)
{
  if (!mesh.isValid())
    return false;

  std::ofstream out(filename.c_str(), std::ios::out | std::ios::binary);
  if (!out) {
    return false;
  }

  const int precision = 9;

  //write vertices
  for (size_t i = 0; i < mesh.numVertices; ++i) {
    out << 'v';
    for (int k = 0; k < 3; ++k) {
      out << ' ' << std::setprecision(precision) << mesh.vertices[3 * i + k];
    }
    out << "\n";
  }

  //write texcoords
  const bool hasTexCoords = mesh.hasTexCoords();
  if (hasTexCoords) {
    for (size_t i = 0; i < mesh.numVertices; ++i) {
      out << "vt";
      for (int k = 0; k < 2; ++k) {
        out << ' ' << std::setprecision(precision) << mesh.texCoords[2 * i + k];
      }
      out << "\n";
    }
  }

  //write normals
  const bool hasNormals = mesh.hasNormals();
  if (hasNormals) {
    for (size_t i = 0; i < mesh.numVertices; ++i) {
      out << "vn";
      for (int k = 0; k < 3; ++k) {
        out << ' ' << std::setprecision(precision) << mesh.normals[3 * i + k];
      }
      out << "\n";
    }
  }

  //write faces
  if (!hasTexCoords && !hasNormals) {
    for (size_t i = 0; i < mesh.numTriangles; ++i) {
      out << 'f';
      for (int k = 0; k < 3; ++k) {
        const uint32_t ind =
          mesh.triangles[3 * i + k] + 1; //indices start from 1 in OBJ
        out << ' ' << ind;
      }
      out << "\n";
    }
  } else if (hasTexCoords) {
    if (hasNormals) {
      for (size_t i = 0; i < mesh.numTriangles; ++i) {
        out << 'f';
        for (int k = 0; k < 3; ++k) {
          const uint32_t ind =
            mesh.triangles[3 * i + k] + 1; //indices start from 1 in OBJ
          out << ' ' << ind << '/' << ind << '/' << ind;
        }
        out << "\n";
      }
    } else {
      for (size_t i = 0; i < mesh.numTriangles; ++i) {
        out << 'f';
        for (int k = 0; k < 3; ++k) {
          const uint32_t ind =
            mesh.triangles[3 * i + k] + 1; //indices start from 1 in OBJ
          out << ' ' << ind << '/' << ind;
        }
        out << "\n";
      }
    }
  } else if (hasNormals) {
    for (size_t i = 0; i < mesh.numTriangles; ++i) {
      out << 'f';
      for (int k = 0; k < 3; ++k) {
        const uint32_t ind =
          mesh.triangles[3 * i + k] + 1; //indices start from 1 in OBJ
        out << ' ' << ind << "//" << ind;
      }
      out << "\n";
    }
  }

  return true;
}
