#include "TexCoordComputationCommon.hpp"

#include <cassert>
#include <iostream>

#include "Mesh.hpp"

void
checkTexCoords(const Mesh &mesh)
{
  if (!mesh.hasTexCoords())
    return;

  const uint32_t numtexcoords = mesh.numVertices;
  const uint32_t numTriangles = mesh.numTriangles;
  const uint32_t *triangles = mesh.triangles;

  const float eps = 0.000001f;
  uint32_t nb_eps = 0;

  uint32_t min_tri_idx = numTriangles;
  float min_dist = std::numeric_limits<float>::max();
  uint32_t nb_min = 0;

  for (uint32_t i = 0; i < numTriangles; ++i) {

    const uint32_t t_id_0 = triangles[3 * i + 0];
    const uint32_t t_id_1 = triangles[3 * i + 1];
    const uint32_t t_id_2 = triangles[3 * i + 2];

    if (t_id_0 > numtexcoords || t_id_1 > numtexcoords ||
        t_id_2 > numtexcoords) {
      std::cerr << "ERROR: invalid texcoord for triangle i=" << i
                << " t_id_0=" << t_id_0 << " t_id_1=" << t_id_1
                << " t_id_2=" << t_id_2 << " for numtexcoords=" << numtexcoords
                << "\n";
      exit(10);
    }

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

    float d = std::min(d01, std::min(d02, d12));
    if (d < min_dist) {
      min_dist = d;
      min_tri_idx = i;
      nb_min = 1;
    } else if (d == min_dist) {
      ++nb_min;
    }

    if (d < eps) {
      ++nb_eps;
    }
  }

  std::cerr << "\n";
  std::cerr << "For triangle " << min_tri_idx
            << "  min texcoord dist=" << min_dist << " [for " << nb_min
            << " triangles]\n";
  {
    uint32_t i = min_tri_idx;

    const uint32_t v_id_0 = triangles[3 * i + 0];
    const uint32_t v_id_1 = triangles[3 * i + 1];
    const uint32_t v_id_2 = triangles[3 * i + 2];

    const uint32_t t_id_0 = triangles[3 * i + 0];
    const uint32_t t_id_1 = triangles[3 * i + 1];
    const uint32_t t_id_2 = triangles[3 * i + 2];

    float v0_x = mesh.vertices[3 * v_id_0 + 0];
    float v0_y = mesh.vertices[3 * v_id_0 + 1];
    float v0_z = mesh.vertices[3 * v_id_0 + 2];
    float v0_tx = mesh.texCoords[2 * t_id_0 + 0];
    float v0_ty = mesh.texCoords[2 * t_id_0 + 1];

    float v1_x = mesh.vertices[3 * v_id_1 + 0];
    float v1_y = mesh.vertices[3 * v_id_1 + 1];
    float v1_z = mesh.vertices[3 * v_id_1 + 2];
    float v1_tx = mesh.texCoords[2 * t_id_1 + 0];
    float v1_ty = mesh.texCoords[2 * t_id_1 + 1];

    float v2_x = mesh.vertices[3 * v_id_2 + 0];
    float v2_y = mesh.vertices[3 * v_id_2 + 1];
    float v2_z = mesh.vertices[3 * v_id_2 + 2];
    float v2_tx = mesh.texCoords[2 * t_id_2 + 0];
    float v2_ty = mesh.texCoords[2 * t_id_2 + 1];

    float di01 = (v0_x - v1_x) * (v0_x - v1_x) + (v0_y - v1_y) * (v0_y - v1_y) +
                 (v0_z - v1_z) * (v0_z - v1_z);
    float di02 = (v0_x - v2_x) * (v0_x - v2_x) + (v0_y - v2_y) * (v0_y - v2_y) +
                 (v0_z - v2_z) * (v0_z - v2_z);
    float di12 = (v1_x - v2_x) * (v1_x - v2_x) + (v1_y - v2_y) * (v1_y - v2_y) +
                 (v1_z - v2_z) * (v1_z - v2_z);

    float d01 =
      (v0_tx - v1_tx) * (v0_tx - v1_tx) + (v0_ty - v1_ty) * (v0_ty - v1_ty);
    float d02 =
      (v0_tx - v2_tx) * (v0_tx - v2_tx) + (v0_ty - v2_ty) * (v0_ty - v2_ty);
    float d12 =
      (v1_tx - v2_tx) * (v1_tx - v2_tx) + (v1_ty - v2_ty) * (v1_ty - v2_ty);

    std::cerr << "texcoords dists : d01=" << d01 << " d02=" << d02
              << " d12=" << d12 << "\n";
    std::cerr << "vertices dists : d01=" << di01 << " d02=" << di02
              << " d12=" << di12 << "\n";
    std::cerr << "v0: x=" << v0_x << " y=" << v0_y << " z=" << v0_z
              << " tx=" << v0_tx << " ty=" << v0_ty << "\n";
    std::cerr << "v1: x=" << v1_x << " y=" << v1_y << " z=" << v1_z
              << " tx=" << v1_tx << " ty=" << v1_ty << "\n";
    std::cerr << "v2: x=" << v2_x << " y=" << v2_y << " z=" << v2_z
              << " tx=" << v2_tx << " ty=" << v2_ty << "\n";
  }
  std::cerr << "###> " << nb_eps << " triangles/" << numTriangles << "="
            << (nb_eps * 100.f) / (float)numTriangles
            << "% with dist between texcoords < " << eps << "\n";

  std::cerr << "\n";
}

