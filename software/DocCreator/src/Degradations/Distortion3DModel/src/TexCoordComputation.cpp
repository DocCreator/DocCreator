#define _USE_MATH_DEFINES // for Visual

#include "TexCoordComputation.hpp"

#include <cmath> //M_PI

#include <algorithm>
#include <fstream>
#include <iomanip> //DEBUG
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "TexCoordComputationCommon.hpp"

#define CHECK_TEXCOORDS 1
//#define DEBUG_TEXCOORDS 1
//#define TIME_TEXCOORDS 1

#ifdef TIME_TEXCOORDS
#include <opencv2/core/core.hpp> //cv::getTickCount()
#endif                           //TIME_TEXCOORDS

#include "Mesh.hpp"

class Vector2
{
public:
  Vector2(float x_in, float y_in)
    : x(x_in)
    , y(y_in)
  {}
  Vector2()
    : x(0)
    , y(0)
  {}
  float length() const { return sqrt(x * x + y * y); }
  float x;
  float y;
};

class Vector3
{
public:
  Vector3(float x_ = 0.f, float y_ = 0.f, float z_ = 0.f)
    : x(x_)
    , y(y_)
    , z(z_)
  {}

  float length() const
  {
    return sqrtf(x * x + y * y + z * z); //B:warning:costly
  }

  void normalize()
  {
    const float l = length();
    x /= l;
    y /= l;
    z /= l;
  }

  float x;
  float y;
  float z;
};

/*
 * Plane in Oxyz: f() = ax + by + cz + d
 */
class Plane
{
public:
  Plane(float a_, float b_, float c_, float d_)
    : a(a_)
    , b(b_)
    , c(c_)
    , d(d_)
    , n(a_, b_, c_)
  {}

  Plane()
    : a(0)
    , b(0)
    , c(0)
    , d(0)
    , n(0, 0, 0)
  {}

  //B:TODO: why ione version of f() check for GLM_dEpsilon and not the other ???

  float f(float x, float y, float z) const
  {
    return (a * x + b * y + c * z + d);
  }

    /*
        void prinft(Vector3 v) const {
        float result = (v.y +d);
        //        result -=result;
        }
      */

#define dEpsilon 0.0005f

  float f(Vector3 v) const
  {
    float result = (a * v.x + b * v.y + c * v.z + d);
    if (fabs(result) <= dEpsilon)
      result = 0;

    return result;
  }
  float length() const { return sqrt(a * a + b * b + c * c); }
  float a;
  float b;
  float c;
  float d;
  Vector3 n;
};

class Line3
{
public:
  Line3(Vector3 v1, Vector3 v2)
    : n(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z)
    , M0(v1.x, v1.y, v1.z)
  {}

  Vector3 getPoint(float y) const
  {
    const float t = (y - M0.y) / n.y; //B:TODO:OPTIM: precompute 1/n.y ???
    const Vector3 p(M0.x + n.x * t, y, M0.z + n.z * t);
    return p;
  }

  Vector3 n;
  Vector3 M0;
};

/*
 *
 * IsEqualZero for a double : dx < 10E-6.
 *
 */
static inline bool
IsEqualZero(float dX)
{
  return fabsf(dX) <= 10E-6f;
}

//B:CODE:OLD: que faisait la vieille methode : glmCorrectTexCoor()
//B:CODE:OLD: que faisait la vieille methode : testTextureCoordinates()
//B:CODE:OLD: que faisait la vieille methode : glmCorrectReadingOBJByIOStream()
//B:CODE:OLD A quoi servaient les fonctions morphologiques ???
//    blurred(), morphologyErision(), morphologyDilatation(),
//    morphologyClosing()

void
debugTexCoords(Mesh &mesh)
{
  if (!mesh.hasTexCoords())
    return;

  Mesh mesh2;
  mesh.copyTo(mesh2);

  //const uint32_t numTexCoords = mesh.numVertices;
  const uint32_t numTriangles = mesh.numTriangles;
  const uint32_t *triangles = mesh.triangles;

  //uint32_t min_tri_idx = numTriangles;
  //float min_dist = std::numeric_limits<float>::max();
  //uint32_t nb_min = 0;

  for (uint32_t i = 0; i < numTriangles; ++i) {

    const uint32_t t_id_0 = triangles[3 * i + 0];
    const uint32_t t_id_1 = triangles[3 * i + 1];
    const uint32_t t_id_2 = triangles[3 * i + 2];

    float v0_tx = mesh.texCoords[2 * t_id_0 + 0];
    float v0_ty = mesh.texCoords[2 * t_id_0 + 1];

    float v1_tx = mesh.texCoords[2 * t_id_1 + 0];
    float v1_ty = mesh.texCoords[2 * t_id_1 + 1];

    float v2_tx = mesh.texCoords[2 * t_id_2 + 0];
    float v2_ty = mesh.texCoords[2 * t_id_2 + 1];

    float d01 =
      (v0_tx - v1_tx) * (v0_tx - v1_tx) + (v0_ty - v1_ty) * (v0_ty - v1_ty);
    float d02 =
      (v0_tx - v2_tx) * (v0_tx - v2_tx) + (v0_ty - v2_ty) * (v0_ty - v2_ty);
    float d12 =
      (v1_tx - v2_tx) * (v1_tx - v2_tx) + (v1_ty - v2_ty) * (v1_ty - v2_ty);

    mesh2.texCoords[2 * t_id_0 + 0] = 1;
    mesh2.texCoords[2 * t_id_0 + 1] = 1;
    mesh2.texCoords[2 * t_id_1 + 0] = 1;
    mesh2.texCoords[2 * t_id_1 + 1] = 1;
    mesh2.texCoords[2 * t_id_2 + 0] = 1;
    mesh2.texCoords[2 * t_id_2 + 1] = 1;

    const float eps = 0.000001f;
#if 0
    if (d01 < eps) {
      mesh2.texCoords[2*t_id_0 + 0] = 0;
      mesh2.texCoords[2*t_id_0 + 1] = 0;
      mesh2.texCoords[2*t_id_1 + 0] = 0;
      mesh2.texCoords[2*t_id_1 + 1] = 0;
    }
    if (d02 < eps) {
      mesh2.texCoords[2*t_id_0 + 0] = 0;
      mesh2.texCoords[2*t_id_0 + 1] = 0;
      mesh2.texCoords[2*t_id_2 + 0] = 0;
      mesh2.texCoords[2*t_id_2 + 1] = 0;
    }
    if (d12 < eps) {
      mesh2.texCoords[2*t_id_1 + 0] = 0;
      mesh2.texCoords[2*t_id_1 + 1] = 0;
      mesh2.texCoords[2*t_id_2 + 0] = 0;
      mesh2.texCoords[2*t_id_2 + 1] = 0;
    }
#else
    if (d01 < eps || d02 < eps || d12 < eps) {
      mesh2.texCoords[2 * t_id_0 + 0] = 0;
      mesh2.texCoords[2 * t_id_0 + 1] = 0;
      mesh2.texCoords[2 * t_id_1 + 0] = 0;
      mesh2.texCoords[2 * t_id_1 + 1] = 0;
      mesh2.texCoords[2 * t_id_2 + 0] = 0;
      mesh2.texCoords[2 * t_id_2 + 1] = 0;
    }

#endif
  }

  std::swap(mesh.texCoords, mesh2.texCoords);
}

void
rotateAroundMean(Mesh &mesh)
{
  const uint32_t numvertices = mesh.numVertices;
  float *vertices = mesh.vertices;

  Vector2 minX_p2(std::numeric_limits<uint32_t>::max(),
                  -(float)std::numeric_limits<uint32_t>::max());
  Vector2 minY_p2(-(float)std::numeric_limits<uint32_t>::max(),
                  std::numeric_limits<uint32_t>::max());
  Vector2 maxX_p2(-(float)std::numeric_limits<uint32_t>::max(),
                  std::numeric_limits<uint32_t>::max());
  Vector2 maxY_p2(std::numeric_limits<uint32_t>::max(),
                  -(float)std::numeric_limits<uint32_t>::max());
  uint32_t max_x_idx = std::numeric_limits<uint32_t>::min();
  float max_x = -std::numeric_limits<float>::max();
  uint32_t max_y_idx = std::numeric_limits<uint32_t>::min();
  float max_y = -std::numeric_limits<float>::max();
  uint32_t min_x_idx = std::numeric_limits<uint32_t>::max();
  float min_x = std::numeric_limits<float>::max();
  uint32_t min_y_idx = std::numeric_limits<uint32_t>::max();
  float min_y = std::numeric_limits<float>::max();

  for (uint32_t i = 0; i < numvertices; ++i) { //vertices indices start from 1
    const float x = vertices[3 * i + 0];
    const float y = vertices[3 * i + 1];

#if 0

    if (maxX_p2.x < x ||
	(maxX_p2.x == x && y < maxX_p2.y)) {  //maxX avec min y
      maxX_p2.x = x;
      maxX_p2.y = y;
      max_x_idx = i;
    }
    if (minX_p2.x > x ||
	(minX_p2.x == x && y > minX_p2.y)) { //minX avec max y
      minX_p2.x = x;
      minX_p2.y = y;
      min_x_idx = i;
    }
    
    if (maxY_p2.y < y ||
	(maxY_p2.y == y && x < maxY_p2.x)) {  //maxY avec min x
      maxY_p2.x = x;
      maxY_p2.y = y;
      max_y_idx = i;
    }
    if (minY_p2.y > y ||
	(minY_p2.y == y && x > minY_p2.x)) { //minY avec max x
      minY_p2.x = x;
      minY_p2.y = y;
      min_y_idx = i;
    }

#else

    if (max_x < x) {
      max_x = x;
      max_x_idx = i;
    }
    if (min_x > x) {
      min_x = x;
      min_x_idx = i;
    }

    if (max_y < y) {
      max_y = y;
      max_y_idx = i;
    }
    if (min_y > y) {
      min_y = y;
      min_y_idx = i;
    }

#endif
  }

  Vector3 p_max_x(vertices[3 * max_x_idx + 0],
                  vertices[3 * max_x_idx + 1],
                  vertices[3 * max_x_idx + 2]);
  Vector3 p_max_y(vertices[3 * max_y_idx + 0],
                  vertices[3 * max_y_idx + 1],
                  vertices[3 * max_y_idx + 2]);
  Vector3 p_min_x(vertices[3 * min_x_idx + 0],
                  vertices[3 * min_x_idx + 1],
                  vertices[3 * min_x_idx + 2]);
  Vector3 p_min_y(vertices[3 * min_y_idx + 0],
                  vertices[3 * min_y_idx + 1],
                  vertices[3 * min_y_idx + 2]);

  Vector3 p_avg2((p_max_x.x + p_max_y.x) / 2,
                 (p_max_x.y + p_max_y.y) / 2,
                 (p_max_x.z + p_max_y.z) / 2);
  Vector3 p_avg1((p_min_x.x + p_min_y.x) / 2,
                 (p_min_x.y + p_min_y.y) / 2,
                 (p_min_x.z + p_min_y.z) / 2);

  Vector3 p_avg3((p_avg2.x + p_avg1.x) / 2,
                 (p_avg2.y + p_avg1.y) / 2,
                 (p_avg2.z + p_avg1.z) / 2);

  Vector2 p12(p_avg2.x - p_avg1.x, p_avg2.y - p_avg1.y);

#if 1
  std::cerr << "min_x_idx=" << min_x_idx << "\n";
  std::cerr << "min_y_idx=" << min_y_idx << "\n";
  std::cerr << "max_x_idx=" << max_x_idx << "\n";
  std::cerr << "max_y_idx=" << max_y_idx << "\n";

  std::cerr << "p_max_x: [" << p_max_x.x << ", " << p_max_x.y << ", "
            << p_max_x.z << "]\n";
  std::cerr << "p_max_y: [" << p_max_y.x << ", " << p_max_y.y << ", "
            << p_max_y.z << "]\n";
  std::cerr << "p_min_x: [" << p_min_x.x << ", " << p_min_x.y << ", "
            << p_min_x.z << "]\n";
  std::cerr << "p_min_y: [" << p_min_y.x << ", " << p_min_y.y << ", "
            << p_min_y.z << "]\n";

  std::cerr << "AABB width=" << p_max_x.x - p_min_x.x << "\n";
  std::cerr << "AABB height=" << p_max_y.y - p_min_y.y << "\n";

  std::cerr << "p_avg1: [" << p_avg1.x << ", " << p_avg1.y << ", " << p_avg1.z
            << "]\n";
  std::cerr << "p_avg2: [" << p_avg2.x << ", " << p_avg2.y << ", " << p_avg2.z
            << "]\n";
  std::cerr << "p_avg3 [" << p_avg3.x << ", " << p_avg3.y << ", " << p_avg3.z
            << "]\n";
  std::cerr << "p12: [" << p12.x << ", " << p12.y << "]\n";

  std::cerr << "atan2(p12.y, p12.x)=" << atan2(p12.y, p12.x) << "\n";
  if (p12.x == p12.y)
    std::cerr << "*** NO ROTATION ***\n";

#endif

  //if (p12.x != p12.y)
  {
    const float angle = static_cast<float>(M_PI / 2 - atan2(p12.y, p12.x));
    //const float angle = 0; //DEBUG

#ifdef DEBUG_TEXCOORDS
    std::cerr << "rotateAroundMean() angle=" << angle
              << "rads=" << angle * 180 / M_PI << "degrees\n";
//printf("### angle hex=%x\n", *(unsigned int *)&angle);
#endif //DEBUG_TEXCOORDS

    const float scaleFactor = 1.f;

#ifdef DEBUG_TEXCOORDS
    std::cerr << "scale factor = " << scaleFactor << " ******************\n";
#endif //DEBUG_TEXCOORDS

    Vector3 p_avg3_s(
      p_avg3.x * scaleFactor, p_avg3.y * scaleFactor, p_avg3.z * scaleFactor);

    const float cos_angle = cos(angle);
    const float sin_angle = sin(angle);
    //Boris: TODO:OPTIM:PARALLEL?
    for (uint32_t i = 0; i < numvertices; ++i) { //vertices indices start from 1
      vertices[3 * i + 0] = vertices[3 * i + 0] - p_avg3.x;
      vertices[3 * i + 1] = vertices[3 * i + 1] - p_avg3.y;
      vertices[3 * i + 2] = vertices[3 * i + 2] - p_avg3.z;

      float x =
        vertices[3 * i + 0] * cos_angle - vertices[3 * i + 1] * sin_angle;
      float y =
        vertices[3 * i + 0] * sin_angle + vertices[3 * i + 1] * cos_angle;
      vertices[3 * i + 0] = x;
      vertices[3 * i + 1] = y;
      //

      {
        //scale
        vertices[3 * i + 0] *= scaleFactor;
        vertices[3 * i + 1] *= scaleFactor;
        vertices[3 * i + 2] *= scaleFactor;
      }

      vertices[3 * i + 0] = vertices[3 * i + 0] + p_avg3_s.x;
      vertices[3 * i + 1] = vertices[3 * i + 1] + p_avg3_s.y;
      vertices[3 * i + 2] = vertices[3 * i + 2] + p_avg3_s.z;
    }
  }

#ifdef DEBUG_TEXCOORDS
  { //DEBUG
    float max_x = -std::numeric_limits<float>::max();
    uint32_t max_y_idx = std::numeric_limits<uint32_t>::min();
    float max_y = -std::numeric_limits<float>::max();
    uint32_t min_x_idx = std::numeric_limits<uint32_t>::max();
    float min_x = std::numeric_limits<float>::max();
    uint32_t min_y_idx = std::numeric_limits<uint32_t>::max();
    float min_y = std::numeric_limits<float>::max();
    for (uint32_t i = 0; i < numvertices; ++i) { //vertices indices start from 1
      const float x = vertices[3 * i + 0];
      const float y = vertices[3 * i + 1];
      if (max_x < x) {
        max_x = x;
        max_x_idx = i;
      }

      if (min_x > x) {
        min_x = x;
        min_x_idx = i;
      }

      if (max_y < y) {
        max_y = y;
        max_y_idx = i;
      }
      if (min_y > y) {
        min_y = y;
        min_y_idx = i;
      }
    }
    std::cerr << "*** After roration:\n";
    std::cerr << "min_x=" << min_x << " min_y=" << min_y << " max_x=" << max_x
              << " max_y=" << max_y << "\n";
    std::cerr << "AABB width=" << max_x - min_x << "\n";
    std::cerr << "AABB height=" << max_y - min_y << "\n";
  }
#endif //DEBUG_TEXCOORDS
}

#ifdef DEBUG_TEXCOORDS

//DEBUG
float G_yC = 162.223038;
float G_y_eps = 0.04;
float G_y0 = G_yC - G_y_eps;
float G_y1 = G_yC + G_y_eps;

float G_xC = -350.667236;
float G_x_eps = 4;
float G_x0 = G_xC - G_x_eps;
float G_x1 = G_xC + G_x_eps;

//DEBUG
struct DEBUG_VertexIndexSorter
{
  const Mesh &m_mesh;

  DEBUG_VertexIndexSorter(const Mesh &mesh)
    : m_mesh(mesh)
  {}

