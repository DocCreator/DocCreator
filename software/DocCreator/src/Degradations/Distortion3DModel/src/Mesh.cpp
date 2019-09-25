#define _USE_MATH_DEFINES // for Visual

#include "Mesh.hpp"

#include <cmath> //M_PI

#include <cassert>
#include <cstdlib>  //malloc
#include <cstring>  //memset
#include <iostream> //DEBUG
#include <vector>

Mesh::Mesh()
  : numVertices(0)
  , vertices(nullptr)
  , normals(nullptr)
  , texCoords(nullptr)
  , numTriangles(0)
  , triangles(nullptr)
{}

Mesh::~Mesh()
{
  clear();
}

void
Mesh::copyFrom(const Mesh &m)
{
  assert(&m != this);

  allocateVertices(m.numVertices);
  memcpy(vertices, m.vertices, 3 * m.numVertices * sizeof(float));
  if (m.hasNormals()) {
    allocateNormals();
    memcpy(normals, m.normals, 3 * m.numVertices * sizeof(float));
  }
  if (m.hasTexCoords()) {
    allocateTexCoords();
    memcpy(texCoords, m.texCoords, 2 * m.numVertices * sizeof(float));
  }
  allocateTriangles(m.numTriangles);
  memcpy(triangles, m.triangles, 3 * m.numTriangles * sizeof(uint32_t));
}

Mesh::Mesh(const Mesh &m)
{
  copyFrom(m);
}

Mesh &
Mesh::operator=(const Mesh &m)
{
  if (this != &m) {
    copyFrom(m);
  }
  return *this;
}

void
Mesh::clear()
{
  freeVertices();
  freeNormals();
  freeTexCoords();
  freeTriangles();
  assert(!isValid() && !hasNormals() && !hasTexCoords());
}

void
Mesh::allocateVertices(uint32_t numVerts)
{
  if (numVertices != numVerts) {
    freeVertices();
    freeNormals();
    freeTexCoords();

    numVertices = numVerts;

    if (numVertices > 0) {
      const size_t s = numVertices * 3 * sizeof(float);
      vertices = static_cast<float *>( malloc(s) );
      if (vertices == nullptr) {
	numVertices = 0;
      }
    }
  }
}

void
Mesh::allocateNormals()
{
  freeNormals();

  if (numVertices > 0) {
    const size_t s = numVertices * 3 * sizeof(float);
    normals = static_cast<float *>( malloc(s) );
  }
}

void
Mesh::allocateTexCoords()
{
  freeTexCoords();

  if (numVertices > 0) {
    const size_t s = numVertices * 2 * sizeof(float);
    texCoords = static_cast<float *>( malloc(s) );
  }
}

void
Mesh::allocateTriangles(uint32_t numTris)
{
  if (numTris != numTriangles) {
    freeTriangles();

    numTriangles = numTris;
    if (numTris > 0) {
      const size_t s = numTris * 3 * sizeof(uint32_t);
      triangles = (uint32_t *)malloc(s);
    }
  }
}

void
Mesh::freeVertices()
{
  free(vertices);
  vertices = nullptr;
}

void
Mesh::freeNormals()
{
  free(normals);
  normals = nullptr;
  assert(!hasNormals());
}

void
Mesh::freeTexCoords()
{
  free(texCoords);
  texCoords = nullptr;
  assert(!hasTexCoords());
}

void
Mesh::freeTriangles()
{
  free(triangles);
  triangles = nullptr;
}

void
Mesh::copyTo(Mesh &m)
{
  m.allocateVertices(numVertices);
  if (hasNormals())
    m.allocateNormals();
  if (hasTexCoords())
    m.allocateTexCoords();
  m.allocateTriangles(numTriangles);

  const uint32_t numTriangles3 = numTriangles * 3;
  for (uint32_t i = 0; i < numTriangles3; ++i) {
    m.triangles[i] = triangles[i];
  }

  const uint32_t numVertices3 = numVertices * 3;
  for (uint32_t i = 0; i < numVertices3; ++i) {
    m.vertices[i] = vertices[i];
  }
  if (hasNormals()) {
    for (uint32_t i = 0; i < numVertices3; ++i) {
      m.normals[i] = normals[i];
    }
    assert(m.hasNormals());
  }
  if (hasTexCoords()) {
    const uint32_t numVertices2 = numVertices * 2;
    for (uint32_t i = 0; i < numVertices2; ++i) {
      m.texCoords[i] = texCoords[i];
    }
    assert(m.hasTexCoords());
  }
}

void
Mesh::swap(Mesh &m)
{
  std::swap(m.vertices, vertices);
  std::swap(m.normals, normals);
  std::swap(m.texCoords, texCoords);
  std::swap(m.numVertices, numVertices);
  std::swap(m.numTriangles, numTriangles);
  std::swap(m.triangles, triangles);

  assert(m.isValid() == isValid());
  assert(m.hasNormals() == hasNormals());
  assert(m.hasTexCoords() == hasTexCoords());
}

void
Mesh::unitize()
{
  float minB[3], maxB[3];
  getAABB(minB, maxB);

  float center[3];
  float dim[3];
  for (int k = 0; k < 3; ++k) {
    center[k] = (maxB[k] + minB[k]) * 0.5f;
    dim[k] = maxB[k] - minB[k];
  }

  const float scale = 2.0f / std::max(dim[0], std::max(dim[1], dim[2]));

  for (uint32_t i = 0; i < numVertices; ++i) {
    for (int k = 0; k < 3; ++k) {
      vertices[3 * i + k] -= center[k];
      vertices[3 * i + k] *= scale;
    }
  }
}

void
Mesh::normalizeNormals()
{
  assert(hasNormals());

  const size_t numVerts = numVertices;
  for (size_t i = 0; i < numVerts; ++i) {
    const float x = normals[3 * i + 0];
    const float y = normals[3 * i + 1];
    const float z = normals[3 * i + 2];

    const float length2 = x * x + y * y + z * z;
    if (length2 > 0) {
      const float length = sqrt(x * x + y * y + z * z);

      const float inv_length = 1.f / length;

      normals[3 * i + 0] *= inv_length;
      normals[3 * i + 1] *= inv_length;
      normals[3 * i + 2] *= inv_length;
    }
  }
}

void
Mesh::computeNormals()
{
  if (normals == nullptr)
    allocateNormals();

  assert(normals);

  /*
  {
    //check if all vertices are used in triangles
    std::vector<bool> used(numVertices, false);
    const size_t numTris3 = numTriangles*3;
    for (size_t i=0; i<numTris3; ++i) {
      used[triangles[i]] = true;
    }
    bool allUsed = true;
    for (size_t i=0; i<numVertices; ++i) {
      if (! used[i]) {
        std::cerr<<"vertex "<<i<<" is not used in any triangle !!!!\n";
        allUsed = false;
        break;
      }
    }
  }
  */

  //set all (vertex) normals to zero
  memset(normals, 0, numVertices * 3 * sizeof(float));

  //compute face normals and accumulate in vertex normals
  const size_t numTris = numTriangles;
  const uint32_t *tris = triangles;
  for (size_t i = 0; i < numTris; ++i) {
    const uint32_t v_id_0 = tris[3 * i + 0];
    const uint32_t v_id_1 = tris[3 * i + 1];
    const uint32_t v_id_2 = tris[3 * i + 2];

    //compute cross products of (01)^(02)
    const float v01_x = (vertices[3 * v_id_1 + 0] - vertices[3 * v_id_0 + 0]);
    const float v01_y = (vertices[3 * v_id_1 + 1] - vertices[3 * v_id_0 + 1]);
    const float v01_z = (vertices[3 * v_id_1 + 2] - vertices[3 * v_id_0 + 2]);
    const float v02_x(vertices[3 * v_id_2 + 0] - vertices[3 * v_id_0 + 0]);
    const float v02_y(vertices[3 * v_id_2 + 1] - vertices[3 * v_id_0 + 1]);
    const float v02_z(vertices[3 * v_id_2 + 2] - vertices[3 * v_id_0 + 2]);
    const float n_x(v01_y * v02_z - v01_z * v02_y);
    const float n_y(v01_z * v02_x - v01_x * v02_z);
    const float n_z(v01_x * v02_y - v01_y * v02_x);

    normals[3 * v_id_0 + 0] += n_x;
    normals[3 * v_id_0 + 1] += n_y;
    normals[3 * v_id_0 + 2] += n_z;

    normals[3 * v_id_1 + 0] += n_x;
    normals[3 * v_id_1 + 1] += n_y;
    normals[3 * v_id_1 + 2] += n_z;

    normals[3 * v_id_2 + 0] += n_x;
    normals[3 * v_id_2 + 1] += n_y;
    normals[3 * v_id_2 + 2] += n_z;
  }

  //normalize
  normalizeNormals();

  assert(hasNormals());
}

void
Mesh::getAABB(float minB[3], float maxB[3]) const
{
  for (int k = 0; k < 3; ++k)
    maxB[k] = minB[k] = vertices[3 * 0 + k];

  for (uint32_t i = 1; i < numVertices; ++i) { //start from 1
    for (int k = 0; k < 3; ++k) {
      const float v = vertices[3 * i + k];
      if (v > maxB[k])
        maxB[k] = v;
      else if (v < minB[k])
        minB[k] = v;
    }
  }
}