void
moveTo_X_min(Mesh &mesh, float min_x)
{
  assert(mesh.isValid());

  const uint32_t numvertices = mesh.numVertices;
  float *vertices = mesh.vertices;
  for (uint32_t i = 0; i < numvertices; ++i) {
    vertices[3 * i + 0] -= min_x;
  }
}

float
moveTo_X_min(Mesh &mesh)
{
  assert(mesh.isValid());

  //compute min X coords
  float min_x = std::numeric_limits<float>::max();
  const uint32_t numvertices = mesh.numVertices;
  const float *vertices = mesh.vertices;
  for (uint32_t i = 0; i < numvertices; ++i) {
    if (vertices[3 * i + 0] < min_x) {
      min_x = vertices[3 * i + 0];
    }
  }
  //std::cerr<<"moveTo_X_min() min_x="<<min_x<<"\n";

  moveTo_X_min(mesh, min_x);

  return min_x;
}

void
moveTo_XZ_min(Mesh &mesh)
{
  float min_x = std::numeric_limits<float>::max();
  float min_z = std::numeric_limits<float>::max();

  const uint32_t numvertices = mesh.numVertices;
  float *vertices = mesh.vertices;

  //compute min X & Z coords
  for (uint32_t i = 0; i < numvertices; ++i) {
    if (vertices[3 * i + 0] < min_x) {
      min_x = vertices[3 * i + 0];
    }
    if (vertices[3 * i + 2] < min_z) {
      min_z = vertices[3 * i + 2];
    }
  }
  std::cerr << "moveTo_XZ_min() min_x=" << min_x << " min_z=" << min_z << "\n";

  // translate X coords
  for (uint32_t i = 0; i < numvertices; ++i) {
    vertices[3 * i + 0] -= min_x;
    vertices[3 * i + 2] -= min_z;
  }
}

void
normalizeTexCoords(Mesh &mesh)
{
  assert(mesh.hasTexCoords());

  float *texcoords = mesh.texCoords;
  const uint32_t numtexcoords = mesh.numVertices;

  //search min/max of texcoords
  float tex_min_x = texcoords[2 * 0 + 0];
  float tex_min_y = texcoords[2 * 0 + 1];
  float tex_max_x = texcoords[2 * 0 + 0];
  float tex_max_y = texcoords[2 * 0 + 1];

  //std::cerr<<"#normalizeTexCoords init: min_x="<<tex_min_x<<" max_x="<<tex_max_x<<" min_y="<<tex_min_y<<" max_y="<<tex_max_y<<"\n";

  for (uint32_t i = 1; i < numtexcoords; ++i) { //start from 1
    const float tx = texcoords[2 * i + 0];
    if (tx < tex_min_x)
      tex_min_x = tx;
    else if (tx > tex_max_x)
      tex_max_x = tx;

    const float ty = texcoords[2 * i + 1];
    if (ty < tex_min_y)
      tex_min_y = ty;
    else if (ty > tex_max_y)
      tex_max_y = ty;
  }

  const float width = (tex_max_x - tex_min_x);
  const float height = (tex_max_y - tex_min_y);

  const float f_x = width > 0 ? 1.f / width : 1.f;
  const float f_y = height > 0 ? 1.f / height : 1.f;

#ifndef NDEBUG
  std::cerr << "#normalizeTexCoords: texcoords min_x=" << tex_min_x
            << " max_x=" << tex_max_x << " min_y=" << tex_min_y
            << " max_y=" << tex_max_y << "\n";
  std::cerr << "#normalizeTexCoords: => w=" << width << " h=" << height
            << " => f_x=" << f_x << " f_y=" << f_y << "\n";
#endif //NDEBUG

  for (uint32_t i = 0; i < numtexcoords; ++i) {
    texcoords[2 * i + 0] = (texcoords[2 * i + 0] - tex_min_x) * f_x;
    texcoords[2 * i + 1] = (texcoords[2 * i + 1] - tex_min_y) * f_y;
  }

#ifndef NDEBUG
  { //DEBUG
    float texMin_x = texcoords[2 * 0 + 0];
    float texMin_y = texcoords[2 * 0 + 1];
    float texMax_x = texcoords[2 * 0 + 0];
    float texMax_y = texcoords[2 * 0 + 1];
    for (uint32_t i = 1; i < numtexcoords; ++i) { //start from 1
      const float tx = texcoords[2 * i + 0];
      if (tx < texMin_x)
        texMin_x = tx;
      else if (tx > texMax_x)
        texMax_x = tx;

      const float ty = texcoords[2 * i + 1];
      if (ty < texMin_y)
        texMin_y = ty;
      else if (ty > texMax_y)
        texMax_y = ty;
    }
    std::cerr << "#normalizeTexCoords: after: tex_min_x=" << texMin_x
              << " tex_min_y=" << texMin_y << " tex_max_x=" << texMax_x
              << " tex_max_y=" << texMax_y << "\n";
  }    //DEBUG end
#endif //NDEBUG
}

