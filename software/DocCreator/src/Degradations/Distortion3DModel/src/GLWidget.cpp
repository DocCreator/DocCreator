#define NOMINMAX          //for Visual
#define _USE_MATH_DEFINES // for Visual

#include "GLWidget.hpp"

#include <algorithm> //min

#include <cmath> //M_PI_2

#include <chrono>
#include <iostream>

#include <QDebug>
#include <QImage>

#include "Mesh.hpp"
#include "brs.hpp"
#include "obj.hpp"

#include "TexCoordComputation.hpp"
#include "TexCoordComputationB.hpp"

#include "GLMesh.hpp"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include <QDesktopWidget>
#include <QGuiApplication>

#if QT_VERSION >= 0x050400
#include <QOpenGLContext>
#endif

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static const float DEFAULT_CAMERA_PHY = 0;      //0.5f;
static const float DEFAULT_CAMERA_THETA = 0;    //0.2f;
static const float DEFAULT_CAMERA_DIST = 0.87f; //1.5f;
static const float DEFAULT_CAMERA_FOV = static_cast<float>(M_PI_2);

static const float DEFAULT_LIGHT_X = 0.1f;
static const float DEFAULT_LIGHT_Y = 0.1f;
static const float DEFAULT_LIGHT_Z = 5.0f;

static const float DEFAULT_AMBIENT = 0.01f;
static const float DEFAULT_DIFFUSE = 0.81f;
static const float DEFAULT_SPECULAR = 0.16f;
static const float DEFAULT_SPECULAR_EXPONENT = 6.0f;

static const Eigen::Vector4f DEFAULT_SPHERE_COLOR(0.6f, 0.9f, 0.1f, 0.8f);

#if QT_VERSION < 0x050400
static QGLFormat
P_makeFormat()
{
  QGLFormat f = QGLFormat::defaultFormat();
  f.setVersion(3, 3);
  f.setProfile(QGLFormat::CoreProfile);
  return f;
}
#endif

GLWidget::GLWidget(QWidget *parent)
  :
#if QT_VERSION < 0x050400
  QGLWidget(P_makeFormat(), parent)
#else
  QOpenGLWidget(parent)
#endif
  , m_mode(Mode::MOVE_CAMERA)
  , m_lastMousePos()
  , m_camera()
  , m_camPhy(DEFAULT_CAMERA_PHY)
  , m_camTheta(DEFAULT_CAMERA_THETA)
  , m_camDist(DEFAULT_CAMERA_DIST)
  , m_camLookAt(Eigen::Vector3f::Zero())
  , m_camFov(DEFAULT_CAMERA_FOV)
  , m_object(nullptr)
  , m_meshGL(nullptr)
  , m_program()
  , m_textureId(0)
  , m_samplerId(0)
  , m_minFilter(GL_NEAREST)
  , m_magFilter(GL_NEAREST)
  , m_tex_width(0)
  , m_tex_height(0)
  , m_mesh()
  , m_sphere()
  , m_sphereObject(nullptr)
  , m_sphereMeshGL()
  , m_sphereProgram()
  , m_sphereColor(DEFAULT_SPHERE_COLOR)
  , m_useBackground(false)
  , m_useBackgroundTexture(false)
  , m_backgroundMesh()
  , m_backgroundObject(nullptr)
  , m_backgroundMeshGL(nullptr)
  , m_backgroundProgram()
  , m_backgroundTextureId(0)
  , m_backgroundSamplerId(0)
  , m_backgroundMinFilter(GL_NEAREST)
  , m_backgroundMagFilter(GL_NEAREST)
  , m_selectedVertices()
{
#if QT_VERSION >= 0x050400
  QSurfaceFormat format;
  //format.setDepthBufferSize(24);
  format.setAlphaBufferSize(8);
  //format.setStencilBufferSize(8);
  //#ifndef Q_OS_LINUX
  format.setVersion(3, 3);
  //#endif
  format.setProfile(QSurfaceFormat::CoreProfile);
  //format.setProfile(QSurfaceFormat::CompatibilityProfile);
  setFormat(format);

#endif

  setFocusPolicy(Qt::StrongFocus); //to accept both keyboard & mouse events

  m_lightPos(0) = DEFAULT_LIGHT_X;
  m_lightPos(1) = DEFAULT_LIGHT_Y;
  m_lightPos(2) = DEFAULT_LIGHT_Z;
  m_lightPos(3) = 1;
  m_ambient = Eigen::Vector3f::Constant(DEFAULT_AMBIENT);
  m_diffuse = Eigen::Vector3f::Constant(DEFAULT_DIFFUSE);
  m_specular = Eigen::Vector3f::Constant(DEFAULT_SPECULAR);
  m_specularExponent = DEFAULT_SPECULAR_EXPONENT;

  m_useTexture = false;
  m_texMat = Eigen::Matrix3f::Identity();

  //resize(600, 500);

  //TODO:OPTIM: do that only when with use selection/degradation mode for the first time ?
  m_sphere = makeSphereMesh(0.2f, 8, 8);

  /*
   //DEBUG
   {
     const std::string filename = "/tmp/sphere.obj";
     const bool ok = writeOBJ(filename, m_sphere);
     if (ok)
       std::cerr<<"******* WRITE sphere: "<<filename<<"\n";
   }
   */

  m_backgroundTexMat = Eigen::Matrix3f::Identity();
}

GLWidget::~GLWidget()
{
  delete m_object;
  delete m_meshGL;

  delete m_sphereObject;
  delete m_sphereMeshGL;
}

