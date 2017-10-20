#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cstdlib>


struct Edge
{
  uint32_t vertexIndex[2];

  Edge(uint32_t v0=-1, uint32_t v1=-1)
  {
    vertexIndex[0] = v0;
    vertexIndex[1] = v1;
  }


  bool operator==(Edge e2) const
  {
    return vertexIndex[0] == e2.vertexIndex[0]
      && vertexIndex[1] == e2.vertexIndex[1];
  }

};

std::ostream &
operator<<(std::ostream &out, Edge e)
{
  out<<"("<<e.vertexIndex[0]<<", "<<e.vertexIndex[1]<<")";
  return out;
}

std::ostream &
operator<<(std::ostream &out, const std::vector<Edge> &edges)
{
  const size_t sz = edges.size();
  out<<"[";
  for (size_t i=0; i<sz; ++i)
    out<<" "<<edges[i];
  out<<" ]";
  return out;
}

std::ostream &
operator<<(std::ostream &out, const std::vector<std::vector<Edge> > &edgesV)
{
  const size_t sz = edgesV.size();
  for (size_t i=0; i<sz; ++i)
    out<<i<<" "<<edgesV[i]<<"\n";
  return out;
}

//@param[in, out] labels
size_t
removeHoles(std::vector<uint32_t> &labels)
{
  //we have labels :         0 0 2 0 2 0 4 0 4 2
  //we want to rename them:  0 0 1 0 1 0 2 0 2 1
  //If we have a hole in indices [like here "1" is missing between "0" and "2"]
  // we are sure that this indice is not present in remaining values


#ifndef NDEBUG
  for (size_t i=0; i<labels.size(); ++i) 
    assert(labels[i] <= i);
#endif //NDEBUG

  uint32_t cpt = 0;
  for (size_t i=0; i<labels.size(); ++i) {
    const uint32_t v = labels[i];
    if (v > cpt) {
      for (size_t j=i; j<labels.size(); ++j) {
	assert(labels[j] != cpt);
	if (labels[j] == v)
	  labels[j] = cpt;
      }
      ++cpt;
    }
    else if (v == cpt)
      ++cpt;
  }
  return cpt;
}


bool
compareLabels(const std::vector<uint32_t> &labels, const std::vector<uint32_t> &labelsC)
{
  const size_t sz = labels.size();
  if (sz != labelsC.size()) {
    std::cerr<<"ERROR: different sizes "<<labels.size()<<" vs "<<labelsC.size()<<"\n";
    return false;
  }
  for (size_t i=0; i<sz; ++i) {
    if (labels[i] != labelsC[i]) {
      std::cerr<<"labels["<<i<<"]="<<labels[i]<<" != labelsC["<<i<<"]="<<labelsC[i]<<"\n";
      return false;
    }
  }

  return true;
}

void
testRemoveHoles1()
{
  std::vector<uint32_t> labels;
  labels.push_back(0);
  labels.push_back(0);
  labels.push_back(2);
  labels.push_back(0);
  labels.push_back(2);
  labels.push_back(0);
  labels.push_back(4);
  labels.push_back(0);
  labels.push_back(4);
  labels.push_back(2);

  std::vector<uint32_t> labelsC;
  labelsC.push_back(0);
  labelsC.push_back(0);
  labelsC.push_back(1);
  labelsC.push_back(0);
  labelsC.push_back(1);
  labelsC.push_back(0);
  labelsC.push_back(2);
  labelsC.push_back(0);
  labelsC.push_back(2);
  labelsC.push_back(1);


  removeHoles(labels);

  if (! compareLabels(labels, labelsC)) {
    std::cerr<<"testRemoveHoles1() failed\n";
    exit(10);
  }

  
}