//Remove triangles that have two same indices.
void
Mesh::removeDegenerateTrianglesIndices()
{
  assert(isValid());

  Mesh m;
  m.allocateTriangles(numTriangles);

  uint32_t *dst = m.triangles;
  const uint32_t *tri = triangles;
  assert(dst);
  assert(tri);
  for (uint32_t i = 0; i < numTriangles; ++i) {

    bool toKeep = true;

    int i1 = tri[2];
    for (int k = 0; k < 3; ++k) {
      int i2 = tri[k];
      if (i1 == i2) {
        toKeep = false;
        break;
      }
      dst[k] = tri[k];

      i1 = i2;
    }

    if (toKeep)
      dst += 3;

    tri += 3;
  }

  const uint32_t newNumTriangles = (dst - m.triangles) / 3;
  if (newNumTriangles != numTriangles) {

    std::cerr << "!!!!!! removeDegenerateTrianglesIndices numTriangles old="
              << numTriangles << " new=" << newNumTriangles << "\n";

    std::swap(m.numTriangles, numTriangles);
    std::swap(m.triangles, triangles);
  }

  assert(isValid());
}

void
Mesh::copyToWithNewIndices(Mesh &m,
                           uint32_t *newVIndices,
                           uint32_t newNumVertices)
{
  assert(!m.isValid());

  m.allocateVertices(newNumVertices);
  if (hasNormals())
    m.allocateNormals();
  if (hasTexCoords())
    m.allocateTexCoords();
  m.allocateTriangles(numTriangles);

  const uint32_t INVALID = -1;

  //copy triangles with new indices
  const uint32_t numTriangles3 = numTriangles * 3;
  for (uint32_t i = 0; i < numTriangles3; ++i) {
    assert(newVIndices[triangles[i]] != INVALID);
    m.triangles[i] = newVIndices[triangles[i]];
  }
  //copy vertices
  for (uint32_t i = 0; i < numVertices; ++i) {
    const uint32_t oldInd = i;
    assert(oldInd < numVertices);
    const uint32_t newInd = newVIndices[i];
    if (newInd != INVALID) {
      assert(newInd < m.numVertices);
      m.vertices[3 * newInd + 0] = vertices[3 * oldInd + 0];
      m.vertices[3 * newInd + 1] = vertices[3 * oldInd + 1];
      m.vertices[3 * newInd + 2] = vertices[3 * oldInd + 2];
      if (hasNormals()) {
        assert(m.hasNormals());
        m.normals[3 * newInd + 0] = normals[3 * oldInd + 0];
        m.normals[3 * newInd + 1] = normals[3 * oldInd + 1];
        m.normals[3 * newInd + 2] = normals[3 * oldInd + 2];
      }
      if (hasTexCoords()) {
        assert(m.hasTexCoords());
        m.texCoords[2 * newInd + 0] = texCoords[2 * oldInd + 0];
        m.texCoords[2 * newInd + 1] = texCoords[2 * oldInd + 1];
      }
    }
  }
}

void
Mesh::removeDuplicatedVertices()
{
  assert(isValid());

  if (numVertices > 1) {
    const uint32_t INVALID = -1;
    std::vector<uint32_t> newVIndices(numVertices,
                                      INVALID); //newVIndices[oldInd]==newInd

    const float eps = std::numeric_limits<float>::epsilon() * 2.f;

    //TODO:OPTIM: QUADRATIC !!!!

    uint32_t vi = 0;
    for (uint32_t i = 0; i < numVertices; ++i) {
      const float *v1 = &vertices[3 * i];
      if (newVIndices[i] == INVALID) {
        newVIndices[i] = vi;
        for (uint32_t j = i + 1; j < numVertices; ++j) {
          const float *v2 = &vertices[3 * j];

          if (fabs(v1[0] - v2[0]) < eps && fabs(v1[1] - v2[1]) < eps &&
              fabs(v1[2] - v2[2]) < eps) {

#if 0
	    std::cerr<<"vertice["<<i<<"]=("<<v1[0]<<", "<<v1[1]<<", "<<v1[2]<<") == vertice["<<j<<"]=("<<v2[0]<<", "<<v2[1]<<", "<<v2[2]<<") \n";

#endif

            newVIndices[j] = vi;
          }
        }
        ++vi;
      }
    }

    if (vi < numVertices) {
      std::cerr << "!!!!!! removeDuplicatedVertices(): " << numVertices - vi
                << " duplicate vertices\n";

      Mesh m;
      copyToWithNewIndices(m, &newVIndices[0], vi);
      this->swap(m);

      removeDegenerateTrianglesIndices();
    }
  }

  assert(isValid());
}

void
Mesh::removeNonReferencedVertices()
{
  assert(isValid());

  std::vector<bool> referenced(numVertices, false);
  size_t numUsed = 0;
  const uint32_t numTriangles3 = numTriangles * 3;
  for (uint32_t i = 0; i < numTriangles3; ++i) {
    assert(triangles);
    const uint32_t ind = triangles[i];
    numUsed += (referenced[ind] == false);

    referenced[ind] = true;
  }

  if (numUsed != numVertices) {
    assert(numUsed < numVertices);

    std::cerr << "!!!!!! removeNonReferencedVertices() : "
              << numVertices - numUsed << " non-referenced vertices ("
              << numUsed << " used)\n";

    const uint32_t INVALID = -1;
    uint32_t vi = 0;
    std::vector<uint32_t> newVIndices(numVertices,
                                      INVALID); //newVIndices[oldInd]==newInd
    for (uint32_t i = 0; i < numVertices; ++i) {
      if (referenced[i]) {
        newVIndices[i] = vi;
        ++vi;
      }
    }

    assert(vi == numUsed);

    Mesh m;
    copyToWithNewIndices(m, &newVIndices[0], vi);
    this->swap(m);
  }

  assert(isValid());
}

#ifndef NDEBUG
static void
checkIndices(uint32_t numVertices, uint32_t numIndices, uint32_t *indices)
{
  bool hasError = false;
  for (uint32_t i = 0; i < numIndices; ++i) {
    if (indices[i] >= numVertices) {
      std::cerr << "ERROR: i=" << i << " indices[i]=" << indices[i]
                << " >= numVertices=" << numVertices << "\n";
      hasError = true;
      break;
    }
  }
  if (hasError) {
    exit(10);
  }
  std::cerr << "all indices ok\n";
}
#endif //NDEBUG

#include "VertexCacheOptimizer/VertexCacheOptimizer.hpp"

void
Mesh::optimizeTriangleOrdering()
{
  Mesh m;
  m.allocateVertices(numVertices);
  if (hasNormals())
    m.allocateNormals();
  if (hasTexCoords())
    m.allocateTexCoords();
  m.allocateTriangles(numTriangles);

  const uint32_t numTriangles3 = numTriangles * 3;
  for (uint32_t i = 0; i < numTriangles3; ++i) {
    m.triangles[i] = triangles[i];
  }

#ifndef NDEBUG
  checkIndices(numVertices, numTriangles * 3, m.triangles);
#endif

  optimizeIndexOrder(numVertices, numTriangles3, m.triangles);

#ifndef NDEBUG
  checkIndices(numVertices, numTriangles * 3, m.triangles);
#endif

  //TODO: some CODE DUPLICATION with removeNonReferencedVertices()

  const uint32_t INVALID = -1;
  std::vector<uint32_t> newVIndices(numVertices,
                                    INVALID); //newVIndices[oldInd]==newInd
  uint32_t vi = 0;
  for (uint32_t i = 0; i < numTriangles3; ++i) {
    if (newVIndices[m.triangles[i]] == INVALID) {
      assert(vi < numVertices);
      newVIndices[m.triangles[i]] = vi;
      ++vi;
    }
    assert(newVIndices[m.triangles[i]] != INVALID);
    assert(newVIndices[m.triangles[i]] < numVertices);
    m.triangles[i] = newVIndices[m.triangles[i]];
  }

  for (uint32_t i = 0; i < numVertices; ++i) {
    const uint32_t oldInd = i;
    const uint32_t newInd = newVIndices[i];
    assert(oldInd < numVertices);
    assert(newInd < numVertices);
    m.vertices[3 * newInd + 0] = vertices[3 * oldInd + 0];
    m.vertices[3 * newInd + 1] = vertices[3 * oldInd + 1];
    m.vertices[3 * newInd + 2] = vertices[3 * oldInd + 2];
    if (hasNormals()) {
      assert(m.hasNormals());
      m.normals[3 * newInd + 0] = normals[3 * oldInd + 0];
      m.normals[3 * newInd + 1] = normals[3 * oldInd + 1];
      m.normals[3 * newInd + 2] = normals[3 * oldInd + 2];
    }
    if (hasTexCoords()) {
      assert(m.hasTexCoords());
      m.texCoords[2 * newInd + 0] = texCoords[2 * oldInd + 0];
      m.texCoords[2 * newInd + 1] = texCoords[2 * oldInd + 1];
    }
  }

  this->swap(m);
}

  //-------------------------------------------

#include <Eigen/Dense>

void
Mesh::align0()
{

  const uint32_t numV = numVertices;
  if (numV == 0)
    return;

  float mean[3] = { 0, 0, 0 };
  const float *v = vertices;
  for (uint32_t i = 0; i < numV; ++i) {
    mean[0] += v[3 * i + 0];
    mean[1] += v[3 * i + 1];
    mean[2] += v[3 * i + 2];
  }
  const float inv_numV = 1.f / numV;
  mean[0] *= inv_numV;
  mean[1] *= inv_numV;
  mean[2] *= inv_numV;

  std::cerr << "mean= " << mean[0] << ", " << mean[1] << ", " << mean[2]
            << "\n";

  const float inv_numV1 = 1.f / (numV - 1);

  //Eigen::Matrix3f == Eigen::Matrix<float, 3, 3>
  Eigen::Matrix3f covMat = Eigen::Matrix3f::Zero(3, 3);

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {

      const float *vrt = v;
      for (uint32_t k = 0; k < numV; ++k) {
        covMat(i, j) += (mean[i] - vrt[i]) * (mean[j] - vrt[j]);
        vrt += 3;
      }
      covMat(i, j) *= inv_numV1;
    }
  }

  //std::cerr<<"covMat:\n "<<covMat<<"\n";
}