  bool operator()(uint32_t i, uint32_t j) const
  {
    if (i >= m_mesh.numVertices) {
      std::cerr << "ERROR: DEBUG_VertexIndexSorter i=" << i
                << " > m_mesh.numVertices=" << m_mesh.numVertices << "\n";
      exit(10);
    }
    if (j >= m_mesh.numVertices) {
      std::cerr << "ERROR: DEBUG_VertexIndexSorter j=" << j
                << " > m_mesh.numVertices=" << m_mesh.numVertices << "\n";
      exit(10);
    }

    assert(i < m_mesh.numVertices);
    assert(j < m_mesh.numVertices);

    if (m_mesh.vertices[3 * i + 1] < m_mesh.vertices[3 * j + 1]) //y
      return true;
    else if (m_mesh.vertices[3 * i + 1] == m_mesh.vertices[3 * j + 1] &&
             m_mesh.vertices[3 * i + 0] < m_mesh.vertices[3 * j + 0]) //y & x
      return true;
    else if (m_mesh.vertices[3 * i + 1] == m_mesh.vertices[3 * j + 1] &&
             m_mesh.vertices[3 * i + 0] == m_mesh.vertices[3 * j + 0] &&
             m_mesh.vertices[3 * i + 2] < m_mesh.vertices[3 * j + 2])
      return true;

    return false;
  }
};

#endif //DEBUG_TEXCOORDS

//B:TODO: remplacer Idx par VIdx ???? pour spécifier que c'est un index de sommet !!!

namespace {
struct Y_Idx
{
  float y;
  uint32_t idx;

  Y_Idx(float y_ = 0.f, uint32_t idx_ = 0)
    : y(y_)
    , idx(idx_)
  {}
};

struct Y_Idx_SorterY
{
  bool operator()(Y_Idx a, Y_Idx b) const { return a.y < b.y; }
};

struct Y_Idx_CompareY
{
  bool operator()(Y_Idx a, Y_Idx b) const { return a.y == b.y; }
};
} //end anonymous namespace

void
getIntersectionPlanes(const Mesh &mesh, std::vector<Y_Idx> &ys)
{
  if (!mesh.isValid()) {
    ys.clear();
    return;
  }

  const uint32_t numvertices = mesh.numVertices;
  assert(numvertices != 0);
  const float *vertices = mesh.vertices;

  ys.reserve(numvertices);

  for (uint32_t i = 0; i < numvertices; ++i) {
    ys.push_back(Y_Idx(vertices[3 * i + 1], i));
  }

  std::sort(ys.begin(), ys.end(), Y_Idx_SorterY());
  //B:TODO:OPTIM: no need to store indexes !!!! We could only store y ?!

  ys.erase(std::unique(ys.begin(), ys.end(), Y_Idx_CompareY()), ys.end());
}

struct X_Z
{
  float x;
  float z;
};

struct X_Z_SorterX_Z
{
  inline bool operator()(X_Z a, X_Z b) const
  {
    return a.x < b.x || (a.x == b.x && a.z < b.z);
  }
};

#if 1

struct L3
{
  L3(const float *v1, const float *v2)
  {
    assert(v1[1] != v2[1]);

    n[0] = v2[0] - v1[0];
    n[1] = 1.f / (v2[1] - v1[1]); //store 1/n.y
    n[2] = v2[2] - v1[2];
    M0[0] = v1[0];
    M0[1] = v1[1];
    M0[2] = v1[2];
  }

  X_Z getIntersectionPoint(float y) const
  {
    X_Z xz;
    const float t = (y - M0[1]) * n[1];
    assert(0.f < t && t < 1.f);
    xz.x = M0[0] + n[0] * t;
    xz.z = M0[2] + n[2] * t;
    return xz;
  }

  float n[3];
  float M0[3];
};

#else

//This version is more compact (stores 4 floats instead of 6)
// but is seems that results are less precise.

//For
//v0 = (0.366518617, 0.370499969, 0.0284997057)
//v1 = (0.363615572, 0.365600705, 0.0286619943)
//y = 0.366518319
// We need to use doubles instead of floats to get the same result than first
// version

struct L3
{
  L3(const float *v1, const float *v2)
  {
    assert(v1[1] != v2[1]);

    float n0 = v2[0] - v1[0];
    float inv_n1 = 1.f / (v2[1] - v1[1]);
    float n2 = v2[2] - v1[2];

    b0 = v1[0] - n0 * inv_n1 * v1[1];
    a0 = n0 * inv_n1;

    b2 = v1[2] - n2 * inv_n1 * v1[1];
    a2 = n2 * inv_n1;
  }

  X_Z getIntersectionPoint(float y) const
  {
    X_Z xz;
    xz.x = a0 * y + b0;
    xz.z = a2 * y + b2;
    return xz;
  }

  float a0, b0;
  float a2, b2;
};

#endif //0

struct IndSorter
{
  const std::vector<uint32_t> &m_inds;

  explicit IndSorter(const std::vector<uint32_t> &inds)
    : m_inds(inds)
  {}

  inline bool operator()(uint32_t a, uint32_t b) const
  {
    return m_inds[a] < m_inds[b];
  }
};

struct IndiceSorterE
{
  const std::vector<uint32_t> &m_bins;

  explicit IndiceSorterE(const std::vector<uint32_t> &bins)
    : m_bins(bins)
  {}

  inline bool operator()(uint32_t a, uint32_t b) const
  {
    return m_bins[a] < m_bins[b];
  }
};

class SpacePartionnerYEdges
{
public:
  SpacePartionnerYEdges()
    : m_numBins(0)
    , m_step(0)
  {}

  SpacePartionnerYEdges(size_t numYs,
                        const std::vector<uint32_t> &vidx2yPlaneIdx,
                        const std::vector<Edge> &edges)
    : m_numBins(0)
    , m_step(0)
  {
    init(numYs, vidx2yPlaneIdx, edges);
  }

  void init(size_t numYs,
            const std::vector<uint32_t> &vidx2yPlaneIdx,
            const std::vector<Edge> &edges)
  {
    const size_t numEdges = edges.size();
    const size_t numVertices = vidx2yPlaneIdx.size();

    const int numEdgesPerBin = 1024; //approximate/wished for

    m_numBins = std::max((size_t)1, numEdges / numEdgesPerBin);
    m_step = numYs / (float)m_numBins;

    const float reserve_size_factor = 2.65f; //arbitrary

#ifndef NDEBUG
    std::cerr << "SpacePartionnerYEdges: m_numBins=" << m_numBins
              << " m_step=" << m_step << "\n";
#endif

    /*
    {//DEBUG
      std::cerr<<"m_numBins="<<m_numBins<<" m_step="<<m_step<<"\n";
      //for (uint32_t i=0; i<m_numBins; ++i) {
      //std::cerr<<"bin "<<i<<" start="<<m_yMin + i*m_step<<" end="<<m_yMin + (i+1)*m_step<<"\n";
      //}

    }//DEBUG
    */

    size_t DBG_NB_ON_SEVERAL_BINS = 0;

    const uint32_t INVALID_BIN = m_numBins + 1;

    //first, get bin of all vertices
    std::vector<uint32_t> verticesBins(
      numVertices, INVALID_BIN); //TODO:OPTIM:useless initialization
    for (size_t i = 0; i < numVertices; ++i) {
      const uint32_t y = vidx2yPlaneIdx[i]; //(not really y, but y index)
      const uint32_t bin = getBin(y);
      verticesBins[i] = bin;
    }

    //second, get intervals of bins for all edges
    std::vector<uint32_t> edgesBins;
    const size_t reserve_size =
      static_cast<size_t>(reserve_size_factor * numEdges); //arbitrary size
    //edges may be in several bins, thus reserve_size must be superior to numTriangles
    edgesBins.reserve(reserve_size);
    std::vector<uint32_t> edgesIdx;
    edgesIdx.reserve(reserve_size);

    for (size_t i = 0; i < numEdges; ++i) {

      const uint32_t v_id_0 = edges[i].vertexIndex[0];
      const uint32_t v_id_1 = edges[i].vertexIndex[1];

      const uint32_t bin_0 = verticesBins[v_id_0];
      const uint32_t bin_1 = verticesBins[v_id_1];

      uint32_t bin_min = bin_0;
      uint32_t bin_max = bin_1;
      if (bin_0 > bin_1) {
        bin_min = bin_1;
        bin_max = bin_0;
      }

      for (uint32_t b = bin_min; b <= bin_max; ++b) {
        edgesBins.push_back(b);
        edgesIdx.push_back(i);
      }

      { //DEBUG
        if (bin_min != bin_max)
          ++DBG_NB_ON_SEVERAL_BINS;

        //if (bin_max >= m_numBins-1)
        //std::cerr<<"triangle "<<i<<" bins vert="<<bin_0<<", "<< bin_1<<", "<<bin_2<<" -> ["<<bin_min<<", "<<bin_max<<"]  !!!\n";
      } //DEBUG

      /*
      std::cerr<<"edge "<<i<<" bins vert="<<bin_0<<", "<< bin_1<<" -> ["<<bin_min<<", "<<bin_max<<"]";
      if (bin_min != bin_max)
        std::cerr<<" *** ";
      std::cerr<<"\n";
      */
    }
    assert(edgesBins.size() == edgesIdx.size());

    //std::cerr<<"edgesBins.size="<<edgesBins.size()<<" edgesBins.capacity="<<edgesBins.capacity()<<" numEdges="<<numEdges<<"\n";
    /*
    {//DEBUG
      std::cerr<<"For "<<edgesBins.size()<<" bins:\n";
      std::cerr<<"edgesBins:\n";
      for (uint32_t i=0; i<edgesBins.size(); ++i)
        std::cerr<<edgesBins[i]<<" ";
      std::cerr<<"\n";
      std::cerr<<"edgesIdx:\n";
      for (uint32_t i=0; i<edgesIdx.size(); ++i)
        std::cerr<<edgesIdx[i]<<" ";
      std::cerr<<"\n";
    }//DEBUG
    */

    //we sort indices according to edgesBins, to be able to have edgesIdx sorted according to edgesBins
    const size_t sz = edgesBins.size();
    std::vector<uint32_t> indices(sz);
    for (uint32_t i = 0; i < sz; ++i) {
      indices[i] = i;
    }
    std::sort(indices.begin(), indices.end(), IndiceSorterE(edgesBins));

    /*
    {//DEBUG
      std::cerr<<"After sort:\n";
      std::cerr<<"trianglesBins:\n";
      for (uint32_t i=0; i<trianglesBins.size(); ++i)
        std::cerr<<trianglesBins[indices[i]]<<" ";
      std::cerr<<"\n";
      std::cerr<<"trianglesIdx:\n";
      for (uint32_t i=0; i<trianglesIdx.size(); ++i)
        std::cerr<<trianglesIdx[indices[i]]<<" ";
      std::cerr<<"\n";
    }//DEBUG
    */

    m_indices.resize(sz);
    for (uint32_t i = 0; i < sz; ++i) {
      m_indices[i] = edgesIdx[indices[i]];
    }

    m_binStarts.resize(m_numBins + 1);
    m_binStarts[0] = 0;
    uint32_t currBin = 0;
    uint32_t binIdx = 1;
    for (uint32_t i = 1; i < sz; ++i) { //start from 1
      uint32_t bin = edgesBins[indices[i]];
      while (currBin < bin) {
        //std::cerr<<"m_binStarts["<<binIdx<<"]="<<i<<"\n";
        m_binStarts[binIdx] = i;
        ++currBin;
        ++binIdx;
      }
    }
    for (; binIdx <= m_numBins; ++binIdx) {
      m_binStarts[binIdx] = sz;
      std::cerr << "m_binStarts[" << binIdx << "]=" << sz << " !\n";
    }
    //m_binStarts[m_numBins] = sz;

    /*
    {//DEBUG
      std::cerr<<"m_binStarts.size()="<<m_binStarts.size()<<" m_numBins="<<m_numBins<<"\n";

      //for (size_t i=0; i<m_binStarts.size()-1; ++i) {
      for (size_t i=m_binStarts.size()-3; i<m_binStarts.size()-1; ++i) {
        std::cerr<<"bin "<<i<<" : start="<<m_binStarts[i]<<" end="<<m_binStarts[i+1]<<"\n";
        for (size_t j=m_binStarts[i]; j<m_binStarts[i+1]; ++j)
          std::cerr<<" "<<m_indices[j];
        std::cerr<<"\n";
     }
      std::cerr<<"\n";

    }//DEBUG
    */

    { //DEBUG

      std::cerr << "DBG_NB_ON_SEVERAL_BINS=" << DBG_NB_ON_SEVERAL_BINS << " / "
                << numEdges << " edges = "
                << DBG_NB_ON_SEVERAL_BINS * 100.0 / (double)numEdges << "%\n";
      std::cerr << "edgesBins.size=" << edgesBins.size()
                << " edgesBins.capacity=" << edgesBins.capacity()
                << " numEdges=" << numEdges << "\n";
      std::cerr << "edgesBins.size/numEdges="
                << edgesBins.size() / (double)numEdges << "\n";
      std::cerr << "reserve_size=" << reserve_size << "\n";
      if (reserve_size < edgesBins.size())
        std::cerr << "TODO:OPTIM: increase reserve_size to at least "
                  << edgesBins.size() << "\n";

      size_t numMinBin = 0;
      size_t minNb = std::numeric_limits<size_t>::max();
      size_t maxNb = std::numeric_limits<size_t>::min();
      size_t m0 = 0, m1 = 0, m2 = 0;

      for (size_t i = 0; i < m_binStarts.size() - 1; ++i) {
        const size_t s = m_binStarts[i + 1] - m_binStarts[i];
        ++m0;
        m1 += s;
        m2 += s * s;
        if (s < minNb) {
          minNb = s;
          numMinBin = 1;
        } else if (s == minNb) {
          ++numMinBin;
        }
        if (s > maxNb)
          maxNb = s;
      }
      std::cerr << "For " << m_numBins << " bins:\n";
      std::cerr << "minNb=" << minNb << " [for " << numMinBin
                << " bins] ; maxNb=" << maxNb << "\n";
      const double inv_m0 = 1. / m0;
      std::cerr << "mean=" << m1 * inv_m0
                << " stdDev=" << sqrt((m2 - m1 * m1 * inv_m0) * inv_m0) << "\n";
      std::cerr << "\n";

    } //DEBUG

    assert(m_binStarts.size() == m_numBins + 1);
  }

  void getEdgeIndicesForY(uint32_t yInd,
                          const uint32_t *&indices,
                          uint32_t &numIndices) const
  {
    const uint32_t bin = getBin(yInd);
    assert(bin < m_numBins && bin + 1 < m_binStarts.size());
    const size_t binStart = m_binStarts[bin];
    const size_t binEnd = m_binStarts[bin + 1];
    assert(binStart <= binEnd);
    numIndices = binEnd - binStart;
    indices = &(m_indices[binStart]);
  }

protected:
  inline uint32_t getBin(uint32_t y) const
  {
    //return (y>m_yMin ? static_cast<size_t>( (y-m_yMin)/m_step ) : 0);
    //const uint32_t bin = (y<m_yMax ? (y>m_yMin ? static_cast<size_t>( (y-m_yMin)/m_step ) : 0) : m_numBins-1);

    const uint32_t bin = static_cast<uint32_t>(y / m_step);
    assert(bin < m_numBins);

    return bin;
  }

private:
  uint32_t m_numBins;
  float m_step;

  std::vector<uint32_t> m_indices;
  std::vector<size_t> m_binStarts;
  //we store a vector of vector as two vectors.
};

#ifdef TIME_TEXCOORDS
#include <sys/time.h>
static double
P_getTime(struct timeval t0, struct timeval t1)
{
  return (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_usec - t0.tv_usec) / 1000.0;
}
#endif //TIME_TEXCOORDS

static inline float
computeDist(float x0, float z0, float x1, float z1)
{
  const float dx = x1 - x0;
  const float dz = z1 - z0;
  const float dist = sqrt(dx * dx + dz * dz);
  return dist;
}

/*
  @a verticesIndices and @a startYIndices are an array of array.

  We know that triangles edges have their extremities on two Y planes
  (because y planes are defined to cross each point of triangles).
  Thus we have to compute intersections only on the interior of edges.


  We cannot compute the intersections points for all edges at once,
  because it takes too much memory.


 */