/*
Warning: this method must be called when a valid context is available,
thus inside or after initializeGL()

*/
void
GLWidget::printGLInfos()
{
  std::cout << "OpenGL version: \"" << glGetString(GL_VERSION) << "\""
            << std::endl;

#if QT_VERSION < 0x050400
  QGLFormat format = context()->format();
  std::string profileStr =
    ((format.profile() == QGLFormat::CoreProfile)
       ? "Core"
       : (format.profile() == QGLFormat::CompatibilityProfile) ? "Compatibility"
                                                               : "No profile");
#else
  QSurfaceFormat format = context()->format();
  std::string profileStr =
    ((format.profile() == QSurfaceFormat::CoreProfile)
       ? "Core"
       : (format.profile() == QSurfaceFormat::CompatibilityProfile)
           ? "Compatibility"
           : "No profile");
#endif

  std::cout << "OpenGL context: " << format.majorVersion() << "."
            << format.minorVersion() << " " << profileStr << "\n";
}

void
GLWidget::initializeGL()
{

#if defined(_WIN32) || defined(_WIN64)
  if (!gladLoadGL()) {
    std::cerr << "ERROR: unable to initialize OpenGL functions via GLAD !!!\n";
  }
#endif //_WIN32 || _WIN64

  printGLInfos();

  glClearColor(0., 0., 0., 1.);

  glEnable(GL_DEPTH_TEST);

  GL_CHECK_ERROR_ALWAYS();

  const std::string vert =
#include "shaders/simple.vert"
    ;
  const std::string frag =
#include "shaders/simple.frag"
    ;

  /*
  const bool ok = m_program.loadFromFiles(SRC_DIR"/shaders/simple.vert",
                                    SRC_DIR"/shaders/simple.frag");
  */
  const bool ok = m_program.loadFromStrings(vert, frag);
  if (!ok) {
    std::cerr << "ERROR: unable to load shaders\n";
    exit(10);
  }

  GL_CHECK_ERROR_ALWAYS();

  //For sphere :

  const std::string vertS =
#include "shaders/sphere.vert"
    ;
  const std::string fragS =
#include "shaders/sphere.frag"
    ;
  const bool okS = m_sphereProgram.loadFromStrings(vertS, fragS);
  if (!okS) {
    std::cerr << "ERROR: unable to load (sphere) shaders\n";
    exit(10);
  }

  assert(m_sphere.isValid());
#if QT_VERSION >= 0x050400
  assert(context() == QOpenGLContext::currentContext());
#endif 

  m_sphereMeshGL = new GLMesh;
  m_sphereMeshGL->init(m_sphere);
  m_sphereObject = new GLObject;
  m_sphereObject->attachShader(&m_sphereProgram);
  m_sphereObject->attachMesh(m_sphereMeshGL);

  GL_CHECK_ERROR_ALWAYS();

  //For background :

  const std::string vertB =
#include "shaders/background.vert"
    ;
  const std::string fragB =
#include "shaders/background.frag"
    ;
  const bool okP = m_backgroundProgram.loadFromStrings(vertB, fragB);
  if (!okP) {
    std::cerr << "ERROR: unable to load (background) shaders\n";
    exit(10);
  }

  m_backgroundMesh = makePlane();

#if QT_VERSION >= 0x050400
  assert(context() == QOpenGLContext::currentContext());
#endif

  m_backgroundMeshGL = new GLMesh;
  m_backgroundMeshGL->init(m_backgroundMesh);

  m_backgroundObject = new GLObject;
  m_backgroundObject->attachShader(&m_backgroundProgram);

  m_backgroundObject->attachMesh(m_backgroundMeshGL);

  GL_CHECK_ERROR_ALWAYS();

  //std::cerr << "GLWidget::initializeGL() done\n";
}

bool
GLWidget::loadMesh(const QString &meshFilename)
{
  bool result = false;

  //std::cerr<<"loadMesh "<<meshFilename.toStdString()<<"\n";

  const std::string filename = meshFilename.toStdString();

  Mesh &m = m_mesh;

  bool checkAndOptimize = false;

  if (isBRSFile(filename)) {
    result = readBRS(filename, m);
    if (!result) {
      std::cerr << "ERROR: unable to read BRS file: " << filename << "\n";
    }
    checkAndOptimize =
      false; //we consider that BRS files are already optimized !
  } else if (isOBJFile(filename)) {
    result = readOBJ(filename, m);
    if (!result) {
      std::cerr << "ERROR: unable to read OBJ file: " << filename << "\n";
    }
    checkAndOptimize = true;
  } else {
    std::cerr << "ERROR: unhandled file format for: " << filename << "\n";
    result = false;
  }

  if (!m_mesh.isValid()) {
    std::cerr << "Mesh is not valid !\n";
    result = false;
  }

  if (result == false)
    return false;

  if (checkAndOptimize) {

    if (!check_EdgesH(m_mesh)) {
      std::cerr << "Unable to handle this (non 2-manifold) mesh \n";
      result = false;
    }

    if (result == false)
      return false;

    m_mesh.removeDegenerateTrianglesIndices();
    m_mesh.removeNonReferencedVertices();
    //m_mesh.removeDuplicatedVertices(); //TODO:OPTIM:QUADRATIC !!!
    keepOnlyLargestCC(m_mesh);

    m.optimizeTriangleOrdering();
  }

  assert(check_EdgesH(m_mesh));

  if (!m.hasNormals()) {
    auto start = std::chrono::steady_clock::now();
    m.computeNormals();
    auto end = std::chrono::steady_clock::now();
    std::cerr << "time compute normals: "
              << std::chrono::duration<double, std::milli>(end - start).count()
              << "ms\n";
  } else {
    auto start = std::chrono::steady_clock::now();
    m.normalizeNormals();
    auto end = std::chrono::steady_clock::now();
    std::cerr << "time normalize normals: "
              << std::chrono::duration<double, std::milli>(end - start).count()
              << "ms\n";
  }

  //TODO: check if there is texcoords & compute them if not !!!

  initGLForMesh();

  updateCameraLookAt();

  emit hasTexCoords(m_mesh.hasTexCoords());

  updateBackgroundTransformation(); 

  update();

  return result;
}

