#include "TexCoordComputationB.hpp"

#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <iomanip> //DEBUG
#include <iostream>
#include <sstream>
#include <vector>

#include "Mesh.hpp"

#include "TexCoordComputationCommon.hpp"

#define CHECK_TEXCOORDS 1
//#define DEBUG_TEXCOORDS 1
//#define TIME_TEXCOORDS 1

//#define VERBOSE 1

struct X_Z
{
  float x;
  float z;
};

struct X_Z_e
{
  float x;
  float z;
  uint32_t eIdx;

  X_Z_e()
    : x(0)
    , z(0)
    , eIdx(-1)
  {}

  explicit X_Z_e(X_Z xz, uint32_t ei = -1)
    : x(xz.x)
    , z(xz.z)
    , eIdx(ei)
  {}
};

struct X_Z_e_SorterX_Z
{
  inline bool operator()(X_Z_e a, X_Z_e b) const
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

struct IndiceSorterE
{
  explicit IndiceSorterE(const std::vector<uint32_t> &bins)
    : m_bins(bins)
  {}

  inline bool operator()(uint32_t a, uint32_t b) const
  {
    return m_bins[a] < m_bins[b];
  }

private:
  const std::vector<uint32_t> &m_bins;
};

template<typename EDGE>
class SpacePartionnerYEdges
{
public:
  SpacePartionnerYEdges()
    : m_numBins(0)
    , m_step(0)
  {}

  SpacePartionnerYEdges(size_t numYs,
                        const std::vector<uint32_t> &vidx2yPlaneIdx,
                        const std::vector<EDGE> &edges)
    : m_numBins(0)
    , m_step(0)
  {
    init(numYs, vidx2yPlaneIdx, edges);
  }