void
Mesh::align()
{

  const uint32_t numV = numVertices;
  if (numV == 0)
    return;

  float mean[3] = { 0, 0, 0 };
  const float *v = vertices;
  for (uint32_t i = 0; i < numV; ++i) {
    mean[0] += v[3 * i + 0];
    mean[1] += v[3 * i + 1];
    mean[2] += v[3 * i + 2];
  }
  const float inv_numV = 1.f / numV;
  mean[0] *= inv_numV;
  mean[1] *= inv_numV;
  mean[2] *= inv_numV;

  std::cerr << "mean= " << mean[0] << ", " << mean[1] << ", " << mean[2]
            << "\n";

  //Eigen::Matrix3f == Eigen::Matrix<float, 3, 3>
  Eigen::Matrix3f covMat = Eigen::Matrix3f::Zero(3, 3);

  const float *vrt = v;

  for (uint32_t k = 0; k < numV; ++k) {

    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        covMat(i, j) += (mean[i] - vrt[i]) * (mean[j] - vrt[j]);
      }
    }
    vrt += 3;
  }
  const float inv_numV1 = 1.f / (numV - 1);
  covMat *= inv_numV1;

  //std::cerr<<"covMat:\n "<<covMat<<"\n";
}

void
Mesh::alignB()
{

  const uint32_t numV = numVertices;
  if (numV == 0)
    return;

  float mean[3] = { 0, 0, 0 };
  const float *v = vertices;
  for (uint32_t i = 0; i < numV; ++i) {
    mean[0] += v[3 * i + 0];
    mean[1] += v[3 * i + 1];
    mean[2] += v[3 * i + 2];
  }
  const float inv_numV = 1.f / numV;
  mean[0] *= inv_numV;
  mean[1] *= inv_numV;
  mean[2] *= inv_numV;

  std::cerr << "mean= " << mean[0] << ", " << mean[1] << ", " << mean[2]
            << "\n";

  //Eigen::Matrix3f == Eigen::Matrix<float, 3, 3>
  Eigen::Matrix3f covMat = Eigen::Matrix3f::Zero(3, 3);

  //float *vrt = v;
  //DEBUG
  float *vrtp = (float *)malloc(3 * numV * sizeof(float));
  if (vrtp == nullptr)
    return;
  float *vrt = vrtp;
  memcpy(vrt, v, 3 * numV * sizeof(float));

  for (uint32_t k = 0; k < numV; ++k) {

    vrt[0] -= mean[0];
    vrt[1] -= mean[1];
    vrt[2] -= mean[2];

    for (int i = 0; i < 3; ++i) {

      for (int j = i; j < 3;
           ++j) { //start from i   [covariance matrix is symmetrical]
        covMat(i, j) +=
          vrt[i] * vrt[j]; //(mean[i] - vrt[i]) * (mean[j] - vrt[j]);
      }
    }

    vrt += 3;
  }

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < i; ++j) {
      covMat(i, j) = covMat(j, i);
    }
  }

  const float inv_numV1 = 1.f / (numV - 1);
  covMat *= inv_numV1;

  //std::cerr<<"covMat:\n "<<covMat<<"\n";

  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> es(covMat);

  /*
  {//DEBUG
    std::cerr<<"eigen values: "<<es.eigenvalues().transpose()<<"\n";

    Eigen::MatrixXf eigenVectors = es.eigenvectors();
    std::cerr<<"eigen vec1 = "<<eigenVectors.col(0).transpose()<<"\n";
    std::cerr<<"eigen vec2 = "<<eigenVectors.col(1).transpose()<<"\n";
    std::cerr<<"eigen vec3 = "<<eigenVectors.col(2).transpose()<<"\n";
  }
  */

  free(vrtp);
}

void
Mesh::alignC()
{
  const uint32_t numV = numVertices;
  if (numV == 0)
    return;

  const float *v = vertices;

  Eigen::MatrixXf X(numV, 3);
  const float *vrt = v;
  for (uint32_t k = 0; k < numV; ++k) {
    X(k, 0) = vrt[0];
    X(k, 1) = vrt[1];
    X(k, 2) = vrt[2];

    vrt += 3;
  }

  //std::cerr<<"mean="<<X.colwise().mean()<<"\n";

  Eigen::MatrixXf centered = X.rowwise() - X.colwise().mean();

  Eigen::MatrixXf cov = centered.adjoint() * centered;

  //std::cerr<<"covMat:\n "<<cov<<"\n";

  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> es(cov);

  /*
    {//DEBUG
      std::cerr<<"eigen values: "<<es.eigenvalues().transpose()<<"\n";

      Eigen::MatrixXf eigenVectors = es.eigenvectors();
      std::cerr<<"eigen vec1 = "<<eigenVectors.col(0).transpose()<<"\n";
      std::cerr<<"eigen vec2 = "<<eigenVectors.col(1).transpose()<<"\n";
      std::cerr<<"eigen vec3 = "<<eigenVectors.col(2).transpose()<<"\n";
    }
    */

#if 0
  Eigen::MatrixXf Ry(3, 3);
  //Ry << 0, 0, 1, 0, 1, 0, -1, 0, 0;
  Ry << 1, 0, 0, 0, 1, 0, 0, 0, 1;
  std::cerr<<"Ry\n"<<Ry<<"\n";

    Eigen::MatrixXf eigenVectors = es.eigenvectors();
    
    Eigen::MatrixXf eigenVectors2(3, 3);
    eigenVectors2.col(1) = eigenVectors.col(2);
    eigenVectors2.col(0) = eigenVectors.col(1);
    eigenVectors2.col(2) = eigenVectors.col(0);
    eigenVectors = eigenVectors2;


    Eigen::MatrixXf Xt = X * eigenVectors * Ry;

  //Eigen::MatrixXf Xt = X * es.eigenvectors() * Ry;

  float *vd = vertices;
  for (uint32_t k=0; k<numV; ++k) {
    vd[0] = Xt(k, 0);
    vd[1] = Xt(k, 1);
    vd[2] = Xt(k, 2);

    vd+=3;
  }
#endif //1
}

Eigen::MatrixXf
Mesh::getEigenVectors()
{

  const uint32_t numV = numVertices;

  const float *v = vertices;

  Eigen::MatrixXf X(numV, 3);
  const float *vrt = v;
  for (uint32_t k = 0; k < numV; ++k) {
    X(k, 0) = vrt[0];
    X(k, 1) = vrt[1];
    X(k, 2) = vrt[2];

    vrt += 3;
  }

  Eigen::MatrixXf centered = X.rowwise() - X.colwise().mean();

  Eigen::MatrixXf cov = centered.adjoint() * centered;

  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> es(cov);

  return es.eigenvectors();
}

Eigen::MatrixXf
Mesh::getEigenVectors(const std::vector<uint32_t> &indices)
{
  const uint32_t numV = indices.size();

  Eigen::MatrixXf X(numV, 3);

  const float *v = vertices;
  const float *vrt = v;
  for (uint32_t k = 0; k < numV; ++k) {
    uint32_t ind = indices[k];
    assert(ind < numVertices);
    X(k, 0) = vrt[3 * ind + 0];
    X(k, 1) = vrt[3 * ind + 1];
    X(k, 2) = vrt[3 * ind + 2];
  }

  Eigen::MatrixXf centered = X.rowwise() - X.colwise().mean();

  Eigen::MatrixXf cov = centered.adjoint() * centered;

  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> es(cov);

  return es.eigenvectors();
}

//-------------------------------------------------------------------

Mesh
makePlane()
{
  Mesh m;

  const uint32_t numVertices = 4;
  const uint32_t numTriangles = 2;

  m.allocateVertices(numVertices);
  m.allocateTexCoords();
  m.allocateTriangles(numTriangles);

  assert(m.triangles);
  m.triangles[0 * 3 + 0] = 0;
  m.triangles[0 * 3 + 1] = 1;
  m.triangles[0 * 3 + 2] = 2;
  m.triangles[1 * 3 + 0] = 0;
  m.triangles[1 * 3 + 1] = 2;
  m.triangles[1 * 3 + 2] = 3;

  assert(m.vertices);
  m.vertices[0 * 3 + 0] = -1;
  m.vertices[0 * 3 + 1] = -1;
  m.vertices[0 * 3 + 2] = 0;

  m.vertices[1 * 3 + 0] = 1;
  m.vertices[1 * 3 + 1] = -1;
  m.vertices[1 * 3 + 2] = 0;

  m.vertices[2 * 3 + 0] = 1;
  m.vertices[2 * 3 + 1] = 1;
  m.vertices[2 * 3 + 2] = 0;

  m.vertices[3 * 3 + 0] = -1;
  m.vertices[3 * 3 + 1] = 1;
  m.vertices[3 * 3 + 2] = 0;

  assert(m.texCoords);
  m.texCoords[0 * 2 + 0] = 0;
  m.texCoords[0 * 2 + 1] = 0;

  m.texCoords[1 * 2 + 0] = 1;
  m.texCoords[1 * 2 + 1] = 0;

  m.texCoords[2 * 2 + 0] = 1;
  m.texCoords[2 * 2 + 1] = 1;

  m.texCoords[3 * 2 + 0] = 0;
  m.texCoords[3 * 2 + 1] = 1;

  m.computeNormals();

  assert(m.isValid());
  assert(m.hasTexCoords());
  assert(m.hasNormals());

  return m;
}