void
GLWidget::initGLForMesh()
{
  assert(m_mesh.isValid());

  m_mesh.unitize();

  GL_CHECK_ERROR_ALWAYS();

  makeCurrent();

#if QT_VERSION >= 0x050400
  assert(context() == QOpenGLContext::currentContext());
#endif

  delete m_meshGL;
  delete m_object;

  m_meshGL = new GLMesh;
  m_meshGL->init(m_mesh);

  GL_CHECK_ERROR_ALWAYS();

  m_object = new GLObject;
  m_object->attachShader(&m_program);
  m_object->attachMesh(m_meshGL);
  //m_object->setTransformation(Matrix4f::Identity());

  GL_CHECK_ERROR_ALWAYS();
}

//B
//cf http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
//@warning  return 0 for v=0.
/*
static
unsigned int
nextPowerOfTwo(unsigned int v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}
*/

void
GLWidget::setTexture(const QImage &image)
{
  if (image.isNull()) {
    emit hasImage(false);
    return; //exit(10);
  }

  makeCurrent();

#if QT_VERSION >= 0x050400
  assert(context() == QOpenGLContext::currentContext());
#endif

  //std::cerr<<"GLWidget::setTexture w="<<image.width()<<" h="<<image.height()<<"\n";

  QImage img = image;

#if 0
  unsigned int w = img.width();
  unsigned wu = nextPowerOfTwo(w);
  unsigned int h = img.height();
  unsigned hu = nextPowerOfTwo(h);
  if (w != wu || h != hu) {
    std::cerr<<"RESIZE image from "<<w<<"x"<<h<<" to "<<wu<<"x"<<hu<<"\n";
    img = img.scaled(wu, hu);
  }
#endif

  assert(!img.isNull());
#if 1
  img = img.mirrored(); // miroir vertical pour être compatible avec OpenGL

  if (img.format() != QImage::Format_RGB32 &&
      img.format() != QImage::Format_ARGB32) {
    std::cerr << "CONVERT image to 32-bit format\n";
    img = img.convertToFormat(QImage::Format_RGB32);
  }
#else
  if (img.format() != QImage::Format_RGB32 &&
      img.format() != QImage::Format_ARGB32) {
    std::cerr << "CONVERT image to 32-bit format\n";
    img = img.convertToFormat(QImage::Format_RGB32);
  }

  img = QGLWidget::convertToGLFormat(img);
#endif

  GL_CHECK_ERROR_ALWAYS();

  glGenTextures(1, &m_textureId);
  glBindTexture(GL_TEXTURE_2D, m_textureId);

  m_program.activate();

  GL_CHECK_ERROR_ALWAYS();

  glGenSamplers(1, &m_samplerId);
  glSamplerParameteri(m_samplerId, GL_TEXTURE_MIN_FILTER, m_minFilter);
  glSamplerParameteri(m_samplerId, GL_TEXTURE_MAG_FILTER, m_magFilter);

  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA,
               img.width(),
               img.height(),
               0,
               GL_BGRA,
               GL_UNSIGNED_BYTE,
               img.bits());
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GL_CHECK_ERROR_ALWAYS();

  glGenerateMipmap(GL_TEXTURE_2D);

  GL_CHECK_ERROR_ALWAYS();

  m_tex_width = img.width();
  m_tex_height = img.height();

  emit hasImage(true);

  update();
}

void
GLWidget::resizeGL(int w, int h)
{
  //std::cerr<<"resizeGL w="<<w<<" h="<<h<<"\n";

#if QT_VERSION >= 0x050400
  assert(context() == QOpenGLContext::currentContext());
#endif

  glViewport(0, 0, w, h);

  assert(h > 0);
  m_camera.setPerspective(m_camFov, w / static_cast<float>(h), .01f, 10.f, 2.f);

  GL_CHECK_ERROR_ALWAYS();
}

void
GLWidget::updateCameraLookAt()
{
  Eigen::Vector3f camPos =
    m_camLookAt +
    m_camDist * Eigen::Vector3f(std::sin(m_camPhy) * std::cos(m_camTheta),
                                std::sin(m_camTheta),
                                std::cos(m_camPhy) * std::cos(m_camTheta));

  m_camera.lookAt(camPos, m_camLookAt, Eigen::Vector3f::UnitY());

  //std::cerr<<"camera.viewMatrix:\n"<<m_camera.getViewMatrix()<<"\n";
}