void
computeTexCoords1(Mesh &mesh)
{
  if (!mesh.isValid())
    return;

#ifdef TIME_TEXCOORDS
  struct timeval t0, t1;
#endif //TIME_TEXCOORDS

  //rotateAroundMean(mesh); // the spine // Oy

  float original_min_x = moveTo_X_min(mesh); // translate to X_min
  //moveTo_XZ_min(mesh);// translate to X_min & Z_min

  float MINV[3], MAXV[3];
  mesh.getAABB(MINV, MAXV);
#ifndef NDEBUG
  const float X_MIN = MINV[0];
#endif //NDEBUG

#ifdef TIME_TEXCOORDS
  gettimeofday(&t0, nullptr);
#endif //TIME_TEXCOORDS

  mesh.allocateTexCoords();

#ifdef TIME_TEXCOORDS
  gettimeofday(&t1, nullptr);
  std::cerr << "time allocateTexCoords: " << P_getTime(t0, t1) << "ms for "
            << mesh.numVertices << " vertices\n";
#endif //TIME_TEXCOORDS

  std::vector<uint32_t> verticesIndices;
  std::vector<uint32_t> startYIndices;

#ifdef TIME_TEXCOORDS
  gettimeofday(&t0, nullptr);
#endif //TIME_TEXCOORDS

  getIntersectionPlanesVertices(mesh, verticesIndices, startYIndices);

#ifdef TIME_TEXCOORDS
  gettimeofday(&t1, nullptr);
  std::cerr << "time getIntersectionPlanesVertices: " << P_getTime(t0, t1)
            << "ms for " << mesh.numVertices << " vertices\n";
#endif //TIME_TEXCOORDS

  const uint32_t numvertices = mesh.numVertices;

  //- build array to have for a given vidx its corresponding Y plane start index.
  std::vector<uint32_t> vidx2yPlaneIdx(numvertices); //TODO:OPTIM: hidden memset
  assert(startYIndices.size() > 1);
  const size_t numStartYIndices = startYIndices.size() - 1; //== numYs
  for (size_t i = 0; i < numStartYIndices; ++i) {
    const uint32_t start = startYIndices[i];
    const uint32_t end = startYIndices[i + 1];
    for (uint32_t j = start; j < end; ++j) {
      const uint32_t vidx = verticesIndices[j];
      vidx2yPlaneIdx[vidx] =
        i; //we put index in startYIndices, to be able to have next index just doing +1.
    }
  }

    /*
        Suppose we have :
                         0  1  2  3  4  5  6  7  8  9
        vertexIndices :  2  7  5  8  9  6  4  3  0  1  ...
        startYIndices :  0  3  5  6  8 10
        (it means that vertices 2,7,5 have same y0 ; 8,9 have same y1 ; 6 have y2 ; 4,3 have y3 ; 0,1 have y4, ...)

        vidx2yPlaneIdx:  4  4  0  3  3  0  2  0  1  1

        If we have an edge 0-5 :
           => vidx2yPlaneIdx[0]=4  vidx2yPlaneIdx[5]=0
            => this edge will cross planes : vidx2yPlaneIdx[5]+1=1 to vidx2yPlaneIdx[0]-1=3 included.
                    that is the Ys of vertexIndices[startYIndices[1]] to vertexIndices[startYIndices[3]]
                            to startYIndices[vidx2yPlaneIdx[0]-1]=startYIndices[4-1]=0


       */

#ifdef TIME_TEXCOORDS
  gettimeofday(&t0, nullptr);
#endif //TIME_TEXCOORDS

  std::vector<Edge> edges;
  edges = getEdges(mesh);

  edges.shrink_to_fit();

#ifdef TIME_TEXCOORDS
  gettimeofday(&t1, nullptr);
  std::cerr << "time getEdges: " << P_getTime(t0, t1) << "ms\n";
#endif //TIME_TEXCOORDS

  std::cerr << "edges.size()=" << edges.size()
            << " [capacity=" << edges.capacity() << "]\n";
  std::cerr << "vidx2yPlaneIdx.size()=" << vidx2yPlaneIdx.size()
            << " [capacity=" << vidx2yPlaneIdx.capacity() << "]\n";

  const size_t numYs = numStartYIndices;

  std::cerr << "numYs=" << numYs << "\n";

#ifdef TIME_TEXCOORDS
  gettimeofday(&t0, nullptr);
#endif //TIME_TEXCOORDS

  SpacePartionnerYEdges spye(numYs, vidx2yPlaneIdx, edges);

#ifdef TIME_TEXCOORDS
  gettimeofday(&t1, nullptr);
  std::cerr << "time SpacePartionnerYEdges: " << P_getTime(t0, t1) << "ms\n";
#endif //TIME_TEXCOORDS

  std::vector<X_Z> intersectionPoints;

  const size_t reserve_size = 3000; //arbitrary
  intersectionPoints.reserve(reserve_size);

#ifdef TIME_TEXCOORDS
  struct timeval tt0, tt1;
  size_t DBG_MAX_NUM_EDGES = 0;
  size_t DBG_MAX_INTERSECTION_POINTS = 0;
  size_t DBG_MAX_INTERSPTS_CAPA = 0;
  double timeGetEdgesForY = 0;
  double timeIntersectPts = 0;
  double timeSort = 0;
  double timeMerge = 0;
  size_t DBG_NUM_EDGES = 0;
  size_t DBG_NUM_EDGES_USED = 0;
  size_t DBG_NUM_EDGES_USED2 = 0;

  gettimeofday(&t0, nullptr);
#endif //TIME_TEXCOORDS

  for (size_t yi = 0; yi < numYs; ++yi) {

    const uint32_t start = startYIndices[yi];
    const uint32_t indv = verticesIndices[start];
    const float y = mesh.vertices[3 * indv + 1];

    //We get the X coord of the last original vertex on this y line.
    //We do not have to compute intersection points with edges
    // with two extremities beyond this X coord.
    assert(yi < startYIndices.size());
    const uint32_t end = startYIndices[yi + 1];
    assert(start < end);
    const uint32_t indLast = verticesIndices[end - 1];
    const float lastX = mesh.vertices[3 * indLast + 0];
    assert(mesh.vertices[3 * indLast + 1] == y);

#ifdef TIME_TEXCOORDS
    gettimeofday(&tt0, nullptr);
#endif //TIME_TEXCOORDS

    uint32_t numEdgesIndices = 0;
    const uint32_t *edgesIndices = nullptr;
    spye.getEdgeIndicesForY(yi, edgesIndices, numEdgesIndices);

#ifdef TIME_TEXCOORDS
    gettimeofday(&tt1, nullptr);
    timeGetEdgesForY += P_getTime(tt0, tt1);
    if (numEdgesIndices > DBG_MAX_NUM_EDGES)
      DBG_MAX_NUM_EDGES = numEdgesIndices;
    DBG_NUM_EDGES += numEdgesIndices;
#endif //TIME_TEXCOORDS

    intersectionPoints.clear();
    intersectionPoints.reserve(numEdgesIndices / 2);

#ifdef TIME_TEXCOORDS
    if (intersectionPoints.capacity() > DBG_MAX_INTERSPTS_CAPA)
      DBG_MAX_INTERSPTS_CAPA = intersectionPoints.capacity();
    gettimeofday(&tt0, nullptr);
#endif //TIME_TEXCOORDS

    for (uint32_t i = 0; i < numEdgesIndices; ++i) {

      const uint32_t ind = edgesIndices[i];
      const uint32_t e0 = edges[ind].vertexIndex[0];
      const uint32_t e1 = edges[ind].vertexIndex[1];

      const float *v0 = &mesh.vertices[3 * e0];
      const float *v1 = &mesh.vertices[3 * e1];

      const float x0 = v0[0];
      const float x1 = v1[0];

      if (x0 < lastX || x1 < lastX) {
      //At least one of the edge extremity is inferior to last X coord
      //thus we could have an intersection before last X.

#ifdef TIME_TEXCOORDS
        DBG_NUM_EDGES_USED += 1;
#endif //TIME_TEXCOORDS

        uint32_t yInd_i0 = vidx2yPlaneIdx[e0];
        uint32_t yInd_i1 = vidx2yPlaneIdx[e1];
        if (yInd_i0 > yInd_i1)
          std::swap(yInd_i0, yInd_i1);
        assert(yInd_i0 <= yInd_i1);
        //all planes in ]yInd0; yInd1[ intersect this edge

        if (yInd_i0 < yi && yi < yInd_i1) {

#ifdef TIME_TEXCOORDS
          DBG_NUM_EDGES_USED2 += 1;
#endif //TIME_TEXCOORDS

          L3 line(v0, v1);

          X_Z intersectionPt = line.getIntersectionPoint(y);

          intersectionPoints.push_back(intersectionPt);
        }
      }
    }
#ifdef TIME_TEXCOORDS
    gettimeofday(&tt1, nullptr);
    timeIntersectPts += P_getTime(tt0, tt1);

    if (intersectionPoints.size() > DBG_MAX_INTERSECTION_POINTS)
      DBG_MAX_INTERSECTION_POINTS = intersectionPoints.size();
    gettimeofday(&tt0, nullptr);
#endif //TIME_TEXCOORDS

    //sort intersection point according to X then Z
    std::sort(
      intersectionPoints.begin(), intersectionPoints.end(), X_Z_SorterX_Z());

#ifdef TIME_TEXCOORDS
    gettimeofday(&tt1, nullptr);
    timeSort += P_getTime(tt0, tt1);
    gettimeofday(&tt0, nullptr);
#endif //TIME_TEXCOORDS

    //For a given Y plane
    //we have two sorted lists of points (sorted according to X then Z) :
    //- original vertices from mesh on this Y plane
    //- intersections points between edges and Y plane
    //We merge them and compute texcoords on the fly.
    //REM: Once we have processed all the original vertices,
    //we do not need to process the remaining intersection points.

    uint32_t iV = startYIndices[yi];
    const uint32_t endV = startYIndices[yi + 1];
    uint32_t iI = 0;
    const uint32_t endI = intersectionPoints.size();

    assert(iV < endV);

    const float xV0 = mesh.vertices[3 * verticesIndices[iV] + 0];
    const float zV0 = mesh.vertices[3 * verticesIndices[iV] + 2];
    const float xI0 = (iI != endI ? intersectionPoints[iI].x : xV0 + 1);
    const float zI0 = (iI != endI ? intersectionPoints[iI].z : zV0 + 1);

    float prev_x = 0; //always start from 0 ???
    float prev_z = (xV0 < xI0 ? zV0 : zI0);

    float L = 0;

    //B: ??? Is zeroPoint necessary ???
    //B: that is do we have to init prev_x to 0 or to the first x on the line ???

    //B: We do nothing special for the vertices/intersectionPts
    //   with the same Z   !!!????

    while (iV != endV) {
      if (iI == endI) {
        for (; iV != endV; ++iV) {
          const uint32_t indV = verticesIndices[iV];
          const float xV = mesh.vertices[3 * indV + 0];
          const float zV = mesh.vertices[3 * indV + 2];
          L += computeDist(prev_x, prev_z, xV, zV);
          mesh.texCoords[2 * indV + 0] = L;
          mesh.texCoords[2 * indV + 1] = y;

#ifndef NDEBUG
          if (xV < X_MIN)
            std::cerr
              << "xV=" << xV << " < " << X_MIN << "   zV=" << zV << " L=" << L
              << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
#endif //NDEBUG

          prev_x = xV;
          prev_z = zV;
        }
        break;
      }

      const uint32_t indV = verticesIndices[iV];
      const float xV = mesh.vertices[3 * indV + 0];
      const float zV = mesh.vertices[3 * indV + 2];
      const float xI = intersectionPoints[iI].x;
      const float zI = intersectionPoints[iI].z;

      if (xI < xV || (xI == xV && zI < zV)) {
        L += computeDist(prev_x, prev_z, xI, zI);

#ifndef NDEBUG
        if (xI < X_MIN)
          std::cerr
            << "xI=" << xI << " < " << X_MIN << "   zI=" << zI << " L=" << L
            << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
#endif //NDEBUG

        prev_x = xI;
        prev_z = zI;
        ++iI;
      } else {
        L += computeDist(prev_x, prev_z, xV, zV);
        mesh.texCoords[2 * indV + 0] = L;
        mesh.texCoords[2 * indV + 1] = y;

#ifndef NDEBUG
        if (xV < X_MIN)
          std::cerr
            << "xV=" << xV << " < " << X_MIN << "   zV=" << zV << " L=" << L
            << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
#endif //NDEBUG

        prev_x = xV;
        prev_z = zV;
        ++iV;
      }

      //?
    }
      //No need to process potential remaining intersection points here.

#ifdef TIME_TEXCOORDS
    gettimeofday(&tt1, nullptr);
    timeMerge += P_getTime(tt0, tt1);
#endif //TIME_TEXCOORDS
  }

#ifdef TIME_TEXCOORDS
  gettimeofday(&t1, nullptr);
  std::cerr << "time traverse planes: " << P_getTime(t0, t1) << "ms for "
            << numYs << " planes\n";
  std::cerr << "DBG_MAX_NUM_EDGES=" << DBG_MAX_NUM_EDGES << "\n";
  std::cerr << "DBG_MAX_INTERSPTS_CAPA=" << DBG_MAX_INTERSPTS_CAPA << "\n";
  std::cerr << "DBG_MAX_INTERSECTION_POINTS=" << DBG_MAX_INTERSECTION_POINTS
            << "\n"; //use it for reserve !
  std::cerr << "timeGetEdgesForY=" << timeGetEdgesForY << "ms\n";
  std::cerr << "timeIntersectPts=" << timeIntersectPts << "ms\n";
  std::cerr << "timeSort=" << timeSort << "ms\n";
  std::cerr << "timeMerge=" << timeMerge << "ms\n";
  std::cerr << "DBG_NUM_EDGES=" << DBG_NUM_EDGES
            << " mean=" << DBG_NUM_EDGES / (float)numYs << "\n";
  std::cerr << "DBG_NUM_EDGES_USED=" << DBG_NUM_EDGES_USED
            << " mean=" << DBG_NUM_EDGES_USED / (float)numYs << "\n";
  std::cerr << "DBG_NUM_EDGES_USED2=" << DBG_NUM_EDGES_USED2
            << " mean=" << DBG_NUM_EDGES_USED2 / (float)numYs << "\n";

  gettimeofday(&t0, nullptr);
#endif //TIME_TEXCOORDS

  normalizeTexCoords(mesh);

#ifdef CHECK_TEXCOORDS
  checkTexCoords(mesh);
//debugTexCoords(mesh);
#endif

  //B: Useless ? Or do not do here ???

  if (!mesh.hasNormals())
    mesh.computeNormals();

  moveTo_X_min(mesh, -original_min_x);

  mesh.unitize();

#ifdef TIME_TEXCOORDS
  gettimeofday(&t1, nullptr);
  std::cerr << "time normalize+move+unitary: " << P_getTime(t0, t1) << "ms\n";
#endif //TIME_TEXCOORDS
}

namespace {

struct X_Z_Idx
{
  float x;
  float z;
  uint32_t idx;

  X_Z_Idx(float x_ = 0.f, float z_ = 0.f, uint32_t idx_ = 0)
    : x(x_)
    , z(z_)
    , idx(idx_)
  {}
};

struct X_Z_Idx_SorterX
{
  inline bool operator()(X_Z_Idx a, X_Z_Idx b)
    const //TODO:OPTIM???: pass by const reference ???
  {
    return a.x < b.x;
  }
};

struct X_Z_Idx_SorterIdx
{
  inline bool operator()(X_Z_Idx a, X_Z_Idx b)
    const //TODO:OPTIM???: pass by const reference ???
  {
    return a.idx < b.idx;
  }
};

struct X_Z_Idx_SorterX_Z
{
  inline bool operator()(X_Z_Idx a, X_Z_Idx b)
    const //TODO:OPTIM???: pass by const reference ???
  {
    return a.x < b.x || (a.x == b.x && a.z < b.z);
  }
};

struct X_Z_Idx_CompareIdx
{
  inline bool operator()(X_Z_Idx a, X_Z_Idx b)
    const //TODO:OPTIM???: pass by const reference ???
  {
    return a.idx == b.idx;
  }
};

struct X_Z_Idx_CompareXZ
{
  inline bool operator()(X_Z_Idx a, X_Z_Idx b)
    const //TODO:OPTIM???: pass by const reference ???
  {
    return a.x == b.x && a.z == b.z; //exact comparison !
  }
};

struct X_Z_Idx_CompareX
{
  inline bool operator()(X_Z_Idx a, X_Z_Idx b)
    const //TODO:OPTIM???: pass by const reference ???
  {
    return a.x == b.x; //exact comparison !
  }
};

struct X_Z_Idx_SmallX
{
  float m_maxX;

  explicit X_Z_Idx_SmallX(float maxX)
    : m_maxX(maxX)
  {}

  bool operator()(X_Z_Idx a) const { return a.x <= m_maxX; }
};

} //end anonymous namespace

static const uint32_t INVALID_INDEX = -1;

struct IndiceSorter
{
  const std::vector<uint32_t> &m_trianglesBins;

  explicit IndiceSorter(const std::vector<uint32_t> &trianglesBins)
    : m_trianglesBins(trianglesBins)
  {}

  inline bool operator()(uint32_t a, uint32_t b) const
  {
    return m_trianglesBins[a] < m_trianglesBins[b];
  }
};

class SpacePartionnerY
{
public:
  SpacePartionnerY()
    : m_yMin(0)
    , m_yMax(-1)
    , m_numBins(0)
    , m_step(0)
  {}

  SpacePartionnerY(const Mesh &mesh, float yMin, float yMax)
    : m_yMin(0)
    , m_yMax(-1)
    , m_numBins(0)
    , m_step(0)
  {
    init(mesh, yMin, yMax);
  }