Mesh
makeSphereMesh(float radius,
               unsigned int rings,
               unsigned int sectors,
               bool withTexCoords)
{
  assert(rings > 1);
  assert(sectors > 1);

  Mesh m;

  const uint32_t rs = rings * sectors;
  const uint32_t numVertices = rs;
  const uint32_t numTriangles = (rings - 1) * (sectors - 1) * 2;

  m.allocateVertices(numVertices);
  m.allocateNormals();
  if (withTexCoords)
    m.allocateTexCoords();
  m.allocateTriangles(numTriangles);

  assert(rings > 1);
  assert(sectors > 1);
  const float R = 1.f / (rings - 1);
  const float S = 1.f / (sectors - 1);

  float *v = m.vertices;
  float *n = m.normals;
  float *t = m.texCoords;

  for (unsigned int r = 0; r < rings; ++r) {
    const float rR = r * R;
    const float y = static_cast<float>(sin(-M_PI_2 + M_PI * rR));
    const float sinPirR = static_cast<float>(sin(M_PI * rR));
    for (unsigned int s = 0; s < sectors; ++s) {
      const float sS = s * S;
      const float sS2Pi = static_cast<float>(2 * M_PI * sS);
      const float x = sin(sS2Pi) * sinPirR;
      const float z = cos(sS2Pi) * sinPirR;

      *v++ = x * radius;
      *v++ = y * radius;
      *v++ = z * radius;

      *n++ = -x;
      *n++ = -y;
      *n++ = -z;

      if (withTexCoords) {
        *t++ = sS;
        *t++ = rR;
      }
    }
  }

#ifndef NDEBUG
  std::cerr << "makeSphereMesh(radius=" << radius << ", rings=" << rings
            << ", sectors=" << sectors << ", withTexCoords=" << withTexCoords
            << ") => #Triangles=" << m.numTriangles << "\n";
#endif //NDEBUG

  uint32_t *tr = m.triangles;

  const unsigned int ringsM1 = rings - 1;
  const unsigned int sectorsM1 = sectors - 1;
  for (unsigned int r = 0; r < ringsM1; ++r) {
    const uint32_t rSectors = r * sectors;
    const uint32_t rp1Sectors = rSectors + sectors; //(r+1)*sectors;
    for (unsigned int s = 0; s < sectorsM1; ++s) {
      const uint32_t i0 = rp1Sectors + s;
      const uint32_t i1 = i0 + 1; //rp1Sectors + (s+1);
      const uint32_t i2 = rSectors + (s + 1);
      const uint32_t i3 = rSectors + s;

#if 0
      *tr++ = i0;
      *tr++ = i1;
      *tr++ = i2;
      *tr++ = i0;
      *tr++ = i2;
      *tr++ = i3;
#else
      *tr++ = i3;
      *tr++ = i2;
      *tr++ = i1;
      *tr++ = i3;
      *tr++ = i1;
      *tr++ = i0;
#endif
    }
  }

  return m;
}

  //-------------------------------------------------------------------

#include <unordered_map>
#include <unordered_set>

namespace std {

template<>
struct hash<Edge> : public std::unary_function<Edge, std::size_t>
{
  std::size_t operator()(Edge e) const
  {
    return static_cast<std::size_t>((e.vertexIndex[0] * 39) ^
                                    (e.vertexIndex[1] * 31));
  }
};
}

bool
check_EdgesH(const Mesh &mesh)
{
  std::vector<Edge> edges;

  using HASH_MAP = std::unordered_map<Edge, uint32_t>;
  HASH_MAP edgesH;  //store triangle indices
  HASH_MAP edgesH2; //store frequencies

  bool result = true;

  //const size_t vertexCount = mesh.numVertices;
  const size_t triangleCount = mesh.numTriangles;

  const size_t maxEdgeCount = triangleCount * 3;
  edgesH.reserve(maxEdgeCount);

  const uint32_t *triangles = mesh.triangles;
  for (size_t a = 0; a < triangleCount; ++a) {
    uint32_t i1 = triangles[3 * a + 2];
    for (int b = 0; b < 3; ++b) {
      const uint32_t i2 = triangles[3 * a + b];
      Edge e;
      e.vertexIndex[0] = i1;
      e.vertexIndex[1] = i2;
      HASH_MAP::const_iterator itF = edgesH.find(e);
      if (itF != edgesH.end() && itF->first == e) {
        std::cerr
          << "Edge between vertex indices " << i1 << " & " << i2
          << " present both for tri " << itF->second << " & " << a
          << " with same orientation  ! There is a wrong orientation !!\n";

        std::cerr << "  tri " << a << ": " << triangles[3 * a + 0] << ", "
                  << triangles[3 * a + 1] << ", " << triangles[3 * a + 2]
                  << "\n";
        std::cerr << "  tri " << itF->second << ": "
                  << triangles[3 * itF->second + 0] << ", "
                  << triangles[3 * itF->second + 1] << ", "
                  << triangles[3 * itF->second + 2] << "\n";

        for (uint32_t k = 0; k < triangleCount; ++k) {
          if (triangles[3 * k + 0] == i1 || triangles[3 * k + 1] == i1 ||
              triangles[3 * k + 2] == i1 || triangles[3 * k + 0] == i2 ||
              triangles[3 * k + 1] == i2 || triangles[3 * k + 2] == i2) {
            std::cerr << "  other tri " << k << ": " << triangles[3 * k + 0]
                      << ", " << triangles[3 * k + 1] << ", "
                      << triangles[3 * k + 2] << "\n";
          }
        }

        result = false;
      }
      edgesH[e] = a;

      Edge e2;
      if (i1 < i2) {
        e2.vertexIndex[0] = i1;
        e2.vertexIndex[1] = i2;
      } else {
        e2.vertexIndex[0] = i2;
        e2.vertexIndex[1] = i1;
      }
      edgesH2[e2] += 1;

      i1 = i2;
    }
  }

  const size_t numEdges = edgesH.size();
  edges.resize(numEdges);
  for (HASH_MAP::const_iterator it = edgesH2.cbegin(); it != edgesH2.cend();
       ++it) {
    if (it->second > 2) {
      std::cerr << "Edge between vertex indices " << it->first.vertexIndex[0]
                << " & " << it->first.vertexIndex[1] << " present "
                << it->second << " times ! \n";

      result = false;
    }
  }

  return result;
}

std::vector<Edge>
getEdgesH(const Mesh &mesh)
{
  std::vector<Edge> edges;

  using HASH_MAP = std::unordered_set<Edge>;
  HASH_MAP edgesH;

  //const size_t vertexCount = mesh.numVertices;
  const size_t triangleCount = mesh.numTriangles;

  const size_t maxEdgeCount = triangleCount * 3;
  edgesH.reserve(maxEdgeCount);

  const uint32_t *triangles = mesh.triangles;
  for (size_t a = 0; a < triangleCount; ++a) {
    uint32_t i1 = triangles[3 * a + 2];
    for (int b = 0; b < 3; ++b) {
      const uint32_t i2 = triangles[3 * a + b];
      Edge e;
      if (i1 < i2) {
        e.vertexIndex[0] = i1;
        e.vertexIndex[1] = i2;
      } else {
        e.vertexIndex[0] = i2;
        e.vertexIndex[1] = i1;
      }
      edgesH.insert(e);

      i1 = i2;
    }
  }

  const size_t numEdges = edgesH.size();
  edges.resize(numEdges);
  size_t i = 0;
  for (HASH_MAP::const_iterator it = edgesH.cbegin(); it != edgesH.cend();
       ++it, ++i) {
    edges[i] = *it;
  }

  return edges;
}