void
GLWidget::paintGL()
{

#if QT_VERSION >= 0x050400
  assert(context() == QOpenGLContext::currentContext());
#endif

  //std::cerr<<"paintGL()\n";
  GL_CHECK_ERROR_ALWAYS();

  GL_CHECK_ERROR();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  GL_CHECK_ERROR_ALWAYS();

#if 1 //DEBUGALL

  if (m_useBackground) {

    //std::cerr<<"*** paint background \n";

    m_backgroundProgram.activate();

    if (m_backgroundTextureId != 0u
	&& m_useBackgroundTexture) {
      GL_CHECK_ERROR_ALWAYS();

      //std::cerr<<"paintGL m_backgroundTextureId="<<m_backgroundTextureId<<"  m_useBackgroundTexture="<<m_useBackgroundTexture<<"\n";

      glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

      glBindSampler(0, m_backgroundSamplerId);
      glSamplerParameteri(
        m_backgroundSamplerId, GL_TEXTURE_MIN_FILTER, m_backgroundMinFilter);
      glSamplerParameteri(
        m_backgroundSamplerId, GL_TEXTURE_MAG_FILTER, m_backgroundMagFilter);

      glActiveTexture(GL_TEXTURE0 + 0);
      glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

      GLint sampler = m_backgroundProgram.getUniformLocation("colorMap");

      //std::cerr<<"background sampler="<<sampler<<"\n";

      glUniform1i(sampler, 0);

      GL_CHECK_ERROR_ALWAYS();
    }
    //else {
    //glBindTexture(GL_TEXTURE_2D, 0);
    //}

    //std::cerr<<"paint backgroundObject  m_useBackgroundTexture="<<m_useBackgroundTexture<<"\n";

    int useTextureLoc = m_backgroundProgram.getUniformLocation("use_texture");
    if (useTextureLoc >= 0)
      glUniform1i(useTextureLoc, static_cast<GLint>(m_useBackgroundTexture));

    if (m_useTexture) {
      int texMatLoc = m_backgroundProgram.getUniformLocation("tex_matrix");
      if (texMatLoc >= 0)
        glUniformMatrix3fv(texMatLoc, 1, GL_FALSE, m_backgroundTexMat.data());
    }

#if QT_VERSION >= 0x050400
    assert(context() == QOpenGLContext::currentContext());
#endif

    assert(m_backgroundObject);
    m_backgroundObject->draw(m_camera);
  }

  if (m_object != nullptr) {

    m_program.activate();

    if (m_textureId != 0) {

      //B:TODO:DESIGN: not sure m_program is active !!?!
      // It will fail if we have several programs !

      //std::cerr<<"paintGL m_textureId="<<m_textureId<<"\n";

      GL_CHECK_ERROR_ALWAYS();

      glBindTexture(GL_TEXTURE_2D, m_textureId);

      glBindSampler(0, m_samplerId);
      glSamplerParameteri(m_samplerId, GL_TEXTURE_MIN_FILTER, m_minFilter);
      glSamplerParameteri(m_samplerId, GL_TEXTURE_MAG_FILTER, m_magFilter);

      glActiveTexture(GL_TEXTURE0 + 0);
      glBindTexture(GL_TEXTURE_2D, m_textureId);

      GLint sampler = m_program.getUniformLocation("colorMap");

      glUniform1i(sampler, 0);

      //std::cerr<<"texture sampler="<<sampler<<"\n";

      GL_CHECK_ERROR_ALWAYS();
    }

    {
      int lightPosViewLoc = m_program.getUniformLocation("light_position_view");
      if (lightPosViewLoc >= 0) {
        Eigen::Vector4f lightPosView = m_camera.getViewMatrix() * m_lightPos;
        //std::cerr<<"lightPosView: "<<lightPosView(0)<<", "<<lightPosView(1)<<", "<<lightPosView(2)<<"\n";
        glUniform3f(
          lightPosViewLoc, lightPosView(0), lightPosView(1), lightPosView(2));
      }

      int lightAmbientLoc = m_program.getUniformLocation("light_ambient");
      if (lightAmbientLoc >= 0) {
        //std::cerr<<"m_ambient: "<<m_ambient(0)<<", "<<m_ambient(1)<<", "<<m_ambient(2)<<"\n";
        glUniform3fv(lightAmbientLoc, 1, m_ambient.data());
      }
      int lightDiffuseLoc = m_program.getUniformLocation("light_diffuse");
      if (lightDiffuseLoc >= 0) {
        //std::cerr<<"m_diffuse: "<<m_diffuse(0)<<", "<<m_diffuse(1)<<", "<<m_diffuse(2)<<"\n";
        glUniform3fv(lightDiffuseLoc, 1, m_diffuse.data());
      }
      int lightSpecularLoc = m_program.getUniformLocation("light_specular");
      if (lightSpecularLoc >= 0) {
        //std::cerr<<"m_specular: "<<m_specular(0)<<", "<<m_specular(1)<<", "<<m_specular(2)<<"\n";
        glUniform3fv(lightSpecularLoc, 1, m_specular.data());
      }

      int specularExpLoc = m_program.getUniformLocation("specular_exponent");
      if (specularExpLoc >= 0) {
        //std::cerr<<"m_specularExponent: "<<m_specularExponent<<"\n";
        glUniform1f(specularExpLoc, m_specularExponent);
      }

      int useTextureLoc = m_program.getUniformLocation("use_texture");
      if (useTextureLoc >= 0)
        glUniform1i(useTextureLoc, static_cast<GLint>(m_useTexture));

      if (m_useTexture) {
        int texMatLoc = m_program.getUniformLocation("tex_matrix");
        if (texMatLoc >= 0)
          glUniformMatrix3fv(texMatLoc, 1, GL_FALSE, m_texMat.data());
      }
    }

#if QT_VERSION >= 0x050400
    assert(context() == QOpenGLContext::currentContext());
#endif

    m_object->draw(m_camera);
  }

  if (!m_selectedVertices.empty()) {

    //std::cerr<<"m_selectedVertices.size="<<m_selectedVertices.size()<<"\n";

    m_sphereProgram.activate();

    GL_CHECK_ERROR_ALWAYS();
    {
      int lightPosViewLoc =
        m_sphereProgram.getUniformLocation("light_position_view");
      if (lightPosViewLoc >= 0) {
        Eigen::Vector4f lightPosView = m_camera.getViewMatrix() * m_lightPos;
        //std::cerr<<"lightPosView: "<<lightPosView(0)<<", "<<lightPosView(1)<<", "<<lightPosView(2)<<"\n";
        glUniform3f(
          lightPosViewLoc, lightPosView(0), lightPosView(1), lightPosView(2));
      }

      int lightAmbientLoc = m_sphereProgram.getUniformLocation("light_ambient");
      if (lightAmbientLoc >= 0) {
        //std::cerr<<"m_ambient: "<<m_ambient(0)<<", "<<m_ambient(1)<<", "<<m_ambient(2)<<"\n";
        glUniform3fv(lightAmbientLoc, 1, m_ambient.data());
      }
      int lightDiffuseLoc = m_sphereProgram.getUniformLocation("light_diffuse");
      if (lightDiffuseLoc >= 0) {
        //std::cerr<<"m_diffuse: "<<m_diffuse(0)<<", "<<m_diffuse(1)<<", "<<m_diffuse(2)<<"\n";
        glUniform3fv(lightDiffuseLoc, 1, m_diffuse.data());
      }
      int lightSpecularLoc =
        m_sphereProgram.getUniformLocation("light_specular");
      if (lightSpecularLoc >= 0) {
        //std::cerr<<"m_specular: "<<m_specular(0)<<", "<<m_specular(1)<<", "<<m_specular(2)<<"\n";
        glUniform3fv(lightSpecularLoc, 1, m_specular.data());
      }

      GL_CHECK_ERROR_ALWAYS();

      int specularExpLoc =
        m_sphereProgram.getUniformLocation("specular_exponent");
      if (specularExpLoc >= 0) {
        //std::cerr<<"m_specularExponent: "<<m_specularExponent<<"\n";
        glUniform1f(specularExpLoc, m_specularExponent);
      }

      GL_CHECK_ERROR_ALWAYS();

      int sphereColorLoc = m_sphereProgram.getUniformLocation("sphere_color");
      if (sphereColorLoc >= 0) {
        //std::cerr<<"m_specular: "<<m_specular(0)<<", "<<m_specular(1)<<", "<<m_specular(2)<<"\n";
        glUniform4fv(sphereColorLoc, 1, m_sphereColor.data());
      }

      GL_CHECK_ERROR_ALWAYS();
    }

    Eigen::Matrix4f m = Eigen::Matrix4f::Identity(4, 4);

    const float SPHERE_SCALE = 0.04f;
    m(0, 0) = SPHERE_SCALE;
    m(1, 1) = SPHERE_SCALE;
    m(2, 2) = SPHERE_SCALE;

    for (const uint32_t ind : m_selectedVertices) {

      assert(ind < m_mesh.numVertices);

//translate m_sphereObject to vertex position.
#if 1
      m(0, 3) = m_mesh.vertices[3 * ind + 0];
      m(1, 3) = m_mesh.vertices[3 * ind + 1];
      m(2, 3) = m_mesh.vertices[3 * ind + 2];
#else
      m(3, 0) = m_mesh.vertices[3 * ind + 0];
      m(3, 1) = m_mesh.vertices[3 * ind + 1];
      m(3, 2) = m_mesh.vertices[3 * ind + 2];
#endif
      m_sphereObject->setTransformation(m);

      GL_CHECK_ERROR_ALWAYS();

#if QT_VERSION >= 0x050400
      assert(context() == QOpenGLContext::currentContext());
#endif

      m_sphereObject->draw(m_camera);
      GL_CHECK_ERROR_ALWAYS();
    }
  }

#endif //DEBUGALL
}