/*
  Do not use the exact tex_max_x
  but compute an approximate tex_max_x from several of the large tex_x.

 */
void
normalizeTexCoordsB(Mesh &mesh)
{
  assert(mesh.hasTexCoords());

  float *texcoords = mesh.texCoords;
  const uint32_t numtexcoords = mesh.numVertices;

  //search min/max of texcoords
  float tex_min_x = texcoords[2 * 0 + 0];
  float tex_min_y = texcoords[2 * 0 + 1];
  float tex_max_x = texcoords[2 * 0 + 0];
  float tex_max_y = texcoords[2 * 0 + 1];

  std::vector<float> xs(mesh.numVertices);
  for (uint32_t i = 0; i < numtexcoords; ++i) {

    const float tx = texcoords[2 * i + 0];
    if (tx < tex_min_x)
      tex_min_x = tx;
    else if (tx > tex_max_x)
      tex_max_x = tx;

    xs[i] = tx;

    const float ty = texcoords[2 * i + 1];
    if (ty < tex_min_y)
      tex_min_y = ty;
    else if (ty > tex_max_y)
      tex_max_y = ty;
  }
  const float percent = 0.005f; //percentage for xs used to compute xMax
  const uint32_t min_num = 2;
  //const uint32_t max_num = 200;
  //const uint32_t num_x_for_maxX = std::min(std::max(static_cast<uint32_t>(percent * numtexcoords), min_num), max_num);
  const uint32_t num_x_for_maxX =
    std::max(static_cast<uint32_t>(percent * numtexcoords), min_num);
  assert(num_x_for_maxX < numtexcoords);

  std::vector<float>::iterator itM =
    xs.begin() + (numtexcoords - num_x_for_maxX);
  std::partial_sort(xs.begin(), itM, xs.end());
  float m0 = 0, m1 = 0, m2 = 0;
  for (std::vector<float>::iterator it = itM; it != xs.end(); ++it) {
    ++m0;
    float x = *it;
    m1 += x;
    m2 += x * x;
  }
  if (m0 > 0) {
    float inv_m0 = 1.f / m0;
    float mean = m1 * inv_m0;
    float stdDev = sqrt((m2 - m1 * m1 * inv_m0) * inv_m0);
    float xmax = mean + 1.2f * stdDev;
#ifndef NDEBUG
    std::cerr << "#normalizeTexCoordsB: tex_max_x=" << tex_max_x
              << " num_x_for_maxX=" << num_x_for_maxX << "/" << numtexcoords
              << " : mean=" << mean << " stdDev=" << stdDev << " xmax=" << xmax
              << "\n";
#endif //NDEBUG
    if (xmax < tex_max_x) {
      tex_max_x = xmax;
    }
  }

  const float width = (tex_max_x - tex_min_x);
  const float height = (tex_max_y - tex_min_y);

  const float f_x = width > 0 ? 1.f / width : 1.f;
  const float f_y = height > 0 ? 1.f / height : 1.f;

#ifndef NDEBUG
  std::cerr << "#normalizeTexCoordsB: texcoords min_x=" << tex_min_x
            << " max_x=" << tex_max_x << " min_y=" << tex_min_y
            << " max_y=" << tex_max_y << "\n";
  std::cerr << "#normalizeTexCoordsB: => w=" << width << " h=" << height
            << " => f_x=" << f_x << " f_y=" << f_y << "\n";
#endif //NDEBUG

  for (uint32_t i = 0; i < numtexcoords; ++i) {
    texcoords[2 * i + 0] = (texcoords[2 * i + 0] - tex_min_x) * f_x;
    texcoords[2 * i + 1] = (texcoords[2 * i + 1] - tex_min_y) * f_y;
  }

#ifndef NDEBUG
  { //DEBUG
    float texMin_x = texcoords[2 * 0 + 0];
    float texMin_y = texcoords[2 * 0 + 1];
    float texMax_x = texcoords[2 * 0 + 0];
    float texMax_y = texcoords[2 * 0 + 1];
    for (uint32_t i = 1; i < numtexcoords; ++i) { //start from 1
      const float tx = texcoords[2 * i + 0];
      if (tx < texMin_x)
        texMin_x = tx;
      else if (tx > texMax_x)
        texMax_x = tx;

      const float ty = texcoords[2 * i + 1];
      if (ty < texMin_y)
        texMin_y = ty;
      else if (ty > texMax_y)
        texMax_y = ty;
    }
    std::cerr << "#normalizeTexCoordsB: after: tex_min_x=" << texMin_x
              << " tex_min_y=" << texMin_y << " tex_max_x=" << texMax_x
              << " tex_max_y=" << texMax_y << "\n";
  }    //DEBUG end
#endif //NDEBUG
}