std::vector<Edge>
getEdges(const Mesh &mesh)
{
  std::vector<Edge> edges;

  //Method adapted from:
  //Lengyel, Eric.
  //“Building an Edge List for an Arbitrary Mesh”.
  //Terathon Software 3D Graphics Library, 2005.
  //http://www.terathon.com/code/edges.html

  //Slightly adapated as:
  //-we do not have "closed" mesh (thus we have border edges seen only once)
  //-we do not need the triangleIndices in Edge,
  //-we do not have unsigned short but uint32_t for indices..

  const size_t vertexCount = mesh.numVertices;
  const size_t triangleCount = mesh.numTriangles;

  const size_t maxEdgeCount = triangleCount * 3;

  edges.resize(maxEdgeCount);
  Edge *edgeArray = &edges[0];

  uint32_t *firstEdge = new uint32_t[vertexCount + maxEdgeCount];
  uint32_t *nextEdge = firstEdge + vertexCount;

  const uint32_t INVALID = -1; //0xFFFFFFFF;

  for (size_t a = 0; a < vertexCount; ++a)
    firstEdge[a] = INVALID;

  // First pass over all triangles. This finds all the edges satisfying the
  // condition that the first vertex index is less than the second vertex index
  // when the direction from the first vertex to the second vertex represents
  // a counterclockwise winding around the triangle to which the edge belongs.
  // For each edge found, the edge index is stored in a linked list of edges
  // belonging to the lower-numbered vertex index i. This allows us to quickly
  // find an edge in the second pass whose higher-numbered vertex index is i.

  size_t edgeCount = 0;
  const uint32_t *triangles = mesh.triangles;
  for (size_t a = 0; a < triangleCount; ++a) {
    uint32_t i1 = triangles[3 * a + 2];
    for (int b = 0; b < 3; ++b) {
      const uint32_t i2 = triangles[3 * a + b];
      if (i1 < i2) {
        Edge *edge = &edgeArray[edgeCount];

        edge->vertexIndex[0] = i1;
        edge->vertexIndex[1] = i2;

        uint32_t edgeIndex = firstEdge[i1];
        if (edgeIndex == INVALID) {
          firstEdge[i1] = edgeCount;
        } else {
          for (;;) {
            uint32_t index = nextEdge[edgeIndex];
            if (index == INVALID) {
              nextEdge[edgeIndex] = edgeCount;
              break;
            }
            edgeIndex = index;
          }
        }
        nextEdge[edgeCount] = INVALID;
        ++edgeCount;
      }

      i1 = i2;
    }
  }

  // Second pass over all triangles. This finds all the edges satisfying the
  // condition that the first vertex index is greater than the second vertex
  // index
  // when the direction from the first vertex to the second vertex represents
  // a counterclockwise winding around the triangle to which the edge belongs.
  // For each of these edges, the same edge should have already been found in
  // the first pass for a different triangle. So we search the list of edges
  // for the higher-numbered vertex index for the matching edge and fill in the
  // second triangle index. The maximum number of comparisons in this search for
  // any vertex is the number of edges having that vertex as an endpoint.

  for (uint32_t a = 0; a < triangleCount; ++a) {
    uint32_t i1 = triangles[3 * a + 2];
    for (int b = 0; b < 3; ++b) {
      const uint32_t i2 = triangles[3 * a + b];
      if (i1 > i2) {

        bool found = false;
        uint32_t prev_edgeIndex = firstEdge[i2];
        for (uint32_t edgeIndex = firstEdge[i2]; edgeIndex != INVALID;
             edgeIndex = nextEdge[edgeIndex]) {
          Edge *edge = &edgeArray[edgeIndex];
          if (edge->vertexIndex[1] == i1) {
            assert(edge->vertexIndex[0] == i2);
            found = true;
            break;
          }
          prev_edgeIndex = edgeIndex;
        }
        if (found == false) {
          assert(edgeCount < maxEdgeCount);
          Edge *edge = &edgeArray[edgeCount];
          edge->vertexIndex[0] = i2;
          edge->vertexIndex[1] = i1;

          if (prev_edgeIndex == INVALID) {
            assert(firstEdge[i2] == INVALID);
            firstEdge[i2] = edgeCount;
          } else {
            assert(nextEdge[prev_edgeIndex] == INVALID);
            nextEdge[prev_edgeIndex] = edgeCount;
          }
          nextEdge[edgeCount] = INVALID;
          ++edgeCount;
        }
      }

      i1 = i2;
    }
  }

  edges.resize(edgeCount);

  delete[] firstEdge;

  return edges;
}

void
getEdgesF(const Mesh &mesh, std::vector<EdgeF> &edges)
{

  //Method adapted from:
  //Lengyel, Eric.
  //“Building an Edge List for an Arbitrary Mesh”.
  //Terathon Software 3D Graphics Library, 2005.
  //http://www.terathon.com/code/edges.html

  //Slightly adapated as:
  //-we do not have "closed" mesh (thus we have border edges seen only once)
  //-we do not need the triangleIndices in Edge,
  //-we do not have unsigned short but uint32_t for indices..

  const size_t vertexCount = mesh.numVertices;
  const size_t triangleCount = mesh.numTriangles;

  const size_t maxEdgeCount = triangleCount * 3;

  edges.resize(maxEdgeCount);
  EdgeF *edgeArray = &edges[0];

  uint32_t *firstEdge = new uint32_t[vertexCount + maxEdgeCount];
  uint32_t *nextEdge = firstEdge + vertexCount;

  const uint32_t INVALID = -1; //0xFFFFFFFF;

  for (size_t a = 0; a < vertexCount; ++a)
    firstEdge[a] = INVALID;

  // First pass over all triangles. This finds all the edges satisfying the
  // condition that the first vertex index is less than the second vertex index
  // when the direction from the first vertex to the second vertex represents
  // a counterclockwise winding around the triangle to which the edge belongs.
  // For each edge found, the edge index is stored in a linked list of edges
  // belonging to the lower-numbered vertex index i. This allows us to quickly
  // find an edge in the second pass whose higher-numbered vertex index is i.

  size_t edgeCount = 0;
  const uint32_t *triangles = mesh.triangles;
  for (size_t a = 0; a < triangleCount; ++a) {
    uint32_t i1 = triangles[3 * a + 2];
    for (int b = 0; b < 3; ++b) {
      const uint32_t i2 = triangles[3 * a + b];
      if (i1 < i2) {
        EdgeF *edge = &edgeArray[edgeCount];

        edge->vertexIndex[0] = i1;
        edge->vertexIndex[1] = i2;
        edge->faceIndex[0] = a;
        edge->faceIndex[1] = INVALID;

        uint32_t edgeIndex = firstEdge[i1];
        if (edgeIndex == INVALID) {
          firstEdge[i1] = edgeCount;
        } else {
          for (;;) {
            uint32_t index = nextEdge[edgeIndex];
            if (index == INVALID) {
              nextEdge[edgeIndex] = edgeCount;
              break;
            }
            edgeIndex = index;
          }
        }
        nextEdge[edgeCount] = INVALID;
        ++edgeCount;
      }

      i1 = i2;
    }
  }

  // Second pass over all triangles. This finds all the edges satisfying the
  // condition that the first vertex index is greater than the second vertex
  // index
  // when the direction from the first vertex to the second vertex represents
  // a counterclockwise winding around the triangle to which the edge belongs.
  // For each of these edges, the same edge should have already been found in
  // the first pass for a different triangle. So we search the list of edges
  // for the higher-numbered vertex index for the matching edge and fill in the
  // second triangle index. The maximum number of comparisons in this search for
  // any vertex is the number of edges having that vertex as an endpoint.

  for (uint32_t a = 0; a < triangleCount; ++a) {
    uint32_t i1 = triangles[3 * a + 2];
    for (int b = 0; b < 3; ++b) {
      const uint32_t i2 = triangles[3 * a + b];
      if (i1 > i2) {

        bool found = false;
        uint32_t prev_edgeIndex = firstEdge[i2];
        for (uint32_t edgeIndex = firstEdge[i2]; edgeIndex != INVALID;
             edgeIndex = nextEdge[edgeIndex]) {
          EdgeF *edge = &edgeArray[edgeIndex];
          if (edge->vertexIndex[1] == i1) {
            assert(edge->vertexIndex[0] == i2);
            assert(edge->faceIndex[1] == INVALID);
            edge->faceIndex[1] = a;
            found = true;
            break;
          }
          prev_edgeIndex = edgeIndex;
        }
        if (found == false) {
          assert(edgeCount < maxEdgeCount);
          EdgeF *edge = &edgeArray[edgeCount];
          edge->vertexIndex[0] = i2;
          edge->vertexIndex[1] = i1;
          edge->faceIndex[0] = a;
          edge->faceIndex[1] = INVALID;

          if (prev_edgeIndex == INVALID) {
            assert(firstEdge[i2] == INVALID);
            firstEdge[i2] = edgeCount;
          } else {
            assert(nextEdge[prev_edgeIndex] == INVALID);
            nextEdge[prev_edgeIndex] = edgeCount;
          }
          nextEdge[edgeCount] = INVALID;
          ++edgeCount;
        }
      }

      i1 = i2;
    }
  }

  edges.resize(edgeCount);

  delete[] firstEdge;
}

std::vector<Edge>
getBorderEdges(const Mesh &mesh)
{
  //SOME CODE DUPLICATION with getEdges() !!!!

  std::vector<Edge> borderEdges;

  //Method adapted from:
  //Lengyel, Eric.
  //“Building an Edge List for an Arbitrary Mesh”.
  //Terathon Software 3D Graphics Library, 2005.
  //http://www.terathon.com/code/edges.html

  //Slightly adapated as:
  //-we do not have "closed" mesh (thus we have border edges seen only once)
  //-we do not need the triangleIndices in Edge,
  //-we do not have unsigned short but uint32_t for indices..

  const size_t vertexCount = mesh.numVertices;
  const size_t triangleCount = mesh.numTriangles;

  const size_t maxEdgeCount = triangleCount * 3;

  borderEdges.reserve(triangleCount / 2); //arbitrary

  std::vector<Edge> edges(maxEdgeCount); //useless hidden memset
  Edge *edgeArray = &edges[0];

  uint32_t *firstEdge = new uint32_t[vertexCount + maxEdgeCount];
  uint32_t *nextEdge = firstEdge + vertexCount;

  const uint32_t INVALID = -1; //0xFFFFFFFF;

  for (size_t a = 0; a < vertexCount; ++a)
    firstEdge[a] = INVALID;

  // First pass over all triangles. This finds all the edges satisfying the
  // condition that the first vertex index is less than the second vertex index
  // when the direction from the first vertex to the second vertex represents
  // a counterclockwise winding around the triangle to which the edge belongs.
  // For each edge found, the edge index is stored in a linked list of edges
  // belonging to the lower-numbered vertex index i. This allows us to quickly
  // find an edge in the second pass whose higher-numbered vertex index is i.

  size_t edgeCount = 0;
  const uint32_t *triangles = mesh.triangles;
  for (size_t a = 0; a < triangleCount; ++a) {
    uint32_t i1 = triangles[3 * a + 2];
    for (int b = 0; b < 3; ++b) {
      const uint32_t i2 = triangles[3 * a + b];
      if (i1 < i2) {
        Edge *edge = &edgeArray[edgeCount];

        edge->vertexIndex[0] = i1;
        edge->vertexIndex[1] = i2;

        uint32_t edgeIndex = firstEdge[i1];
        if (edgeIndex == INVALID) {
          firstEdge[i1] = edgeCount;
        } else {
          for (;;) {
            uint32_t index = nextEdge[edgeIndex];
            if (index == INVALID) {
              nextEdge[edgeIndex] = edgeCount;
              break;
            }
            edgeIndex = index;
          }
        }
        nextEdge[edgeCount] = INVALID;
        ++edgeCount;
      }

      i1 = i2;
    }
  }

  std::vector<bool> visitedTwice(edgeCount, false);

  // Second pass over all triangles. This finds all the edges satisfying the
  // condition that the first vertex index is greater than the second vertex
  // index
  // when the direction from the first vertex to the second vertex represents
  // a counterclockwise winding around the triangle to which the edge belongs.
  // For each of these edges, the same edge should have already been found in
  // the first pass for a different triangle. So we search the list of edges
  // for the higher-numbered vertex index for the matching edge and fill in the
  // second triangle index. The maximum number of comparisons in this search for
  // any vertex is the number of edges having that vertex as an endpoint.

  for (uint32_t a = 0; a < triangleCount; ++a) {
    uint32_t i1 = triangles[3 * a + 2];
    for (int b = 0; b < 3; ++b) {
      const uint32_t i2 = triangles[3 * a + b];
      if (i1 > i2) {

        bool found = false;
        for (uint32_t edgeIndex = firstEdge[i2]; edgeIndex != INVALID;
             edgeIndex = nextEdge[edgeIndex]) {
          Edge *edge = &edgeArray[edgeIndex];
          if (edge->vertexIndex[1] == i1) {
            assert(edge->vertexIndex[0] == i2);
            visitedTwice[edgeIndex] = true;
            found = true;
            break;
          }
        }
        if (found == false) {
          Edge edge;
          edge.vertexIndex[0] = i2;
          edge.vertexIndex[1] = i1;
          borderEdges.push_back(edge);
        }
      }

      i1 = i2;
    }
  }

  {
    const size_t sz = visitedTwice.size();
    for (size_t i = 0; i < sz; ++i) {
      if (!visitedTwice[i]) {
        borderEdges.push_back(edges[i]);
      }
    }
  }

  //edges.resize(edgeCount);

  delete[] firstEdge;

  return borderEdges;
}