void
testRemoveHoles2()
{
  std::vector<uint32_t> labels;
  labels.push_back(0);
  labels.push_back(0);
  labels.push_back(2);
  labels.push_back(0);
  labels.push_back(3);
  labels.push_back(0);
  labels.push_back(6);
  labels.push_back(0);
  labels.push_back(2);
  labels.push_back(3);

  std::vector<uint32_t> labelsC;
  labelsC.push_back(0);
  labelsC.push_back(0);
  labelsC.push_back(1);
  labelsC.push_back(0);
  labelsC.push_back(2);
  labelsC.push_back(0);
  labelsC.push_back(3);
  labelsC.push_back(0);
  labelsC.push_back(1);
  labelsC.push_back(2);

  removeHoles(labels);

  if (! compareLabels(labels, labelsC)) {
    std::cerr<<"testRemoveHoles1() failed\n";
    exit(10);
  }

  
}


void
getContours(const std::vector<Edge> &edges,
	    std::vector<std::vector<Edge> > &outEdges)
{
  const size_t numEdges = edges.size();
  

  std::vector<Edge> extremities;
  extremities.reserve(numEdges/3); //arbitrary

  typedef uint32_t label; //we can probably use something smaller than uint32_t... 
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


  for (size_t i=0; i<numEdges; ++i) {

    assert(contourLabels[i] == INVALID);
      {

      //edge not yet used in any contour
      //start new contour

      const Edge &e = edges[i];

      assert(e.vertexIndex[0] < e.vertexIndex[1]);

      //check if it can be added to an existing contour
      
      bool found = false;
      const size_t numContours = extremities.size();
      for (size_t k=0; k<numContours; ++k) {
	if (extremities[k].vertexIndex[0] == e.vertexIndex[0]) {
	  extremities[k].vertexIndex[0] = e.vertexIndex[1];
	  contourLabels[i] = k;
	  //std::cerr<<"edge "<<e<<" label="<<k<<" extremities[k]="<<extremities[k]<<"\n";
	  found = true;
	  break;
	}
	else if (extremities[k].vertexIndex[0] == e.vertexIndex[1]) {
	  extremities[k].vertexIndex[0] = e.vertexIndex[0];
	  contourLabels[i] = k;
	  //std::cerr<<"edge "<<e<<" label="<<k<<" extremities[k]="<<extremities[k]<<"\n";
	  found = true;
	  break;
	}
	else if (extremities[k].vertexIndex[1] == e.vertexIndex[0]) {
	  extremities[k].vertexIndex[1] = e.vertexIndex[1];
	  contourLabels[i] = k;
	  //std::cerr<<"edge "<<e<<" label="<<k<<" extremities[k]="<<extremities[k]<<"\n";
	  found = true;
	  break;
	}
	else if (extremities[k].vertexIndex[1] == e.vertexIndex[1]) {
	  extremities[k].vertexIndex[1] = e.vertexIndex[0];
	  contourLabels[i] = k;
	  //std::cerr<<"edge "<<e<<" label="<<k<<" extremities[k]="<<extremities[k]<<"\n";
	  found = true;
	  break;
	}
      }

      if (! found) {
	//start a new contour
	contourLabels[i] = numContours;
	extremities.push_back(e);
      }
    }

  }
    
  const size_t numContours = extremities.size();
  std::vector<label> contourLabelsChanges(numContours);
  for (size_t i=0; i<numContours; ++i) {
    contourLabelsChanges[i] = i;
  }
  
  
  std::vector<bool> extremityValid(numContours, true);
  for (size_t i=0; i<numContours; ++i) {
    if (extremityValid[i]) {
      
      Edge &ext_i = extremities[i];
      
      bool change = false;
      do {
	change = false;

	for (size_t j=i+1; j<numContours; ++j) {
	  
	  if (extremityValid[j]) {
	    
	    bool found = false;

	    Edge &ext_j = extremities[j];
	    
	    if (ext_i.vertexIndex[0] == ext_j.vertexIndex[0]) {
	      //std::cerr<<"extremities["<<i<<"]="<<extremities[i]<<"+"<<"extremities["<<j<<"]="<<extremities[j];
	      ext_i.vertexIndex[0] = ext_j.vertexIndex[1];
	      //std::cerr<<"=>"<<ext_i<<"\n";
	      found = true;
	    }
	    else if (ext_i.vertexIndex[0] == ext_j.vertexIndex[1]) {
	      //std::cerr<<"extremities["<<i<<"]="<<extremities[i]<<"+"<<"extremities["<<j<<"]="<<extremities[j];
	      ext_i.vertexIndex[0] = ext_j.vertexIndex[0];
	      //std::cerr<<"=>"<<ext_i<<"\n";
	      found = true;
	      }
	    else if (ext_i.vertexIndex[1] == ext_j.vertexIndex[0]) {
	      //std::cerr<<"extremities["<<i<<"]="<<extremities[i]<<"+"<<"extremities["<<j<<"]="<<extremities[j];
	      ext_i.vertexIndex[1] = ext_j.vertexIndex[1];
	      //std::cerr<<"=>"<<ext_i<<"\n";
	      found = true;
	    }
	    else if (ext_i.vertexIndex[1] == ext_j.vertexIndex[1]) {
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
      }
      while (change == true);
    }
  }
    

  
  std::cerr<<"contourLabelsChanges:\n";
  for (size_t i=0; i<contourLabelsChanges.size(); ++i)
    std::cerr<<contourLabelsChanges[i]<<" ";
  std::cerr<<"\n";

  const size_t numFinalContours = removeHoles(contourLabelsChanges);

  std::cerr<<"contourLabelsChanges':\n";
  for (size_t i=0; i<contourLabelsChanges.size(); ++i)
    std::cerr<<contourLabelsChanges[i]<<" ";
  std::cerr<<"\n";


  
  std::cerr<<"numFinalContours="<<numFinalContours<<"\n";

  outEdges.resize(numFinalContours);
  for (size_t i=0; i<numEdges; ++i) {
    assert(contourLabels[i] < contourLabelsChanges.size());
    //std::cerr<<"edges["<<i<<"]="<<edges[i]<<" label="<<contourLabels[i]<<" finaleLabel=contourLabelsChanges[contourLabels[i]]="<<contourLabelsChanges[contourLabels[i]]<<"\n";
    const size_t ind = contourLabelsChanges[contourLabels[i]];
    assert(ind < numFinalContours);
    outEdges[ind].push_back(edges[i]);
  }
    

#ifndef NDEBUG
  {
    size_t totalNumEdges = 0;
    for (size_t i=0; i<outEdges.size(); ++i) {
      totalNumEdges += outEdges[i].size();
    }
    assert(totalNumEdges == edges.size());

  }
#endif //NDEBUG



}

struct EdgeSorter
{
  inline bool operator()(Edge e1, Edge e2) const
  {
    return e1.vertexIndex[0] < e2.vertexIndex[0] || (e1.vertexIndex[0] == e2.vertexIndex[0] && e1.vertexIndex[1] < e2.vertexIndex[1]);
  }
};

struct EdgeVectorSorter
{
  inline bool operator()(const std::vector<Edge> &e1, const std::vector<Edge> &e2) const
  {
    if (! e1.empty() && ! e2.empty()) 
      return EdgeSorter()(e1[0], e2[0]); //compare only first edge
    return false;
  }
};

void
sortContours(std::vector<std::vector<Edge> > &outEdges)
{
  const size_t s = outEdges.size();
  for (size_t i=0; i<s; ++i) {
    std::sort(outEdges[i].begin(), outEdges[i].end(), EdgeSorter());
  }
  
  std::sort(outEdges.begin(), outEdges.end(), EdgeVectorSorter());
}

bool
compareContours(std::vector<std::vector<Edge> > &outEdges, std::vector<std::vector<Edge> > &outEdgesC)
{
  sortContours(outEdges);
  sortContours(outEdgesC);
  const size_t s = outEdges.size();
  if (s != outEdgesC.size()) {
    
    std::cerr<<"ERROR: Not same number of contours "<<s<<" vs "<<outEdgesC.size()<<"\n";
    std::cerr<<"correct:\n"<<outEdgesC<<"\n";
    std::cerr<<"me:\n"<<outEdges<<"\n";
    return false;
  }


  for (size_t i=0; i<s; ++i) {
    const size_t s1 = outEdges[i].size();
    const size_t s2 = outEdgesC[i].size();
    if (s1 != s2) {
      std::cerr<<"ERROR: Not same number of edges in contour "<<i<<": "<<s1<<" vs "<<s2<<"\n";
      std::cerr<<"correct:\n"<<outEdgesC[i]<<"\n";
    std::cerr<<"me:\n"<<outEdges[i]<<"\n";
      return false;
    }
    for (size_t j=0; j<s1; ++j) {
      if (! ((outEdges[i])[j] == (outEdges[i])[j])) {
	std::cerr<<"ERROR: in contour "<<i<<" edge "<<j<<" is different between contours: "<<(outEdges[i])[j]<<" vs "<<(outEdges[i])[j]<<"\n";
	return false;
      }
    }

  }


  return true;
}


void
test1()
{
  std::vector<Edge> edges;
  edges.push_back(Edge(1, 6));
  edges.push_back(Edge(3, 4));
  edges.push_back(Edge(8, 9));
  edges.push_back(Edge(5, 7));
  edges.push_back(Edge(2, 7));
  edges.push_back(Edge(4, 5));
  edges.push_back(Edge(8, 10));
  edges.push_back(Edge(3, 6));
  edges.push_back(Edge(9, 10));
  edges.push_back(Edge(11, 12));
  edges.push_back(Edge(12, 13));
  edges.push_back(Edge(13, 14));
  edges.push_back(Edge(1, 2));
  edges.push_back(Edge(11, 14));

  std::vector<std::vector<Edge> > outEdgesC;
  outEdgesC.resize(3);
  outEdgesC[0].push_back(Edge(1, 6));
  outEdgesC[0].push_back(Edge(1, 2));
  outEdgesC[0].push_back(Edge(2, 7));
  outEdgesC[0].push_back(Edge(5, 7));
  outEdgesC[0].push_back(Edge(4, 5));
  outEdgesC[0].push_back(Edge(3, 4));
  outEdgesC[0].push_back(Edge(3, 6));

  outEdgesC[1].push_back(Edge(8, 9));
  outEdgesC[1].push_back(Edge(9, 10));
  outEdgesC[1].push_back(Edge(8, 10));

  outEdgesC[2].push_back(Edge(11, 12));
  outEdgesC[2].push_back(Edge(12, 13));
  outEdgesC[2].push_back(Edge(13, 14));
  outEdgesC[2].push_back(Edge(11, 14));


  std::vector<std::vector<Edge> > outEdges;
  
  getContours(edges, outEdges);

  if (! compareContours(outEdges, outEdgesC)) {
    std::cerr<<"test1 failed\n";
    std::cerr<<"input: "<<edges<<"\n";
    exit(10);
  }

  std::cerr<<"### test1 passed\n";
}

void
test1b()
{
  std::vector<Edge> edges;
  edges.push_back(Edge(1, 6));
  edges.push_back(Edge(3, 4));
  edges.push_back(Edge(8, 9));
  edges.push_back(Edge(5, 7));
  edges.push_back(Edge(2, 7));
  edges.push_back(Edge(13, 14));
  edges.push_back(Edge(1, 2));
  edges.push_back(Edge(11, 14));
  edges.push_back(Edge(4, 5));
  edges.push_back(Edge(8, 10));
  edges.push_back(Edge(3, 6));
  edges.push_back(Edge(9, 10));
  edges.push_back(Edge(11, 12));
  edges.push_back(Edge(12, 13));

  std::vector<std::vector<Edge> > outEdgesC;
  outEdgesC.resize(3);
  outEdgesC[0].push_back(Edge(1, 6));
  outEdgesC[0].push_back(Edge(1, 2));
  outEdgesC[0].push_back(Edge(2, 7));
  outEdgesC[0].push_back(Edge(5, 7));
  outEdgesC[0].push_back(Edge(4, 5));
  outEdgesC[0].push_back(Edge(3, 4));
  outEdgesC[0].push_back(Edge(3, 6));

  outEdgesC[1].push_back(Edge(8, 9));
  outEdgesC[1].push_back(Edge(9, 10));
  outEdgesC[1].push_back(Edge(8, 10));

  outEdgesC[2].push_back(Edge(11, 12));
  outEdgesC[2].push_back(Edge(12, 13));
  outEdgesC[2].push_back(Edge(13, 14));
  outEdgesC[2].push_back(Edge(11, 14));


  std::vector<std::vector<Edge> > outEdges;
  
  getContours(edges, outEdges);

  if (! compareContours(outEdges, outEdgesC)) {
    std::cerr<<"test1b failed\n";
    std::cerr<<"input: "<<edges<<"\n";
    exit(10);
  }

  std::cerr<<"### test1b passed\n";
}

void
test1c()
{
  std::vector<Edge> edges;

  edges.push_back(Edge(1, 2));
  edges.push_back(Edge(11, 14));
  edges.push_back(Edge(4, 5));
  edges.push_back(Edge(8, 10));
  edges.push_back(Edge(3, 6));
  edges.push_back(Edge(1, 6));
  edges.push_back(Edge(3, 4));
  edges.push_back(Edge(8, 9));
  edges.push_back(Edge(5, 7));
  edges.push_back(Edge(2, 7));
  edges.push_back(Edge(13, 14));
  edges.push_back(Edge(9, 10));
  edges.push_back(Edge(11, 12));
  edges.push_back(Edge(12, 13));

  std::vector<std::vector<Edge> > outEdgesC;
  outEdgesC.resize(3);
  outEdgesC[0].push_back(Edge(1, 6));
  outEdgesC[0].push_back(Edge(1, 2));
  outEdgesC[0].push_back(Edge(2, 7));
  outEdgesC[0].push_back(Edge(5, 7));
  outEdgesC[0].push_back(Edge(4, 5));
  outEdgesC[0].push_back(Edge(3, 4));
  outEdgesC[0].push_back(Edge(3, 6));

  outEdgesC[1].push_back(Edge(8, 9));
  outEdgesC[1].push_back(Edge(9, 10));
  outEdgesC[1].push_back(Edge(8, 10));

  outEdgesC[2].push_back(Edge(11, 12));
  outEdgesC[2].push_back(Edge(12, 13));
  outEdgesC[2].push_back(Edge(13, 14));
  outEdgesC[2].push_back(Edge(11, 14));


  std::vector<std::vector<Edge> > outEdges;
  
  getContours(edges, outEdges);

  if (! compareContours(outEdges, outEdgesC)) {
    std::cerr<<"test1c failed\n";
    std::cerr<<"input: "<<edges<<"\n";
    exit(10);
  }

  std::cerr<<"### test1c passed\n";
}

void
test1d()
{
  std::vector<Edge> edges;

  edges.push_back(Edge(11, 14));
  edges.push_back(Edge(1, 2));
  edges.push_back(Edge(4, 5));
  edges.push_back(Edge(3, 6));
  edges.push_back(Edge(8, 10));
  edges.push_back(Edge(12, 13));
  edges.push_back(Edge(1, 6));
  edges.push_back(Edge(8, 9));
  edges.push_back(Edge(3, 4));
  edges.push_back(Edge(5, 7));
  edges.push_back(Edge(2, 7));
  edges.push_back(Edge(9, 10));
  edges.push_back(Edge(13, 14));
  edges.push_back(Edge(11, 12));

  std::vector<std::vector<Edge> > outEdgesC;
  outEdgesC.resize(3);
  outEdgesC[0].push_back(Edge(1, 6));
  outEdgesC[0].push_back(Edge(1, 2));
  outEdgesC[0].push_back(Edge(2, 7));
  outEdgesC[0].push_back(Edge(5, 7));
  outEdgesC[0].push_back(Edge(4, 5));
  outEdgesC[0].push_back(Edge(3, 4));
  outEdgesC[0].push_back(Edge(3, 6));

  outEdgesC[1].push_back(Edge(8, 9));
  outEdgesC[1].push_back(Edge(9, 10));
  outEdgesC[1].push_back(Edge(8, 10));

  outEdgesC[2].push_back(Edge(11, 12));
  outEdgesC[2].push_back(Edge(12, 13));
  outEdgesC[2].push_back(Edge(13, 14));
  outEdgesC[2].push_back(Edge(11, 14));


  std::vector<std::vector<Edge> > outEdges;
  
  getContours(edges, outEdges);

  if (! compareContours(outEdges, outEdgesC)) {
    std::cerr<<"test1d failed\n";
    std::cerr<<"input: "<<edges<<"\n";
    exit(10);
  }

  std::cerr<<"### test1d passed\n";
}

void
test2()
{
  std::vector<Edge> edges;

  edges.push_back(Edge(18, 19));
  edges.push_back(Edge(8, 9));
  edges.push_back(Edge(11, 14));
  edges.push_back(Edge(1, 2));
  edges.push_back(Edge(4, 5));
  edges.push_back(Edge(3, 6));
  edges.push_back(Edge(8, 10));
  edges.push_back(Edge(12, 13));
  edges.push_back(Edge(1, 6));
  edges.push_back(Edge(15, 16));
  edges.push_back(Edge(5, 7));
  edges.push_back(Edge(2, 7));
  edges.push_back(Edge(9, 10));
  edges.push_back(Edge(17, 18));
  edges.push_back(Edge(13, 14));
  edges.push_back(Edge(11, 12));
  edges.push_back(Edge(4, 16));
  edges.push_back(Edge(3, 15));
  edges.push_back(Edge(17, 19));

  std::vector<std::vector<Edge> > outEdgesC;
  outEdgesC.resize(4);
  outEdgesC[0].push_back(Edge(1, 6));
  outEdgesC[0].push_back(Edge(1, 2));
  outEdgesC[0].push_back(Edge(2, 7));
  outEdgesC[0].push_back(Edge(5, 7));
  outEdgesC[0].push_back(Edge(4, 5));
  outEdgesC[0].push_back(Edge(15, 16));
  outEdgesC[0].push_back(Edge(4, 16));
  outEdgesC[0].push_back(Edge(3, 15));
  outEdgesC[0].push_back(Edge(3, 6));

  outEdgesC[1].push_back(Edge(8, 9));
  outEdgesC[1].push_back(Edge(9, 10));
  outEdgesC[1].push_back(Edge(8, 10));

  outEdgesC[2].push_back(Edge(11, 12));
  outEdgesC[2].push_back(Edge(12, 13));
  outEdgesC[2].push_back(Edge(13, 14));
  outEdgesC[2].push_back(Edge(11, 14));

  outEdgesC[3].push_back(Edge(18, 19));
  outEdgesC[3].push_back(Edge(17, 18));
  outEdgesC[3].push_back(Edge(17, 19));

  std::vector<std::vector<Edge> > outEdges;
  
  getContours(edges, outEdges);

  if (! compareContours(outEdges, outEdgesC)) {
    std::cerr<<"test2 failed\n";
    std::cerr<<"input: "<<edges<<"\n";
    exit(10);
  }

  std::cerr<<"### test2 passed\n";
}


int
main()
{
  testRemoveHoles1();
  testRemoveHoles2();

  test1();
  test1b();
  test1c();
  test1d();

  test2();

}