  void init(const Mesh &mesh, float yMin, float yMax)
  {
    assert(yMin <= yMax);
    m_yMin = yMin;
    m_yMax = yMax;
    std::cerr << "REAL m_yMin=" << m_yMin << " m_yMax=" << m_yMax << "\n";

#if 1
    const uint32_t numTriangles = mesh.numTriangles;
    const int numTrianglesPerBin =
      256; //128;   //256;  //512;  //1024; //1024+512;
    const float reserve_size_factor =
      5.64f; //10.28f; //5.64f; //3.32f; //2.16f; //1.78f
#else
    //DEBUG

    const uint32_t numTriangles =
      std::min((uint32_t)20000, mesh.numTriangles); //DEBUG
    const int numTrianglesPerBin = 1024;            //DEBUG

    { //DEBUG

      const uint32_t *triangles = mesh.triangles;
      for (uint32_t i = 0; i < numTriangles; ++i) {
        const uint32_t v_id_0 = triangles[3 * i + 0];
        const uint32_t v_id_1 = triangles[3 * i + 1];
        const uint32_t v_id_2 = triangles[3 * i + 2];

        float y0 = mesh.vertices[3 * v_id_0 + 1];
        float y1 = mesh.vertices[3 * v_id_1 + 1];
        float y2 = mesh.vertices[3 * v_id_2 + 1];

        //std::cerr<<"tri "<<i<<" y v: "<<y0<<", "<<y1<<", "<<y2<<"\n";

        if (i == 0) {
          m_yMin = std::min(y0, std::min(y1, y2));
          m_yMax = std::max(y0, std::max(y1, y2));
        } else {
          m_yMin = std::min(m_yMin, std::min(y0, std::min(y1, y2)));
          m_yMax = std::max(m_yMax, std::max(y0, std::max(y1, y2)));
        }
      }
      std::cerr << "DEBUG m_yMin=" << m_yMin << " m_yMax=" << m_yMax << "\n";
    } //DEBUG

#endif //0

    assert(m_yMin <= m_yMax);

    m_numBins = std::max((uint32_t)1, numTriangles / numTrianglesPerBin);
    m_step = (m_yMax - m_yMin) / (float)m_numBins;

#ifndef NDEBUG
    std::cerr << "SpacePartionnerY: m_numBins=" << m_numBins
              << " m_step=" << m_step << "\n";
#endif

    /*
    {//DEBUG
      std::cerr<<"m_numBins="<<m_numBins<<" m_step="<<m_step<<"\n";

      //for (uint32_t i=0; i<m_numBins; ++i) {
      //std::cerr<<"bin "<<i<<" start="<<m_yMin + i*m_step<<" end="<<m_yMin + (i+1)*m_step<<"\n";
      //}

    }//DEBUG
    */

    size_t DBG_NB_ON_SEVERAL_BINS = 0;

    const uint32_t INVALID_BIN = m_numBins + 1;

    //first, get bin of all vertices
    const float *vertices = mesh.vertices;
    const uint32_t numvertices = mesh.numVertices;
    std::vector<uint32_t> verticesBins(
      numvertices, INVALID_BIN); //TODO:OPTIM:useless initialization
    for (uint32_t i = 0; i < numvertices; ++i) {
      const float y = vertices[3 * i + 1];
      const uint32_t bin = getBin(y);
      verticesBins[i] = bin;
    }

    //second, get intervals of bins for all triangles
    const uint32_t *triangles = mesh.triangles;
    std::vector<uint32_t> trianglesBins;
    const size_t reserve_size =
      static_cast<size_t>(reserve_size_factor * numTriangles); //arbitrary size
    //triangles may be in several bins, thus reserve_size must be superior to numTriangles
    trianglesBins.reserve(reserve_size);
    std::vector<uint32_t> trianglesIdx;
    trianglesIdx.reserve(reserve_size);

    for (uint32_t i = 0; i < numTriangles; ++i) {
      const uint32_t v_id_0 = triangles[3 * i + 0];
      const uint32_t v_id_1 = triangles[3 * i + 1];
      const uint32_t v_id_2 = triangles[3 * i + 2];

      const uint32_t bin_0 = verticesBins[v_id_0];
      const uint32_t bin_1 = verticesBins[v_id_1];
      const uint32_t bin_2 = verticesBins[v_id_2];

      uint32_t bin_min = std::min(bin_0, std::min(bin_1, bin_2));
      uint32_t bin_max = std::max(bin_0, std::max(bin_1, bin_2));

      for (uint32_t b = bin_min; b <= bin_max; ++b) {
        trianglesBins.push_back(b);
        trianglesIdx.push_back(i);
      }

      { //DEBUG
        if (bin_min != bin_max)
          ++DBG_NB_ON_SEVERAL_BINS;

        //if (bin_max >= m_numBins-1)
        //std::cerr<<"triangle "<<i<<" bins vert="<<bin_0<<", "<< bin_1<<", "<<bin_2<<" -> ["<<bin_min<<", "<<bin_max<<"]  !!!\n";

      } //DEBUG

      /*
      std::cerr<<"triangle "<<i<<" bins vert="<<bin_0<<", "<< bin_1<<", "<<bin_2<<" -> ["<<bin_min<<", "<<bin_max<<"]";
      if (bin_min != bin_max)
        std::cerr<<" *** ";
      std::cerr<<"\n";
      */
    }
    assert(trianglesBins.size() == trianglesIdx.size());

    //std::cerr<<"trianglesBins.size="<<trianglesBins.size()<<" trianglesBins.capacity="<<trianglesBins.capacity()<<" numTriangles="<<numTriangles<<"\n";

    /*
    {//DEBUG
      std::cerr<<"For "<<trianglesBins.size()<<" bins:\n";
      std::cerr<<"trianglesBins:\n";
      for (uint32_t i=0; i<trianglesBins.size(); ++i)
        std::cerr<<trianglesBins[i]<<" ";
      std::cerr<<"\n";
      std::cerr<<"trianglesIdx:\n";
      for (uint32_t i=0; i<trianglesIdx.size(); ++i)
        std::cerr<<trianglesIdx[i]<<" ";
      std::cerr<<"\n";
    }//DEBUG
    */

    //we sort indices according to trianglesBins, to be able to have trianglesIdx sorted according to trianglesBins
    const size_t sz = trianglesBins.size();
    std::vector<uint32_t> indices(sz);
    for (uint32_t i = 0; i < sz; ++i) {
      indices[i] = i;
    }
    std::sort(indices.begin(), indices.end(), IndiceSorter(trianglesBins));

    /*
    {//DEBUG
      std::cerr<<"After sort:\n";
      std::cerr<<"trianglesBins:\n";
      for (uint32_t i=0; i<trianglesBins.size(); ++i)
        std::cerr<<trianglesBins[indices[i]]<<" ";
      std::cerr<<"\n";
      std::cerr<<"trianglesIdx:\n";
      for (uint32_t i=0; i<trianglesIdx.size(); ++i)
        std::cerr<<trianglesIdx[indices[i]]<<" ";
      std::cerr<<"\n";
    }//DEBUG
    */

    m_indices.resize(sz);
    for (uint32_t i = 0; i < sz; ++i) {
      m_indices[i] = trianglesIdx[indices[i]];
    }

    m_binStarts.resize(m_numBins + 1);
    m_binStarts[0] = 0;
    uint32_t currBin = 0;
    uint32_t binIdx = 1;
    for (uint32_t i = 1; i < sz; ++i) { //start from 1
      uint32_t bin = trianglesBins[indices[i]];
      while (currBin < bin) {
        //std::cerr<<"m_binStarts["<<binIdx<<"]="<<i<<"\n";
        m_binStarts[binIdx] = i;
        ++currBin;
        ++binIdx;
      }
    }
    for (; binIdx <= m_numBins; ++binIdx) {
      m_binStarts[binIdx] = sz;
      std::cerr << "m_binStarts[" << binIdx << "]=" << sz << " !\n";
    }
    //m_binStarts[m_numBins] = sz;

    /*
    {//DEBUG
      std::cerr<<"m_binStarts.size()="<<m_binStarts.size()<<" m_numBins="<<m_numBins<<"\n";

      //for (size_t i=0; i<m_binStarts.size()-1; ++i) {
      for (size_t i=m_binStarts.size()-3; i<m_binStarts.size()-1; ++i) {
        std::cerr<<"bin "<<i<<" : start="<<m_binStarts[i]<<" end="<<m_binStarts[i+1]<<"\n";
        for (size_t j=m_binStarts[i]; j<m_binStarts[i+1]; ++j)
          std::cerr<<" "<<m_indices[j];
        std::cerr<<"\n";
     }
      std::cerr<<"\n";

    }//DEBUG
    */

    { //DEBUG

      std::cerr << "DBG_NB_ON_SEVERAL_BINS=" << DBG_NB_ON_SEVERAL_BINS << " / "
                << numTriangles << " tris="
                << DBG_NB_ON_SEVERAL_BINS * 100.0 / (double)numTriangles
                << "%\n";
      std::cerr << "trianglesBins.size=" << trianglesBins.size()
                << " trianglesBins.capacity=" << trianglesBins.capacity()
                << " numTriangles=" << numTriangles << "\n";
      std::cerr << "trianglesBins.size/numTriangles="
                << trianglesBins.size() / (double)numTriangles << "\n";
      std::cerr << "reserve_size=" << reserve_size << "\n";
      if (reserve_size < trianglesBins.size())
        std::cerr << "TODO:OPTIM: increase reserve_size to at least "
                  << trianglesBins.size() << "\n";

      size_t numMinBin = 0;
      size_t minNb = std::numeric_limits<size_t>::max();
      size_t maxNb = std::numeric_limits<size_t>::min();
      size_t m0 = 0, m1 = 0, m2 = 0;

      for (size_t i = 0; i < m_binStarts.size() - 1; ++i) {
        const size_t s = m_binStarts[i + 1] - m_binStarts[i];
        ++m0;
        m1 += s;
        m2 += s * s;
        if (s < minNb) {
          minNb = s;
          numMinBin = 1;
        } else if (s == minNb) {
          ++numMinBin;
        }
        if (s > maxNb)
          maxNb = s;
      }
      std::cerr << "For " << m_numBins << " bins:\n";
      std::cerr << "minNb=" << minNb << " [for " << numMinBin
                << " bins] ; maxNb=" << maxNb << "\n";
      const double inv_m0 = 1. / m0;
      std::cerr << "mean=" << m1 * inv_m0
                << " stdDev=" << sqrt((m2 - m1 * m1 * inv_m0) * inv_m0) << "\n";
      std::cerr << "\n";

    } //DEBUG

    assert(m_binStarts.size() == m_numBins + 1);
  }

  void getTriangleIndicesForY(float y,
                              const uint32_t *&indices,
                              uint32_t &numIndices) const
  {
    const uint32_t bin = getBin(y);
    assert(bin < m_numBins && bin + 1 < m_binStarts.size());
    const size_t binStart = m_binStarts[bin];
    const size_t binEnd = m_binStarts[bin + 1];
    assert(binStart <= binEnd);
    numIndices = binEnd - binStart;
    indices = &(m_indices[binStart]);
  }

protected:
  uint32_t getBin(float y) const
  {
    //return (y>m_yMin ? static_cast<size_t>( (y-m_yMin)/m_step ) : 0);
    const uint32_t bin =
      (y < m_yMax
         ? (y > m_yMin ? static_cast<size_t>((y - m_yMin) / m_step) : 0)
         : m_numBins - 1);

    return bin;
  }

private:
  float m_yMin;
  float m_yMax;
  uint32_t m_numBins;
  float m_step;

  std::vector<uint32_t> m_indices;
  std::vector<size_t> m_binStarts;
  //we store a vector of vector as two vectors.
};

struct SorterY
{
  const std::vector<float> &m_ys;

  explicit SorterY(const std::vector<float> &ys)
    : m_ys(ys)
  {}

  inline bool operator()(uint32_t i1, uint32_t i2) const
  {
    return m_ys[i1] < m_ys[i2];
    //TODO:OPTIM? on pourrait se passer de ys et acceder directement vertices... ???
  }
};

struct CompareY
{
  const std::vector<float> &m_ys;

  explicit CompareY(const std::vector<float> &ys)
    : m_ys(ys)
  {}

  bool operator()(uint32_t i1, uint32_t i2) const
  {
    return m_ys[i1] == m_ys[i2]; //exact comparison
    //TODO:OPTIM? on pourrait se passer de v_y et acceder directement vertices... ???
  }
};

class SpacePartionnerYb
{
public:
  SpacePartionnerYb() {}

  explicit SpacePartionnerYb(const Mesh &mesh) { init(mesh); }

  void init(const Mesh &mesh)
  {
    /*
      We want :
      - sorted and unique y values : to have all the planes to traverse/test
      - for each vertex to which y plane it belongs !


      example:
      v_indices : 0 1 2 3  4 5 6 7 8
      v_y       : 2 2 1 0 -1 0 2 1 0

      sort(y):
      v_indices :  4 3 5 8 2 7 1 6 0
      v_y       : -1 0 0 0 1 1 2 2 2


      v_unique_y_ind :  0 1 2 3
      v_unique_y     : -1 0 1 2


      v_indices_unique_y_ind : 0 1 2 3 4 5 6 7 8
      v_indices_unique_y     : 3 3 2 1 0 1 3 2 1

     */

    //double t0 = (double)cv::getTickCount();

    const uint32_t numvertices = mesh.numVertices;
    const float *vertices = mesh.vertices;
    std::vector<uint32_t> v_indices;
    std::vector<float> v_y;
    v_indices.reserve(numvertices);
    v_y.reserve(numvertices);

    for (uint32_t i = 0; i < numvertices; ++i) {
      v_indices.push_back(i);
      v_y.push_back(vertices[3 * i + 1]);
    }
    //on pourrait se passer de v_y et acceder directement vertices...
    std::sort(v_indices.begin(), v_indices.end(), SorterY(v_y));

    //t0 = ((double)cv::getTickCount() - t0)/cv::getTickFrequency();
    //std::cerr<<"time sort indices (according to y) : "<<t0<<"s\n";
    //t0 = (double)cv::getTickCount();

    std::vector<uint32_t> v_indices_unique_y(
      numvertices); //TODO:OPTIM: hidden memset

    std::vector<float> &v_unique_y = m_unique_y;
    v_unique_y.reserve(numvertices);

    float prev_y = v_y[v_indices[0]];
    uint32_t ind_u = 0; //index in unique_y list
    v_indices_unique_y[v_indices[0]] = ind_u;
    v_unique_y.push_back(prev_y);
    for (uint32_t i = 1; i < numvertices; ++i) { //start from 1
      const uint32_t v_ind = v_indices[i];
      const float y = v_y[v_ind];
      if (y != prev_y) {
        v_unique_y.push_back(y);
        prev_y = y;
        ++ind_u;
      }
      v_indices_unique_y[v_ind] = ind_u;
    }
    assert(!v_unique_y.empty());
    assert(*(v_unique_y.rbegin()) == std::numeric_limits<float>::max());
    //v_unique_y.resize(v_unique_y.size()-1);

    const size_t unique_y_size = v_unique_y.size();

    //t0 = ((double)cv::getTickCount() - t0)/cv::getTickFrequency();
    //std::cerr<<"time unique_y & indices_unique_y  : "<<t0<<"s : unique_y_size="<<unique_y_size<<" numvertices="<<numvertices<<"\n";

    const uint32_t numTriangles = mesh.numTriangles;

    std::cerr << "v_indices_unique_y.size()=" << v_indices_unique_y.size()
              << " numvertices=" << numvertices
              << " unique_y_size=" << unique_y_size << "\n";
    assert(v_indices_unique_y.size() == numvertices);

    { //DEBUG
      for (uint32_t i = 0; i < numvertices; ++i) {
        //if (v_indices_unique_y[i]>=unique_y_size)
        //std::cerr<<"v_indices_unique_y["<<i<<"]="<<v_indices_unique_y[i]<<" unique_y_size="<<unique_y_size<<"\n";
        assert(v_indices_unique_y[i] < unique_y_size);
      }

    } //DEBUG

    //STAT
    size_t DBG_NB_ON_SEVERAL_BINS = 0;
    size_t DBG_NB_BINS_MAX = 0;

    //t0 = (double)cv::getTickCount();

    const uint32_t numBins = unique_y_size;

    const uint32_t *triangles = mesh.triangles;
    std::vector<uint32_t> trianglesBins;
    const size_t reserve_size =
      3 *
      numTriangles; //arbitrary size   //B: numTriangles or unique_y_size ????????????
    //triangles may be in several bins, thus reserve_size must be superior to numTriangles
    trianglesBins.reserve(reserve_size);
    std::vector<uint32_t> trianglesIdx;
    trianglesIdx.reserve(reserve_size);

    //t0 = ((double)cv::getTickCount() - t0)/cv::getTickFrequency();
    //std::cerr<<"time reserve  : "<<t0<<"s : for numTriangles="<<numTriangles<<"\n";
    std::cerr << "trianglesBins.size()=" << trianglesBins.size()
              << " capacity=" << trianglesBins.capacity() << "\n";
    std::cerr << "trianglesIdx.size() =" << trianglesIdx.size()
              << " capacity=" << trianglesIdx.capacity() << "\n";
    //t0 = (double)cv::getTickCount();

    for (uint32_t i = 0; i < numTriangles; ++i) {
      const uint32_t v_id_0 = triangles[3 * i + 0];
      const uint32_t v_id_1 = triangles[3 * i + 1];
      const uint32_t v_id_2 = triangles[3 * i + 2];

      const uint32_t ind_u_y_0 = v_indices_unique_y[v_id_0];
      const uint32_t ind_u_y_1 = v_indices_unique_y[v_id_1];
      const uint32_t ind_u_y_2 = v_indices_unique_y[v_id_2];

      const uint32_t min_ind_u_y =
        std::min(ind_u_y_0, std::min(ind_u_y_1, ind_u_y_2));
      const uint32_t max_ind_u_y =
        std::max(ind_u_y_0, std::max(ind_u_y_1, ind_u_y_2));

      for (uint32_t b = min_ind_u_y; b <= max_ind_u_y; ++b) {
        trianglesBins.push_back(b);
        trianglesIdx.push_back(i);
      }

      //STAT
      if (min_ind_u_y != max_ind_u_y) {
        ++DBG_NB_ON_SEVERAL_BINS;
        const uint32_t nbB = max_ind_u_y - min_ind_u_y;
        if (nbB > DBG_NB_BINS_MAX) {
          //std::cerr<<"nbBins="<<nbB<<" minBin="<<min_ind_u_y<<" maxBin="<<max_ind_u_y<<"\n";
          //std::cerr<<"v_id0="<<v_id_0<<" y_ind="<<ind_u_y_0<<"; "<<"v_id1="<<v_id_1<<" y_ind="<<ind_u_y_1<<"; "<<"v_id2="<<v_id_2<<" y_ind="<<ind_u_y_2<<"\n";
          DBG_NB_BINS_MAX = nbB;
        }
      }
    }
    assert(trianglesBins.size() == trianglesIdx.size());

    //t0 = ((double)cv::getTickCount() - t0)/cv::getTickFrequency();
    //std::cerr<<"time push bins & idx  : "<<t0<<"s : for numTriangles="<<numTriangles<<"\n";
    std::cerr << "trianglesBins.size()=" << trianglesBins.size()
              << " capacity=" << trianglesBins.capacity() << "\n";
    std::cerr << "trianglesIdx.size() =" << trianglesIdx.size()
              << " capacity=" << trianglesIdx.capacity() << "\n";
    std::cerr << "trianglesBins.size()/numTriangles="
              << trianglesBins.size() / (double)numTriangles << "\n";
    std::cerr << "DBG_NB_ON_SEVERAL_BINS=" << DBG_NB_ON_SEVERAL_BINS << "\n";
    std::cerr << "DBG_NB_BINS_MAX=" << DBG_NB_BINS_MAX << "\n";
    //t0 = (double)cv::getTickCount();

    //we sort indices according to trianglesBins, to be able to have trianglesIdx sorted according to trianglesBins
    const size_t tsz = trianglesBins.size();
    std::vector<uint32_t> indices(tsz);
    for (uint32_t i = 0; i < tsz; ++i) {
      indices[i] = i;
    }
    std::sort(indices.begin(), indices.end(), IndiceSorter(trianglesBins));

    m_indices.resize(tsz);
    for (uint32_t i = 0; i < tsz; ++i) {
      m_indices[i] = trianglesIdx[indices[i]];
    }

    m_binStarts.resize(numBins + 1);
    m_binStarts[0] = 0;
    uint32_t currBin = 0;
    uint32_t binIdx = 1;
    for (uint32_t i = 1; i < tsz; ++i) { //start from 1
      uint32_t bin = trianglesBins[indices[i]];
      while (currBin < bin) {
        //std::cerr<<"m_binStarts["<<binIdx<<"]="<<i<<"\n";
        m_binStarts[binIdx] = i;
        ++currBin;
        ++binIdx;
      }
    }
    for (; binIdx <= numBins; ++binIdx) {
      m_binStarts[binIdx] = tsz;
      std::cerr << "m_binStarts[" << binIdx << "]=" << tsz << " !\n";
    }

    //t0 = ((double)cv::getTickCount() - t0)/cv::getTickFrequency();
    //std::cerr<<"time sort & set starts  : "<<t0<<"s : for numBins="<<numBins<<"\n";

    { //DEBUG

      std::cerr << "DBG_NB_ON_SEVERAL_BINS=" << DBG_NB_ON_SEVERAL_BINS << " / "
                << numTriangles << " tris="
                << DBG_NB_ON_SEVERAL_BINS * 100.0 / (double)numTriangles
                << "%\n";
      std::cerr << "trianglesBins.size=" << trianglesBins.size()
                << " trianglesBins.capacity=" << trianglesBins.capacity()
                << " numTriangles=" << numTriangles << "\n";
      std::cerr << "trianglesBins.size/numTriangles="
                << trianglesBins.size() / (double)numTriangles << "\n";
      std::cerr << "reserve_size=" << reserve_size << "\n";

      size_t numMinBin = 0;
      size_t minNb = std::numeric_limits<size_t>::max();
      size_t maxNb = std::numeric_limits<size_t>::min();
      size_t m0 = 0, m1 = 0, m2 = 0;

      for (size_t i = 0; i < m_binStarts.size() - 1; ++i) {
        const size_t s = m_binStarts[i + 1] - m_binStarts[i];
        ++m0;
        m1 += s;
        m2 += s * s;
        if (s < minNb) {
          minNb = s;
          numMinBin = 1;
        } else if (s == minNb) {
          ++numMinBin;
        }
        if (s > maxNb)
          maxNb = s;
      }
      std::cerr << "For " << numBins << " bins:\n";
      std::cerr << "minNb=" << minNb << " [for " << numMinBin
                << " bins] ; maxNb=" << maxNb << "\n";
      const double inv_m0 = 1. / m0;
      std::cerr << "mean=" << m1 * inv_m0
                << " stdDev=" << sqrt((m2 - m1 * m1 * inv_m0) * inv_m0) << "\n";
      std::cerr << "\n";

    } //DEBUG

    assert(m_binStarts.size() == numBins + 1);
  }