QImage
GLWidget::takeScreenshot()
{
#if QT_VERSION >= 0x050400
  assert(context() == QOpenGLContext::currentContext());
#endif

  QImage img =
#if QT_VERSION < 0x050400
    grabFrameBuffer();
#else
    QOpenGLWidget::grabFramebuffer();
#endif

  return img;
}

void
GLWidget::setHiResSize(int &prevW, int &prevH)
{
  prevW = size().width();
  prevH = size().height();

  int w = 0, h = 0;

  if (m_tex_width != 0 && m_tex_height != 0) {
    w = m_tex_width;
    h = m_tex_height;
  } else {
    QDesktopWidget desktop;
    w = desktop.geometry().width();
    h = desktop.geometry().height();
  }

  GLint dims[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &dims[0]);
  w = std::min(w, dims[0]);
  h = std::min(h, dims[1]); //TODO: keep aspect ratio !!!

  this->setMaximumSize(w, h);
  this->setMinimumSize(w, h);
  this->resize(w, h);

  qApp->processEvents(
    QEventLoop::ExcludeUserInputEvents); //qApp->processEvents();
}

void
GLWidget::resetLowResSize(int prevW, int prevH)
{
  this->setMinimumSize(prevW, prevH);
  this->resize(prevW, prevH);
}

QImage
GLWidget::takeScreenshotHiRes()
{
  int cw, ch;
  setHiResSize(cw, ch);

  QImage img = takeScreenshot();

  resetLowResSize(cw, ch);

  return img;
}

static float
P_bounded_rand(float min, float max)
{
  return (min + (static_cast<float>(rand()) / RAND_MAX * (max - min + 1)));
}

static QString
makeFilename(const QString &prefix, size_t i, size_t w, const QString &ext)
{
  const QString number = QString("%1").arg(i, w, 10, QChar('0'));

  const QString filename = prefix + number + ext;

  return filename;
}

static size_t
P_getWidth(size_t num)
{
  size_t w = 0;
  size_t n = num;
  while (n > 0) {
    ++w;
    n /= 10;
  }
  return w;
}

