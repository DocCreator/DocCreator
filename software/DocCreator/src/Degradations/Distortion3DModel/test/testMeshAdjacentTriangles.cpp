
#include "Mesh.hpp"
#include <iostream>

bool
checkEqual(const std::vector<uint32_t> &v1,
	   const std::vector<uint32_t> &v2)
{
  if (v1.size() != v2.size()) {
    std::cerr<<"ERROR: wrong size : "<<v1.size()<<" vs "<<v2.size()<<"\n";
    return false;
  }

  for (size_t i=0; i<v2.size(); ++i) {
    if (v1[i] != v2[i]) {
      std::cerr<<"ERROR: difference at i="<<i<<" v1[i]="<<v1[i]<<" v2[i]="<<v2[i]<<"\n";
      const uint32_t j = i/3;
      for (int k=0; k<3; ++k) 
	std::cerr<<"v1[3*"<<j<<"+"<<k<<"]="<<v1[3*j+k]<<" v2[3*"<<j<<"+"<<k<<"]="<<v2[3*j+k]<<"\n";
      return false;
    }
  }

  return true;
}
      

void
test1()
{
  const uint32_t numTriangles = 10;
  const uint32_t numVertices = 10;
  
  Mesh m;

  m.allocateTriangles(numTriangles);

  m.allocateVertices(numVertices);
  assert(m.isValid());

  
  uint32_t *tris = m.triangles;
  tris[3*0+0] = 0; tris[3*0+1] = 1; tris[3*0+2] = 7; 
  tris[3*1+0] = 0; tris[3*1+1] = 2; tris[3*1+2] = 1; 
  tris[3*2+0] = 2; tris[3*2+1] = 3; tris[3*2+2] = 1; 
  tris[3*3+0] = 1; tris[3*3+1] = 3; tris[3*3+2] = 4; 
  tris[3*4+0] = 1; tris[3*4+1] = 4; tris[3*4+2] = 5; 
  tris[3*5+0] = 0; tris[3*5+1] = 8; tris[3*5+2] = 2; 
  tris[3*6+0] = 9; tris[3*6+1] = 8; tris[3*6+2] = 0; 
  tris[3*7+0] = 6; tris[3*7+1] = 9; tris[3*7+2] = 0; 
  tris[3*8+0] = 6; tris[3*8+1] = 0; tris[3*8+2] = 7; 
  tris[3*9+0] = 5; tris[3*9+1] = 7; tris[3*9+2] = 1; 

  
  std::vector<uint32_t> adjs(3*numTriangles, 333);
  adjs[3*0+0] = 8; adjs[3*0+1] = 1; adjs[3*0+2] = 9; 
  adjs[3*1+0] = 0; adjs[3*1+1] = 5; adjs[3*1+2] = 2; 
  adjs[3*2+0] = 1; adjs[3*2+1] = -1; adjs[3*2+2] = 3; 
  adjs[3*3+0] = 4; adjs[3*3+1] = 2; adjs[3*3+2] = -1; 
  adjs[3*4+0] = 9; adjs[3*4+1] = 3; adjs[3*4+2] = -1; 
  adjs[3*5+0] = 1; adjs[3*5+1] = 6; adjs[3*5+2] = -1; 
  adjs[3*6+0] = 7; adjs[3*6+1] = -1; adjs[3*6+2] = 5; 
  adjs[3*7+0] = 8; adjs[3*7+1] = -1; adjs[3*7+2] = 6; 
  adjs[3*8+0] = -1; adjs[3*8+1] = 7; adjs[3*8+2] = 0; 
  adjs[3*9+0] = 4; adjs[3*9+1] = -1; adjs[3*9+2] = 0; 
  
  std::vector<uint32_t> adjacentTris;
  getAdjacentTriangles(m, adjacentTris);

   

  if (! checkEqual(adjs, adjacentTris)) {
    std::cerr<<"ERROR test1 failed !!!\n";
    exit(10);
  }
  std::cerr<<"test1 passed\n";

}