  const std::vector<float> &getUniqueY() { return m_unique_y; }

  void getTriangleIndicesForY(uint32_t ind_u_y,
                              const uint32_t *&indices,
                              uint32_t &numIndices) const
  {
    const uint32_t bin = ind_u_y;
    //assert(bin<m_numBins);
    assert(bin + 1 < m_binStarts.size());
    const size_t binStart = m_binStarts[bin];
    const size_t binEnd = m_binStarts[bin + 1];
    assert(binStart <= binEnd);

    numIndices = binEnd - binStart;
    indices = &(m_indices[binStart]);
  }

private:
  //we store a vector of vector as two vectors.
  std::vector<uint32_t> m_indices;
  std::vector<size_t> m_binStarts;

  //unique y
  std::vector<float> m_unique_y;
};

#ifdef TIME_TEXCOORDS
static size_t STAT_NB_TRIANGLES_TESTED = 0;
static size_t STAT_NB_TRIANGLES_TESTED2 = 0;
static size_t STAT_NB_TRIANGLES_2D = 0;
static size_t STAT_NB_EARLY_EXIT = 0;
static size_t STAT_MAX_XZIDXS_TMP_SIZE = 0;
static size_t STAT_XZIDXS_TMP_SIZE_W_DUP = 0;
static size_t STAT_XZIDXS_TMP_SIZE = 0;
static size_t STAT_XZIDXS_TMP_SIZE_1 = 0;
static size_t STAT_MAX_XZIDXS_SIZE = 0;
static size_t STAT_XZIDXS_SIZE_W_DUP = 0;
static size_t STAT_XZIDXS_SIZE_SORT = 0;
static size_t STAT_XZIDXS_SIZE = 0;
static size_t STAT_MAX_XZIDXS_TSIZE = 0;
static size_t STAT_NB_BRANCH_1 = 0;
static size_t STAT_NB_BRANCH_1_1 = 0;
static size_t STAT_NB_BRANCH_1_2 = 0;
static size_t STAT_NB_BRANCH_1_3 = 0;
static size_t STAT_NB_BRANCH_2 = 0;
static size_t STAT_NB_BRANCH_2_1 = 0;
static size_t STAT_NB_BRANCH_2_1_1 = 0;
static size_t STAT_NB_BRANCH_2_1_2 = 0;
static size_t STAT_NB_BRANCH_2_1_2_1 = 0;
static size_t STAT_NB_BRANCH_2_1_2_2 = 0;
static size_t STAT_NB_BRANCH_2_2 = 0;
static size_t STAT_NB_BRANCH_2_2_1 = 0;
static size_t STAT_NB_BRANCH_2_2_1_1 = 0;
static size_t STAT_NB_BRANCH_2_2_2 = 0;
static size_t STAT_NB_BRANCH_2_2_2_1 = 0;
static double STAT_time_getIntersectionLine_tests = 0;
static double STAT_time_getIntersectionLine_intersects = 0;
static double STAT_time_getIntersectionLine_sort = 0;
static double STAT_time_getIntersectionLine_remove = 0;
static size_t STAT_NB_SORT = 0;
static size_t STAT_NB_ELTS_SORTED = 0;
static size_t STAT_NB_ELTS_KEPT = 0;
static size_t STAT_MAX_NB_ELTS_REMOVED = 0;
static size_t STAT_MAX_NB_ELTS_REMOVED_NB_ELTS = 0;
static size_t STAT_NB_ELTS_REMOVED = 0;
#endif //TIME_TEXCOORDS

void
sortZAccordingToNeighbours(std::vector<X_Z_Idx> &xzidxs)
{
  const size_t sz = xzidxs.size();
  for (size_t i = 1; i < sz; ++i) { //start from 1

    if (xzidxs[i - 1].x == xzidxs[i].x) {
      size_t first = i - 1;
      size_t last = i;
      for (size_t k = i + 1; k < sz; ++k) {
        if (xzidxs[k - 1].x == xzidxs[k].x)
          ++last;
        else
          break;
      }

      //std::cerr<<"i="<<i<<" first="<<first<<" xzidxs[first].x="<<xzidxs[first].x<<" last="<<last<<" xzidxs[last].x="<<xzidxs[last].x<<"\n";

      if (first > 0 && last < sz - 1) {
        size_t prev = first - 1;
        size_t next = last + 1;
        assert(xzidxs[prev].x < xzidxs[first].x);
        assert(xzidxs[last].x < xzidxs[next].x);
        float distZ_prev_first = fabs(xzidxs[prev].z - xzidxs[first].z);
        float distZ_prev_last = fabs(xzidxs[prev].z - xzidxs[last].z);
        float distZ_next_last = fabs(xzidxs[next].z - xzidxs[last].z);
        float distZ_next_first = fabs(xzidxs[next].z - xzidxs[first].z);
        if (distZ_prev_first > distZ_prev_last ||
            distZ_next_last > distZ_next_first) {
          for (size_t k = 0; k < (last - first) / 2; ++k) {
            std::swap(xzidxs[first + k], xzidxs[last - k]);
          }
          //std::cerr<<"change at x="<<xzidxs[first].x <<" :   first="<<first<<" last="<<last<<"\n";
          //if (first+1<last)
          //std::cerr<<"CHANGE MORE THAN TWO\n";

          /*
          std::cerr<<"prev="<<prev<<" x="<<xzidxs[prev].x<<" z="<<xzidxs[prev].z<<" ;";
          for (int k=first; k<=last; ++k) {
            std::cerr<<" "<<k<<" x="<<xzidxs[k].x<<" z="<<xzidxs[k].z<<" ;";
          }
          std::cerr<<" next="<<next<<" x="<<xzidxs[next].x<<" z="<<xzidxs[next].z<<" ;";
          std::cerr<<"\n";
          */
        } else {
          //std::cerr<<"No change at x="<<xzidxs[first].x <<" :   first="<<first<<" last="<<last<<"\n";
        }

      } else {
        std::cerr << "TODO change at extremity !!!\n";
      }

      i = last;
    }
  }
}