bool
GLWidget::takeRandomScreenshots(const QString &prefix,
                                const QString &ext,
                                size_t numScreenshots,
                                float minPhy,
                                float maxPhy,
                                float minTheta,
                                float maxTheta)
{

  QGuiApplication::setOverrideCursor(Qt::BusyCursor);

  //TODO:BUG: I have a core dump on this on linux !!!!!!!

#ifdef __APPLE__
  int cw, ch;
  setHiResSize(cw, ch);
#endif //__APPLE__

  float prev_phy = m_camPhy;
  float prev_theta = m_camTheta;

  size_t w = P_getWidth(numScreenshots);

  for (size_t i = 0; i < numScreenshots; ++i) {

    const float phy = P_bounded_rand(minPhy, maxPhy);
    const float theta = P_bounded_rand(minTheta, maxTheta);

    m_camPhy = phy;
    m_camTheta = theta;

    updateCameraLookAt();
    update();

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    QImage img = takeScreenshot();

    QString filename = makeFilename(prefix, i, w, ext);

    bool saveOk = img.save(filename);

    if (!saveOk)
      return false;
  }

#ifdef __APPLE__
  resetLowResSize(cw, ch);
#endif //__APPLE__

  m_camPhy = prev_phy;
  m_camTheta = prev_theta;

  updateCameraLookAt();
  update();

  QGuiApplication::restoreOverrideCursor();

  return true;
}

void
GLWidget::keyPressEvent(QKeyEvent *event)
{
  //unsigned char key = event->key();
  //keyboard(key,0,1);

  switch (event->key()) {
    case Qt::Key_S:
      if (m_mode != Mode::SELECTION_VERTEX) {
        m_mode = Mode::SELECTION_VERTEX;
        QGuiApplication::setOverrideCursor(Qt::CrossCursor);
        std::cerr << "Mode::SELECTION_VERTEX\n";
        m_selectedVertices.clear();
      }
      else {
        m_mode = Mode::MOVE_CAMERA;
        QGuiApplication::restoreOverrideCursor();
        std::cerr << "Mode::MOVE_CAMERA\n";
      }
      break;

    default:
#if QT_VERSION < 0x050400
      QGLWidget::keyPressEvent(event);
#else
      QOpenGLWidget::keyPressEvent(event);
#endif
  }
}

//B: à faire dans GLCamera ???
void
GLWidget::getRayInWorld(QPoint pos,
                        Eigen::Vector3f &rayOrigin,
                        Eigen::Vector3f &rayDirection)
{
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport); //return x, y, w, h

  const int width = viewport[2];
  const int height = viewport[3];

  //2D viewport coords in [0;Width]x[Height;0]
  float x = pos.x();
  float y = height - pos.y();

  assert(x >= 0 && x < width);
  assert(y >= 0 && y < height);
  assert(width > 0 && height > 0);
  //3D Normalized Device Coordinates [-1;1]x[-1;1]x[-1;1]
  x = 2.f * x / static_cast<float>(width) - 1.0f;
  y = 2.f * y / static_cast<float>(height) - 1.0f;
  //z = 1.0f;
  //Eigen::Vector3f ray_nds(x, y, z);

#if 0
  //http://antongerdelan.net/opengl/raycasting.html

  //4D Homogeneous Clip Coordinates [-1;1]x[-1;1]x[-1;1]x[-1;1]
  // we want z to point forward, that is negative z direction in OpenGL
  Eigen::Vector4f ray_clip(x, y, -1.f, 1.f);

  //4D Eye/Camera Coordinates  [-x;x]x[-y;y]x[-z;z]x[-w;w]
  const Eigen::Matrix4f projection_mat = m_camera.getProjectionMatrix();
  Eigen::Vector4f ray_eye = projection_mat.inverse() * ray_clip;
  ray_eye = Eigen::Vector4f(ray_eye(0), ray_eye(1), -1.0, 0.0); //force forward
  
  //4D World Coordinates   [-x;x]x[-y;y]x[-z;z]x[-w;w]
  const Eigen::Matrix4f view_mat = m_camera.getViewMatrix();
  Eigen::Vector4f ray_world = view_mat.inverse() * ray_eye;
  rayDirection(0) = ray_world(0);
  rayDirection(1) = ray_world(1);
  rayDirection(2) = ray_world(2);
  //rayDirection.normalize();
  //rayOrigin = m_camera.getPosition();
  
  rayOrigin = rayDirection;
  rayDirection = (rayOrigin-m_camera.getPosition()).normalized();

  //std::cerr<<"rayOrigin="<<rayOrigin.transpose()<<" rayDirection="<<rayDirection.transpose()<<"\n";

#endif

  //http://stackoverflow.com/questions/18660248/ray-casting-from-mouse-with-opengl-2

  Eigen::Vector4f ndc1(x, y, -1.f, 1.f);

  //(m_camera.getProjectionMatrix()*m_camera.getViewMatrix()).inverse() is clip2world matrix.

  Eigen::Vector4f w1 =
    (m_camera.getProjectionMatrix() * m_camera.getViewMatrix()).inverse() *
    ndc1;
  Eigen::Vector3f w1b(w1(0) / w1(3), w1(1) / w1(3), w1(2) / w1(3));

  rayOrigin = w1b;
  rayDirection = (w1b - m_camera.getPosition()).normalized();

  //"all rays pass through the camera's origin."
}

//return squared distance
static inline float
distancePointRay(Eigen::Vector3f point,
                 Eigen::Vector3f rayOrigin,
                 Eigen::Vector3f rayDirection)
{
  assert((rayDirection - rayDirection.normalized()).squaredNorm() < 0.000001);

  return rayDirection.cross(rayOrigin - point)
    .squaredNorm(); // / rayDirection.norm()
}