struct EdgeSorter
{
  inline bool operator()(Edge e1, Edge e2) const
  {
    return e1.vertexIndex[0] < e2.vertexIndex[0] ||
           (e1.vertexIndex[0] == e2.vertexIndex[0] &&
            e1.vertexIndex[1] < e2.vertexIndex[1]);
  }
};

void
checkEdges(std::vector<Edge> &edges)
{
  std::sort(edges.begin(), edges.end(), EdgeSorter());

  const size_t sz = edges.size();

  for (size_t i = 0; i < sz; ++i) {
    if (edges[i].vertexIndex[0] >= edges[i].vertexIndex[1]) {
      std::cerr << "ERROR: invalid edge " << i
                << ": e0=" << edges[i].vertexIndex[0]
                << " >= e1=" << edges[i].vertexIndex[1] << "\n";
      exit(10);
    }
    for (size_t j = i + 1; j < sz; ++j) {
      if (!(edges[i].vertexIndex[0] < edges[j].vertexIndex[0] ||
            (edges[i].vertexIndex[0] == edges[j].vertexIndex[0] &&
             edges[i].vertexIndex[1] < edges[j].vertexIndex[1]))) {

        std::cerr << "ERROR: in edge order: " << i
                  << ": e0=" << edges[i].vertexIndex[0]
                  << " e1=" << edges[i].vertexIndex[1] << " ; " << j
                  << ": e0=" << edges[j].vertexIndex[0]
                  << " e1=" << edges[j].vertexIndex[1] << "\n";
        exit(10);
      }
    }
  }
}
void
checkEdgesLight(std::vector<Edge> &edges)
{
  std::sort(edges.begin(), edges.end(), EdgeSorter());

  const size_t sz = edges.size();

  for (size_t i = 0; i < sz; ++i) {
    if (edges[i].vertexIndex[0] >= edges[i].vertexIndex[1]) {
      std::cerr << "ERROR: invalid edge " << i
                << ": e0=" << edges[i].vertexIndex[0]
                << " >= e1=" << edges[i].vertexIndex[1] << "\n";
      exit(10);
    }
  }
}

void
compareEdges(std::vector<Edge> &edges1, std::vector<Edge> &edges2)
{
  checkEdgesLight(edges2); //checkEdges(edges2);
  std::cerr << "edges2 ok\n";
  checkEdgesLight(edges1); //checkEdges(edges1);
  std::cerr << "edges1 ok\n";

  if (edges1.size() != edges2.size()) {
    std::cerr << "ERROR: edges have not the same size : " << edges1.size()
              << " vs " << edges2.size() << "\n";
    exit(10);
  }

  std::sort(edges1.begin(), edges1.end(), EdgeSorter());
  std::sort(edges2.begin(), edges2.end(), EdgeSorter());

  for (size_t i = 0; i < edges1.size(); ++i) {
    if (edges1[i].vertexIndex[0] != edges2[i].vertexIndex[0] ||
        edges1[i].vertexIndex[1] != edges2[i].vertexIndex[1]) {
      std::cerr << "edges1[" << i << "]=[" << edges1[i].vertexIndex[0] << ", "
                << edges1[i].vertexIndex[1] << "]\n";
      std::cerr << "edges2[" << i << "]=[" << edges2[i].vertexIndex[0] << ", "
                << edges2[i].vertexIndex[1] << "]\n";
      exit(10);
    }
  }
}

#include <chrono>

void
DEBUG_checkEdges(const Mesh &mesh)
{
  std::vector<Edge> edges, edgesH;

  {
    auto start = std::chrono::steady_clock::now();

    edges = getEdges(mesh);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = (end - start);
    std::cerr << "getEdges time=" << diff.count() << "s for " << edges.size()
              << " edges\n";
  }
  {
    auto start = std::chrono::steady_clock::now();

    edgesH = getEdgesH(mesh);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = (end - start);
    std::cerr << "getEdges time=" << diff.count() << "s for " << edgesH.size()
              << " edges\n";
  }

  compareEdges(edges, edgesH);

  std::cerr << "getEdges OK !\n";
}

//B: it can probably be faster if we consider that we have closed contours
// and no loops (no "8")

using label = uint32_t; //we can probably use something smaller than uint32_t...

//@param[in, out] labels
size_t
removeHoles(std::vector<label> &labels)
{
//we have labels :         0 0 2 0 2 0 4 0 4 2
//we want to rename them:  0 0 1 0 1 0 2 0 2 1
//If we have a hole in indices [like here "1" is missing between "0" and "2"]
// we are sure that this indice is not present in remaining values

#ifndef NDEBUG
  for (size_t i = 0; i < labels.size(); ++i)
    assert(labels[i] <= i);
#endif //NDEBUG

  label cpt = 0;
  for (size_t i = 0; i < labels.size(); ++i) {
    const label v = labels[i];
    if (v > cpt) {
      for (size_t j = i; j < labels.size(); ++j) {
        assert(labels[j] != cpt);
        if (labels[j] == v)
          labels[j] = cpt;
      }
      ++cpt;
    } else if (v == cpt)
      ++cpt;
  }
  return cpt;
}