void
getIntersectionLine(const Mesh &mesh,
                    float y0,
                    uint32_t
#ifndef NDEBUG
		    y0_idx
#endif //NDEBUG
		    ,
                    std::vector<X_Z_Idx> &xzidxs,
                    std::vector<X_Z_Idx> &xzidxs_tmp,
                    const SpacePartionnerY &
#if 0
		    spy
#endif
		    )
{
  //REM:OPTIM: resize to zero but keep capacity (we want to avoid memory allocations).
  xzidxs.clear();
  xzidxs_tmp.clear();

  //const Plane p_Y(0, 1, 0, -y0);

  const float *vertices = mesh.vertices;

  //Vector3 p0, p1, p2;

  const uint32_t *triangles = mesh.triangles;
  const uint32_t numTriangles = mesh.numTriangles;

  //double t0 = (double)cv::getTickCount();

  //std::cerr<<"----------------------------------------------\n";

#if 1
  for (uint32_t i = 0; i < numTriangles; ++i) {

    const uint32_t v_id_0 = triangles[3 * i + 0];
    const uint32_t v_id_1 = triangles[3 * i + 1];
    const uint32_t v_id_2 = triangles[3 * i + 2];
#else

  uint32_t numTrianglesIndices = 0;
  const uint32_t *trianglesIndices = nullptr;
  spy.getTriangleIndicesForY(y0, trianglesIndices, numTrianglesIndices);
  for (uint32_t i = 0; i < numTrianglesIndices; ++i) {

    const uint32_t ind = trianglesIndices[i];
    assert(ind < numTriangles);
    const uint32_t v_id_0 = triangles[3 * ind + 0];
    const uint32_t v_id_1 = triangles[3 * ind + 1];
    const uint32_t v_id_2 = triangles[3 * ind + 2];

#endif

#ifdef TIME_TEXCOORDS
    ++STAT_NB_TRIANGLES_TESTED;
#endif //TIME_TEXCOORDS

#if 0
    if (v_id_0 == v_id_1 || v_id_0 == v_id_2 || v_id_1 == v_id_2) {
      //two identical vertices for triangle : triangle reduced to an edge
      //We consider that intersections, if any, will be introduced by other connected triangles
      //thus we can skip this one
      ++STAT_NB_TRIANGLES_2D;
      continue;
    }
    //std::cerr<<"y0="<<y0<<" v_id_0="<<v_id_0<<" v_id_1="<<v_id_1<<" v_id_2="<<v_id_2<<"\n";
#endif //0

    assert(v_id_0 != v_id_1 && v_id_0 != v_id_2 && v_id_1 != v_id_2);

    const Vector3 p0(vertices[3 * v_id_0 + 0],
                     vertices[3 * v_id_0 + 1],
                     vertices[3 * v_id_0 + 2]);
    const Vector3 p1(vertices[3 * v_id_1 + 0],
                     vertices[3 * v_id_1 + 1],
                     vertices[3 * v_id_1 + 2]);
    const Vector3 p2(vertices[3 * v_id_2 + 0],
                     vertices[3 * v_id_2 + 1],
                     vertices[3 * v_id_2 + 2]);

    const float f0 = p0.y - y0; //p_Y.f(p0);
    const float f1 = p1.y - y0; //p_Y.f(p1);
    const float f2 = p2.y - y0; //p_Y.f(p2);
    assert((v_id_0 != y0_idx) || f0 == 0.f);
    assert((v_id_1 != y0_idx) || f1 == 0.f);
    assert((v_id_2 != y0_idx) || f2 == 0.f);

    if (f0 < 0 && f1 < 0 && f2 < 0) {
    // The plane P does not intersect with the triangle
    // All points are inferior (left) to plane.

#ifdef TIME_TEXCOORDS
      ++STAT_NB_EARLY_EXIT;
#endif //TIME_TEXCOORDS

      //TODO:OPTIM:TEST: si on enleve les doublons plus tard, on peut peut-etre se passer de ce test ????

      continue;
    }

    if (f0 > 0 && f1 > 0 && f2 > 0) {
// The plane P does not intersect with the triangle
// All points are superior (right) to plane.
#ifdef TIME_TEXCOORDS
      ++STAT_NB_EARLY_EXIT;
#endif //TIME_TEXCOORDS
      continue;
    }

#ifdef TIME_TEXCOORDS
    ++STAT_NB_TRIANGLES_TESTED2;
#endif //TIME_TEXCOORDS
    /*
      REM:
      We consider intersections with vertices and edges of triangles with infinite plane at y0.
      As edges or vertices are shared among triangles, intersection points will most often belong to several triangles.
      If intersection is on an original vertex of the mesh, this point will appear for each triangle it belongs to.
      If intersection is on an edge, the point will appear for the two triangles sharing this edge.

      As true points at the same y seem very rare in the considered meshes,
      we push intersection points on original vertices in 'xzidxs_tmp' and we will remove duplicates from this vector based on their v_idx.
      We push intersection points on edges in 'xzidxs' and we will remove duplicates based on their (x, z) coordinates.

      We have far more points in xzidxs than in xzidxs_tmp.

     */

    //TODO:OPTIM: comment éviter le tri+unique sur xzidxs ?
    // Pourrait-on tirer partie du fait que les points apparaissent au plus (?)
    // deux fois ?

    //TODO:OPTIM??? faire un cache à 1 élément pour les vrais index
    // (comme on a souvent au plus 1 element par z)
    // si l'element précédement empilé est cet élement, alors on fait moins de
    // calculs ????

    /*

    std::cerr<<"v_id_0="<<v_id_0<<" p0=["<<p0.x<<", "<<p0.y<<", "<<p0.z<<"]\n";
    std::cerr<<"v_id_1="<<v_id_1<<" p1=["<<p1.x<<", "<<p1.y<<", "<<p1.z<<"]\n";
    std::cerr<<"v_id_2="<<v_id_2<<" p2=["<<p2.x<<", "<<p2.y<<", "<<p2.z<<"]\n";
    std::cerr<<"f0="<<f0<<" f1="<<f1<<" f2="<<f2<<"\n";

    //std::cerr<<"p0.y-y0="<<p0.y - y0<<"    p_Y.f(p0)="<<p_Y.f(p0)<<"\n";
    //std::cerr<<"float(0*p0.x + 1*p0.y + 0*p0.z -y0)="<<float(0*p0.x + 1*p0.y + 0*p0.z -y0)<<"\n";
    //std::cerr<<"double(0*p0.x + 1*p0.y + 0*p0.z -y0)="<<double(0*p0.x + 1*p0.y + 0*p0.z -y0)<<"\n";
    //exit(10);
    */

    //- Find intersections points

    const float f01 = f0 * f1;
    const float f02 = f0 * f2;
    const float f12 = f1 * f2;
    const float f012 = f0 * f1 * f2;

    if (!IsEqualZero(f012)) {
    //B: case where no vertex is on the plane.

#ifdef TIME_TEXCOORDS
      ++STAT_NB_BRANCH_1;
#endif //TIME_TEXCOORDS

      if (f01 < 0) {
#ifdef TIME_TEXCOORDS
        ++STAT_NB_BRANCH_1_1;
#endif //TIME_TEXCOORDS

        //double t2 = (double)cv::getTickCount();

        const Line3 line01(p0, p1);
        const Vector3 p_intersection01 = line01.getPoint(y0);

        //t2 = ((double)cv::getTickCount() - t2)/cv::getTickFrequency();
        //STAT_time_getIntersectionLine_intersects += t2;

        //std::cerr<<"y="<<y0<<" inter01 x="<<p_intersection01.x<<" z="<<p_intersection01.z<<"\n";
        xzidxs.push_back(
          X_Z_Idx(p_intersection01.x, p_intersection01.z, INVALID_INDEX));

        //std::cerr<<"push 1_1    y0="<<y0<<" p01 x="<<p_intersection01.x<<" z="<<p_intersection01.z<<"\n";
      }
      if (f02 < 0) {
#ifdef TIME_TEXCOORDS
        ++STAT_NB_BRANCH_1_2;
#endif //TIME_TEXCOORDS
        //double t2 = (double)cv::getTickCount();

        const Line3 line02(p0, p2);
        const Vector3 p_intersection02 = line02.getPoint(y0);
        //std::cerr<<"y="<<y0<<" inter02 x="<<p_intersection02.x<<" z="<<p_intersection02.z<<"\n";

        //t2 = ((double)cv::getTickCount() - t2)/cv::getTickFrequency();
        //STAT_time_getIntersectionLine_intersects += t2;

        xzidxs.push_back(
          X_Z_Idx(p_intersection02.x, p_intersection02.z, INVALID_INDEX));

        //std::cerr<<"push 1_2    y0="<<y0<<" p02 x="<<p_intersection02.x<<" z="<<p_intersection02.z<<"\n";
      }
      if (f12 < 0) {
#ifdef TIME_TEXCOORDS
        ++STAT_NB_BRANCH_1_3;
#endif //TIME_TEXCOORDS
        //double t2 = (double)cv::getTickCount();

        const Line3 line12(p1, p2);
        const Vector3 p_intersection12 = line12.getPoint(y0);

        //t2 = ((double)cv::getTickCount() - t2)/cv::getTickFrequency();
        //STAT_time_getIntersectionLine_intersects += t2;
        //std::cerr<<"y="<<y0<<" inter12 x="<<p_intersection12.x<<" z="<<p_intersection12.z<<"\n";

        xzidxs.push_back(
          X_Z_Idx(p_intersection12.x, p_intersection12.z, INVALID_INDEX));

        //std::cerr<<"push 1_3    y0="<<y0<<" p12 x="<<p_intersection12.x<<" z="<<p_intersection12.z<<"\n";
      }
    } else {
    //B
    //f012 ~0
    //case where at least one of the vertices is on the plane

#ifdef TIME_TEXCOORDS
      ++STAT_NB_BRANCH_2;
#endif //TIME_TEXCOORDS

      if (IsEqualZero(f0)) {

#ifdef TIME_TEXCOORDS
        ++STAT_NB_BRANCH_2_1;
#endif //TIME_TEXCOORDS

        //std::cerr<<"pusT 2_1    y0="<<y0<<" v_id_0="<<v_id_0<<" x="<<p0.x<<" z="<<p0.z<<"\n";
        xzidxs_tmp.push_back(X_Z_Idx(p0.x, p0.z, v_id_0));

        if (f12 < 0) {
        //case where we also have an intersection with edge 12

#ifdef TIME_TEXCOORDS
          ++STAT_NB_BRANCH_2_1_1;
#endif //TIME_TEXCOORDS
          //double t2 = (double)cv::getTickCount();

          const Line3 line12(p1, p2);
          const Vector3 p_intersection12 = line12.getPoint(y0);

          //t2 = ((double)cv::getTickCount() - t2)/cv::getTickFrequency();
          //STAT_time_getIntersectionLine_intersects += t2;

          xzidxs.push_back(
            X_Z_Idx(p_intersection12.x, p_intersection12.z, INVALID_INDEX));
          //std::cerr<<"push 2_1_1   y0="<<y0<<" p12 x="<<p_intersection12.x<<" z="<<p_intersection12.z<<"\n";

          //TODO: Add asserts !!! Other vertices should not be on the plane !

        } else if (IsEqualZero(f12)) {
        //case where we also have one of the two other vertices on the plane

#ifdef TIME_TEXCOORDS
          ++STAT_NB_BRANCH_2_1_2;
#endif //TIME_TEXCOORDS

          if (IsEqualZero(f1)) {
          //vertex 1 is also on the plance

#ifdef TIME_TEXCOORDS
            ++STAT_NB_BRANCH_2_1_2_1;
#endif //TIME_TEXCOORDS
            //std::cerr<<"pusT 2_1_2_1 y0="<<y0<<" v_id_1="<<v_id_1<<" x="<<p1.x<<" z="<<p1.z<<"\n";
            xzidxs_tmp.push_back(X_Z_Idx(p1.x, p1.z, v_id_1));
          }
          if (
            IsEqualZero(
              f2)) { //B:TODO:OPTIM: Add an else ! We cannot have the three vertices on the plane here ???
                     //vertex 2 is also on the plane

#ifdef TIME_TEXCOORDS
            ++STAT_NB_BRANCH_2_1_2_2;
#endif //TIME_TEXCOORDS
            //std::cerr<<"pusT 2_1_2_2 y0="<<y0<<" v_id_2="<<v_id_1<<" x="<<p2.x<<" z="<<p2.z<<"\n";
            xzidxs_tmp.push_back(X_Z_Idx(p2.x, p2.z, v_id_2));
          }
        }
        //else 1 & 2 are on the same size of the plane, no other intersection.
      } else {
      //f0 != 0
      //0 is not on the plane
      //thus it is 1 or/and 2.

#ifdef TIME_TEXCOORDS
        ++STAT_NB_BRANCH_2_2;
#endif //TIME_TEXCOORDS

        if (IsEqualZero(f1)) {
          //1 is on the plane

          //std::cerr<<"pusT 2_2_1 y0="<<y0<<" v_id_1="<<v_id_1<<" x="<<p1.x<<" z="<<p1.z<<"\n";
          xzidxs_tmp.push_back(X_Z_Idx(p1.x, p1.z, v_id_1));

#ifdef TIME_TEXCOORDS
          ++STAT_NB_BRANCH_2_2_1;
#endif //TIME_TEXCOORDS

          if (f02 < 0) {
          //we also have an intersection with edge 02

#ifdef TIME_TEXCOORDS
            ++STAT_NB_BRANCH_2_2_1_1;
#endif //TIME_TEXCOORDS
            //double t2 = (double)cv::getTickCount();

            const Line3 line02(p0, p2);
            const Vector3 p_intersection02 = line02.getPoint(y0);

            //t2 = ((double)cv::getTickCount() - t2)/cv::getTickFrequency();
            //STAT_time_getIntersectionLine_intersects += t2;

            xzidxs.push_back(
              X_Z_Idx(p_intersection02.x, p_intersection02.z, INVALID_INDEX));
            //std::cerr<<"push 2_2_1_1  y0="<<y0<<" p02 x="<<p_intersection02.x<<" z="<<p_intersection02.z<<"\n";
          }
        }
        if (IsEqualZero(f2)) {

#ifdef TIME_TEXCOORDS
          ++STAT_NB_BRANCH_2_2_2;
#endif //TIME_TEXCOORDS

          //std::cerr<<"pusT 2_2_2   y0="<<y0<<" v_id_2="<<v_id_2<<" x="<<p2.x<<" z="<<p2.z<<"\n";
          xzidxs_tmp.push_back(X_Z_Idx(p2.x, p2.z, v_id_2));

          if (f01 < 0) {

#ifdef TIME_TEXCOORDS
            ++STAT_NB_BRANCH_2_2_2_1;
#endif //TIME_TEXCOORDS
            //double t2 = (double)cv::getTickCount();

            const Line3 line01(p0, p1);
            const Vector3 p_intersection01 = line01.getPoint(y0);

            //t2 = ((double)cv::getTickCount() - t2)/cv::getTickFrequency();
            //STAT_time_getIntersectionLine_intersects += t2;

            xzidxs.push_back(
              X_Z_Idx(p_intersection01.x, p_intersection01.z, INVALID_INDEX));
            //std::cerr<<"push 2_2_2_1  y0="<<y0<<" p01 x="<<p_intersection01.x<<" z="<<p_intersection01.z<<"\n";
          }
        }
      }
    }
  }

  //t0 = ((double)cv::getTickCount() - t0)/cv::getTickFrequency();
  //STAT_time_getIntersectionLine_tests += t0;

  //double t1 = (double)cv::getTickCount();

  //std::cerr<<"numTrianglesIndices="<<numTrianglesIndices<<"\n";
  std::cerr << "y0=" << y0 << "\n";
  std::cerr << "xzidxs_tmp with duplicates: size=" << xzidxs_tmp.size() << "\n";
  std::cerr << "xzidxs with duplicates: size=" << xzidxs.size() << "\n";
#ifdef TIME_TEXCOORDS
/*
  std::cerr<<"STAT_NB_BRANCH_1_1="<<STAT_NB_BRANCH_1_1<<"\n";
  std::cerr<<"STAT_NB_BRANCH_1_2="<<STAT_NB_BRANCH_1_2<<"\n";
  std::cerr<<"STAT_NB_BRANCH_1_3="<<STAT_NB_BRANCH_1_3<<"\n";
  std::cerr<<"STAT_NB_BRANCH_2_1_1="<<STAT_NB_BRANCH_2_1_1<<"\n";
  std::cerr<<"STAT_NB_BRANCH_2_2_1_1="<<STAT_NB_BRANCH_2_2_1_1<<"\n";
  std::cerr<<"STAT_NB_BRANCH_2_2_2_1="<<STAT_NB_BRANCH_2_2_2_1<<"\n";
  */
#endif //TIME_TEXCOORDS

  /*
    We may have no intersection on an exisiting vertex
    in the case where the mesh have vertices not referenced in any triangle.

    Indeed we traverse vertices to find y planes, but traverse triangles to find intersections.
   */
  if (xzidxs_tmp.empty())
    return;

  //sort xzidxs_tmp according to v_idx, move duplicates to the end.
  //[we should have few unique points in xzidxs_tmp, and thus overall, even if they are share by several tris, few points to sort]
  std::sort(xzidxs_tmp.begin(), xzidxs_tmp.end(), X_Z_Idx_SorterIdx());
  const std::vector<X_Z_Idx>::iterator itNE =
    std::unique(xzidxs_tmp.begin(), xzidxs_tmp.end(), X_Z_Idx_CompareIdx());

#ifdef TIME_TEXCOORDS
  STAT_XZIDXS_TMP_SIZE_W_DUP += xzidxs_tmp.size();
  if (xzidxs_tmp.size() > STAT_MAX_XZIDXS_TMP_SIZE) {
    STAT_MAX_XZIDXS_TMP_SIZE = xzidxs_tmp.size();
    //std::cerr<<"xzidxs_tmp.size()="<<xzidxs_tmp.size()<<" ; unique="<<(itNE-xzidxs_tmp.begin())<<" xzidxs.size()="<<xzidxs.size()<<"\n";
  }
  const size_t nbValidIdx = (itNE - xzidxs_tmp.begin());
  STAT_XZIDXS_TMP_SIZE += nbValidIdx;
  STAT_XZIDXS_TMP_SIZE_1 += (nbValidIdx == 1);
  assert(!xzidxs_tmp.empty()); //we should have at least one point at this y0 !
#endif                         //TIME_TEXCOORDS

  //remove duplicates (same idx) from xzidxs_tmp
  xzidxs_tmp.erase(itNE, xzidxs_tmp.end());

  //we then sort xzidxs_tmp according to x coord [few elements to sort]
  // (it allows to determine if we need a zero_point,
  // to know the maximum x,
  // and to add them to xzidxs without sorting afterwards)
  //std::sort(xzidxs_tmp.begin(), xzidxs_tmp.end(), X_Z_Idx_SorterX());
  std::sort(xzidxs_tmp.begin(), xzidxs_tmp.end(), X_Z_Idx_SorterX_Z());

  assert(
    !xzidxs_tmp.empty()); //we still should have at least one point at this y0 !
  const float maxX = xzidxs_tmp.rbegin()->x;

#ifdef DEBUG_TEXCOORDS
  std::cerr << "without duplicates : xzidxs_tmp.size()=" << xzidxs_tmp.size()
            << "\n";
  std::cerr << "with duplicates & > maxX=" << maxX
            << " : xzidxs.size()=" << xzidxs.size() << "\n";
#endif //DEBUG_TEXCOORDS

  //partition xzidxs to avoid to sort x bigger than maxX
  std::vector<X_Z_Idx>::iterator itB =
    std::partition(xzidxs.begin(), xzidxs.end(), X_Z_Idx_SmallX(maxX));

  //sort xzidxs (just part inferior to maxX) according to x,z coordinates, and move duplicates to the end.
  std::sort(xzidxs.begin(), itB, X_Z_Idx_SorterX_Z());

  const std::vector<X_Z_Idx>::iterator itNE2 =
    std::unique(xzidxs.begin(), itB, X_Z_Idx_CompareXZ());
  //const std::vector<X_Z_Idx>::iterator itNE2 = std::unique(xzidxs.begin(), itB, X_Z_Idx_CompareX());
  //B:REM: here, in original Cuong's code, points with the same x (but different z) are merged ! Only one (arbitrary) is kept !
  // I DON'T UNDERSTAND !!!
  // we should use X_Z_Idx_CompareX() instead of  X_Z_Idx_CompareXZ() to have the
  // same behavior !

  //OPTIM??? is it necessary to sort according to X_Z or just X would suffice ????

#ifdef TIME_TEXCOORDS
  STAT_XZIDXS_SIZE_SORT += (itB - xzidxs.begin());
  STAT_XZIDXS_SIZE_W_DUP += xzidxs.size();
  if (xzidxs.size() > STAT_MAX_XZIDXS_SIZE) {
    STAT_MAX_XZIDXS_SIZE = xzidxs.size();
  }
  //const size_t nbDuplicates0 = (xzidxs.end()-itNE2);
  //std::cerr<<"Number of duplicates0="<<nbDuplicates0<<" / "<<xzidxs.size()<<" ; xzidxs_tmp.size="<<xzidxs_tmp.size()<<" y0="<<y0<<"\n";
  STAT_XZIDXS_SIZE += (itNE2 - xzidxs.begin());
#endif //TIME_TEXCOORDS

  //remove duplicates from xzidxs
  xzidxs.erase(itNE2, xzidxs.end());

  std::cerr << "without dups & <= maxX : xzidxs.size()=" << xzidxs.size()
            << "\n";

  /*
  //copy xzidxs_tmp into xzidxs
  for (std::vector<X_Z_Idx>::iterator it= xzidxs_tmp.begin(); it != itNE; ++it) {
    //std::cerr<<it->idx<<" ";
    xzidxs.push_back(*it);
  }
  */

  //Add zero_point (point at x==0) if necessary
  //(we give it the same z than the point with the smallest x)

  bool needsZeroPoint = false;
  X_Z_Idx zeroPoint(0, -std::numeric_limits<float>::max(), INVALID_INDEX);
  const X_Z_Idx a = *(xzidxs_tmp.begin());
  const X_Z_Idx b =
    (!xzidxs.empty()
       ? *(xzidxs.begin())
       : X_Z_Idx(std::numeric_limits<float>::max(), 0, INVALID_INDEX));
  if (a.x < b.x) {
    if (a.x != 0) {
      //assert(a.x > 0);
      needsZeroPoint = true;
      zeroPoint.x = 0; //a.x;
      zeroPoint.z = a.z;
    }
  } else {
    if (b.x != 0) {
      //assert(b.x > 0);
      needsZeroPoint = true;
      zeroPoint.x = 0; //b.x;
      zeroPoint.z = b.z;
    }
  }

  //DEBUG
  //needsZeroPoint = false;

  if (needsZeroPoint) {
    //We add it at the beginning of xzidxs_tmp
    // As xzidxs_tmp has few elements, it should not entail a big/slow move of
    // elements.
    // (and as xzidxs_tmp has sufficient capacity, it should not reallocate).
    xzidxs_tmp.insert(xzidxs_tmp.begin(), zeroPoint);

    std::cerr << "insert zero_pt : x=" << zeroPoint.x << " z=" << zeroPoint.z
              << "\n";
  }

  /*
  std::cerr<<"before merge: "<<xzidxs_tmp.size()<<" xzidxs_tmp: \n";
  for (int k=0; k<std::min((size_t)20, xzidxs_tmp.size()); ++k) {
    std::cerr<<" "<<k<<" x="<<xzidxs_tmp[k].x<<" z="<<xzidxs_tmp[k].z<<" ;";
  }
  std::cerr<<"\n";
  std::cerr<<"before merge: "<<xzidxs.size()<<" xzidxs: \n";
  for (int k=0; k<std::min((size_t)20, xzidxs.size()); ++k) {
    std::cerr<<" "<<k<<" x="<<xzidxs[k].x<<" z="<<xzidxs[k].z<<" ;";
  }
  std::cerr<<"\n\n";
  */

  //We then merge the two sorted vectors
  const size_t s1 = xzidxs.size();
  const size_t s2 = xzidxs_tmp.size();
  const size_t s3 = s1 + s2;
  xzidxs.resize(
    s3); //(it should not reallocate as xzidxs has sufficient capacity...)
  std::vector<X_Z_Idx>::iterator it = xzidxs.begin() + s1;
  std::copy(xzidxs_tmp.begin(), xzidxs_tmp.end(), it);
  //std::inplace_merge(xzidxs.begin(), it, xzidxs.end(), X_Z_Idx_SorterX());
  std::inplace_merge(xzidxs.begin(), it, xzidxs.end(), X_Z_Idx_SorterX_Z());

  //20/02/2015
  // We sort according to x & Z, both  xzidxs & xzidxs_tmp
  // We then merge them and keep them sorted.
  // Then, we inverse some of the consecutive elts with same x but different z

  /*
  std::cerr<<"after merge: xzidxs.size()="<<xzidxs.size()<<" : \n";
  for (int k=0; k<std::min((size_t)20, xzidxs.size()); ++k) {
    std::cerr<<" "<<k<<" x="<<xzidxs[k].x<<" z="<<xzidxs[k].z<<" ;";
  }
  std::cerr<<"\n\n";
  */

  sortZAccordingToNeighbours(xzidxs);

  //std::cerr<<"s1="<<s1<<" s2="<<s2<<" s3="<<s3<<" xzidxs.size()="<<xzidxs.size()<<"\n";

  //TODO:OPTIM??? : est-ce que ce serait plus efficace de le faire "à la main"
  // On sait que zeroPoint si présent, va se retrouver au début et donc forcer le
  // déplacement des autres elts...
  //Ou empiler le zero_point tout le temps et des le debut dans xzidxs ? (ca evite de pousser tous les elements plus tard)

#ifdef TIME_TEXCOORDS
  if (xzidxs.size() > STAT_MAX_XZIDXS_TSIZE)
    STAT_MAX_XZIDXS_TSIZE = xzidxs.size();
#endif //TIME_TEXCOORDS

    //sort according to x
    //std::sort(xzidxs.begin(), xzidxs.end(), X_Z_Idx_SorterX());

#ifdef CHECK_DUPLICATES
  {
    std::sort(xzidxs.begin(), xzidxs.end(), X_Z_Idx_SorterX_Z());
    const std::vector<X_Z_Idx>::iterator itNE2 =
      std::unique(xzidxs.begin(), xzidxs.end(), X_Z_Idx_CompareXZ());
    const size_t nbDuplicates = (xzidxs.end() - itNE2);
    if (nbDuplicates != 0) {
      std::cerr << "y0=" << y0 << " Number of duplicates=" << nbDuplicates
                << "/" << xzidxs.size() << " after fusion ! Exiting !!!\n";

      if (xzidxs.size() < 10) {
        std::cerr << "non duplicates:\n";
        for (std::vector<X_Z_Idx>::iterator it = xzidxs.begin(); it != itNE2;
             ++it)
          std::cerr << it->x << " " << it->z << " idx=" << it->idx << "\n";
        std::cerr << "duplicates:\n";
        for (std::vector<X_Z_Idx>::iterator it = itNE2; it != xzidxs.end();
             ++it)
          std::cerr << it->x << " " << it->z << " idx=" << it->idx << "\n";
      }
      exit(13);
    }
  }
#endif //CHECK_DUPLICATES

    //TODO:OPTIM? A-t-on vraiment besoin de stocker z ? On pourrait y re-accéder avec idx !
    // Est-ce que ça serait plus efficace (cache, pour tri) de stocker x,[z], idx
    // dans 3 tableaux séparés ?

#ifdef TIME_TEXCOORDS
  //t1 = ((double)cv::getTickCount() - t1)/cv::getTickFrequency();
  //STAT_time_getIntersectionLine_sort += t1;
  ++STAT_NB_SORT;
  STAT_NB_ELTS_SORTED += xzidxs.size();
#endif //TIME_TEXCOORDS

  /*
    //check first point is zero_point & have the minimum x.
    assert(xzidxs.begin()->x == 0 && xzidxs.begin()->idx == INVALID_INDEX);
    assert(xzidxs.begin()->x <= (++(xzidxs.begin()))->x);
    //we set its z value to the z of the second point (true intersection point on vertex or edge)
    xzidxs.begin()->z = (++(xzidxs.begin()))->z;
    */

#if 0 //We already have removed elts with x superior to maxX 

  double t2 = (double)cv::getTickCount();

  /*
  {//DEBUG
    for (size_t i=0; i<xzidxs.size(); ++i)
      if (i<3 || xzidxs[i].idx != INVALID_INDEX)
	std::cerr<<i<<") y0="<<y0<<" x="<<xzidxs[i].x<<" z="<<xzidxs[i].z<<" idx="<<xzidxs[i].idx<<"\n";

  }//DEBUG
  */
  
  //TODO:OPTIM: est-ce que cette optimisation apporte quelque chose  ??? ?????????
  //TODO:OPTIM : which version is faster
  //TODO:OPTIM : can we find the last valid index faster ?
  // we could sort the xzidxs_tmp according to x (often just one element after unique) & search for this specific index !
  // oui !! comme on a les xzidxs_tmp triés on pourrait virer tous les elts avec un x supérieur de xzidxs (avant de le trier !)
#if 0
  // Est-ce une bonne idée de parcourir le vecteur à l'envers ???
  //We traverse xzidrs backward to remove the last points that are not true vertices of the mesh
  std::vector<X_Z_Idx>::const_reverse_iterator itR = xzidxs.rbegin();
  const std::vector<X_Z_Idx>::const_reverse_iterator itREnd = xzidxs.rend();
  size_t s = 0;
  for ( ; itR != itREnd; ++itR) {
    if (itR->idx != INVALID_INDEX)
      break;
    ++s;
  }
#else
  //We traverse xzidrs forward till we have crossed nbValidIdx, and remove the rest 
  std::vector<X_Z_Idx>::const_iterator itF = xzidxs.cbegin();
  const std::vector<X_Z_Idx>::const_iterator itFEnd = xzidxs.cend();
  size_t s = 0;
  size_t nv = 0;
  for ( ; itF != itFEnd; ++itF) {
    ++s;
    if (itF->idx != INVALID_INDEX) {
      ++nv;
      if (nv == nbValidIdx) {
	break;
      }
    }
  }
  s = xzidxs.size()-s; //B: just to keep the rest of the code identical, in particular //xzidxs.resize(xzidxs.size() - s);
#endif

  assert(s <= xzidxs.size());
#ifdef TIME_TEXCOORDS
  if (s > STAT_MAX_NB_ELTS_REMOVED) {
    STAT_MAX_NB_ELTS_REMOVED = s;
    STAT_MAX_NB_ELTS_REMOVED_NB_ELTS=xzidxs.size();
  }
  STAT_NB_ELTS_REMOVED += s;
#endif //TIME_TEXCOORDS

  xzidxs.resize(xzidxs.size() - s);

#ifdef TIME_TEXCOORDS
  t2 = ((double)cv::getTickCount() - t2)/cv::getTickFrequency();
  STAT_time_getIntersectionLine_remove += t2;
  STAT_NB_ELTS_KEPT+=xzidxs.size();
#endif //TIME_TEXCOORDS

  //Ainsi dans fonction suivante, quand on est arrivé à ce point, on peut s'arreter !!!!

#endif //0 //maxX
}

//B//B//B
void
computeTexCoords0(Mesh &mesh)
{
// float minx, maxx, miny, maxy, minz, maxz;
// //getModelAABB(model, minx, maxx, miny, maxy, minz, maxz);
// //getModelAABBbyTris(model, minx, maxx, miny, maxy, minz, maxz);
// float minB[3], maxB[3];
// mesh.getAABB(minB, maxB);
// minx = minB[0];
// maxx = maxB[0];
// miny = minB[1];
// maxy = maxB[1];
// minz = minB[2];
// maxz = maxB[2];

//removeDuplicateAndUnusedVertices1(model); exit(10);

#ifdef TIME_TEXCOORDS
  double t = (double)cv::getTickCount();
#endif //TIME_TEXCOORDS

  //B: est-ce qu'on veut vraiment modifier la position du modèle ????

  //DEBUG //
  rotateAroundMean(mesh); // the spine // Oy

  //DEBUG //
  moveTo_X_min(mesh); // translate to X_min

  //t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cerr<<"time move model: "<<t<<"s\n";
  //t = (double)cv::getTickCount();

  std::vector<Y_Idx> ys;
  getIntersectionPlanes(mesh, ys);

  //t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cerr<<"time get y planes: "<<t<<"s for "<<ys.size()<<" planes / "<<mesh.numVertices<<" vertices\n";
  {
    float min_dist_y = std::numeric_limits<float>::max();
    size_t min_dist_i = 0;
    for (size_t i = 1; i < ys.size(); ++i) {
      float d = fabs(ys[i].y - ys[i - 1].y);
      if (d < min_dist_y) {
        min_dist_y = d;
        min_dist_i = i;
      }
    }
    std::cerr << " >> minimum distance between two y planes=" << min_dist_y
              << " between ys[" << (min_dist_i - 1)
              << "]=" << std::setprecision(9) << ys[min_dist_i - 1].y
              << " & ys[" << min_dist_i << "]=" << std::setprecision(9)
              << ys[min_dist_i].y << "\n";
    //exit(10);
  }

  //double t1 = (double)cv::getTickCount();

  SpacePartionnerY spy(mesh, ys.begin()->y, ys.rbegin()->y);

  //t1 = ((double)cv::getTickCount() - t1)/cv::getTickFrequency();
  //std::cerr<<"time SpacePartionnerY init: "<<t1<<"s for "<<ys.size()<<" planes\n";
  //std::cerr<<"time getYPlanes+ SpacePartionnerY init: "<<t+t1<<"s for "<<ys.size()<<" planes\n";

  //float dbg_min_y = ys.begin()->y;
  //float dbg_max_y = ys.rbegin()->y;

  std::cerr << "minY=" << ys.begin()->y << " maxY=" << ys.rbegin()->y << "\n";

#if 0
  std::cerr<<"\nSpacePartionnerYb:\n";
  //t1 = (double)cv::getTickCount();

  SpacePartionnerYb spyb(model);

  //t1 = ((double)cv::getTickCount() - t1)/cv::getTickFrequency();
  //std::cerr<<"time SpacePartionnerYb init: "<<t1<<"s for "<<spyb.getUniqueY().size()<<" planes\n";
#endif //0

  //t = (double)cv::getTickCount();

  mesh.allocateTexCoords();

  //t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cerr<<"time allocate texture mem: "<<t<<"s\n";

#if 1 //DEBUG
  memset(mesh.texCoords, 0, sizeof(float) * mesh.numVertices * 2);
  { //DEBUG
    //check if all points tex coords set
    size_t numZeroTexCoords = 0;
    const uint32_t numtexcoords = mesh.numVertices;
    const float *texcoords = mesh.texCoords;
    for (uint32_t i = 0; i < numtexcoords; ++i) {
      numZeroTexCoords +=
        (texcoords[2 * i + 0] == 0 && texcoords[2 * i + 1] == 0);
    }
    std::cerr << "Après allocateTextureCoordinateMemory: numZeroTexCoords="
              << numZeroTexCoords << "\n";
  }
#endif

#ifdef TIME_TEXCOORDS
  double time_getIntersectionLine = 0;
//t = (double)cv::getTickCount();
#endif //TIME_TEXCOORDS

#ifdef CHECK_TEXCOORDS
  using MAP = std::map<uint32_t, uint32_t>;
  MAP m;
#endif

  float *texcoords = mesh.texCoords;

  uint32_t t_idx = 0;

  std::vector<X_Z_Idx> xzidxs;
  std::vector<X_Z_Idx> xzidxs_tmp;
  xzidxs.reserve(4400); //arbitrary size [to avoid allocations]
  xzidxs_tmp.reserve(
    36); //arbitrary size [to avoid allocations] //should be the max number of triangles connected to a given vertex //B: I have seen 31 !

  static int DBG_nCoords = 0;
  //double t100 = (double)cv::getTickCount();

  //float dbg_min_y2 = dbg_min_y + (dbg_max_y-dbg_min_y)*0.505;
  //float dbg_max_y2 = dbg_max_y - (dbg_max_y-dbg_min_y)*0.472;
  //float dbg_min_y2 = 162.223038;
  //float dbg_max_y2 = 162.226654;
  //std::cerr<<"dbg_min_y2="<<dbg_min_y2<<" dbg_max_y2="<<dbg_max_y2<<"\n";

  std::vector<Y_Idx>::const_iterator itY = ys.cbegin();
  assert(!ys.empty());
  float prev_y = itY->y - 1; //different of all the other ys
  const std::vector<Y_Idx>::const_iterator itYEnd = ys.cend();
  size_t dbg = 0;
  for (; itY != itYEnd; ++itY) {
    const float y = itY->y;
    const uint32_t idx = itY->idx;

    /*
    if (y != dbg_min_y && y != dbg_max_y) {
    */

    /*
    if (y < dbg_min_y2)
      continue;
    if (y > dbg_max_y2)
      exit(10);
    */

    /*
    }
    */

    /*
    if (y < 162.00823)
      continue;
    if (y > 162.008256)
      break;
    */

    //if (y != prev_y)
    assert(y != prev_y);
    {

      //double t1 = cv::getTickCount();

      getIntersectionLine(
        mesh,
        y,
        idx,
        xzidxs,
        xzidxs_tmp,
        spy); //clear & fill xzidxs ; xzidxs_tmp is just a cache

      std::cerr << "******************************************* y=" << y
                << " xzidxs.size()=" << xzidxs.size() << "\n";
      {
        const size_t M = 7;
        for (size_t i = 0; i < std::min(M, xzidxs.size()); ++i) {
          std::cerr << "[x=" << xzidxs[i].x << " z=" << xzidxs[i].z
                    << " id=" << xzidxs[i].idx << "] ";
        }
        if (xzidxs.size() > M)
          std::cerr << "...";
        std::cerr << "\n";
      }

      //t1 = ((double)cv::getTickCount() - t1)/cv::getTickFrequency();
      //time_getIntersectionLine += t1;

      /*
      static const size_t STEP = 10000;
      if (dbg % STEP == 0) {
        t100 = ((double)cv::getTickCount() - t100)/cv::getTickFrequency();
        std::cerr<<"time getIntersectionLine: "<<t1<<"s t"<<STEP<<"="<<t100<<"s mean="<<t100/(double)STEP<<" y="<<y<<" xzidxs.size="<<xzidxs.size()<<"  "<<dbg<<"/"<<ys.size()<<"\n";
        t100 = (double)cv::getTickCount();
      }
      */

      //std::cerr<<"y="<<y<<"\n";

      assert(!xzidxs.empty());

      //std::cerr<<"*** y="<<std::setprecision(9)<<y<<" xzidxs.size()="<<xzidxs.size()<<"\n";

      //initialization & handle first point

      std::vector<X_Z_Idx>::const_iterator itX = xzidxs.cbegin();
      const std::vector<X_Z_Idx>::const_iterator itXEnd = xzidxs.cend();
      float prev_x = itX->x;
      float prev_z = itX->z;

      float L = 0;

      uint32_t v_idx0 = itX->idx;

      //if (y < 162.008256) {
      //std::cerr<<"\n"<<xzidxs.size()<<" intersections with plane y="<<std::setprecision(9)<<y<<"\n";
      //}

      if (v_idx0 != INVALID_INDEX) {

        t_idx = v_idx0;

#ifdef CHECK_TEXCOORDS
        if (m.find(v_idx0) != m.end()) {
          std::cerr << "ERROR0: le v_idx0=" << v_idx0
                    << " already in map with t_idx=" << m[v_idx0]
                    << " (current t_idx=" << t_idx << ") !!! y=" << y
                    << " (minY=" << ys.begin()->y << " prev_y=" << prev_y
                    << " dbg=" << dbg << ")\n";
          exit(10);
        }
        if (t_idx >= mesh.numVertices) {
          std::cerr << "ERROR0: invalid t_idx=" << t_idx
                    << " >= mesh.numVertices=" << mesh.numVertices << "\n";
          std::cerr << " y=" << y << " (minY=" << ys.begin()->y << ")\n";
          exit(10);
        }

        assert(m.find(v_idx0) == m.end());
        //m.insert(std::pair<uint32_t, uint32_t>(v_idx0, t_idx));
        m[v_idx0] = t_idx;
        ++DBG_nCoords;
#endif

        //if (y >267.5 && y < 268.5) {
        //std::cerr<<"y="<<std::setprecision(9)<<y<<" v_idx0="<<v_idx0<<" x="<<std::setprecision(9)<<prev_x<<" z="<<std::setprecision(9)<<prev_z<<" dx=0 dz=0 dist="<<0<<" L="<<0<<"\n";
        //std::cerr<<"texcoords idx="<<v_idx0<<" L="<<std::setprecision(9)<<L<<" y="<<std::setprecision(9)<<y<<"\n";
        //}

        //if (fabs(y-837.512) < 0.001)
        //std::cerr<<" L="<<L<<" y="<<y<<"\n";

        // project on 2D plan and preserve the distance between two vertices
        // with the same Y-coordinate
        assert(t_idx < mesh.numVertices); // "<=" because texture coordinate
                                          // index starts from 1
        texcoords[2 * t_idx + 0] = L;
        //DEBUG_FX texcoords[2*t_idx + 0] = 0.1;
        texcoords[2 * t_idx + 1] = y;

        //++t_idx;
      }

      //handle rest of the points

      ++itX;

      for (; itX != itXEnd; ++itX) {
        const float x = itX->x;
        const float z = itX->z;
        const uint32_t v_idx = itX->idx;

        assert(prev_x <= x); //elts are sorted according to x

        const float dx = x - prev_x;
        const float dz = z - prev_z;
        const float dist =
          sqrt(dx * dx + dz * dz); // distance between the two vertices

        //if (fabs(y-837.512) < 0.001)
        //std::cerr<<"idx="<<v_idx<<" x1="<<x<<" x2="<<prev_x<<" z1="<<z<<" z2="<<prev_z<<" dx"<<dx<<" dz="<<dz<<" ds="<<dist<<"\n";

        //L += dist;
        //DEBUG
        if (dx > 0)
          L += dist;

        //if (y < 162.008256) {
        //std::cerr<<"y="<<std::setprecision(9)<<y<<" v_idx="<<(int)(v_idx!=INVALID_INDEX ? v_idx : -1)<<" x="<<std::setprecision(9)<<x<<" z="<<std::setprecision(9)<<z<<" dx="<<dx<<" dz="<<dz<<" dist="<<dist<<" L="<<L<<"\n";
        //}

        /*
        if (dist <= std::numeric_limits<float>::epsilon()) {
          std::cerr<<"WARNING: dist="<<dist<<" almost 0 for x="<<x<<" prev_x="<<prev_x<<" z="<<z<<" prev_z="<<prev_z<<" v_idx="<<v_idx<<" t_idx="<<t_idx<<" y="<<y<<" (minY="<<ys.begin()->y<<" prev_y="<<prev_y<<" dbg="<<dbg<<")\n";
        }

        //assert(dist > std;::numeric_limist<float>::epsilon());
        */

        if (v_idx != INVALID_INDEX) {
          //this is a true vertex from the mesh

          t_idx = v_idx;

//DEBUG
//std::cerr<<"y="<<std::setprecision(9)<<y<<" v_idx="<<v_idx<<" x="<<std::setprecision(9)<<x<<" z="<<std::setprecision(9)<<z<<" dist="<<dist<<" L="<<L<<"\n";
#ifdef CHECK_TEXCOORDS
          if (m.find(v_idx) != m.end()) {
            std::cerr << "ERROR: le v_idx=" << v_idx
                      << " already in map with t_idx=" << m[v_idx]
                      << " (current t_idx=" << t_idx << ") !!! y=" << y
                      << " (minY=" << ys.begin()->y << " prev_y=" << prev_y
                      << " dbg=" << dbg << ")\n";
            exit(10);
          }
          if (t_idx >= mesh.numVertices) {
            std::cerr << "ERROR: invalid t_idx=" << t_idx
                      << " >= mesh.numVertices=" << mesh.numVertices << "\n";
            std::cerr << " y=" << y << " (minY=" << ys.begin()->y << ")\n";
            exit(10);
          }

          assert(m.find(v_idx) == m.end());
          //m.insert(std::pair<uint32_t, uint32_t>(v_idx, t_idx));
          m[v_idx] = t_idx;
          ++DBG_nCoords;
#endif
          //if (y < 162.008256)
          /*
        if (y >265.5 && y < 270.5) {
            if (x == prev_x) {
              std::cerr<<"same x="<<x<<" with y="<<y<<" prev_z="<<prev_z<<" z="<<z<<"\n";
            }

            if (x > 119 && x <123) {
            assert(v_idx <= mesh.numVertices);
            std::cerr<<"### y="<<std::setprecision(9)<<y<<"="<<std::setprecision(9)<<mesh.vertices[3*v_idx+1]<<" v_idx="<<v_idx<<" x="<<std::setprecision(9)<<x<<"="<<std::setprecision(9)<<mesh.vertices[3*v_idx+0]<<" z="<<std::setprecision(9)<<z<<"="<<std::setprecision(9)<<mesh.vertices[3*v_idx+2]<<" L="<<std::setprecision(9)<<L<<"\n";

            //std::cerr<<"*** t_idx="<<t_idx<<" L="<<L<<" y="<<std::setprecision(9)<<y<<" x="<<std::setprecision(9)<<x<<" z="<<std::setprecision(9)<<z<<"  dx="<<dx<<" dz="<<dz<<" \n";
            //std::cerr<<"texcoords idx="<<v_idx<<" L="<<std::setprecision(9)<<L<<" y="<<std::setprecision(9)<<y<<"\n";
            }
          }
          */

          // project on 2D plan and preserve the distance between two vertices
          // with the same Y-coordinate
          assert(t_idx < mesh.numVertices);
          texcoords[2 * t_idx + 0] = L;
          //DEBUG_FX texcoords[2*t_idx + 0] = 0.1;
          texcoords[2 * t_idx + 1] = y;

          //++t_idx;
        }

        prev_x = x;
        prev_z = z;
      }
    }

    prev_y = y;

    /*
    ++dbg;

    if (DEBUG==48745) {
      std::cerr<<"END DEBUG\n";
      exit(11);
    }
    else
      std::cerr<<"\n";
    */
  }

  //t = (double)cv::getTickCount();

  //- normalize texture coordinates
  normalizeTexCoords(mesh);

  //t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cerr<<"time normalize tex coords: "<<t<<"s\n";

  /*
    std::map<std::string, int>::iterator it = map_y.begin();
    int nb_y = std::max(map_y.size()/100, (size_t)10);
    std::map<int, int> v_t_idx;                                                 // texture coordinate index is started by 1
    int t_idx=1, index=0;
    float y;

    it = map_y.begin();

    for(;it!=map_y.end(); ++it, ++index){
      std::string s = (*it).first;

      y = atof(s.c_str());

      glmIntersectionLineProjection(model, y, (*it).second, v_t_idx, t_idx); // project a 3D line on the texture plan to calculate
      // texture coordinates
    }
    */

  //t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cerr<<"time compute tex coords: "<<t<<"s\n";

#ifdef TIME_TEXCOORDS
  {
    std::cerr << "getIntersectionLine()=" << time_getIntersectionLine
              << "s / total time=" << t << "\n";
    std::cerr << "getIntersectionLine_tests="
              << STAT_time_getIntersectionLine_tests << "s\n";
    //std::cerr<<"  getIntersectionLine_intersects="<<STAT_time_getIntersectionLine_intersects<<"s\n";
    std::cerr << "getIntersectionLine_sort ="
              << STAT_time_getIntersectionLine_sort << "s\n";
    std::cerr << "getIntersectionLine_remove="
              << STAT_time_getIntersectionLine_remove << "s\n";

    std::cerr << "NB_SORT=" << STAT_NB_SORT << "\n";
    std::cerr << "NB_ELTS_SORTED=" << STAT_NB_ELTS_SORTED
              << " mean=" << STAT_NB_ELTS_SORTED / (double)STAT_NB_SORT << "\n";
    std::cerr << "NB_ELTS_KEPT  =" << STAT_NB_ELTS_KEPT
              << " mean=" << STAT_NB_ELTS_KEPT / (double)STAT_NB_SORT
              << "  %=" << STAT_NB_ELTS_KEPT * 100. / STAT_NB_ELTS_SORTED
              << "\n";
    std::cerr << "NB_ELTS_REMOVED  =" << STAT_NB_ELTS_REMOVED
              << " mean=" << STAT_NB_ELTS_REMOVED / (double)STAT_NB_SORT
              << "  %=" << STAT_NB_ELTS_REMOVED * 100. / STAT_NB_ELTS_SORTED
              << "\n";
    std::cerr << "MAX_NB_ELTS_REMOVED=" << STAT_MAX_NB_ELTS_REMOVED << " / "
              << STAT_MAX_NB_ELTS_REMOVED_NB_ELTS << " => "
              << STAT_MAX_NB_ELTS_REMOVED * 100.0 /
                   (double)STAT_MAX_NB_ELTS_REMOVED_NB_ELTS
              << "%\n";

    std::cerr << "stats:\n";
    std::cerr << "NB_TRIANGLES_TESTED=" << STAT_NB_TRIANGLES_TESTED << "\n";
    std::cerr << "NB_TRIANGLES_TESTED completely=" << STAT_NB_TRIANGLES_TESTED2
              << "\n";
    std::cerr << "NB_EARLY_EXIT=" << STAT_NB_EARLY_EXIT << " => "
              << STAT_NB_EARLY_EXIT * 100.0 / STAT_NB_TRIANGLES_TESTED << "%\n";
    std::cerr << "STAT_NB_TRIANGLES_2D=" << STAT_NB_TRIANGLES_2D << "\n";

    std::cerr << "MAX_XZIDXS_TOTAL_SIZE=" << STAT_MAX_XZIDXS_TSIZE << "\n";
    std::cerr << "MAX_XZIDXS_SIZE=" << STAT_MAX_XZIDXS_SIZE << "\n";
    std::cerr << "MAX_XZIDXS_TMP_SIZE=" << STAT_MAX_XZIDXS_TMP_SIZE << "\n";

    std::cerr << "mean XZIDXS_TMP_SIZE_W_DUP="
              << STAT_XZIDXS_TMP_SIZE_W_DUP / (double)ys.size() << "\n";
    std::cerr << "mean XZIDXS_TMP_SIZE="
              << STAT_XZIDXS_TMP_SIZE / (double)ys.size() << "\n";
    std::cerr << "XZIDXS_TMP_SIZE_1=" << STAT_XZIDXS_TMP_SIZE_1
              << " mean=" << STAT_XZIDXS_TMP_SIZE_1 / (double)ys.size() << "\n";
    std::cerr << "mean XZIDXS_SIZE_W_DUP="
              << STAT_XZIDXS_SIZE_W_DUP / (double)ys.size() << "\n";
    std::cerr << "mean XZIDXS_SIZE_SORT="
              << STAT_XZIDXS_SIZE_SORT / (double)ys.size() << "\n";
    std::cerr << "mean XZIDXS_SIZE=" << STAT_XZIDXS_SIZE / (double)ys.size()
              << "\n";

    std::cerr << "stats branches:\n";
    std::cerr << "NB_BRANCH_1       = " << std::setw(11) << STAT_NB_BRANCH_1
              << "\n";
    std::cerr << "NB_BRANCH_1_1     = " << std::setw(11) << STAT_NB_BRANCH_1_1
              << "\n";
    std::cerr << "NB_BRANCH_1_2     = " << std::setw(11) << STAT_NB_BRANCH_1_2
              << "\n";
    std::cerr << "NB_BRANCH_1_3     = " << std::setw(11) << STAT_NB_BRANCH_1_3
              << "\n";
    std::cerr << "NB_BRANCH_2       = " << std::setw(11) << STAT_NB_BRANCH_2
              << "\n";
    std::cerr << "NB_BRANCH_2_1     = " << std::setw(11) << STAT_NB_BRANCH_2_1
              << "\n";
    std::cerr << "NB_BRANCH_2_1_1   = " << std::setw(11) << STAT_NB_BRANCH_2_1_1
              << "\n";
    std::cerr << "NB_BRANCH_2_1_2   = " << std::setw(11) << STAT_NB_BRANCH_2_1_2
              << "\n";
    std::cerr << "NB_BRANCH_2_1_2   = " << std::setw(11) << STAT_NB_BRANCH_2_1_2
              << "\n";
    std::cerr << "NB_BRANCH_2_1_2_1 = " << std::setw(11)
              << STAT_NB_BRANCH_2_1_2_1 << "\n";
    std::cerr << "NB_BRANCH_2_1_2_2 = " << std::setw(11)
              << STAT_NB_BRANCH_2_1_2_2 << "\n";
    std::cerr << "NB_BRANCH_2_2     = " << std::setw(11) << STAT_NB_BRANCH_2_2
              << "\n";
    std::cerr << "NB_BRANCH_2_2_1   = " << std::setw(11) << STAT_NB_BRANCH_2_2_1
              << "\n";
    std::cerr << "NB_BRANCH_2_2_1_1 = " << std::setw(11)
              << STAT_NB_BRANCH_2_2_1_1 << "\n";
    std::cerr << "NB_BRANCH_2_2_2   = " << std::setw(11) << STAT_NB_BRANCH_2_2_2
              << "\n";
    std::cerr << "NB_BRANCH_2_2_2_1 = " << std::setw(11)
              << STAT_NB_BRANCH_2_2_2_1 << "\n";
  }
#endif //TIME_TEXCOORDS

#ifdef DEBUG_TEXCOORDS
  { //DEBUG

    //check if all points tex coords set
    size_t numZeroTexCoords = 0;
    for (uint32_t i = 0; i < mesh.numVertices; ++i) {
      numZeroTexCoords +=
        (mesh.texCoords[2 * i + 0] == 0 && mesh.texCoords[2 * i + 1] == 0);
    }

#ifdef CHECK_TEXCOORDS
    size_t numVtxNotProcessed = 0;
    std::map<uint32_t, uint32_t>::iterator v_t_idx_iterator;
    for (uint32_t i = 0; i < mesh.numVertices; ++i) {
      v_t_idx_iterator = m.find(i);
      if (v_t_idx_iterator == m.end()) {
        ++numVtxNotProcessed;
      }
    }
    std::cerr << "numVtxNotProcessed             = " << numVtxNotProcessed
              << "\n";
#endif
    std::cerr << "numZeroTexCoords               = " << numZeroTexCoords
              << "\n";
    std::cerr << "mesh.numtexcoords-DBG_nCoords="
              << mesh.numVertices - DBG_nCoords
              << " (mesh.numtexcoords=" << mesh.numVertices
              << " DBG_nCoords=" << DBG_nCoords << ")\n";

  }    //DEBUG
#endif //DEBUG_TEXCOORDS

    /*
        //B: remplissage de mesh.triangles[i].tindices[k]
        //B:TODO:OPTIM: is a memcpy possible ???
        for (int i=0; i<mesh.numTriangles; ++i) {
          for (int k=0; k<3; ++k) {
            mesh.triangles[i].tindices[k] = mesh.triangles[i].vindices[k];
          }
        }
        */

    /*
          //code d'origine
        std::map<int, int>::iterator itera = v_t_idx.begin();             // build a triangle : f : v1/t1 v2/t2 v3/t3
        for(int i=0;i<model->numTriangles;i++){
            for(int k=0;k<3;k++){
                itera = v_t_idx.find(mesh.triangles[i].vindices[k]);
                if(itera!=v_t_idx.end()){
                    mesh.triangles[i].tindices[k] = (*itera).second;
                }
            }
        }
         */

#ifdef CHECK_TEXCOORDS
  checkTexCoords(mesh);
//debugTexCoords(mesh);
#endif

#ifdef DEBUG_TEXCOORDS
  { //DEBUG

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < mesh.numVertices; ++i) {
      uint32_t v_idx = i;
      float x = mesh.vertices[3 * v_idx + 0];
      float y = mesh.vertices[3 * v_idx + 1];
      //float z = mesh.vertices[3*v_idx+2];

      if (x >= G_x0 && x <= G_x1 && y >= G_y0 && y <= G_y1)
        indices.push_back(i);
    }

    std::sort(indices.begin(), indices.end(), DEBUG_VertexIndexSorter(mesh));

    std::cerr << "---------------\n";
    const size_t numIndices = indices.size();
    std::cerr << numIndices << " vertices in x=[" << std::setprecision(9)
              << G_x0 << "; " << std::setprecision(9) << G_x1 << "] && y=["
              << std::setprecision(9) << G_y0 << "; " << std::setprecision(9)
              << G_y1 << "]\n";
    for (size_t i = 0; i < numIndices; ++i) {
      uint32_t v_idx = indices[i];
      float x = mesh.vertices[3 * v_idx + 0];
      float y = mesh.vertices[3 * v_idx + 1];
      float z = mesh.vertices[3 * v_idx + 2];
      uint32_t t_idx = v_idx;
      float u = mesh.texCoords[2 * t_idx + 0];
      float v = mesh.texCoords[2 * t_idx + 1];

      std::cerr << " y=" << std::setprecision(9) << y
                << " x=" << std::setprecision(9) << x
                << " z=" << std::setprecision(9) << z << " v_idx=" << v_idx
                << " u=" << u << " v=" << v << "\n";
    }
    std::cerr << "---------------\n";

  }    //DEBUG
#endif //DEBUG_TEXCOORDS

  //t = (double)cv::getTickCount();

  mesh.computeNormals();

  //t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cerr<<"time compute normals: "<<t<<"s\n";

  //t = (double)cv::getTickCount();

  mesh.unitize();

  //t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  //std::cerr<<"time makeUnitary: "<<t<<"s\n";
}