//Warning : always return an index even if no intersection !
static uint32_t
getVertexWithRay(const Mesh &mesh,
                 Eigen::Vector3f rayOrigin,
                 Eigen::Vector3f rayDirection)
{

  uint32_t minInd = mesh.numVertices;
  float minDist = std::numeric_limits<float>::max();
  const uint32_t numVertices = mesh.numVertices;
  const float *vertices = mesh.vertices;
  for (uint32_t i = 0; i < numVertices; ++i) {
    Eigen::Vector3f v(
      vertices[3 * i + 0], vertices[3 * i + 1], vertices[3 * i + 2]);
    const float d2 = distancePointRay(v, rayOrigin, rayDirection);
    if (d2 < minDist) {
      minDist = d2;
      minInd = i;
    }
  }
  return minInd;
}

void
GLWidget::mousePressEvent(QMouseEvent *e)
{
  if (m_mode == Mode::MOVE_CAMERA) {
    m_lastMousePos = e->pos();
  } else if (m_mode == Mode::SELECTION_VERTEX) {
    Eigen::Vector3f rayOrigin, rayDirection;

    getRayInWorld(e->pos(), rayOrigin, rayDirection);

    //std::cerr<<"click at ("<<e->pos().x()<<", "<<e->pos().y()<<") : rayDir="<<rayDirection.transpose()<<"\n";

    const uint32_t ind = getVertexWithRay(m_mesh, rayOrigin, rayDirection);

    //std::cerr<<"  ind="<<ind<<" v=("<<m_mesh.vertices[3*ind+0]<<", "<<m_mesh.vertices[3*ind+1]<<", "<<m_mesh.vertices[3*ind+2]<<")\n";

    if (ind < m_mesh.numVertices) {
      m_selectedVertices.push_back(ind);
      update();
    }
  }

  e->accept();
}

void
GLWidget::mouseReleaseEvent(QMouseEvent *e)
{
  if (m_mode == Mode::MOVE_CAMERA) {
    m_lastMousePos = e->pos();
  } else if (m_mode == Mode::SELECTION_VERTEX) {
  }

  e->accept();
}

void
GLWidget::mouseMoveEvent(QMouseEvent *e)
{
  if (m_mode == Mode::MOVE_CAMERA) {
    if (e->buttons() & Qt::LeftButton) {
      m_camPhy += -float(e->x() - m_lastMousePos.x()) / 256.f;
      m_camTheta += float(e->y() - m_lastMousePos.y()) / 256.f;
      m_camTheta =
        std::min(M_PI_2 - .001, std::max(-M_PI_2 + 0.001, double(m_camTheta)));
      updateCameraLookAt();
      m_lastMousePos = e->pos();
      e->accept();
      update();
    } else if (e->buttons() & Qt::MiddleButton) {
      float offset = m_camDist * std::tan(m_camFov / width());
      Eigen::Vector3f z = m_camera.getPosition() - m_camLookAt;
      Eigen::Vector3f x =
        offset * (Eigen::Vector3f::UnitY().cross(z)).normalized();
      Eigen::Vector3f y = offset * (z.cross(x)).normalized();
      m_camLookAt +=
        -x * (e->x() - m_lastMousePos.x()) + y * (e->y() - m_lastMousePos.y());
      updateCameraLookAt();
      m_lastMousePos = e->pos();
      e->accept();
      update();
    }
  }
}

void
GLWidget::wheelEvent(QWheelEvent *e)
{
  m_camDist *= (e->delta() > 0) ? 1. / 1.02 : 1.02;
  updateCameraLookAt();
  e->accept();
  update();
}

static std::string
getExtension(const std::string &filename)
{
  std::string::size_type pos = filename.rfind('.');
  if (pos != std::string::npos)
    return std::string(filename, pos + 1);
  return std::string();
}

static std::string
toupper(const std::string &s)
{
  //std::transform(s.begin(), s.end(), s.begin(), (int(*)(int))std::toupper);

  std::string so = s;
  for (auto &c : so)
    c = toupper(c);
  return so;
}

static bool
writeMesh(const Mesh &mesh, const std::string &outputMeshFilename)
{
  bool result = false;

  const std::string ext = toupper(getExtension(outputMeshFilename));

  if (ext == "OBJ") {
    result = writeOBJ(outputMeshFilename, mesh);
  } else if (ext == "BRS") {
    result = writeBRS(outputMeshFilename, mesh);
  } else {
    std::cerr << "ERROR: unhandled file format for output file: "
              << outputMeshFilename << "\n";
    result = false;
  }
  return result;
}

void
GLWidget::updateMeshGL()
{
  //for now, we delete & rebuild
  //TODO:OPTIM: do not re-allocate VBO if same size...

#if QT_VERSION >= 0x050400
  assert(context() == QOpenGLContext::currentContext());
#endif

  delete m_meshGL;
  m_meshGL = new GLMesh;
  GL_CHECK_ERROR_ALWAYS();
  m_meshGL->init(m_mesh);
  GL_CHECK_ERROR_ALWAYS();
  assert(m_object);
  m_object->attachMesh(m_meshGL);
}

void
GLWidget::computeTextureCoords()
{
  //computeTexCoords0(m_mesh);
  //writeOBJ(outputMeshFilename.toStdString(), m_mesh);

  m_mesh.freeTexCoords();

  m_mesh.removeDegenerateTrianglesIndices();
  m_mesh.removeNonReferencedVertices();
  //m_mesh.removeDuplicatedVertices(); //TODO:OPTIM:QUADRATIC !!!

  auto start = std::chrono::steady_clock::now();

  computeTexCoords2(m_mesh);

  auto end = std::chrono::steady_clock::now();
  std::cerr << "time compute texcoords: "
            << std::chrono::duration<double, std::milli>(end - start).count()
            << "ms\n";

  assert(m_mesh.hasTexCoords());

  updateMeshGL();

  emit hasTexCoords(m_mesh.hasTexCoords());

  update();
}

