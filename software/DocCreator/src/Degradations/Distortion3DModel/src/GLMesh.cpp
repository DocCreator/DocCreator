#include "GLMesh.hpp"

#include <Eigen/Core>

#include <iostream> //DEBUG

GLMesh::GLMesh()
  : m_vertexArrayId(0)
  , m_vertexBufferId(0)
  , m_indexBufferId(0)
  , m_numIndices(0)
  , m_isInitialized(false)
{}

GLMesh::~GLMesh()
{
  if (m_isInitialized) {
    glDeleteBuffers(1, &m_vertexBufferId);
    glDeleteBuffers(1, &m_indexBufferId);
    glDeleteVertexArrays(1, &m_vertexArrayId);
  }
}

struct Vertex
{
  explicit Vertex(const Eigen::Vector3f &pos = Eigen::Vector3f::Zero(),
                  const Eigen::Vector3f &n = Eigen::Vector3f::Zero(),
                  const Eigen::Vector2f &uv = Eigen::Vector2f::Zero())
    : position(pos)
    , normal(n)
    , texcoord(uv)
  {}

  Eigen::Vector3f position;
  Eigen::Vector3f normal;
  Eigen::Vector2f texcoord;
};

std::ostream &
operator<<(std::ostream &o, const Vertex &v)
{
  o << "[p=" << v.position.transpose() << ", n=" << v.normal.transpose()
    << ", t=" << v.texcoord.transpose() << "]";
  return o;
}

void
GLMesh::init(const Mesh &m)
{
  assert(!m_isInitialized);
  assert(m.hasNormals());
  assert(m.isValid());

  //std::cerr << "GLMesh::init() this="<<this<<" m.numVertices=" << m.numVertices << "\n";

  std::vector<Vertex> vertices(m.numVertices);
  {
    if (m.hasNormals() && m.hasTexCoords()) {
      for (uint32_t i = 0; i < m.numVertices; ++i) {
        vertices[i] = Vertex(
          Eigen::Vector3f(m.vertices[3 * i + 0],
                          m.vertices[3 * i + 1],
                          m.vertices[3 * i + 2]),
          Eigen::Vector3f(
            m.normals[3 * i + 0], m.normals[3 * i + 1], m.normals[3 * i + 2]),
          Eigen::Vector2f(m.texCoords[2 * i + 0], m.texCoords[2 * i + 1]));
      }
    } else if (m.hasNormals()) {
      for (uint32_t i = 0; i < m.numVertices; ++i) {
        vertices[i] = Vertex(Eigen::Vector3f(m.vertices[3 * i + 0],
                                             m.vertices[3 * i + 1],
                                             m.vertices[3 * i + 2]),
                             Eigen::Vector3f(m.normals[3 * i + 0],
                                             m.normals[3 * i + 1],
                                             m.normals[3 * i + 2]));
      }
    } else if (m.hasTexCoords()) {
      for (uint32_t i = 0; i < m.numVertices; ++i) {
        vertices[i] = Vertex(
          Eigen::Vector3f(m.vertices[3 * i + 0],
                          m.vertices[3 * i + 1],
                          m.vertices[3 * i + 2]),
          Eigen::Vector3f::Zero(),
          Eigen::Vector2f(m.texCoords[2 * i + 0], m.texCoords[2 * i + 1]));
      }
    }
  }

  assert(vertices.size() == m.numVertices);

  GL_CHECK_ERROR_ALWAYS();

  //std::cerr<<"glGenVertexArrays="<<&glGenVertexArrays<<" &m_vertexArrayId="<<&m_vertexArrayId<<" m_vertexArrayId="<<m_vertexArrayId<<"\n";

  glGenVertexArrays(1, &m_vertexArrayId);

  GL_CHECK_ERROR_ALWAYS();

  glBindVertexArray(m_vertexArrayId);

  GL_CHECK_ERROR_ALWAYS();

  glGenBuffers(1, &m_vertexBufferId);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(Vertex) * vertices.size(),
    vertices[0].position.data(),
    GL_STATIC_DRAW); //positions/normals/textcoords are all passed at once.

  GL_CHECK_ERROR_ALWAYS();

  glGenBuffers(1, &m_indexBufferId);
  GL_CHECK_ERROR_ALWAYS();

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
  assert(sizeof(GL_UNSIGNED_INT) == sizeof(uint32_t)); //TODO: check
  GL_CHECK_ERROR_ALWAYS();

  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(uint32_t) * m.numTriangles * 3,
               m.triangles,
               GL_STATIC_DRAW);

  GL_CHECK_ERROR_ALWAYS();

  m_numIndices = m.numTriangles * 3;

  glBindVertexArray(0);

  GL_CHECK_ERROR_ALWAYS();

  m_isInitialized = true;
  assert(m_isInitialized);
}

void
GLMesh::draw(GLuint progId) const
{
  GL_CHECK_ERROR_ALWAYS();

  glBindVertexArray(m_vertexArrayId);

  GL_CHECK_ERROR_ALWAYS();

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);

  GL_CHECK_ERROR_ALWAYS();

  GLint vertex_loc = glGetAttribLocation(progId, "vtx_position");
  //B ??? is it necessary to do it each frame ????

  if (vertex_loc >= 0) {
    glVertexAttribPointer(
			  vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(vertex_loc);
  }

  GL_CHECK_ERROR_ALWAYS();

  GLint norm_loc = glGetAttribLocation(progId, "vtx_normal");
  if (norm_loc >= 0) {
    glVertexAttribPointer(
      norm_loc,
      3,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void *)(sizeof(Eigen::Vector3f))); //B: normal after position
    glEnableVertexAttribArray(norm_loc);
  }

  GL_CHECK_ERROR_ALWAYS();

  GLint tex_loc = glGetAttribLocation(progId, "vtx_texcoord");
  if (tex_loc >= 0) {
    glVertexAttribPointer(
      tex_loc,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void *)(2 *
               sizeof(Eigen::Vector3f))); //B: texcoord after position+normal
    glEnableVertexAttribArray(tex_loc);
  }

  //std::cerr<<"vertex_loc="<<vertex_loc<<" norm_loc="<<norm_loc<<" tex_loc="<<tex_loc<<"\n";

  //std::cerr<<"m_numIndices="<<m_numIndices<<"\n";

  GL_CHECK_ERROR_ALWAYS();

  // draw the geometry
  glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, static_cast<void *>(nullptr));

  GL_CHECK_ERROR_ALWAYS();

  if (vertex_loc >= 0)
    glDisableVertexAttribArray(vertex_loc);

  GL_CHECK_ERROR_ALWAYS();

  glBindVertexArray(0);

  GL_CHECK_ERROR_ALWAYS();
}