void
getBorderEdgesAsContours(const Mesh &mesh,
                         std::vector<std::vector<Edge>> &outEdges)
{
  outEdges.clear();

  std::vector<Edge> edges = getBorderEdges(mesh);

  const size_t numEdges = edges.size();

  std::vector<Edge> extremities;
  extremities.reserve(numEdges / 3); //arbitrary

  const label INVALID = -1;
  std::vector<label> contourLabels(numEdges, INVALID);

  /*
  std::vector<bool> used(numEdges, false);
  std::vector<Edge> currentContour;
  currentContour.reserve(numEdges);
  Edge extremity; //extremities of current contour
  extremity.vertexIndex[0] = INVALID;
  extremity.vertexIndex[1] = INVALID;
  */

  //first pass:
  //traverse all edges and compue parts of continuous contour

  for (size_t i = 0; i < numEdges; ++i) {

    assert(contourLabels[i] == INVALID);
    {

      //edge not yet used in any contour
      //start new contour

      const Edge &e = edges[i];

      assert(e.vertexIndex[0] < e.vertexIndex[1]);

      //check if it can be added to an existing contour

      bool found = false;
      const size_t numContours = extremities.size();
      for (size_t k = 0; k < numContours; ++k) {
        if (extremities[k].vertexIndex[0] == e.vertexIndex[0]) {
          extremities[k].vertexIndex[0] = e.vertexIndex[1];
          contourLabels[i] = k;
          //std::cerr<<"edge "<<e<<" label="<<k<<" extremities[k]="<<extremities[k]<<"\n";
          found = true;
          break;
        } else if (extremities[k].vertexIndex[0] == e.vertexIndex[1]) {
          extremities[k].vertexIndex[0] = e.vertexIndex[0];
          contourLabels[i] = k;
          //std::cerr<<"edge "<<e<<" label="<<k<<" extremities[k]="<<extremities[k]<<"\n";
          found = true;
          break;
        } else if (extremities[k].vertexIndex[1] == e.vertexIndex[0]) {
          extremities[k].vertexIndex[1] = e.vertexIndex[1];
          contourLabels[i] = k;
          //std::cerr<<"edge "<<e<<" label="<<k<<" extremities[k]="<<extremities[k]<<"\n";
          found = true;
          break;
        } else if (extremities[k].vertexIndex[1] == e.vertexIndex[1]) {
          extremities[k].vertexIndex[1] = e.vertexIndex[0];
          contourLabels[i] = k;
          //std::cerr<<"edge "<<e<<" label="<<k<<" extremities[k]="<<extremities[k]<<"\n";
          found = true;
          break;
        }
      }

      if (!found) {
        //start a new contour
        contourLabels[i] = numContours;
        extremities.push_back(e);
      }
    }
  }

  const size_t numContours = extremities.size();
  std::vector<label> contourLabelsChanges(numContours);
  for (size_t i = 0; i < numContours; ++i) {
    contourLabelsChanges[i] = i;
  }

  std::vector<bool> extremityValid(numContours, true);
  for (size_t i = 0; i < numContours; ++i) {
    if (extremityValid[i]) {

      Edge &ext_i = extremities[i];

      bool change = false;
      do {
        change = false;

        for (size_t j = i + 1; j < numContours; ++j) {

          if (extremityValid[j]) {

            bool found = false;

            Edge &ext_j = extremities[j];

            if (ext_i.vertexIndex[0] == ext_j.vertexIndex[0]) {
              //std::cerr<<"extremities["<<i<<"]="<<extremities[i]<<"+"<<"extremities["<<j<<"]="<<extremities[j];
              ext_i.vertexIndex[0] = ext_j.vertexIndex[1];
              //std::cerr<<"=>"<<ext_i<<"\n";
              found = true;
            } else if (ext_i.vertexIndex[0] == ext_j.vertexIndex[1]) {
              //std::cerr<<"extremities["<<i<<"]="<<extremities[i]<<"+"<<"extremities["<<j<<"]="<<extremities[j];
              ext_i.vertexIndex[0] = ext_j.vertexIndex[0];
              //std::cerr<<"=>"<<ext_i<<"\n";
              found = true;
            } else if (ext_i.vertexIndex[1] == ext_j.vertexIndex[0]) {
              //std::cerr<<"extremities["<<i<<"]="<<extremities[i]<<"+"<<"extremities["<<j<<"]="<<extremities[j];
              ext_i.vertexIndex[1] = ext_j.vertexIndex[1];
              //std::cerr<<"=>"<<ext_i<<"\n";
              found = true;
            } else if (ext_i.vertexIndex[1] == ext_j.vertexIndex[1]) {
              //std::cerr<<"extremities["<<i<<"]="<<extremities[i]<<"+"<<"extremities["<<j<<"]="<<extremities[j];
              ext_i.vertexIndex[1] = ext_j.vertexIndex[0];
              //std::cerr<<"=>"<<ext_i<<"\n";
              found = true;
            }

            if (found) {
              extremityValid[j] = false;
              contourLabelsChanges[j] = contourLabelsChanges[i];
              //std::cerr<<"contourLabelsChanges["<<j<<"]="<<contourLabelsChanges[j]<<"\n";
              change = true;
            }
          }
        }
      } while (change == true);
    }
  }

  const size_t numFinalContours = removeHoles(contourLabelsChanges);

  outEdges.resize(numFinalContours);
  for (size_t i = 0; i < numEdges; ++i) {
    assert(contourLabels[i] < contourLabelsChanges.size());
    //std::cerr<<"edges["<<i<<"]="<<edges[i]<<" label="<<contourLabels[i]<<" finaleLabel=contourLabelsChanges[contourLabels[i]]="<<contourLabelsChanges[contourLabels[i]]<<"\n";
    const size_t ind = contourLabelsChanges[contourLabels[i]];
    assert(ind < numFinalContours);
    outEdges[ind].push_back(edges[i]);
  }

#ifndef NDEBUG
  {
    size_t totalNumEdges = 0;
    for (size_t i = 0; i < outEdges.size(); ++i) {
      totalNumEdges += outEdges[i].size();
    }
    assert(totalNumEdges == edges.size());
  }
#endif //NDEBUG
}

void
getLargestBorderEdges(const Mesh &mesh, std::vector<Edge> &edges)
{
  std::vector<std::vector<Edge>> outEdges;
  getBorderEdgesAsContours(mesh, outEdges);

  const size_t numContours = outEdges.size();
  size_t maxLength = 0;
  size_t ind = numContours;
  for (size_t i = 0; i < numContours; ++i) {
    const size_t length = outEdges[i].size();
    if (length > maxLength) {
      maxLength = length;
      ind = i;
    }
  }
  assert(ind < numContours);

  edges = outEdges[ind]; //TODO:OPTIM: use C+11 move
}

std::vector<uint32_t>
getBorderPoints(const Mesh &mesh)
{
  std::vector<uint32_t> vertexIndices;

  std::vector<Edge> borderEdges = getBorderEdges(mesh);

  //std::cerr<<"borderEdges.size()="<<borderEdges.size()<<"\n";

  vertexIndices.reserve(2 * borderEdges.size());

  //std::cerr<<"vertexIndices.capacity()="<<vertexIndices.capacity()<<"\n";

  std::vector<Edge>::const_iterator first = borderEdges.cbegin();
  const std::vector<Edge>::const_iterator last = borderEdges.cend();
  for (; first != last; ++first) {
    assert(first->vertexIndex[0] < first->vertexIndex[1]);
    vertexIndices.push_back(first->vertexIndex[0]);
    vertexIndices.push_back(first->vertexIndex[1]);
    //REM: just pushing first->vertexIndex[0] is not enough...
  }

  //std::cerr<<"after push: vertexIndices.size()="<<vertexIndices.size()<<"\n";

  std::sort(vertexIndices.begin(), vertexIndices.end());
  vertexIndices.erase(std::unique(vertexIndices.begin(), vertexIndices.end()),
                      vertexIndices.end());

  //std::cerr<<"after remove: vertexIndices.size()="<<vertexIndices.size()<<" / "<<mesh.numVertices<<"\n";

  return vertexIndices;
}

std::vector<uint32_t>
getBorderPointsB(const Mesh &mesh)
{
  std::vector<uint32_t> vertexIndices;

  const size_t maxNumEdges = 3 * mesh.numTriangles;
  std::vector<Edge> edges(maxNumEdges);

  //std::cerr<<"maxNumEdges="<<maxNumEdges<<"\n";

  size_t edgeCount = 0;
  for (uint32_t i = 0; i < mesh.numTriangles; ++i) {

    uint32_t i1 = mesh.triangles[3 * i + 2];
    for (int k = 0; k < 3; ++k) {
      uint32_t i2 = mesh.triangles[3 * i + k];

      uint32_t e1 = i1, e2 = i2;
      if (i1 > i2)
        std::swap(e1, e2);

      if (e1 == e2) {
        std::cerr << "i=" << i << " mesh.triangles "
                  << mesh.triangles[3 * i + 0] << " "
                  << mesh.triangles[3 * i + 1] << " "
                  << mesh.triangles[3 * i + 2] << "\n";
        exit(10);
      }

      Edge &edge = edges[edgeCount];
      edge.vertexIndex[0] = e1;
      edge.vertexIndex[1] = e2;
      ++edgeCount;

      i1 = i2;
    }
  }
  //std::cerr<<edges<<"\n";

  assert(edgeCount <= maxNumEdges);

  edges.resize(edgeCount);

  std::sort(edges.begin(), edges.end(), EdgeSorter());

  //We remove edges present twice.
  //We consider that we have at most 2 triangles sharing an edge.
  //(that is the same edge is present only two times).

  std::vector<Edge>::iterator result = edges.begin();
  {
    //inspired from std::unique()
    //http://www.cplusplus.com/reference/algorithm/unique/

    std::vector<Edge>::iterator first = edges.begin();
    const std::vector<Edge>::iterator last = edges.end();
    std::vector<Edge>::iterator next = first + 1;

    while (first != last) {
      if (next == last) {
        *result = *first;
        ++result;
        break;
      }

      if (!(*first == *next)) {

        *result = *first;

        //std::cerr<<"first="<<(*first)<<" last="<<*next<<" result="<<*result<<"\n";
        ++result;
        ++first;
        ++next;
      } else {
        //std::cerr<<"skip "<<*first<<"\n";
        first = next + 1;
        next = first + 1;
      }
    }
  }
  edges.resize(result - edges.begin());

  //std::cerr<<"edges.size()="<<edges.size()<<" on border\n";

  //std::cerr<<"after duplicate remove: edges.size()="<<edges.size()<<"\n";
  //std::cerr<<edges<<"\n";

  vertexIndices.reserve(2 * edges.size());

  //std::cerr<<"vertexIndices.capacity()="<<vertexIndices.capacity()<<"\n";

  {
    //Take into account that edges are sorted
    //(instead of pushing every index in @a indices)
    // We check that first index is not the last one inserted.
    // We still have to check for duplicates though !

    std::vector<Edge>::iterator first = edges.begin();
    uint32_t prevIndex = -1;
    const std::vector<Edge>::iterator last = edges.end();
    while (first != last) {
      if (first->vertexIndex[0] != prevIndex)
        vertexIndices.push_back(first->vertexIndex[0]);
      prevIndex = first->vertexIndex[0];
      vertexIndices.push_back(first->vertexIndex[1]);

      ++first;
    }
  }

  std::sort(vertexIndices.begin(), vertexIndices.end());
  vertexIndices.erase(std::unique(vertexIndices.begin(), vertexIndices.end()),
                      vertexIndices.end());

  //std::cerr<<"after remove: vertexIndices.size()="<<vertexIndices.size()<<" / "<<mesh.numVertices<<"\n";

  return vertexIndices;
}

void
check_getBorderPoints(const Mesh &mesh)
{
  std::vector<uint32_t> indices, indices2;
  {
    auto start = std::chrono::steady_clock::now();

    indices = getBorderPoints(mesh);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = (end - start);
    std::cerr << "getPointsOnBorders time=" << diff.count() << "s for "
              << indices.size() << " indices\n";
  }
  {
    auto start = std::chrono::steady_clock::now();

    indices2 = getBorderPointsB(mesh);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = (end - start);
    std::cerr << "getPointsOnBordersB time=" << diff.count() << "s for "
              << indices2.size() << " indices\n";
  }
}