bool
GLWidget::saveMesh(const QString &outputMeshFilename)
{
  const bool writeOk = writeMesh(m_mesh, outputMeshFilename.toStdString());

  return writeOk;
}

void
GLWidget::setLightPos(float x, float y, float z)
{
  m_lightPos(0) = x;
  m_lightPos(1) = y;
  m_lightPos(2) = z;
  m_lightPos(3) = 1;

  update();
}

void
GLWidget::setAmbientDiffuseSpecular(Eigen::Vector3f a,
                                    Eigen::Vector3f d,
                                    Eigen::Vector3f s)
{
  m_ambient = a;
  m_diffuse = d;
  m_specular = s;

  update();
}

void
GLWidget::setSpecularExponent(float se)
{
  m_specularExponent = se;

  update();
}

void
GLWidget::setUseTexture(bool onoff)
{
  m_useTexture = onoff;

  update();
}

void
GLWidget::setTextureMatrix(const Eigen::Matrix3f &texMat)
{
  m_texMat = texMat;

  if (m_useTexture)
    update();
}

void
GLWidget::applyTextureMatrix()
{
  if (m_mesh.hasTexCoords()) {

    for (uint32_t i = 0; i < m_mesh.numVertices; ++i) {
      Eigen::Vector3f v(
        m_mesh.texCoords[2 * i + 0], m_mesh.texCoords[2 * i + 1], 1);
      v = m_texMat * v;
      m_mesh.texCoords[2 * i + 0] = v[0];
      m_mesh.texCoords[2 * i + 1] = v[1];
      //TODO: divide by v[2] ????
    }

    updateMeshGL();
  }
  m_texMat = Eigen::Matrix3f::Identity();

  update();
}

void
GLWidget::applyViewMatrix()
{
  const Eigen::Matrix4f &viewMat = m_camera.getViewMatrix();
  for (uint32_t i = 0; i < m_mesh.numVertices; ++i) {
    Eigen::Vector4f v(m_mesh.vertices[3 * i + 0],
                      m_mesh.vertices[3 * i + 1],
                      m_mesh.vertices[3 * i + 2],
                      1);
    v = viewMat * v;
    m_mesh.vertices[3 * i + 0] = v[0];
    m_mesh.vertices[3 * i + 1] = v[1];
    m_mesh.vertices[3 * i + 2] = v[2];
    //TODO: divide by v[3] ???
  }

  updateMeshGL();

  resetViewMatrix();
}

void
GLWidget::resetViewMatrix()
{
  m_camPhy = DEFAULT_CAMERA_PHY;
  m_camTheta = DEFAULT_CAMERA_THETA;
  m_camDist = DEFAULT_CAMERA_DIST;
  m_camLookAt = Eigen::Vector3f::Zero();
  m_camFov = DEFAULT_CAMERA_FOV;
  updateCameraLookAt();

  update();
}

void
GLWidget::setUseBackgroundTexture(bool onoff)
{
  m_useBackgroundTexture = onoff;
  m_useBackground =
    onoff; //For now, setUSeTexture control both backgroundtexture & background presence

  updateBackgroundTransformation();

  update();
}

void
GLWidget::updateBackgroundTransformation()
{
  if (m_mesh.isValid() && m_useBackground) {

    float minB[3], maxB[3];
    m_mesh.getAABB(minB, maxB);

    float s = 3;
    float sx = s * (maxB[0] - minB[0]);
    float sy = s * (maxB[1] - minB[1]);

    const float zoffset = -0.001f;

    Eigen::Matrix4f t = Eigen::Matrix4f::Identity();
    //scale on X & Y
    t(0, 0) = sx;
    t(1, 1) = sy;
    //translation on Z
    t(2, 3) = minB[2] + zoffset; //Add an offset to avoid z-fighting.
    
    m_backgroundObject->setTransformation(t);
  }

}

void
GLWidget::setBackgroundTexture(const QImage &image)
{
  if (image.isNull()) {
    return;
  }

#if QT_VERSION >= 0x050400
  assert(context() == QOpenGLContext::currentContext());
#endif

  QImage img = image;

  img = img.mirrored(); // miroir vertical pour être compatible avec OpenGL

  if (img.format() != QImage::Format_RGB32 &&
      img.format() != QImage::Format_ARGB32) {
    std::cerr << "CONVERT image to 32-bit format\n";
    img = img.convertToFormat(QImage::Format_RGB32);
  }

  GL_CHECK_ERROR_ALWAYS();

  glGenTextures(1, &m_backgroundTextureId);
  glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

  //std::cerr<<"m_backgroundTextureId="<<m_backgroundTextureId<<"\n";

  m_backgroundProgram.activate();

  GL_CHECK_ERROR_ALWAYS();

  glGenSamplers(1, &m_backgroundSamplerId);
  glSamplerParameteri(
    m_backgroundSamplerId, GL_TEXTURE_MIN_FILTER, m_backgroundMinFilter);
  glSamplerParameteri(
    m_backgroundSamplerId, GL_TEXTURE_MAG_FILTER, m_backgroundMagFilter);

  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA,
               img.width(),
               img.height(),
               0,
               GL_BGRA,
               GL_UNSIGNED_BYTE,
               img.bits());
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GL_CHECK_ERROR_ALWAYS();

  glGenerateMipmap(GL_TEXTURE_2D);

  GL_CHECK_ERROR_ALWAYS();

  update();
}

void
GLWidget::setBackgroundTextureMatrix(const Eigen::Matrix3f &texMat)
{
  m_backgroundTexMat = texMat;

  if (m_useBackgroundTexture)
    update();
}