  void init(size_t numYs,
            const std::vector<uint32_t> &vidx2yPlaneIdx,
            const std::vector<EDGE> &edges)
  {
    const size_t numEdges = edges.size();
    const size_t numVertices = vidx2yPlaneIdx.size();

    const int numEdgesPerBin = 1024; //approximate/wished for

    m_numBins = std::max((size_t)1, numEdges / numEdgesPerBin);
    m_step = numYs / static_cast<float>(m_numBins);

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
    const size_t s = edgesBins.size();
    std::vector<uint32_t> indices(s);
    for (uint32_t i = 0; i < s; ++i) {
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

    m_indices.resize(s);
    for (uint32_t i = 0; i < s; ++i) {
      m_indices[i] = edgesIdx[indices[i]];
    }

    m_binStarts.resize(m_numBins + 1);
    m_binStarts[0] = 0;
    uint32_t currBin = 0;
    uint32_t binIdx = 1;
    for (uint32_t i = 1; i < s; ++i) { //start from 1
      uint32_t bin = edgesBins[indices[i]];
      while (currBin < bin) {
        //std::cerr<<"m_binStarts["<<binIdx<<"]="<<i<<"\n";
        m_binStarts[binIdx] = i;
        ++currBin;
        ++binIdx;
      }
    }
    for (; binIdx <= m_numBins; ++binIdx) {
      m_binStarts[binIdx] = s;
      //std::cerr<<"m_binStarts["<<binIdx<<"]="<<s<<" !\n";
    }
      //m_binStarts[m_numBins] = s;

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

#ifdef DEBUG_TEXCOORDS
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

    }  //DEBUG
#endif //DEBUG_TEXCOORDS

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

namespace {
struct VSorter
{
  const uint32_t *m_tris;

  explicit VSorter(const uint32_t *tris)
    : m_tris(tris)
  {}

  inline bool operator()(uint32_t i1, uint32_t i2) const
  {
    return m_tris[i1] < m_tris[i2];
  }
};
} //end anonymous namespace

class Vertex2Tris
{
public:
  Vertex2Tris()
    : m_indices()
    , m_starts()
  {}

  explicit Vertex2Tris(const Mesh &m)
    : m_indices()
    , m_starts()
  {
    init(m);
  }

  void init(const Mesh &m) { init(m.triangles, m.numTriangles, m.numVertices); }

  void init(const uint32_t *tris, uint32_t numTriangles, uint32_t numVertices)
  {

    const uint32_t numTriangles3 = numTriangles * 3;

    std::vector<uint32_t> triIndices(numTriangles3);
    for (uint32_t i = 0; i < numTriangles; ++i) {
      triIndices[3 * i + 0] = i;
      triIndices[3 * i + 1] = i;
      triIndices[3 * i + 2] = i;
    }
    std::vector<uint32_t> indices(numTriangles3);
    for (uint32_t i = 0; i < numTriangles3; ++i) {
      indices[i] = i;
    }

    //sort indices according to triangles
    std::sort(indices.begin(), indices.end(), VSorter(tris));

    m_indices.resize(numTriangles3);
    for (uint32_t i = 0; i < numTriangles3; ++i) {
      m_indices[i] = triIndices[indices[i]];
    }

    m_starts.resize(numVertices + 1);

    assert(!indices.empty());
    uint32_t vidx0 = tris[indices[0]];
    uint32_t vidx;
    for (vidx = 0; vidx <= vidx0; ++vidx)
      m_starts[vidx] = 0;

    uint32_t prevVidx = vidx0;
    for (uint32_t i = 1; i < numTriangles3; ++i) {
      const uint32_t nextVidx = tris[indices[i]];
      while (prevVidx < nextVidx) {
        ++prevVidx;
        m_starts[prevVidx] = i;
      }
      //prevVidx = nextVidx;
      assert(prevVidx == nextVidx);
    }
    ++prevVidx;
    while (prevVidx <= numVertices) {
      m_starts[prevVidx] = numTriangles3;
      ++prevVidx;
    }
  }

  //get all triangles indices for vertex @a vIdx.
  void getTris(uint32_t vIdx,
               const uint32_t *&indices,
               uint32_t &numIndices) const
  {
    assert(vIdx < m_starts.size() - 1);
    const size_t start = m_starts[vIdx];
    const size_t end = m_starts[vIdx + 1];
    assert(start <= end);
    numIndices = end - start;
    indices = &(m_indices[start]);
  }

  //Check if vertex @a vIdx is in triangle @a triIdx,
  // that is if @a triIdx is in the list of triangles @a vIdx belongs to.
  //
  // !!! DUMB !!! We have this information directly in Mesh.triangles !!!!
  //
  bool isVertexInTri(uint32_t vIdx, uint32_t triIdx) const
  {
    assert(vIdx < m_starts.size() - 1);
    const size_t start = m_starts[vIdx];
    const size_t end = m_starts[vIdx + 1];
    assert(start <= end);
    for (size_t i = start; i < end; ++i) {
      if (m_indices[i] == triIdx)
        return true;
    }
    return false;
  }

  //check if vertices @a vIdx1 and @a vIdx2 are in the same triangle
  // that is share a triangle edge.
  bool inSameTriangle(uint32_t vIdx1, uint32_t vIdx2) const
  {
    assert(vIdx1 < m_starts.size() - 1);
    const size_t start1 = m_starts[vIdx1];
    const size_t end1 = m_starts[vIdx1 + 1];
    assert(vIdx2 < m_starts.size() - 1);
    const size_t start2 = m_starts[vIdx2];
    const size_t end2 = m_starts[vIdx2 + 1];
    for (size_t i = start1; i < end1; ++i) {
      const uint32_t ind_i = m_indices[i];
      for (size_t j = start2; j < end2; ++j) {
        if (ind_i == m_indices[j]) {
          return true;
        }
      }
    }
    return false;
  }

private:
  //we store a vector of vector as two vectors
  std::vector<uint32_t> m_indices;
  std::vector<size_t> m_starts;
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
  const float dist = std::sqrt(dx * dx + dz * dz);
  return dist;
}

struct Segment
{
  X_Z startPt;
  X_Z endPt;
  float length;
  uint32_t startTriIdx;
  uint32_t endTriIdx;

  Segment()
    : startPt()
    , endPt()
    , length(0)
    , startTriIdx(-1)
    , endTriIdx(-1)
  {}

  void swapStartEnd()
  {
    std::swap(endPt.x, startPt.x);
    std::swap(endPt.z, startPt.z);
    std::swap(endTriIdx, startTriIdx);
  }

  bool checkLength() const
  {
    constexpr float eps = std::numeric_limits<float>::epsilon();
    const float d = computeDist(startPt.x, startPt.z, endPt.x, endPt.z);
    if (length < d) {
      if (std::fabs(length - d) > eps) {
        std::cerr << "ERROR: Segment: length=" << std::setprecision(9) << length
                  << " < " << std::setprecision(9) << d << "\n";
        return false;
      }
    }
    return true;
  }
};

struct Segment_Sorter
{
  inline bool operator()(const Segment &s1, const Segment &s2) const
  {
    return s1.startPt.x < s2.startPt.x || s1.endPt.x < s2.startPt.x;
  }
};

void
chainAndMergeIntersectionPts(const std::vector<X_Z_e> &intersectionPoints,
                             const std::vector<EdgeF> &edges,
                             const Vertex2Tris &/*v2t*/, //B:useless ???
                             std::vector<Segment> &segments)
{
  segments.clear();

  const size_t numPts = intersectionPoints.size();
  if (numPts == 0)
    return;

  std::vector<bool> used(numPts,
                         false); //OPTIM: use char ?  allocate only once ?

  for (size_t i = 0; i < numPts; ++i) {
    if (!used[i]) {

      Segment s;
      s.startPt.x = intersectionPoints[i].x;
      s.startPt.z = intersectionPoints[i].z;
      s.endPt.x = intersectionPoints[i].x;
      s.endPt.z = intersectionPoints[i].z;
      s.length = 0;
      const uint32_t eIdx = intersectionPoints[i].eIdx;
      assert(eIdx < edges.size());
      s.startTriIdx = edges[eIdx].faceIndex[0];
      s.endTriIdx = edges[eIdx].faceIndex[1];

      for (size_t j = i + 1; j < numPts; ++j) {
        if (!used[j]) {
          const uint32_t eIdxj = intersectionPoints[j].eIdx;
          assert(eIdxj < edges.size());
          if (edges[eIdxj].faceIndex[0] != (uint32_t)-1 &&
              edges[eIdxj].faceIndex[0] == s.endTriIdx) {
            s.endTriIdx = edges[eIdxj].faceIndex[1];
            s.length += computeDist(s.endPt.x,
                                    s.endPt.z,
                                    intersectionPoints[j].x,
                                    intersectionPoints[j].z);
            s.endPt.x = intersectionPoints[j].x;
            s.endPt.z = intersectionPoints[j].z;
            used[j] = true;
          } else if (edges[eIdxj].faceIndex[1] != (uint32_t)-1 &&
                     edges[eIdxj].faceIndex[1] == s.startTriIdx) {
            s.startTriIdx = edges[eIdxj].faceIndex[0];
            s.length += computeDist(s.startPt.x,
                                    s.startPt.z,
                                    intersectionPoints[j].x,
                                    intersectionPoints[j].z);
            s.startPt.x = intersectionPoints[j].x;
            s.startPt.z = intersectionPoints[j].z;
            used[j] = true;
          } else if (edges[eIdxj].faceIndex[0] != (uint32_t)-1 &&
                     edges[eIdxj].faceIndex[0] == s.startTriIdx) {
            s.startTriIdx = edges[eIdxj].faceIndex[1];
            s.length += computeDist(s.startPt.x,
                                    s.startPt.z,
                                    intersectionPoints[j].x,
                                    intersectionPoints[j].z);
            s.startPt.x = intersectionPoints[j].x;
            s.startPt.z = intersectionPoints[j].z;
            used[j] = true;
          } else if (edges[eIdxj].faceIndex[1] != (uint32_t)-1 &&
                     edges[eIdxj].faceIndex[1] == s.endTriIdx) {
            s.endTriIdx = edges[eIdxj].faceIndex[0];
            s.length += computeDist(s.endPt.x,
                                    s.endPt.z,
                                    intersectionPoints[j].x,
                                    intersectionPoints[j].z);
            s.endPt.x = intersectionPoints[j].x;
            s.endPt.z = intersectionPoints[j].z;
            used[j] = true;
          }
        }
      }

      segments.push_back(s);
    }
  }

  bool change = false;
  do {
    change = false;
    assert(!segments.empty());
    for (std::vector<Segment>::iterator it = segments.begin();
         it != segments.end();
         ++it) {

      Segment &s = *it;
      if (s.startTriIdx != (uint32_t)-1 || s.endTriIdx != (uint32_t)-1) {

        for (std::vector<Segment>::iterator itj = it + 1;
             itj != segments.end();) {

          const Segment &sj = *itj;

          bool merged = false;

          if (s.endTriIdx != (uint32_t)-1 && s.endTriIdx == sj.startTriIdx) {
            s.length +=
              computeDist(s.endPt.x, s.endPt.z, sj.startPt.x, sj.startPt.z);
            s.length += sj.length;
            s.endTriIdx = sj.endTriIdx;
            s.endPt.x = sj.endPt.x;
            s.endPt.z = sj.endPt.z;
            merged = true;
          } else if (s.startTriIdx != (uint32_t)-1 &&
                     s.startTriIdx == sj.endTriIdx) {
            s.length +=
              computeDist(sj.endPt.x, sj.endPt.z, s.startPt.x, s.startPt.z);
            s.length += sj.length;
            s.startTriIdx = sj.startTriIdx;
            s.startPt.x = sj.startPt.x;
            s.startPt.z = sj.startPt.z;
            merged = true;
          } else if (s.startTriIdx != (uint32_t)-1 &&
                     s.startTriIdx == sj.startTriIdx) {
            s.length +=
              computeDist(sj.startPt.x, sj.startPt.z, s.startPt.x, s.startPt.z);
            s.length += sj.length;
            s.startTriIdx = sj.endTriIdx;
            s.startPt.x = sj.endPt.x;
            s.startPt.z = sj.endPt.z;
            merged = true;
          } else if (s.endTriIdx != (uint32_t)-1 &&
                     s.endTriIdx == sj.endTriIdx) {
            s.length +=
              computeDist(s.endPt.x, s.endPt.z, sj.endPt.x, sj.endPt.z);
            s.length += sj.length;
            s.endTriIdx = sj.startTriIdx;
            s.endPt.x = sj.startPt.x;
            s.endPt.z = sj.startPt.z;
            merged = true;
          }

          if (!merged) {
            ++itj;
          } else {
            itj = segments.erase(itj);
            change = true;
          }
        }
      }
    }

  } while (change);

  for (Segment &s : segments) {
    assert(s.checkLength());
    if (s.endPt.x < s.startPt.x) {
      s.swapStartEnd();
      assert(s.checkLength());
    }
  }

  std::sort(segments.begin(), segments.end(), Segment_Sorter());
}

struct Path : public Segment
{
  std::deque<uint32_t> vIndices;
  std::deque<float> lengths;
  bool startIsTri;
  bool endIsTri;

  //lengths.size() == vIndices.size() && lengths[i] is the legnth for vidx vIndices[i].

  //startIsTri & endIsTri indicate if startTriIdx & endTriIdx are indices of triangles (if true)
  // or vertices (if false).

  void set(const Segment &s)
  {
    assert(s.checkLength());

    startPt = s.startPt;
    endPt = s.endPt;
    startTriIdx = s.startTriIdx;
    endTriIdx = s.endTriIdx;
    length = s.length;
    startIsTri = true;
    endIsTri = true;

    assert(checkLengths());
  }

  void swapStartEnd()
  {
    assert(checkLengths());

#ifdef VERBOSE
    { //DEBUG
      std::cerr << "before swapStartEnd(): length=" << length << "\n";
      std::cerr << lengths.size() << " lengths: ";
      for (float l : lengths)
        std::cerr << l << " ";
      std::cerr << "\n";
      std::cerr << lengths.size() << " indices: ";
      for (uint32_t i : vIndices)
        std::cerr << i << " ";
      std::cerr << "\n";
    }
#endif //VERBOSE

    for (float &l : lengths) {
      assert(l <= length);
      l = length - l;
    }
    const size_t sz = vIndices.size();
    const size_t hsz = sz / 2;
    for (size_t i = 0; i < hsz; ++i) {
      std::swap(vIndices[i], vIndices[sz - 1 - i]);
      std::swap(lengths[i], lengths[sz - 1 - i]);
    }

    std::swap(startPt, endPt);
    std::swap(startTriIdx, endTriIdx);
    std::swap(startIsTri, endIsTri);

#ifdef VERBOSE
    { //DEBUG
      std::cerr << "after swapStartEnd(): length=" << length << "\n";
      std::cerr << lengths.size() << " lengths: ";
      for (float l : lengths)
        std::cerr << l << " ";
      std::cerr << "\n";
      std::cerr << lengths.size() << " indices: ";
      for (uint32_t i : vIndices)
        std::cerr << i << " ";
      std::cerr << "\n";
    }
#endif //VERBOSE

    assert(checkLengths());
  }

  bool checkLengths() const
  {
    if (lengths.size() != vIndices.size()) {
      std::cerr << "ERROR: difference in sizes: lengths=" << lengths.size()
                << " indices=" << vIndices.size() << "\n";
      return false;
    }

    const size_t sz = vIndices.size();
    for (size_t i = 1; i < sz; ++i) { //start from 1
      if (lengths[i] <= lengths[i - 1]) {
        constexpr float eps = std::numeric_limits<float>::epsilon();
        if (std::fabs(lengths[i] - lengths[i - 1]) > eps) {
          std::cerr << "ERROR: lengths[" << i << "]=" << std::setprecision(9)
                    << lengths[i] << " <= lengths[" << i - 1
                    << "]=" << std::setprecision(9) << lengths[i - 1] << "\n";
          return false;
        }
      }
    }
    if (sz > 0) {
      if (lengths[sz - 1] > length) {
        constexpr float eps = 2 * std::numeric_limits<float>::epsilon();
        if (std::fabs(lengths[sz - 1] - length) > eps) {
          std::cerr << "ERROR: lengths[" << sz - 1
                    << "]=" << std::setprecision(9) << lengths[sz - 1]
                    << " > length=" << std::setprecision(9) << length << "\n";
          std::cerr << "   fabs(lengths[sz-1]-length)=" << std::setprecision(9)
                    << std::fabs(lengths[sz - 1] - length)
                    << " > eps=" << std::setprecision(9) << eps << "\n";
          return false;
        }
      }
    }

    {
      constexpr float eps = 2 * std::numeric_limits<float>::epsilon();
      const float d = computeDist(startPt.x, startPt.z, endPt.x, endPt.z);
      if (length < d) {
        if (std::fabs(length - d) > eps) {
          std::cerr << "ERROR: length=" << std::setprecision(9) << length
                    << " < computeDist(startPt.x, startPt.z, endPt.x, endPt.z)="
                    << std::setprecision(9) << d << "\n";
          return false;
        }
      }
    }

    if (!startIsTri) {
      assert(!vIndices.empty());
      assert(!lengths.empty());
      if (lengths[0] > std::numeric_limits<float>::epsilon()) {
        std::cerr << "ERROR: !isStartTri && lengths[0]=" << lengths[0]
                  << " > eps=" << std::numeric_limits<float>::epsilon() << "\n";
        return false;
      }
    }

    return true;
  }

  bool checkLengths(const float *vertices, uint32_t numVertices) const
  {
    const size_t sz = vIndices.size();
    for (size_t i = 0; i < sz; ++i) {
      const uint32_t vIdx = vIndices[i];
      if (vIdx >= numVertices) {
        std::cerr << "ERROR: vIdx= " << vIdx
                  << " >= numVertices=" << numVertices << "\n";
        return false;
      }

      constexpr float eps = std::numeric_limits<float>::epsilon();
      const float d = computeDist(
        startPt.x, startPt.z, vertices[3 * vIdx + 0], vertices[3 * vIdx + 2]);
      if (lengths[i] < d) {
        if (std::fabs(lengths[i] - d) > eps) {
          std::cerr << "ERROR: lengths[" << i << "]=" << std::setprecision(9)
                    << lengths[i] << " < computeDist(" << startPt.x << ", "
                    << startPt.z << ", " << vertices[3 * vIdx + 0] << ", "
                    << vertices[3 * vIdx + 2] << "=" << std::setprecision(9)
                    << d << " eps=" << std::setprecision(9) << eps << "\n";
          return false;
        }
      }
    }

    if (!startIsTri) {
      const uint32_t vIdx = startTriIdx;
      if (vIdx >= numVertices) {
        std::cerr << "ERROR: ! startIsTri && vIdx= " << vIdx
                  << " >= numVertices=" << numVertices << "\n";
        return false;
      }
    }
    if (!endIsTri) {
      const uint32_t vIdx = endTriIdx;
      if (vIdx >= numVertices) {
        std::cerr << "ERROR: ! endIsTri && vIdx= " << vIdx
                  << " >= numVertices=" << numVertices << "\n";
        return false;
      }
    }

    return true;
  }
};

/*
  Fuse all paths in first one.

  Suppose paths are sorted.
 */
static void
fusePaths(std::vector<Path> &paths)
{
  size_t numPaths = paths.size();
  if (numPaths < 2)
    return;

  std::vector<Path>::iterator it0 = paths.begin();
  Path &p0 = *it0;

  assert(p0.checkLengths());

  for (std::vector<Path>::iterator it = paths.begin() + 1; it != paths.end();
       ++it) {

    Path &p1 = *it;
    assert(p1.checkLengths());

    const float l =
      computeDist(p0.endPt.x, p0.endPt.z, p1.startPt.x, p1.startPt.z);

    const float la = p0.length + l;

    for (float length : p1.lengths)
      p0.lengths.push_back(length + la);
    for (uint32_t vidx : p1.vIndices)
      p0.vIndices.push_back(vidx);

    p0.endPt.x = p1.endPt.x;
    p0.endPt.z = p1.endPt.z;
    p0.endIsTri = p1.endIsTri;
    p0.endTriIdx = p1.endTriIdx;
    p0.length += l + p1.length;
    if (!p0.lengths.empty())
      p0.length = std::max(p0.length, p0.lengths.back());

    assert(p0.checkLengths());
  }

  paths.resize(1);
}

/*
  Merge segments and intersected vertices in paths.


  @param vertices   original vertex of mesh.
  @param verticesIndices  indices of intersected vertices.


 */
static void
getPaths(const uint32_t *tris,
         uint32_t
#ifndef NDEBUG
	 numTris
#endif //NDEBUG
	 ,
         const float *vertices,
         uint32_t
#ifndef NDEBUG
	 numVertices
#endif //NDEBUG
	 ,
         const uint32_t *verticesIndices,
         uint32_t numIndices,
         const std::vector<Segment> &segments,
         const Vertex2Tris &v2t,
         std::vector<Path> &paths)
{
  paths.clear();

  const uint32_t numSegments = segments.size();

  std::vector<bool> usedSegments(numSegments,
                                 false); //OPTIM: use char ? alloc once ?
  uint32_t numUsedSegments = 0;

  std::vector<bool> usedVertices(numIndices,
                                 false); //OPTIM: use char ? alloc once ?
  uint32_t numUsedVertices = 0;

  //std::cerr<<"*** numSegments="<<numSegments<<" numIndices="<<numIndices<<" ***\n";

  uint32_t is = 0;
  uint32_t iv = 0;
  while (numUsedSegments != numSegments || numUsedVertices != numIndices) {

    if (numUsedSegments != numSegments) {
      assert(is < numSegments);
      while (usedSegments[is])
        ++is;
      assert(is < numSegments && !usedSegments[is]);

      const Segment &s = segments[is];
      assert(s.checkLength());

      //first, seek if segment s can be "linked into" an existing Path
      size_t k = paths.size();
      if (s.endTriIdx != (uint32_t)-1 || s.startTriIdx != (uint32_t)-1) {

        for (k = 0; k < paths.size(); ++k) {
          //if (s.endTriIdx != -1 && paths[k].startIsTri && s.endTriIdx == paths[k].startTriIdx) {
          //if (s.startTriIdx != -1 && paths[k].endIsTri && s.startTriIdx == paths[k].endTriIdx) {

          if (s.startTriIdx != (uint32_t)-1 && !paths[k].endIsTri) {
            assert(paths[k].endTriIdx != (uint32_t)-1);
            assert(paths[k].endTriIdx < numTris);
            if (tris[3 * paths[k].endTriIdx + 0] == s.startTriIdx ||
                tris[3 * paths[k].endTriIdx + 1] == s.startTriIdx ||
                tris[3 * paths[k].endTriIdx + 2] == s.startTriIdx) {
              //path that ends with a vertex and this vertex belongs to the triangle starting the segment.

              assert(paths[k].checkLengths());

              const float l = computeDist(
                paths[k].endPt.x, paths[k].endPt.z, s.startPt.x, s.startPt.z);
              paths[k].length += l + s.length;
              paths[k].endPt.x = s.endPt.x;
              paths[k].endPt.z = s.endPt.z;
              paths[k].endIsTri = true;

              assert(paths[k].checkLengths());
              assert(paths[k].checkLengths(vertices, numVertices));

              break;
            }
          }

          if (s.endTriIdx != (uint32_t)-1 && !paths[k].startIsTri) {
            assert(paths[k].startTriIdx != (uint32_t)-1);
            assert(paths[k].startTriIdx < numTris);
            if (tris[3 * paths[k].startTriIdx + 0] == s.endTriIdx ||
                tris[3 * paths[k].startTriIdx + 1] == s.endTriIdx ||
                tris[3 * paths[k].startTriIdx + 2] == s.endTriIdx) {
              //path that starts with a vertex and this vertex belongs to the triangle ending the segment.

              assert(paths[k].checkLengths());

              const float l = computeDist(
                s.endPt.x, s.endPt.z, paths[k].startPt.x, paths[k].startPt.z);
              const float la = s.length + l;
              for (float &length : paths[k].lengths)
                length += la;
              paths[k].startPt.x = s.startPt.x;
              paths[k].startPt.z = s.startPt.z;
              paths[k].startIsTri = true;

              assert(paths[k].checkLengths());
              assert(paths[k].checkLengths(vertices, numVertices));

              break;
            }
          }
        }
      }

      if (k == paths.size()) {
        Path p;
        p.set(s);
        paths.push_back(p);
      }
      assert(k < paths.size());
      Path &p = paths[k];
      assert(p.checkLengths());

      //std::cerr<<"new path from segment "<<&p<<" ["<<k<<"] : p.startIsTri="<<p.startIsTri<<" triIdx="<<p.startTriIdx<<" p.startPt=("<<p.startPt.x<<", "<<p.startPt.z<<") p.endIsTri="<<p.endIsTri<<" triIdx="<<p.endTriIdx<<" p.endPt=("<<p.endPt.x<<", "<<p.endPt.z<<")\n";

      usedSegments[is] = true;
      ++numUsedSegments;
      ++is;

      while (iv < numIndices && usedVertices[iv])
        ++iv;
      //assert(iv < numIndices && ! usedVertices[iv]);

      for (uint32_t j = iv; j < numIndices; ++j) {

        //std::cerr<<"   j="<<j<<" usedVertices[j]="<<usedVertices[j]<<"\n";

        if (!usedVertices[j]) {

          assert(numUsedVertices < numIndices);

          const uint32_t vIdx = verticesIndices[j];
          assert(vIdx < numVertices);
          const float xV = vertices[3 * vIdx + 0];
          const float zV = vertices[3 * vIdx + 2];

          if (p.endTriIdx != (uint32_t)-1) {
            if ((p.endIsTri && (tris[3 * p.endTriIdx + 0] == vIdx ||
                                tris[3 * p.endTriIdx + 1] == vIdx ||
                                tris[3 * p.endTriIdx + 2] == vIdx)) ||
                (!p.endIsTri && (v2t.inSameTriangle(p.endTriIdx, vIdx)))) {

              //if p.endIsTri, vIdx is one of the vertices of triangle p.endTriIdx
              //else there is an edge between these two vertices (vIdx & p.endTriIdx)

              assert(!p.endIsTri || v2t.isVertexInTri(vIdx, p.endTriIdx));

              assert(p.checkLengths());
              assert(p.checkLengths(vertices, numVertices));

              const float l = computeDist(p.endPt.x, p.endPt.z, xV, zV);
              p.vIndices.push_back(vIdx);
              p.length += l;
              p.lengths.push_back(p.length);
              p.endPt.x = xV;
              p.endPt.z = zV;
              p.endIsTri = false;
              p.endTriIdx = vIdx;

              assert(p.checkLengths());
              assert(p.checkLengths(vertices, numVertices));

              assert(!usedVertices[j]);
#ifdef VERBOSE
              std::cerr << "add (end) vIdx=" << verticesIndices[j]
                        << " in path " << &p << " [" << k << "]\n";
#endif //VERBOSE

              usedVertices[j] = true;
              ++numUsedVertices;

              continue;
            }
          }

          if (p.startTriIdx != (uint32_t)-1) {
            if ((p.startIsTri && (tris[3 * p.startTriIdx + 0] == vIdx ||
                                  tris[3 * p.startTriIdx + 1] == vIdx ||
                                  tris[3 * p.startTriIdx + 2] == vIdx)) ||
                (!p.startIsTri && (v2t.inSameTriangle(p.startTriIdx, vIdx)))) {

              assert(!p.startIsTri || v2t.isVertexInTri(vIdx, p.startTriIdx));

              //if p.startIsTri, vIdx is one of the vertices of triangle p.startTriIdx
              //else there is an edge between these two vertices (vIdx & p.startTriIdx)

              assert(p.checkLengths());
              assert(p.checkLengths(vertices, numVertices));

              const float l = computeDist(p.startPt.x, p.startPt.z, xV, zV);
              for (float &length : p.lengths)
                length += l;
              p.length += l;
              p.vIndices.push_front(vIdx);
              p.lengths.push_front(0);
              p.startPt.x = xV;
              p.startPt.z = zV;
              p.startIsTri = false;
              p.startTriIdx = vIdx;

              assert(p.checkLengths());
              assert(p.checkLengths(vertices, numVertices));

              assert(!usedVertices[j]);
#ifdef VERBOSE
              std::cerr << "add (start) vIdx=" << verticesIndices[j]
                        << " in path " << &p << " [" << k << "]\n";
#endif //VERBOSE

              usedVertices[j] = true;
              ++numUsedVertices;

              continue;
            }
          }
          //DEBUG
          {
            //std::cerr<<" vIdx="<<vIdx<<" is not linked to path ["<<k<<"] : start isTri="<<p.startIsTri<<" "<<(p.startIsTri ? "triIdx=" : "vIdx=")<<p.startTriIdx<<", end isTri="<<p.endIsTri<<" "<<(p.endIsTri ? "triIdx=" : "vIdx=")<<p.endTriIdx<<"\n";
          }
          //DEBUG
        }
      }
    } else if (numUsedVertices != numIndices) {

      while (usedVertices[iv])
        ++iv;
      assert(iv < numIndices && !usedVertices[iv]);

      assert(numUsedVertices < numIndices);

      const uint32_t vIdx = verticesIndices[iv];
      assert(vIdx < numVertices);
      const float xV = vertices[3 * vIdx + 0];
      const float zV = vertices[3 * vIdx + 2];

      //first, seek if vidx can be "linked into" an existing Path
      size_t k = paths.size();
      for (k = 0; k < paths.size(); ++k) {

        if (!paths[k].endIsTri &&
            v2t.inSameTriangle(paths[k].endTriIdx, vIdx)) {

          Path &p = paths[k];

          assert(p.checkLengths());
          assert(p.checkLengths(vertices, numVertices));

          const float l = computeDist(p.endPt.x, p.endPt.z, xV, zV);

          //std::cerr<<"add vidx="<<vIdx<<" pt=("<<xV<<", "<<zV<<") on path["<<k<<"] end: isTri="<<p.endIsTri<<" "<<(p.endIsTri ? "triIdx=" : "vIdx=")<<p.endTriIdx<<" p.endPt=("<<p.endPt.x<<", "<<p.endPt.z<<") p.length="<<p.length<<" dist="<<l<<"\n";

          p.vIndices.push_back(vIdx);
          p.length += l;
          p.lengths.push_back(p.length);
          p.endPt.x = xV;
          p.endPt.z = zV;
          p.endIsTri = false;
          p.endTriIdx = vIdx;

          assert(p.checkLengths());
          assert(p.checkLengths(vertices, numVertices));

          break;
        }
        if (!paths[k].startIsTri &&
            v2t.inSameTriangle(paths[k].startTriIdx, vIdx)) {

          Path &p = paths[k];

          assert(p.checkLengths());
          assert(p.checkLengths(vertices, numVertices));

          const float l = computeDist(p.startPt.x, p.startPt.z, xV, zV);

          //std::cerr<<"add vidx="<<vIdx<<" pt=("<<xV<<", "<<zV<<") on path["<<k<<"] start: isTri="<<p.startIsTri<<" "<<(p.startIsTri ? "triIdx=" : "vIdx=")<<p.startTriIdx<<" p.startPt=("<<p.startPt.x<<", "<<p.startPt.z<<") p.length="<<p.length<<" dist="<<l<<"\n";

          for (float &length : p.lengths)
            length += l;
          p.vIndices.push_front(vIdx);
          p.lengths.push_front(0);
          p.length += l;
          p.startPt.x = xV;
          p.startPt.z = zV;
          p.startIsTri = false;
          p.startTriIdx = vIdx;

          assert(p.checkLengths());
          assert(p.checkLengths(vertices, numVertices));

          break;
        }
      }

      if (k == paths.size()) {
        Path p;
        p.startPt.x = xV;
        p.startPt.z = zV;
        p.endPt.x = xV;
        p.endPt.z = zV;
        p.length = 0;
        p.startTriIdx = vIdx;
        p.startIsTri = false;
        p.endTriIdx = vIdx;
        p.endIsTri = false;

        p.lengths.push_back(0.f);
        p.vIndices.push_back(vIdx);

        //std::cerr<<"add vIdx="<<vIdx<<" in new path "<<&p<<" ["<<k<<"]\n";

        assert(p.checkLengths());
        assert(p.checkLengths(vertices, numVertices));

        paths.push_back(p);
      }

      usedVertices[iv] = true;
      ++numUsedVertices;
    }
  }

  assert(numUsedVertices == numIndices && numUsedSegments == numSegments);

#ifdef VERBOSE
  { //DEBUG
    if (paths.size() != 1) {

      std::cerr << "paths BEFORE MERGE: " << paths.size() << "\n";
      for (size_t k = 0; k < paths.size(); ++k) {
        std::cerr << "  paths[" << k << "]/" << paths.size() << " startPt=("
                  << paths[k].startPt.x << ", " << paths[k].startPt.z
                  << ") endPt=(" << paths[k].endPt.x << ", " << paths[k].endPt.z
                  << ") l=" << paths[k].length << "\n";
        std::cerr << "    startIsTri=" << paths[k].startIsTri << " "
                  << (paths[k].startIsTri ? "triIdx=" : "vIdx=")
                  << paths[k].startTriIdx << " ; endIsTri=" << paths[k].endIsTri
                  << " " << (paths[k].endIsTri ? "triIdx=" : "vIdx=")
                  << paths[k].endTriIdx << "\n";
        std::cerr << "    numVIdx=" << paths[k].lengths.size() << "\n";
        for (size_t j = 0; j < paths[k].lengths.size(); ++j) {
          std::cerr << "      vidx=" << paths[k].vIndices[j] << " pt=("
                    << vertices[3 * paths[k].vIndices[j] + 0] << ", "
                    << vertices[3 * paths[k].vIndices[j] + 2]
                    << ") length=" << paths[k].lengths[j] << "\n";
        }
      }
    }

  }    //DEBUG
#endif //VERBOSE

#ifndef NDEBUG
  {
    //vIdx listed in @a verticesIndices
    ///must all appear in one Path & only in one Path

    uint32_t numUsedVerticesF = 0;
    std::vector<bool> usedVerticesV(numIndices,
                                   false); //OPTIM: use char ? alloc once ?
    for (uint32_t i = 0; i < numIndices; ++i) {
      const uint32_t vIdx = verticesIndices[i];

      for (size_t k = 0; k < paths.size(); ++k) {
        assert(paths[k].vIndices.size() <= numIndices);
        for (size_t j = 0; j < paths[k].vIndices.size(); ++j) {

          if (paths[k].vIndices[j] == vIdx) {

            assert(i < usedVerticesV.size());
            if (usedVerticesV[i] == true) {

              const uint32_t vIdxj = paths[k].vIndices[j];

              std::cerr << "vIdx=" << vIdxj << " is used in path [" << k
                        << "]\n";
              std::cerr << "  paths[" << k << "]/" << paths.size()
                        << " startPt=(" << paths[k].startPt.x << ", "
                        << paths[k].startPt.z << ") endPt=(" << paths[k].endPt.x
                        << ", " << paths[k].endPt.z << ") l=" << paths[k].length
                        << "\n";
              std::cerr << "    startIsTri=" << paths[k].startIsTri << " "
                        << (paths[k].startIsTri ? "triIdx=" : "vIdx=")
                        << paths[k].startTriIdx
                        << " ; endIsTri=" << paths[k].endIsTri << " "
                        << (paths[k].endIsTri ? "triIdx=" : "vIdx=")
                        << paths[k].endTriIdx << "\n";
              std::cerr << "    numVIdx=" << paths[k].lengths.size() << "\n";
              for (size_t jp = 0; jp < paths[k].lengths.size(); ++jp)
                std::cerr << "      vidx=" << paths[k].vIndices[jp] << " pt=("
                          << vertices[3 * paths[k].vIndices[jp] + 0] << ", "
                          << vertices[3 * paths[k].vIndices[jp] + 2]
                          << ") length=" << paths[k].lengths[jp] << "\n";

              {
                size_t kk = k, jj = 0;
                for (kk = 0; kk < k; ++kk) {
                  size_t maxj = (kk != k ? paths[kk].vIndices.size() : j);
                  for (jj = 0; jj < maxj; ++jj) {
                    if (paths[kk].vIndices[jj] == vIdxj) {
                      break;
                    }
                  }
                }

                std::cerr << "and also in path [" << kk << "]\n";
                std::cerr << "  paths[" << kk << "]/" << paths.size()
                          << " startPt=(" << paths[kk].startPt.x << ", "
                          << paths[kk].startPt.z << ") endPt=("
                          << paths[kk].endPt.x << ", " << paths[kk].endPt.z
                          << ") l=" << paths[kk].length << "\n";
                std::cerr << "    startIsTri=" << paths[kk].startIsTri << " "
                          << (paths[kk].startIsTri ? "triIdx=" : "vIdx=")
                          << paths[kk].startTriIdx
                          << " ; endIsTri=" << paths[kk].endIsTri << " "
                          << (paths[kk].endIsTri ? "triIdx=" : "vIdx=")
                          << paths[kk].endTriIdx << "\n";
                std::cerr << "    numVIdx=" << paths[kk].lengths.size() << "\n";
                for (size_t jp = 0; jp < paths[kk].lengths.size(); ++jp)
                  std::cerr << "      vidx=" << paths[kk].vIndices[jp] << " pt=("
                            << vertices[3 * paths[kk].vIndices[jp] + 0] << ", "
                            << vertices[3 * paths[kk].vIndices[jp] + 2]
                            << ") length=" << paths[kk].lengths[jp] << "\n";
              }
            }

            assert(usedVerticesV[i] == false);
            usedVerticesV[i] = true;
            ++numUsedVerticesF;
          }
        }
      }
    }
    assert(numUsedVerticesF == numIndices);
  }
#endif //NDEBUG

  //try to merge Paths two by two
  //[On a mesh without hole, we should have (mostly) one Path per line]
  bool change = false;
  do {

    change = false;
    assert(!paths.empty());

    for (std::vector<Path>::iterator it = paths.begin(); it != paths.end();
         ++it) {

      Path &p = *it;

      if (p.startTriIdx != (uint32_t)-1 || p.endTriIdx != (uint32_t)-1) {

        //std::cerr<<"try to merge path start: isTri="<<p.startIsTri<<" "<<(p.startIsTri ? "triIdx=" : "vIdx=")<<p.startTriIdx<<" p.startPt=("<<p.startPt.x<<", "<<p.startPt.z<<") end: isTri="<<p.endIsTri<<" "<<(p.endIsTri ? "triIdx=" : "vIdx=")<<p.endTriIdx<<" p.endPt=("<<p.endPt.x<<", "<<p.endPt.z<<") length="<<p.length<<"\n";

        for (std::vector<Path>::iterator itj = it + 1;
             itj != paths.end();) { //no increment here

          Path &pj = *itj;

          bool merged = false;

          //std::cerr<<" with path j start: isTri="<<pj.startIsTri<<" "<<(pj.startIsTri ? "triIdx=" : "vIdx=")<<pj.startTriIdx<<" pj.startPt=("<<pj.startPt.x<<", "<<pj.startPt.z<<") end: isTri="<<pj.endIsTri<<" "<<(pj.endIsTri ? "triIdx=" : "vIdx=")<<pj.endTriIdx<<" p.endPt=("<<pj.endPt.x<<", "<<pj.endPt.z<<") length="<<pj.length<<"\n";

          if ((p.endTriIdx != (uint32_t)-1 && pj.startTriIdx != (uint32_t)-1) &&
              ((!p.endIsTri && !pj.startIsTri &&
                v2t.inSameTriangle(p.endTriIdx, pj.startTriIdx)) ||
               (!p.endIsTri && pj.startIsTri &&
                (tris[3 * pj.startTriIdx + 0] == p.endTriIdx ||
                 tris[3 * pj.startTriIdx + 1] == p.endTriIdx ||
                 tris[3 * pj.startTriIdx + 2] == p.endTriIdx)) ||
               (p.endIsTri && !pj.startIsTri &&
                (tris[3 * p.endTriIdx + 0] == pj.startTriIdx ||
                 tris[3 * p.endTriIdx + 1] == pj.startTriIdx ||
                 tris[3 * p.endTriIdx + 2] == pj.startTriIdx)) ||
               (p.endIsTri && pj.startIsTri &&
                p.endTriIdx == pj.startTriIdx))) {

            //vertex - vertex link
            // ||
            //vertex - intersection pt link
            // ||
            //intersection point - vertex link
            // ||
            //intersection point - intersection point link

            assert(p.checkLengths());
            assert(p.checkLengths(vertices, numVertices));
            assert(pj.checkLengths());
            assert(pj.checkLengths(vertices, numVertices));

            //std::cerr<<"before: p.length="<<p.length<<" p.lengths.size()="<<p.lengths.size()<<" p.startPt.x="<<p.startPt.x<<" z="<<p.startPt.z<<" p.endPt.x="<<p.endPt.x<<" z="<<p.endPt.z<<" \n";
            //std::cerr<<"before: pj.length="<<pj.length<<" pj.lengths.size()="<<pj.lengths.size()<<" pj.startPt.x="<<pj.startPt.x<<" z="<<pj.startPt.z<<" pj.endPt.x="<<pj.endPt.x<<" z="<<pj.endPt.z<<" \n";

            float l =
              computeDist(p.endPt.x, p.endPt.z, pj.startPt.x, pj.startPt.z);

            if (!p.endIsTri && !pj.startIsTri &&
                p.vIndices.back() == pj.vIndices.front()) {
              //vertex-vertex link && same vertex
              assert(p.endPt.x == pj.startPt.x && p.endPt.z == pj.startPt.z);
              assert(l <= std::numeric_limits<float>::epsilon());
              assert(!pj.lengths.empty() &&
                     pj.lengths[0] <= std::numeric_limits<float>::epsilon());

              pj.vIndices.pop_front();
              pj.lengths.pop_front();
              l = 0;
            }

            const float la = p.length + l;

            /*
            {//DEBUG
              std::cerr<<"before merge e-s: ";
              std::cerr<<p.lengths.size()<<" p.lengths : ";
              for (float l : p.lengths)
                std::cerr<<l<<" ";
              std::cerr<<"\n";
              std::cerr<<pj.lengths.size()<<" pj.lengths : ";
              for (float l : pj.lengths)
                std::cerr<<l<<" ";
              std::cerr<<"\n";

              std::cerr<<"l="<<l<<" pj.length+l="<<pj.length+l<<"\n";
            }
            */

            for (float length : pj.lengths)
              p.lengths.push_back(length + la);
            for (uint32_t vidx : pj.vIndices)
              p.vIndices.push_back(vidx);

            /*
            {//DEBUG
              std::cerr<<"after merge e-s: ";
              std::cerr<<p.lengths.size()<<" p.lengths : ";
              for (float l : p.lengths)
                std::cerr<<l<<" ";
              std::cerr<<"\n";

              std::cerr<<"l="<<l<<" pj.length+l="<<pj.length+l<<"\n";
            }
            */

            p.endPt.x = pj.endPt.x;
            p.endPt.z = pj.endPt.z;
            p.endTriIdx = pj.endTriIdx;
            p.endIsTri = pj.endIsTri;
            p.length += l + pj.length;

            assert(p.checkLengths());
            assert(p.checkLengths(vertices, numVertices));

            merged = true;
          } else if ((p.startTriIdx != (uint32_t)-1 &&
                      pj.endTriIdx != (uint32_t)-1) &&
                     ((!p.startIsTri && !pj.endIsTri &&
                       v2t.inSameTriangle(p.startTriIdx, pj.endTriIdx)) ||
                      (!p.startIsTri && pj.endIsTri &&
                       (tris[3 * pj.endTriIdx + 0] == p.startTriIdx ||
                        tris[3 * pj.endTriIdx + 1] == p.startTriIdx ||
                        tris[3 * pj.endTriIdx + 2] == p.startTriIdx)) ||
                      (p.startIsTri && !pj.endIsTri &&
                       (tris[3 * p.startTriIdx + 0] == pj.endTriIdx ||
                        tris[3 * p.startTriIdx + 1] == pj.endTriIdx ||
                        tris[3 * p.startTriIdx + 2] == pj.endTriIdx)) ||
                      (p.startIsTri && pj.endIsTri &&
                       p.startTriIdx == pj.endTriIdx))) {

            assert(p.checkLengths());
            assert(p.checkLengths(vertices, numVertices));
            assert(pj.checkLengths());
            assert(pj.checkLengths(vertices, numVertices));

            float l =
              computeDist(pj.endPt.x, pj.endPt.z, p.startPt.x, p.startPt.z);

            if (!p.startIsTri && !pj.endIsTri &&
                p.vIndices.front() == pj.vIndices.back()) {
              //vertex-vertex link && same vertex
              assert(pj.endPt.x == p.startPt.x && pj.endPt.z == p.startPt.z);
              assert(l <= std::numeric_limits<float>::epsilon());
              assert(!p.lengths.empty() &&
                     p.lengths[0] <= std::numeric_limits<float>::epsilon());

              p.vIndices.pop_front();
              p.lengths.pop_front();
              l = 0;
            }

            const float la = pj.length + l;
            /*
            for (float &length :  p.lengths)
              length += la;
            for (float length :  pj.lengths)
              p.lengths.push_front(length);
            for (uint32_t vidx : pj.vIndices)
              p.vIndices.push_front(vidx);
            */
            for (float length : p.lengths)
              pj.lengths.push_back(length + la);
            p.lengths = pj.lengths;
            for (uint32_t vidx : p.vIndices)
              pj.vIndices.push_back(vidx);
            p.vIndices = pj.vIndices;

            p.startPt.x = pj.startPt.x;
            p.startPt.z = pj.startPt.z;
            p.startTriIdx = pj.startTriIdx;
            p.startIsTri = pj.startIsTri;
            p.length += l + pj.length;

            assert(p.checkLengths());
            assert(p.checkLengths(vertices, numVertices));

            merged = true;

          } else if ((p.startTriIdx != (uint32_t)-1 &&
                      pj.startTriIdx != (uint32_t)-1) &&
                     ((!p.startIsTri && !pj.startIsTri &&
                       v2t.inSameTriangle(p.startTriIdx, pj.startTriIdx)) ||
                      (!p.startIsTri && pj.startIsTri &&
                       (tris[3 * pj.startTriIdx + 0] == p.startTriIdx ||
                        tris[3 * pj.startTriIdx + 1] == p.startTriIdx ||
                        tris[3 * pj.startTriIdx + 2] == p.startTriIdx)) ||
                      (p.startIsTri && !pj.startIsTri &&
                       (tris[3 * p.startTriIdx + 0] == pj.startTriIdx ||
                        tris[3 * p.startTriIdx + 1] == pj.startTriIdx ||
                        tris[3 * p.startTriIdx + 2] == pj.startTriIdx)) ||
                      (p.startIsTri && pj.startIsTri &&
                       p.startTriIdx == pj.startTriIdx))) {

            assert(pj.checkLengths());
            assert(pj.checkLengths(vertices, numVertices));

            pj.swapStartEnd();

            assert(p.checkLengths());
            assert(p.checkLengths(vertices, numVertices));
            assert(pj.checkLengths());
            assert(pj.checkLengths(vertices, numVertices));

            float l =
              computeDist(pj.endPt.x, pj.endPt.z, p.startPt.x, p.startPt.z);

            if (!p.startIsTri && !pj.endIsTri &&
                p.vIndices.front() == pj.vIndices.back()) {
              //vertex-vertex link && same vertex
              assert(pj.endPt.x == p.startPt.x && pj.endPt.z == p.startPt.z);
              assert(l <= std::numeric_limits<float>::epsilon());
              assert(!p.lengths.empty() &&
                     p.lengths[0] <= std::numeric_limits<float>::epsilon());

              p.vIndices.pop_front();
              p.lengths.pop_front();
              l = 0;
            }

            /*
            {//DEBUG
              std::cerr<<"before merge: ";
              std::cerr<<p.lengths.size()<<" p.lengths : ";
              for (float l : p.lengths)
                std::cerr<<l<<" ";
              std::cerr<<"\n";
              std::cerr<<pj.lengths.size()<<" pj.lengths : ";
              for (float l : pj.lengths)
                std::cerr<<l<<" ";
              std::cerr<<"\n";

              std::cerr<<"l="<<l<<" pj.length+l="<<pj.length+l<<"\n";
            }
            */

            const float la = pj.length + l;
            for (float length : p.lengths)
              pj.lengths.push_back(length + la);
            p.lengths = pj.lengths;

            for (uint32_t vidx : p.vIndices)
              pj.vIndices.push_back(vidx);
            p.vIndices = pj.vIndices;

            /*
            {//DEBUG
              std::cerr<<"after merge: ";
              std::cerr<<p.lengths.size()<<" p.lengths : ";
              for (float l : p.lengths)
                std::cerr<<l<<" ";
              std::cerr<<"\n";
            }//DEBUG
            */

            p.startPt.x = pj.startPt.x;
            p.startPt.z = pj.startPt.z;
            p.startTriIdx = pj.startTriIdx;
            p.startIsTri = pj.startIsTri;
            p.length += la; //l + pj.length;

            assert(p.checkLengths());
            assert(p.checkLengths(vertices, numVertices));

            merged = true;

          } else if ((p.endTriIdx != (uint32_t)-1 &&
                      pj.endTriIdx != (uint32_t)-1) &&
                     ((!p.endIsTri && !pj.endIsTri &&
                       v2t.inSameTriangle(p.endTriIdx, pj.endTriIdx)) ||
                      (!p.endIsTri && pj.endIsTri &&
                       (tris[3 * pj.endTriIdx + 0] == p.endTriIdx ||
                        tris[3 * pj.endTriIdx + 1] == p.endTriIdx ||
                        tris[3 * pj.endTriIdx + 2] == p.endTriIdx)) ||
                      (p.endIsTri && !pj.endIsTri &&
                       (tris[3 * p.endTriIdx + 0] == pj.endTriIdx ||
                        tris[3 * p.endTriIdx + 1] == pj.endTriIdx ||
                        tris[3 * p.endTriIdx + 2] == pj.endTriIdx)) ||
                      (p.endIsTri && pj.endIsTri &&
                       p.endTriIdx == pj.endTriIdx))) {

            assert(pj.checkLengths());
            assert(pj.checkLengths(vertices, numVertices));

            pj.swapStartEnd();

            assert(pj.checkLengths());
            assert(pj.checkLengths(vertices, numVertices));
            assert(p.checkLengths());
            assert(p.checkLengths(vertices, numVertices));

            float l =
              computeDist(p.endPt.x, p.endPt.z, pj.startPt.x, pj.startPt.z);

            /*
            {//DEBUG
              std::cerr<<"before merge e-e: ";
              std::cerr<<p.lengths.size()<<" p.lengths : ";
              for (float l : p.lengths)
                std::cerr<<l<<" ";
              std::cerr<<"\n";
              std::cerr<<pj.lengths.size()<<" pj.lengths : ";
              for (float l : pj.lengths)
                std::cerr<<l<<" ";
              std::cerr<<"\n";

              std::cerr<<"l="<<l<<" pj.length+l="<<pj.length+l<<"\n";
            }
            */

            if (!p.endIsTri && !pj.startIsTri &&
                p.vIndices.back() == pj.vIndices.front()) {
              //vertex-vertex link && same vertex
              assert(p.endPt.x == pj.startPt.x && p.endPt.z == pj.startPt.z);
              assert(l <= std::numeric_limits<float>::epsilon());
              assert(!pj.lengths.empty() &&
                     pj.lengths[0] <= std::numeric_limits<float>::epsilon());

              pj.vIndices.pop_front();
              pj.lengths.pop_front();
              l = 0;
            }

            const float la = p.length + l;
            for (float length : pj.lengths)
              p.lengths.push_back(length + la);
            for (uint32_t vidx : pj.vIndices)
              p.vIndices.push_back(vidx);

            /*
            {//DEBUG
              std::cerr<<"after merge e-e: ";
              std::cerr<<p.lengths.size()<<" p.lengths : ";
              for (float l : p.lengths)
                std::cerr<<l<<" ";
              std::cerr<<"\n";
              std::cerr<<"l="<<l<<" pj.length+l="<<pj.length+l<<"\n";
            }
            */

            p.endPt.x = pj.endPt.x;
            p.endPt.z = pj.endPt.z;
            p.endTriIdx = pj.endTriIdx;
            p.endIsTri = pj.endIsTri;
            p.length += l + pj.length;
            if (!p.lengths.empty())
              p.length = std::max(p.lengths.back(), p.length);

            assert(p.checkLengths());
            assert(p.checkLengths(vertices, numVertices));

            merged = true;
          }

          if (merged == false) {
            ++itj;

            //std::cerr<<" NO MERGE\n";

          } else {
            itj = paths.erase(itj);

            //std::cerr<<" Merged !\n";

            change = true;
          }
        }
      }
    }

  } while (change == true);

  for (auto &p : paths) {
    if (p.startPt.x > p.endPt.x) {
      p.swapStartEnd();
    }
  }

  if (paths.size() > 1) {
    std::sort(paths.begin(), paths.end(), Segment_Sorter());
  }

#ifdef VERBOSE
  { //DEBUG
    if (paths.size() != 1) {

      std::cerr << "paths AFTER MERGE: " << paths.size() << "\n";
      for (size_t k = 0; k < paths.size(); ++k) {
        std::cerr << "  paths[" << k << "]/" << paths.size() << " startPt=("
                  << paths[k].startPt.x << ", " << paths[k].startPt.z
                  << ") endPt=(" << paths[k].endPt.x << ", " << paths[k].endPt.z
                  << ") l=" << paths[k].length << "\n";
        std::cerr << "    startIsTri=" << paths[k].startIsTri << " "
                  << (paths[k].startIsTri ? "triIdx=" : "vIdx=")
                  << paths[k].startTriIdx << " ; endIsTri=" << paths[k].endIsTri
                  << " " << (paths[k].endIsTri ? "triIdx=" : "vIdx=")
                  << paths[k].endTriIdx << "\n";
        std::cerr << "    numVIdx=" << paths[k].lengths.size() << "\n";
        for (size_t j = 0; j < paths[k].lengths.size(); ++j) {
          std::cerr << "      vidx=" << paths[k].vIndices[j] << " pt=("
                    << vertices[3 * paths[k].vIndices[j] + 0] << ", "
                    << vertices[3 * paths[k].vIndices[j] + 2]
                    << ") length=" << paths[k].lengths[j] << "\n";
        }
      }
    }

    if (paths.size() != 1) {
      std::cerr << "\n !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n";
      std::cerr << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n\n";
    }

  }    //DEBUG
#endif //VERBOSE

  fusePaths(paths);

#ifdef VERBOSE
  { //DEBUG
    std::cerr << "path[0] AFTER FUSE\n";
    std::cerr << "  paths[" << 0 << "] startPt=(" << paths[0].startPt.x << ", "
              << paths[0].startPt.z << ") endPt=(" << paths[0].endPt.x << ", "
              << paths[0].endPt.z << ") l=" << paths[0].length << "\n";
    std::cerr << "    startIsTri=" << paths[0].startIsTri << " "
              << (paths[0].startIsTri ? "triIdx=" : "vIdx=")
              << paths[0].startTriIdx << " ; endIsTri=" << paths[0].endIsTri
              << " " << (paths[0].endIsTri ? "triIdx=" : "vIdx=")
              << paths[0].endTriIdx << "\n";
    std::cerr << "    numVIdx=" << paths[0].lengths.size() << "\n";
    for (size_t j = 0; j < paths[0].lengths.size(); ++j) {
      std::cerr << "      vidx=" << paths[0].vIndices[j] << " pt=("
                << vertices[3 * paths[0].vIndices[j] + 0] << ", "
                << vertices[3 * paths[0].vIndices[j] + 2]
                << ") length=" << paths[0].lengths[j] << "\n";
    }
  }
#endif //VERBOSE

  assert(paths.size() == 1);
}

//Add point at X==0.
static void
addZeroPoint(Path &p)
{
  //B: works only if no folding...

  const float l = computeDist(0, p.startPt.z, p.startPt.x, p.startPt.z);
  if (l > std::numeric_limits<float>::epsilon()) {
    for (float &len : p.lengths)
      len += l;

    p.length += l;
    p.startPt.x = 0;
  }
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
computeTexCoords2(Mesh &mesh)
{
  if (!mesh.isValid())
    return;

#ifdef TIME_TEXCOORDS
  struct timeval t0, t1;
#endif //TIME_TEXCOORDS

  //rotateAroundMean(mesh); // the spine // Oy

  ///DEBUG_MESH_INVARIANT
  float original_min_x = moveTo_X_min(mesh); // translate to X_min
  ////moveTo_XZ_min(mesh);// translate to X_min & Z_min

  std::cerr << "original_min_x=" << original_min_x << "\n";
  //original_min_x = -1.00001;
  //std::cerr<<"original_min_x="<<original_min_x<<"  DEBUG !!!\n";

  float MINV[3], MAXV[3];
  mesh.getAABB(MINV, MAXV);
  //const float X_MIN = MINV[0];

#ifdef TIME_TEXCOORDS
  gettimeofday(&t0, nullptr);
#endif //TIME_TEXCOORDS

  mesh.allocateTexCoords();

#ifdef TIME_TEXCOORDS
  gettimeofday(&t1, nullptr);
  std::cerr << "time allocateTexCoords: " << P_getTime(t0, t1) << "ms for "
            << mesh.numVertices << " vertices\n";
#endif //TIME_TEXCOORDS

#ifndef NDEBUG
  const float TEXCOORD_INVALID = -1000.0f;
  for (uint32_t i = 0; i < mesh.numVertices; ++i) {
    mesh.texCoords[2 * i + 0] = TEXCOORD_INVALID;
    mesh.texCoords[2 * i + 1] = TEXCOORD_INVALID;
  }
#endif //NDEBUG

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

  std::vector<EdgeF> edges;
  getEdgesF(mesh, edges);

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

  SpacePartionnerYEdges<EdgeF> spye(numYs, vidx2yPlaneIdx, edges);

#ifdef TIME_TEXCOORDS
  gettimeofday(&t1, nullptr);
  std::cerr << "time SpacePartionnerYEdges: " << P_getTime(t0, t1) << "ms\n";
#endif //TIME_TEXCOORDS

  /*
  //DEBUG
  size_t YI = 0;
  {//DEBUG
      const float PT_X = -0.0109094; //0.00231814; //0.0023182;
    const float PT_Z = 0.184814; //-0.212511; //-0.212584;
    bool found = false;

    for (size_t yi=0; yi<numYs; ++yi) {
      const uint32_t start = startYIndices[yi];
      const uint32_t ind0 = verticesIndices[start];
      const float y0 = mesh.vertices[3*ind0+1];
      assert(yi < startYIndices.size());
      const uint32_t end = startYIndices[yi+1];
      for (uint32_t k=start; k<end; ++k) {
        const uint32_t ind = verticesIndices[k];
        if (mesh.vertices[3*ind+1] != y0) {
          std::cerr<<"ERROR !!!! y0="<<y0<<" != "<<mesh.vertices[3*ind+1]<<"\n";
          std::cerr<<"for yi="<<yi<<" start="<<start<<" end="<<end<<"\n";
          assert(false);
        }
        const float x = mesh.vertices[3*ind+0];
        const float z = mesh.vertices[3*ind+2];

        const float EPS = 8*std::numeric_limits<float>::epsilon();

        if (fabs(PT_X-x)<EPS && fabs(PT_Z-z)<EPS) {
          found = true;
          YI = yi;
          break;
        }
      }

      if (found) {
        break;
      }

    }
    if (! found) {
      std::cerr<<"Point not found !!! \n";
      exit(10);
    }
    std::cerr<<"pt ("<<PT_X<<", "<<PT_Z<<") found at yi="<<YI<<"\n";

  }//END DEBUG
  */

  std::vector<X_Z_e> intersectionPoints;

  const size_t reserve_sizeIP = 3000; //arbitrary
  intersectionPoints.reserve(reserve_sizeIP);

  const size_t reserve_sizeVL = 13; //arbitrary
  //std::vector<VertexLink> vertexLinks;
  //vertexLinks.reserve(reserve_sizeVL);
  std::vector<Path> paths;
  paths.reserve(reserve_sizeVL);

#ifdef TIME_TEXCOORDS
  gettimeofday(&t0, nullptr);
#endif //TIME_TEXCOORDS

  Vertex2Tris v2t(mesh);

#ifdef TIME_TEXCOORDS
  gettimeofday(&t1, nullptr);
  std::cerr << "time Vertex2Tris: " << P_getTime(t0, t1) << "ms\n";
#endif //TIME_TEXCOORDS

  std::vector<Segment> segments;
  segments.reserve(reserve_sizeIP);

#ifdef TIME_TEXCOORDS
  struct timeval tt0, tt1;
  size_t DBG_MAX_NUM_EDGES = 0;
  size_t DBG_MAX_INTERSECTION_POINTS = 0;
  size_t DBG_MAX_INTERSPTS_CAPA = 0;
  double timeGetEdgesForY = 0;
  double timeIntersectPts = 0;
  double timeSort = 0;
  double timeSortE = 0;
  double timeGetPaths = 0;
  double timeAffectTexCoords = 0;
  size_t DBG_NUM_EDGES = 0;
  size_t DBG_NUM_EDGES_USED = 0;
  size_t DBG_NUM_EDGES_USED2 = 0;
  size_t DBG_MAX_NUM_SEGMENTS = 0;
  size_t DBG_MAX_NUM_VERTEXLINKS = 0;

  gettimeofday(&t0, nullptr);
#endif //TIME_TEXCOORDS

#ifdef VERBOSE
  float DBG_max_path_length = 0;
#endif //VERBOSE

  //DEBUG
  for (size_t yi = 0; yi < numYs; ++yi) {
    //size_t yi = YI; {

    const uint32_t start = startYIndices[yi];
    const uint32_t ind = verticesIndices[start];
    const float y = mesh.vertices[3 * ind + 1];

    //We get the X coord of the last original vertex on this y line.
    //We do not have to compute intersection points with edges
    // with two extremities beyond this X coord.
    assert(yi < startYIndices.size());
#ifndef NDEBUG
    const uint32_t end = startYIndices[yi + 1];
    assert(start < end);
    const uint32_t indLast = verticesIndices[end - 1];
    //const float lastX = mesh.vertices[3*indLast+0];
    assert(mesh.vertices[3 * indLast + 1] == y);
#endif //NDEBUG

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

      const uint32_t eind = edgesIndices[i];
      const uint32_t e0 = edges[eind].vertexIndex[0];
      const uint32_t e1 = edges[eind].vertexIndex[1];

      assert(e0 < mesh.numVertices);
      assert(e1 < mesh.numVertices);

      const float *v0 = &mesh.vertices[3 * e0];
      const float *v1 = &mesh.vertices[3 * e1];

      //const float x0 = v0[0];
      //const float x1 = v1[0];

      //if (x0 < lastX || x1 < lastX)
      {
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

          //std::cerr<<"yi="<<yi<<" y="<<y<<" ; e0="<<e0<<" yInd_i0="<<yInd_i0<<" y0="<<v0[1]<<" ; e1="<<e1<<" yInd_i1="<<yInd_i1<<" y1="<<v1[1]<<"\n";

          L3 line(v0, v1);

          X_Z intersectionPt = line.getIntersectionPoint(y);

          //if (intersectionPt.x <= lastX) //B:NEW: 150504
          intersectionPoints.push_back(X_Z_e(intersectionPt, eind));
        }
        /*
        else if (yInd_i0==yi && yi==yInd_i1) {
          //B: We can detect here that we have and edge completely on Y plane
          // Can we use this information ???

        }
        */
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
      intersectionPoints.begin(), intersectionPoints.end(), X_Z_e_SorterX_Z());
    //B: NOT SURE NECESSARY !!!
    // segments MUST BE SORTED !!!!

#ifdef TIME_TEXCOORDS
    gettimeofday(&tt1, nullptr);
    timeSort += P_getTime(tt0, tt1);
    gettimeofday(&tt0, nullptr);
#endif //TIME_TEXCOORDS

    chainAndMergeIntersectionPts(intersectionPoints, edges, v2t, segments);

#ifdef TIME_TEXCOORDS
    gettimeofday(&tt1, nullptr);
    timeSortE += P_getTime(tt0, tt1);
    if (segments.size() > DBG_MAX_NUM_SEGMENTS)
      DBG_MAX_NUM_SEGMENTS = segments.size();
#endif //TIME_TEXCOORDS

    //- original vertices from mesh on this Y planes
    uint32_t iV = startYIndices[yi];
    uint32_t endV = startYIndices[yi + 1];
    assert(iV < endV);

#ifdef VERBOSE
    std::cerr << "yi=" << yi << "/" << numYs << " y=" << y
              << "  numEdgesIndices=" << numEdgesIndices << "/" << edges.size()
              << " ; intersectionPoints.size()=" << intersectionPoints.size()
              << " ; segments.size()=" << segments.size()
              << " nbVerts=" << endV - iV << "   paths.size()=" << paths.size()
              << "\n";

    {                  //DEBUG
      if (yi == 953) { //if (yi == 3) { //if (segments.size() == 3) {
        for (size_t k = iV; k < endV; ++k) {
          std::cerr << "   vertex vidx=" << verticesIndices[k]
                    << " x=" << mesh.vertices[3 * verticesIndices[k] + 0]
                    << " z=" << mesh.vertices[3 * verticesIndices[k] + 2]
                    << "\n";

          std::cerr << "    belongs to tris: ";
          const uint32_t *indices;
          uint32_t numIndices;
          v2t.getTris(verticesIndices[k], indices, numIndices);
          for (uint32_t l = 0; l < numIndices; ++l)
            std::cerr << indices[l] << " ";
          std::cerr << "\n";
        }

        for (size_t k = 0; k < intersectionPoints.size(); ++k) {
          std::cerr << "  intersectionPoints[" << k
                    << "] eIdx=" << intersectionPoints[k].eIdx
                    << " x=" << intersectionPoints[k].x
                    << " z=" << intersectionPoints[k].z << " start="
                    << edges[intersectionPoints[k].eIdx].faceIndex[0]
                    << " end=" << edges[intersectionPoints[k].eIdx].faceIndex[1]
                    << "\n";
        }

        for (size_t k = 0; k < segments.size(); ++k) {
          std::cerr << "  segments[" << k << "] startPt=("
                    << segments[k].startPt.x << ", " << segments[k].startPt.z
                    << ") endPt=(" << segments[k].endPt.x << ", "
                    << segments[k].endPt.z
                    << ") startTriIdx=" << segments[k].startTriIdx
                    << " endTriIdx=" << segments[k].endTriIdx
                    << " l=" << segments[k].length << "\n";
        }
      }

    }  //DEBUG
#endif //VERBOSE

#ifdef TIME_TEXCOORDS
    gettimeofday(&tt0, nullptr);
#endif //TIME_TEXCOORDS

    getPaths(mesh.triangles,
             mesh.numTriangles,
             mesh.vertices,
             mesh.numVertices,
             &verticesIndices[iV],
             endV - iV,
             segments,
             v2t,
             paths);

    /*
      getVertexLinks(mesh.vertices, mesh.numVertices,
                     &verticesIndices[iV], endV-iV,
                     segments, v2t,
                     vertexLinks);
      */

#ifdef TIME_TEXCOORDS
    gettimeofday(&tt1, nullptr);
    timeGetPaths += P_getTime(tt0, tt1);
    //if (vertexLinks.size() > DBG_MAX_NUM_VERTEXLINKS)
    if (paths.size() > DBG_MAX_NUM_VERTEXLINKS)
      DBG_MAX_NUM_VERTEXLINKS = paths.size();
#endif //TIME_TEXCOORDS

#ifdef VERBOSE
    std::cerr << "yi=" << yi << "/" << numYs << " y=" << y
              << "  numEdgesIndices=" << numEdgesIndices << "/" << edges.size()
              << " ; intersectionPoints.size()=" << intersectionPoints.size()
              << " ; segments.size()=" << segments.size()
              << " nbVerts=" << endV - iV << "   paths.size()=" << paths.size()
              << "\n";

    {                  //DEBUG
      if (yi == 953) { //if (segments.size() == 3) {

        for (size_t k = 0; k < paths.size(); ++k) {
          std::cerr << "  paths[" << k << "]/" << paths.size() << " startPt=("
                    << paths[k].startPt.x << ", " << paths[k].startPt.z
                    << ") endPt=(" << paths[k].endPt.x << ", "
                    << paths[k].endPt.z << ") l=" << paths[k].length << "\n";
          for (size_t j = 0; j < paths[k].lengths.size(); ++j) {
            std::cerr << "      vidx=" << paths[k].vIndices[j] << " pt=("
                      << mesh.vertices[3 * paths[k].vIndices[j] + 0] << ", "
                      << mesh.vertices[3 * paths[k].vIndices[j] + 2]
                      << ") length=" << paths[k].lengths[j] << "\n";
          }
        }
      }

    } //DEBUG
    std::cerr << "\n\n";
#endif //VERBOSE

#ifdef TIME_TEXCOORDS
    gettimeofday(&tt0, nullptr);
#endif //TIME_TEXCOORDS

    assert(paths.size() == 1);
    assert(!paths[0].vIndices.empty());
    assert(paths[0].vIndices.size() == paths[0].lengths.size());

    addZeroPoint(paths[0]);

#ifdef VERBOSE
    { //DEBUG
      std::cerr << "path[0] AFTER ZEROPOINT\n";
      std::cerr << "  paths[" << 0 << "] startPt=(" << paths[0].startPt.x
                << ", " << paths[0].startPt.z << ") endPt=(" << paths[0].endPt.x
                << ", " << paths[0].endPt.z << ") l=" << paths[0].length
                << "\n";
      std::cerr << "    startIsTri=" << paths[0].startIsTri << " "
                << (paths[0].startIsTri ? "triIdx=" : "vIdx=")
                << paths[0].startTriIdx << " ; endIsTri=" << paths[0].endIsTri
                << " " << (paths[0].endIsTri ? "triIdx=" : "vIdx=")
                << paths[0].endTriIdx << "\n";
      std::cerr << "    numVIdx=" << paths[0].lengths.size() << "\n";
      for (size_t j = 0; j < paths[0].lengths.size(); ++j) {
        std::cerr << "      vidx=" << paths[0].vIndices[j] << " pt=("
                  << mesh.vertices[3 * paths[0].vIndices[j] + 0] << ", "
                  << mesh.vertices[3 * paths[0].vIndices[j] + 2]
                  << ") length=" << paths[0].lengths[j] << "\n";
      }
    }

    if (paths[0].lengths.back() > DBG_max_path_length) {
      std::cerr << "#  paths[" << 0 << "] startPt=(" << paths[0].startPt.x
                << ", " << paths[0].startPt.z << ") endPt=(" << paths[0].endPt.x
                << ", " << paths[0].endPt.z << ") LENGTH=" << paths[0].length
                << "\n";
      std::cerr << "    startIsTri=" << paths[0].startIsTri << " "
                << (paths[0].startIsTri ? "triIdx=" : "vIdx=")
                << paths[0].startTriIdx << " ; endIsTri=" << paths[0].endIsTri
                << " " << (paths[0].endIsTri ? "triIdx=" : "vIdx=")
                << paths[0].endTriIdx << "\n";
      std::cerr << "    numVIdx=" << paths[0].lengths.size() << "\n";
      for (size_t j = 0; j < paths[0].lengths.size(); ++j) {
        std::cerr << "      vidx=" << paths[0].vIndices[j] << " pt=("
                  << mesh.vertices[3 * paths[0].vIndices[j] + 0] << ", "
                  << mesh.vertices[3 * paths[0].vIndices[j] + 2]
                  << ") length=" << paths[0].lengths[j] << "\n";
      }
      std::cerr << "  LENGTH at last vertex=" << paths[0].lengths.back()
                << "\n";
      DBG_max_path_length = paths[0].lengths.back();
    }

#endif //VERBOSE

    const size_t numIndices = paths[0].vIndices.size();
    assert(numIndices == endV - iV);
    for (size_t i = 0; i < numIndices; ++i) {
      const uint32_t vIdx = paths[0].vIndices[i];
      const float L = paths[0].lengths[i];

      assert(vIdx < mesh.numVertices);
      assert(mesh.hasTexCoords());
      mesh.texCoords[2 * vIdx + 0] = L;
      mesh.texCoords[2 * vIdx + 1] = y;
    }

#ifdef TIME_TEXCOORDS
    gettimeofday(&tt1, nullptr);
    timeAffectTexCoords += P_getTime(tt0, tt1);
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
  std::cerr << "timeSortE=" << timeSortE << "ms\n";
  std::cerr << "timeGetPaths=" << timeGetPaths << "ms\n";
  std::cerr << "timeAffectTexCoords=" << timeAffectTexCoords << "ms\n";
  std::cerr << "DBG_NUM_EDGES=" << DBG_NUM_EDGES
            << " mean=" << DBG_NUM_EDGES / (float)numYs << "\n";
  std::cerr << "DBG_NUM_EDGES_USED=" << DBG_NUM_EDGES_USED
            << " mean=" << DBG_NUM_EDGES_USED / (float)numYs << "\n";
  std::cerr << "DBG_NUM_EDGES_USED2=" << DBG_NUM_EDGES_USED2
            << " mean=" << DBG_NUM_EDGES_USED2 / (float)numYs << "\n";
  std::cerr << "DBG_MAX_NUM_SEGMENTS=" << DBG_MAX_NUM_SEGMENTS << "\n";
  std::cerr << "DBG_MAX_NUM_VERTEXLINKS=" << DBG_MAX_NUM_VERTEXLINKS << "\n";
#endif //TIME_TEXCOORDS

  //exit(10); //DEBUG

#ifndef NDEBUG
  for (uint32_t i = 0; i < mesh.numVertices; ++i) {
    if (mesh.texCoords[2 * i + 0] == TEXCOORD_INVALID ||
        mesh.texCoords[2 * i + 1] == TEXCOORD_INVALID)
      std::cerr << "**** ERROR: texcoords for vIdx=" << i << ": ("
                << mesh.texCoords[2 * i + 0] << ", "
                << mesh.texCoords[2 * i + 1] << ") ****\n";
  }
#endif //NDEBUG

#ifdef TIME_TEXCOORDS
  gettimeofday(&t0, nullptr);
#endif //TIME_TEXCOORDS

  //normalizeTexCoords(mesh);
  normalizeTexCoordsB(mesh);

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