std::vector<uint32_t>
getLargestBorderPoints(const Mesh &mesh)
{
  //CODE DUPLICATION with getBorderPoints

  std::vector<uint32_t> vertexIndices;

  std::vector<Edge> borderEdges;
  getLargestBorderEdges(mesh, borderEdges);

  //std::cerr<<"borderEdges.size()="<<borderEdges.size()<<"\n";

  vertexIndices.reserve(2 * borderEdges.size());

  //std::cerr<<"vertexIndices.capacity()="<<vertexIndices.capacity()<<"\n";

  std::vector<Edge>::const_iterator first = borderEdges.cbegin();
  const std::vector<Edge>::const_iterator last = borderEdges.cend();
  for (; first != last; ++first) {
    assert(first->vertexIndex[0] < first->vertexIndex[1]);
    vertexIndices.push_back(first->vertexIndex[0]);
    vertexIndices.push_back(first->vertexIndex[1]);
    //REM: just pushing first->vertexIndex[0] is not enough...
  }

  //std::cerr<<"after push: vertexIndices.size()="<<vertexIndices.size()<<"\n";

  std::sort(vertexIndices.begin(), vertexIndices.end());
  vertexIndices.erase(std::unique(vertexIndices.begin(), vertexIndices.end()),
                      vertexIndices.end());

  //std::cerr<<"after remove: vertexIndices.size()="<<vertexIndices.size()<<" / "<<mesh.numVertices<<"\n";

  return vertexIndices;
}

struct Edge1I
{
  uint32_t vertexIndex[2];
  uint32_t triangleIndex0;
};

void
getAdjacentTriangles(const Mesh &mesh, std::vector<uint32_t> &adjacentTris)
{
  //some CODE DUPLICATION with getEdges()

  assert(mesh.isValid());

  const uint32_t INVALID = -1; //0xFFFFFFFF;

  adjacentTris.resize(0);
  adjacentTris.resize(mesh.numTriangles * 3, INVALID);

  std::vector<Edge1I> edges;

  //Method adapted from:
  //Lengyel, Eric.
  //“Building an Edge List for an Arbitrary Mesh”.
  //Terathon Software 3D Graphics Library, 2005.
  //http://www.terathon.com/code/edges.html

  //Slightly adapated as:
  //-we do not have "closed" mesh (thus we have border edges seen only once)
  //-we do not need the triangleIndices in Edge,
  //-we do not have unsigned short but uint32_t for indices..

  const size_t vertexCount = mesh.numVertices;
  const size_t triangleCount = mesh.numTriangles;

  const size_t maxEdgeCount = triangleCount * 3;

  edges.resize(maxEdgeCount);
  Edge1I *edgeArray = &edges[0];

  uint32_t *firstEdge = new uint32_t[vertexCount + maxEdgeCount];
  uint32_t *nextEdge = firstEdge + vertexCount;

  for (size_t a = 0; a < vertexCount; ++a)
    firstEdge[a] = INVALID;

  // First pass over all triangles. This finds all the edges satisfying the
  // condition that the first vertex index is less than the second vertex index
  // when the direction from the first vertex to the second vertex represents
  // a counterclockwise winding around the triangle to which the edge belongs.
  // For each edge found, the edge index is stored in a linked list of edges
  // belonging to the lower-numbered vertex index i. This allows us to quickly
  // find an edge in the second pass whose higher-numbered vertex index is i.

  size_t edgeCount = 0;
  const uint32_t *triangles = mesh.triangles;
  for (size_t a = 0; a < triangleCount; ++a) {
    uint32_t i1 = triangles[3 * a + 2];
    for (int b = 0; b < 3; ++b) {
      const uint32_t i2 = triangles[3 * a + b];
      if (i1 < i2) {
        Edge1I *edge = &edgeArray[edgeCount];

        edge->vertexIndex[0] = i1;
        edge->vertexIndex[1] = i2;
        edge->triangleIndex0 = a;

        uint32_t edgeIndex = firstEdge[i1];
        if (edgeIndex == INVALID) {
          firstEdge[i1] = edgeCount;
        } else {
          for (;;) {
            uint32_t index = nextEdge[edgeIndex];
            if (index == INVALID) {
              nextEdge[edgeIndex] = edgeCount;
              break;
            }
            edgeIndex = index;
          }
        }
        nextEdge[edgeCount] = INVALID;
        ++edgeCount;
      }

      i1 = i2;
    }
  }

  // Second pass over all triangles. This finds all the edges satisfying the
  // condition that the first vertex index is greater than the second vertex
  // index
  // when the direction from the first vertex to the second vertex represents
  // a counterclockwise winding around the triangle to which the edge belongs.
  // For each of these edges, the same edge should have already been found in
  // the first pass for a different triangle. So we search the list of edges
  // for the higher-numbered vertex index for the matching edge and fill in the
  // second triangle index. The maximum number of comparisons in this search for
  // any vertex is the number of edges having that vertex as an endpoint.

  for (uint32_t a = 0; a < triangleCount; ++a) {
    uint32_t i1 = triangles[3 * a + 2];
    for (int b = 0; b < 3; ++b) {
      const uint32_t i2 = triangles[3 * a + b];
      if (i1 > i2) {

        for (uint32_t edgeIndex = firstEdge[i2]; edgeIndex != INVALID;
             edgeIndex = nextEdge[edgeIndex]) {
          Edge1I *edge = &edgeArray[edgeIndex];
          if (edge->vertexIndex[1] == i1) {
            assert(edge->vertexIndex[0] == i2);

            uint32_t c = edge->triangleIndex0;
            assert(c < triangleCount);

            //B:OPTIM?
            //we could store the edge index (here 'd') in the Edge1I structure
            // to avoid to have to look for it...

            uint32_t j1 = triangles[3 * c + 2];
            for (int d = 0; d < 3; ++d) {
              const uint32_t j2 = triangles[3 * c + d];
              if (j1 == i2 && j2 == i1) {
                adjacentTris[3 * c + d] = a;
                break;
              }
              j1 = j2;
            }

            adjacentTris[3 * a + b] = c;

            break;
          }
        }
      }

      i1 = i2;
    }
  }

  //edges.resize(edgeCount);

  delete[] firstEdge;
}

#include <queue>

void
getConnectedComponents(const Mesh &mesh,
                       std::vector<uint32_t> &ccs,
                       std::vector<uint32_t> &ccSizes)
{
  assert(mesh.isValid());

  std::vector<uint32_t> adjacentTris;
  getAdjacentTriangles(mesh, adjacentTris);

  const uint32_t numTriangles = mesh.numTriangles;

  assert(adjacentTris.size() == 3 * numTriangles);

  ccs.resize(numTriangles);

  ccSizes.resize(0);
  ccSizes.reserve(7); //arbitrary

  std::vector<bool> used(numTriangles, false);

  std::queue<uint32_t> q;

  uint32_t id = 0;
  for (uint32_t i = 0; i < numTriangles; ++i) {

    if (used[i])
      continue;

    //new connected component

    ccSizes.push_back(0);
    assert(ccSizes.size() == id + 1);
    assert(q.empty());
    q.push(i);

    while (!q.empty()) {

      uint32_t j = q.front();
      q.pop();

      if (used[j])
        continue;

      used[j] = true;
      ccSizes[id]++;
      ccs[j] = id;

      for (int k = 0; k < 3; ++k) {
        const uint32_t neigh = adjacentTris[3 * j + k];
        if (neigh != (uint32_t)-1 && !used[neigh])
          q.push(neigh);
      }
    }

    ++id;
  }

  assert(ccSizes.size() == id);
}

void
keepOnlyLargestCC(Mesh &mesh)
{
  assert(mesh.isValid());

  std::vector<uint32_t> ccs;
  std::vector<uint32_t> ccSizes;
  getConnectedComponents(mesh, ccs, ccSizes);

  const size_t numCC = ccSizes.size();
  assert(numCC > 0);
  if (numCC > 1) {

    {
      std::cerr << "There are " << numCC
                << " connected components in mesh (of sizes: \n";
      for (size_t i = 0; i < numCC - 1; ++i)
        std::cerr << ccSizes[i] << ", ";
      std::cerr << ccSizes[numCC - 1] << ")\n";
      std::cerr << "Only the largest one will be kept !\n";
    }

    size_t largestIndex = 0;
    size_t largestSize = ccSizes[0];
    for (size_t i = 1; i < numCC; ++i) { //start from 1
      if (ccSizes[i] > largestSize) {
        largestSize = ccSizes[i];
        largestIndex = i;
      }
    }
    assert(largestIndex < ccSizes.size());
    uint32_t newNumTriangles = ccSizes[largestIndex];

    Mesh mesh2;
    mesh2.allocateTriangles(newNumTriangles);
    mesh2.allocateVertices(mesh.numVertices);
    if (mesh.hasNormals())
      mesh2.allocateNormals();
    if (mesh.hasTexCoords())
      mesh2.allocateTexCoords();

    assert(mesh2.triangles);
    const uint32_t oldNumTriangles = mesh.numTriangles;
    uint32_t idx = 0;
    for (uint32_t i = 0; i < oldNumTriangles; ++i) {
      if (ccs[i] == largestIndex) {
        //triangle belongs to kept CC

        assert(idx < newNumTriangles);

        for (int k = 0; k < 3; ++k)
          mesh2.triangles[3 * idx + k] = mesh.triangles[3 * i + k];
        ++idx;
      }
    }
    assert(idx == largestSize);

    mesh2.removeNonReferencedVertices();

    std::swap(mesh, mesh2);
  }
}
