

#include <iostream>
#include <vector>
#include <algorithm>

struct Edge
{
  Edge(uint32_t e1=-1, uint32_t e2=-1)
  {
    e[0] = e1;
    e[1] = e2;
  }

  bool operator==(Edge e2) const
  {
    return e[0] == e2.e[0] && e[1] == e2.e[1];
  }


  uint32_t e[2];
};

struct EdgeSorter
{
  bool operator()(Edge e1, Edge e2) 
  {
    return e1.e[0] < e2.e[0] || (e1.e[0] == e2.e[0] && e1.e[1] < e2.e[1]);
  }
};


std::ostream &
operator<<(std::ostream &out, Edge e1)
{
  out<<"("<<e1.e[0]<<", "<<e1.e[1]<<")";
  return out;
}

int
main()
{

  std::vector<Edge> edges;
  edges.push_back(Edge(0,2));
  edges.push_back(Edge(2,9));
  edges.push_back(Edge(0,9));
  edges.push_back(Edge(0,5));
  edges.push_back(Edge(2,5));
  edges.push_back(Edge(0,2));
  edges.push_back(Edge(0,4));
  edges.push_back(Edge(4,5));
  edges.push_back(Edge(0,5));
  edges.push_back(Edge(4,8));
  edges.push_back(Edge(5,8));
  edges.push_back(Edge(4,5));
  edges.push_back(Edge(0,1));
  edges.push_back(Edge(1,4));
  edges.push_back(Edge(0,4));
  edges.push_back(Edge(1,6));
  edges.push_back(Edge(4,6));
  edges.push_back(Edge(1,4));
  edges.push_back(Edge(4,6));
  edges.push_back(Edge(6,7));
  edges.push_back(Edge(4,7));
  edges.push_back(Edge(4,7));
  edges.push_back(Edge(7,8));
  edges.push_back(Edge(4,8));
  
  std::cerr<<"edges.size()="<<edges.size()<<"\n";

  for (size_t i=0; i<edges.size(); ++i) {
    const Edge &e = edges[i];
    if (e.e[0] >= e.e[1]) {
      std::cerr<<"ERROR: wrong init\n";
      exit(10);
    }
  }


  std::sort(edges.begin(), edges.end(), EdgeSorter());
  
  std::cerr<<"sorted edges : \n";
  for (std::vector<Edge>::iterator it = edges.begin();
       it != edges.end();
       ++it) {
    std::cerr<<(*it)<<"\n";
  }

 
  std::cerr<<"do unique : \n";


  //We remove duplicate edges.
  //We consider that we have at most 2 triangles sharing an edge.
  //(that is the same edge is present only two times).

  std::vector<Edge>::iterator result = edges.begin();
  {
    //inspired from std::unique()
    //http://www.cplusplus.com/reference/algorithm/unique/

    std::vector<Edge>::iterator first = edges.begin(); 
    const std::vector<Edge>::iterator last = edges.end();
    std::vector<Edge>::iterator next = first+1;
    
    while (first != last) {
      if (next == last) {
	*result = *first;
	++result;
	break;
      }
      
      if (! (*first == *next)) {
	
	*result = *first;
	
	//std::cerr<<"first="<<(*first)<<" last="<<*next<<" result="<<*result<<"\n";
	++result;
	++first;
	++next;
      }
      else {
	//std::cerr<<"skip "<<*first<<"\n";
	first = next + 1;
	next = first + 1;  
      }
      
    }
  }
  edges.resize(result-edges.begin());
  
  std::cerr<<"outside edges : \n";
  for (std::vector<Edge>::iterator it = edges.begin();
       it != edges.end();
       ++it) {
    std::cerr<<(*it)<<"\n";
  }

  std::vector<uint32_t> indices;
  indices.reserve(2*edges.size());
  
  {
    //Take into account that edges are sorted
    //(instead of pushing every index in @a indices)
    // We check that first index is not the last one inserted.
    // We still have to check for duplicates though !
    
    std::vector<Edge>::iterator first = edges.begin();
    uint32_t prevIndex = -1;
    const std::vector<Edge>::iterator last = edges.end();
    while (first != last) {
      if (first->e[0] != prevIndex)
	indices.push_back(first->e[0]);
      prevIndex = first->e[0];
      indices.push_back(first->e[1]);
      
      ++first;
    }
  }
  
  std::cerr<<"indices.size()="<<indices.size()<<" 2*edges.size()="<<2*edges.size()<<"\n";
  for (size_t i=0; i<indices.size(); ++i)
    std::cerr<<indices[i]<<" ";
  std::cerr<<"\n";

  
  std::sort(indices.begin(), indices.end());
  indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

  std::cerr<<"indices.size()="<<indices.size()<<"\n";
  for (size_t i=0; i<indices.size(); ++i)
    std::cerr<<indices[i]<<" ";
  std::cerr<<"\n";

  
  return 0;
}
