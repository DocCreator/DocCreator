#include "Mesh2SurfaceMesh.hpp"

#include "Mesh.hpp"
#include <cassert>
#include <vector>

void
mesh2surfaceMesh(const Mesh &m, surface_mesh::Surface_mesh &sm)
{
  assert(m.isValid());

  std::vector<surface_mesh::Texture_coordinate> all_tex_coords;
  std::vector<int> halfedge_tex_idx;
  surface_mesh::Surface_mesh::Halfedge_property<
    surface_mesh::Texture_coordinate>
    tex_coords =
      sm.halfedge_property<surface_mesh::Texture_coordinate>("h:texcoord");
  bool with_tex_coord = false;

  surface_mesh::Surface_mesh::Vertex_property<surface_mesh::Normal> normals;
  surface_mesh::Surface_mesh::Vertex_property<surface_mesh::Texture_coordinate>
    texcoords;
  if (m.hasNormals())
    normals = sm.vertex_property<surface_mesh::Normal>("v:normal");
  if (m.hasTexCoords())
    texcoords =
      sm.vertex_property<surface_mesh::Texture_coordinate>("v:texcoord");

  sm.clear();

  const unsigned int nV = m.numVertices;
  const unsigned int nF = m.numTriangles;
  const unsigned int nE = 3 * nV; //Max value. Use getEdges(m) ???
  sm.reserve(nV, nE, nF);

  surface_mesh::Surface_mesh::Vertex v;
  for (uint32_t i = 0; i < m.numVertices; ++i) {
    surface_mesh::Point p;
    p[0] = m.vertices[3 * i + 0];
    p[1] = m.vertices[3 * i + 1];
    p[2] = m.vertices[3 * i + 2];
    v = sm.add_vertex(p);

    if (m.hasNormals()) {
      surface_mesh::Vec3f n;
      n[0] = m.normals[3 * i + 0];
      n[1] = m.normals[3 * i + 1];
      n[2] = m.normals[3 * i + 2];
      normals[v] = n;
    }

    if (m.hasTexCoords()) {
      surface_mesh::Vec2f t;
      t[0] = m.texCoords[2 * i + 0];
      t[1] = m.texCoords[2 * i + 1];
      texcoords[v][0] = t[0];
      texcoords[v][1] = t[1];
    }
  }

  std::vector<surface_mesh::Surface_mesh::Vertex> vertices(3);
  for (uint32_t i = 0; i < m.numTriangles; ++i) {
    vertices[0] = surface_mesh::Surface_mesh::Vertex(m.triangles[3 * i + 0]);
    vertices[1] = surface_mesh::Surface_mesh::Vertex(m.triangles[3 * i + 1]);
    vertices[2] = surface_mesh::Surface_mesh::Vertex(m.triangles[3 * i + 2]);
    sm.add_face(vertices);
  }
}

void
surfaceMesh2Mesh(const surface_mesh::Surface_mesh &sm, Mesh &m)
{

  bool has_normals = false;
  bool has_texcoords = false;
  surface_mesh::Surface_mesh::Vertex_property<surface_mesh::Normal> normals =
    sm.get_vertex_property<surface_mesh::Normal>("v:normal");
  surface_mesh::Surface_mesh::Vertex_property<surface_mesh::Texture_coordinate>
    texcoords =
      sm.get_vertex_property<surface_mesh::Texture_coordinate>("v:texcoord");
  if (normals)
    has_normals = true;
  if (texcoords)
    has_texcoords = true;

  const uint32_t numVertices = sm.n_vertices();
  const uint32_t numTriangles = sm.n_faces();

  m.allocateVertices(numVertices);
  if (has_normals)
    m.allocateNormals();
  if (has_texcoords)
    m.allocateTexCoords();
  m.allocateTriangles(numTriangles);

  uint32_t i = 0;
  surface_mesh::Surface_mesh::Vertex_property<surface_mesh::Point> points =
    sm.get_vertex_property<surface_mesh::Point>("v:point");
  for (surface_mesh::Surface_mesh::Vertex_iterator vit = sm.vertices_begin();
       vit != sm.vertices_end();
       ++vit, ++i) {
    const surface_mesh::Point &p = points[*vit];
    m.vertices[3 * i + 0] = p[0];
    m.vertices[3 * i + 1] = p[1];
    m.vertices[3 * i + 2] = p[2];

    if (has_normals) {
      const surface_mesh::Normal &n = normals[*vit];
      m.normals[3 * i + 0] = n[0];
      m.normals[3 * i + 1] = n[1];
      m.normals[3 * i + 2] = n[2];
    }
    if (has_texcoords) {
      const surface_mesh::Texture_coordinate &t = texcoords[*vit];
      m.texCoords[2 * i + 0] = t[0];
      m.texCoords[2 * i + 1] = t[1];
    }
  }

  i = 0;
  for (surface_mesh::Surface_mesh::Face_iterator fit = sm.faces_begin();
       fit != sm.faces_end();
       ++fit) {
    assert(sm.valence(*fit) == 3);
    surface_mesh::Surface_mesh::Vertex_around_face_circulator fvit =
      sm.vertices(*fit);
    m.triangles[3 * i + 0] = (*fvit).idx();
    ++fvit;
    m.triangles[3 * i + 1] = (*fvit).idx();
    ++fvit;
    m.triangles[3 * i + 2] = (*fvit).idx();
    ++fvit;
  }
}