void
test2()
{
  const uint32_t numTriangles = 14;
  const uint32_t numVertices = 12;
  
  Mesh m;

  m.allocateTriangles(numTriangles);

  m.allocateVertices(numVertices);
  assert(m.isValid());

  
  uint32_t *tris = m.triangles;
  tris[3*0+0] = 0; tris[3*0+1] = 1; tris[3*0+2] = 7; 
  tris[3*1+0] = 0; tris[3*1+1] = 2; tris[3*1+2] = 1; 
  tris[3*2+0] = 2; tris[3*2+1] = 3; tris[3*2+2] = 1; 
  tris[3*3+0] = 1; tris[3*3+1] = 3; tris[3*3+2] = 4; 
  tris[3*4+0] = 1; tris[3*4+1] = 4; tris[3*4+2] = 5; 
  tris[3*5+0] = 0; tris[3*5+1] = 8; tris[3*5+2] = 2; 
  tris[3*6+0] = 9; tris[3*6+1] = 8; tris[3*6+2] = 0; 
  tris[3*7+0] = 6; tris[3*7+1] = 9; tris[3*7+2] = 0; 
  tris[3*8+0] = 6; tris[3*8+1] = 0; tris[3*8+2] = 7; 
  tris[3*9+0] = 5; tris[3*9+1] = 7; tris[3*9+2] = 1; 
  tris[3*10+0] = 5 ; tris[3*10+1] = 4 ; tris[3*10+2] = 10; 
  tris[3*11+0] = 10; tris[3*11+1] = 7; tris[3*11+2] = 5; 
  tris[3*12+0] = 3; tris[3*12+1] = 11; tris[3*12+2] = 4; 
  tris[3*13+0] = 11; tris[3*13+1] = 10; tris[3*13+2] = 4; 

  
  std::vector<uint32_t> adjs(3*numTriangles, 333);
  adjs[3*0+0] = 8; adjs[3*0+1] = 1; adjs[3*0+2] = 9; 
  adjs[3*1+0] = 0; adjs[3*1+1] = 5; adjs[3*1+2] = 2; 
  adjs[3*2+0] = 1; adjs[3*2+1] = -1; adjs[3*2+2] = 3; 
  adjs[3*3+0] = 4; adjs[3*3+1] = 2; adjs[3*3+2] = 12; 
  adjs[3*4+0] = 9; adjs[3*4+1] = 3; adjs[3*4+2] = 10; 
  adjs[3*5+0] = 1; adjs[3*5+1] = 6; adjs[3*5+2] = -1; 
  adjs[3*6+0] = 7; adjs[3*6+1] = -1; adjs[3*6+2] = 5; 
  adjs[3*7+0] = 8; adjs[3*7+1] = -1; adjs[3*7+2] = 6; 
  adjs[3*8+0] = -1; adjs[3*8+1] = 7; adjs[3*8+2] = 0; 
  adjs[3*9+0] = 4; adjs[3*9+1] = 11; adjs[3*9+2] = 0; 
  adjs[3*10+0] = 11; adjs[3*10+1] = 4; adjs[3*10+2] = 13; 
  adjs[3*11+0] = 10; adjs[3*11+1] = -1; adjs[3*11+2] = 9; 
  adjs[3*12+0] = 3; adjs[3*12+1] = -1; adjs[3*12+2] = 13; 
  adjs[3*13+0] = 12; adjs[3*13+1] = -1; adjs[3*13+2] = 10; 
  
  std::vector<uint32_t> adjacentTris;
  getAdjacentTriangles(m, adjacentTris);

  
  if (! checkEqual(adjs, adjacentTris)) {
    std::cerr<<"ERROR: test2 failed !!!\n";
    exit(10);
  }
  std::cerr<<"test2 passed\n";

}