namespace {
struct SorterY_X
{
  explicit SorterY_X(const float *vertices)
    : m_vertices(vertices)
  {}

  //sort according to y then x.
  bool operator()(uint32_t i, uint32_t j) const
  {
    return (m_vertices[3 * i + 1] < m_vertices[3 * j + 1] ||
            (m_vertices[3 * i + 1] == m_vertices[3 * j + 1] &&
             m_vertices[3 * i + 0] < m_vertices[3 * j + 0]));
  }

  const float *m_vertices;
};
} //end anonymous namespace

/*
  Fill @a verticesIndices with indices of vertices sorted according to Y then X.
  Fill @a startYIndices with indices (in @a verticesIndices) of start of a new Y value.

  We have mesh.vertices[3*verticesIndices[startYIndices[i]]+1] < mesh.vertices[3*verticesIndices[startYIndices[i+1]]+1]
 */
void
getIntersectionPlanesVertices(const Mesh &mesh,
                              std::vector<uint32_t> &verticesIndices,
                              std::vector<uint32_t> &startYIndices)
{
  assert(mesh.isValid());

  const uint32_t numvertices = mesh.numVertices;
  assert(numvertices > 0);

  verticesIndices.resize(numvertices);
  for (uint32_t i = 0; i < numvertices; ++i) {
    verticesIndices[i] = i;
  }

  const float *vertices = mesh.vertices;

  std::sort(
    verticesIndices.begin(), verticesIndices.end(), SorterY_X(vertices));

  startYIndices.reserve(numvertices +
                        1); //worst case: all vertices have a different y.
  startYIndices.push_back(0);
  if (numvertices > 1) {
    const uint32_t ind0 = verticesIndices[0];
    float prev_y = vertices[3 * ind0 + 1];

    for (uint32_t i = 1; i < numvertices; ++i) { //start from 1
      const uint32_t ind = verticesIndices[i];
      const float y = vertices[3 * ind + 1];
      assert(prev_y <= y);
      if (y != prev_y) {
        startYIndices.push_back(i);
        prev_y = y;
      }
    }
  }
  startYIndices.push_back(verticesIndices.size());

  assert(startYIndices.size() <= numvertices + 1);

#ifndef NDEBUG
  {
    if (numvertices > 1) {

      //check that two vertices in the same range in @a startYIndices have the same Y, and have X in increasing order
      //and that ranges are in increasing Y order.

      const uint32_t ind0 = verticesIndices[0];
      float prev_y = vertices[3 * ind0 + 1] - 1; //inferior to all Ys !

      const size_t sz = startYIndices.size();
      assert(sz > 0);
      for (size_t i = 0; i < sz - 1; ++i) {
        const uint32_t start = startYIndices[i];
        const uint32_t end = startYIndices[i + 1];
        assert(start < end);
        assert(start < verticesIndices.size());
        assert(end <= verticesIndices.size());
        const uint32_t ind = verticesIndices[start];
        const float y = vertices[3 * ind + 1];
        float prev_x = vertices[3 * ind + 0];
        for (uint32_t j = start + 1; j < end; ++j) {
          const uint32_t indj = verticesIndices[j];
          const float yj = vertices[3 * indj + 1];
          assert(yj == y);
          const float xj = vertices[3 * indj + 0];
          assert(prev_x <= xj);
          prev_x = xj;
        }
        assert(prev_y < y);
        prev_y = y;
      }
    }
  }
#endif

  assert(startYIndices.size() <= verticesIndices.size() + 1);
  assert(!startYIndices.empty());
  assert(startYIndices[startYIndices.size() - 1] == verticesIndices.size());
}
