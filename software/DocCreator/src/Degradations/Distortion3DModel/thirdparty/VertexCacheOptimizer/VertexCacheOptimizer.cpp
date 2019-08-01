#include "VertexCacheOptimizer.hpp"

/*
 Original code by Fabien Giesen 
 https://github.com/rygorous/simple-ib-compress
 cf https://fgiesen.wordpress.com/2013/12/14/simple-lossless-index-buffer-compression/

 Implements Tom Forsyth Vertex Cache Optimisation :
 http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html

*/

#include <cassert>
#include <cmath>
#include <algorithm>//swap


struct VCacheVert
{
  int CachePos;      // its position in the cache (-1 if not in)
  int Score;         // its score (higher=better)
  int TrisLeft;      // # of not-yet-used tris
  uint32_t *TriList;      // list of triangle indices
  int OpenPos;       // position in "open vertex" list
};

struct VCacheTri
{
  int Score;         // current score (-1 if already done)
  int Inds[3];       // vertex indices
};


void optimizeIndexOrder(uint32_t VertexCount, uint32_t IndexCount, uint32_t *IndexBuffer)
{
  
  VCacheVert *verts = new VCacheVert[VertexCount];
  for(uint32_t i=0;i<VertexCount;i++)
  {
    verts[i].CachePos = -1;
    verts[i].Score = 0;
    verts[i].TrisLeft = 0;
    verts[i].TriList = 0;
    verts[i].OpenPos = -1;
  }

  // prepare triangles
  uint32_t nTris = IndexCount/3;
  VCacheTri *tris = new VCacheTri[nTris];
  uint32_t *indPtr = IndexBuffer;

  for(uint32_t i=0;i<nTris;i++)
  {
    tris[i].Score = 0;

    for(int j=0;j<3;j++)
    {
      int ind = *indPtr++;
      tris[i].Inds[j] = ind;
      verts[ind].TrisLeft++;
    }
  }

  // alloc space for vert->tri indices
  uint32_t *vertTriInd = new uint32_t[nTris*3];
  uint32_t *vertTriPtr = vertTriInd;

  for(uint32_t i=0;i<VertexCount;i++)
  {
    verts[i].TriList = vertTriPtr;
    vertTriPtr += verts[i].TrisLeft;
    verts[i].TrisLeft = 0;
  }

  // make vert->tri tables
  for(uint32_t i=0;i<nTris;i++)
  {
    for(int j=0;j<3;j++)
    {
      int ind = tris[i].Inds[j];
      verts[ind].TriList[verts[ind].TrisLeft] = i;
      verts[ind].TrisLeft++;
    }
  }

  // open vertices
  int *openVerts = new int[VertexCount];
  int openCount = 0;

  // the cache
  static const int cacheSize = 32;
  static const int maxValence = 15;
  int cache[cacheSize+3];
  int pos2Score[cacheSize];
  int val2Score[maxValence+1];

  for(int i=0;i<cacheSize+3;i++)
    cache[i] = -1;

  for(int i=0;i<cacheSize;i++)
  {
    float score = (i<3) ? 0.75f : powf(1.0f - (i-3)/float(cacheSize-3),1.5f);
    pos2Score[i] = (int) (score * 65536.0f + 0.5f);
  }

  val2Score[0] = 0;
  for(int i=1;i<16;i++)
  {
    float score = 2.0f / sqrtf((float) i);
    val2Score[i] = (int) (score * 65536.0f + 0.5f);
  }

  // outer loop: find triangle to start with
  indPtr = IndexBuffer;
  int seedPos = 0;

  while(1)
  {
    int seedScore = -1;
    int seedTri = -1;

    // if there are open vertices, search them for the seed triangle
    // which maximum score.
    for(int i=0;i<openCount;i++)
    {
      VCacheVert *vert = &verts[openVerts[i]];

      for(int j=0;j<vert->TrisLeft;j++)
      {
        int triInd = vert->TriList[j];
        VCacheTri *tri = &tris[triInd];

        if(tri->Score > seedScore)
        {
          seedScore = tri->Score;
          seedTri = triInd;
        }
      }
    }

    // if we haven't found a seed triangle yet, there are no open
    // vertices and we can pick any triangle
    if(seedTri == -1)
    {
      while((uint32_t)seedPos < nTris && tris[seedPos].Score<0)
        seedPos++;

      if((uint32_t)seedPos == nTris) // no triangle left, we're done!
        break;

      seedTri = seedPos;
    }

    // the main loop.
    int bestTriInd = seedTri;
    while(bestTriInd != -1)
    {
      VCacheTri *bestTri = &tris[bestTriInd];

      // mark this triangle as used, remove it from the "remaining tris"
      // list of the vertices it uses, and add it to the index buffer.
      bestTri->Score = -1;

      for(int j=0;j<3;j++)
      {
        int vertInd = bestTri->Inds[j];
        *indPtr++ = vertInd;

        VCacheVert *vert = &verts[vertInd];
        
        // find this triangles' entry
        int k = 0;
        while(vert->TriList[k] != (uint32_t)bestTriInd)
        {
          assert(k < vert->TrisLeft);
          k++;
        }

        // swap it to the end and decrement # of tris left
        if(--vert->TrisLeft)
          std::swap(vert->TriList[k],vert->TriList[vert->TrisLeft]);
        else if(vert->OpenPos >= 0)
          std::swap(openVerts[vert->OpenPos],openVerts[--openCount]);
      }

      // update cache status
      cache[cacheSize] = cache[cacheSize+1] = cache[cacheSize+2] = -1;

      for(int j=0;j<3;j++)
      {
        int ind = bestTri->Inds[j];
        cache[cacheSize+2] = ind;

        // find vertex index
        int pos;
        for(pos=0;cache[pos]!=ind;pos++);

        // move to front
        for(int k=pos;k>0;k--)
          cache[k] = cache[k-1];

        cache[0] = ind;

        // remove sentinel if it wasn't used
        if(pos!=cacheSize+2)
          cache[cacheSize+2] = -1;
      }

      // update vertex scores
      for(int i=0;i<cacheSize+3;i++)
      {
        int vertInd = cache[i];
        if(vertInd == -1)
          continue;

        VCacheVert *vert = &verts[vertInd];

        vert->Score = val2Score[std::min(vert->TrisLeft,maxValence)];
        if(i < cacheSize)
        {
          vert->CachePos = i;
          vert->Score += pos2Score[i];
        }
        else
          vert->CachePos = -1;

        // also add to open vertices list if the vertex is indeed open
        if(vert->OpenPos<0 && vert->TrisLeft)
        {
          vert->OpenPos = openCount;
          openVerts[openCount++] = vertInd;
        }
      }

      // update triangle scores, find new best triangle
      int bestTriScore = -1;
      bestTriInd = -1;

      for(int i=0;i<cacheSize;i++)
      {
        if(cache[i] == -1)
          continue;

        const VCacheVert *vert = &verts[cache[i]];

        for(int j=0;j<vert->TrisLeft;j++)
        {
          int triInd = vert->TriList[j];
          VCacheTri *tri = &tris[triInd];

          assert(tri->Score != -1);

          int score = 0;
          for(int k=0;k<3;k++)
            score += verts[tri->Inds[k]].Score;

          tri->Score = score;
          if(score > bestTriScore)
          {
            bestTriScore = score;
            bestTriInd = triInd;
          }
        }
      }
    }
  }

  // cleanup
  delete[] verts;
  delete[] tris;
  delete[] vertTriInd;
  delete[] openVerts;
}