void
test3()
{
  const uint32_t numTriangles = 17;
  const uint32_t numVertices = 19;
  
  Mesh m;

  m.allocateTriangles(numTriangles);

  m.allocateVertices(numVertices);
  assert(m.isValid());

  
  uint32_t *tris = m.triangles;
  tris[3*0+0] = 0; tris[3*0+1] = 1; tris[3*0+2] = 7; 
  tris[3*1+0] = 0; tris[3*1+1] = 2; tris[3*1+2] = 1; 
  tris[3*2+0] = 2; tris[3*2+1] = 3; tris[3*2+2] = 1; 
  tris[3*3+0] = 1; tris[3*3+1] = 3; tris[3*3+2] = 4; 
  tris[3*4+0] = 1; tris[3*4+1] = 4; tris[3*4+2] = 5; 
  tris[3*5+0] = 0; tris[3*5+1] = 8; tris[3*5+2] = 2; 
  tris[3*6+0] = 9; tris[3*6+1] = 8; tris[3*6+2] = 0; 
  tris[3*7+0] = 6; tris[3*7+1] = 9; tris[3*7+2] = 0; 
  tris[3*8+0] = 6; tris[3*8+1] = 0; tris[3*8+2] = 7; 
  tris[3*9+0] = 5; tris[3*9+1] = 7; tris[3*9+2] = 1; 
  tris[3*10+0] = 5 ; tris[3*10+1] = 4 ; tris[3*10+2] = 10; 
  tris[3*11+0] = 10; tris[3*11+1] = 7; tris[3*11+2] = 5; 
  tris[3*12+0] = 3; tris[3*12+1] = 11; tris[3*12+2] = 4; 
  tris[3*13+0] = 11; tris[3*13+1] = 10; tris[3*13+2] = 4; 
  tris[3*14+0] = 14; tris[3*14+1] = 18; tris[3*14+2] = 16; 
  tris[3*15+0] = 15; tris[3*15+1] = 13; tris[3*15+2] = 17; 
  tris[3*16+0] = 12; tris[3*16+1] = 16; tris[3*16+2] = 18; 

  
  std::vector<uint32_t> adjs(3*numTriangles, 333);
  adjs[3*0+0] = 8; adjs[3*0+1] = 1; adjs[3*0+2] = 9; 
  adjs[3*1+0] = 0; adjs[3*1+1] = 5; adjs[3*1+2] = 2; 
  adjs[3*2+0] = 1; adjs[3*2+1] = -1; adjs[3*2+2] = 3; 
  adjs[3*3+0] = 4; adjs[3*3+1] = 2; adjs[3*3+2] = 12; 
  adjs[3*4+0] = 9; adjs[3*4+1] = 3; adjs[3*4+2] = 10; 
  adjs[3*5+0] = 1; adjs[3*5+1] = 6; adjs[3*5+2] = -1; 
  adjs[3*6+0] = 7; adjs[3*6+1] = -1; adjs[3*6+2] = 5; 
  adjs[3*7+0] = 8; adjs[3*7+1] = -1; adjs[3*7+2] = 6; 
  adjs[3*8+0] = -1; adjs[3*8+1] = 7; adjs[3*8+2] = 0; 
  adjs[3*9+0] = 4; adjs[3*9+1] = 11; adjs[3*9+2] = 0; 
  adjs[3*10+0] = 11; adjs[3*10+1] = 4; adjs[3*10+2] = 13; 
  adjs[3*11+0] = 10; adjs[3*11+1] = -1; adjs[3*11+2] = 9; 
  adjs[3*12+0] = 3; adjs[3*12+1] = -1; adjs[3*12+2] = 13; 
  adjs[3*13+0] = 12; adjs[3*13+1] = -1; adjs[3*13+2] = 10; 
  adjs[3*14+0] = -1; adjs[3*14+1] = -1; adjs[3*14+2] = 16; 
  adjs[3*15+0] = -1; adjs[3*15+1] = -1; adjs[3*15+2] = -1; 
  adjs[3*16+0] = -1; adjs[3*16+1] = -1; adjs[3*16+2] = 14; 
  
  std::vector<uint32_t> adjacentTris;
  getAdjacentTriangles(m, adjacentTris);

  
  if (! checkEqual(adjs, adjacentTris)) {
    std::cerr<<"ERROR: test3 failed !!!\n";
    exit(10);
  }
  std::cerr<<"test3 passed\n";

}



int
main()
{
  test1();
  test2();
  test3();


  return 0;
}
