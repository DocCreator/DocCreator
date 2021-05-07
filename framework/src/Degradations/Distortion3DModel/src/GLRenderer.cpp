
#include "GLRenderer.hpp"

#include <iostream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Mesh.hpp"
#include "brs.hpp"
#include "obj.hpp"

#include "GLMesh.hpp"


unsigned int GLRenderer::GLRenderer::ms_initialized = false;


#define BUFFER_OFFSET(i) ((char*)NULL + (i))


static const float DEFAULT_CAMERA_PHY = 0; //0.5f;
static const float DEFAULT_CAMERA_THETA = 0; //0.2f;
static const float DEFAULT_CAMERA_DIST = 0.87; //1.5f;
static const float DEFAULT_CAMERA_FOV = M_PI_2;

static const float DEFAULT_LIGHT_X = 0.1;
static const float DEFAULT_LIGHT_Y = 0.1;
static const float DEFAULT_LIGHT_Z = 5.0;

static const float DEFAULT_AMBIENT = 0.01;
static const float DEFAULT_DIFFUSE = 0.81;
static const float DEFAULT_SPECULAR = 0.16;
static const float DEFAULT_SPECULAR_EXPONENT = 6.0;


static const float DEFAULT_RANDOM_MIN_PHY = -0.2f;
static const float DEFAULT_RANDOM_MAX_PHY = 0.2f;
static const float DEFAULT_RANDOM_MIN_THETA = -0.2f;
static const float DEFAULT_RANDOM_MAX_THETA = 0.2f;

static const float DEFAULT_RANDOM_MIN_LIGHT_X = -4.0;
static const float DEFAULT_RANDOM_MAX_LIGHT_X = 4.0;
static const float DEFAULT_RANDOM_MIN_LIGHT_Y = -4.0;
static const float DEFAULT_RANDOM_MAX_LIGHT_Y = 4.0;
static const float DEFAULT_RANDOM_MIN_LIGHT_Z = 3.5;
static const float DEFAULT_RANDOM_MAX_LIGHT_Z = 5;

static const float DEFAULT_RANDOM_MIN_AMBIENT = 0.0f;
static const float DEFAULT_RANDOM_MAX_AMBIENT = 0.25f;
static const float DEFAULT_RANDOM_MIN_DIFFUSE = 0.71f;
static const float DEFAULT_RANDOM_MAX_DIFFUSE = 0.91f;
static const float DEFAULT_RANDOM_MIN_SPECULAR = 0.1f;
static const float DEFAULT_RANDOM_MAX_SPECULAR = 0.2f;



static
void error_callback(int /*error*/, const char* description)
{
    fputs(description, stdout);
}

static
void framebufferSize_callback(GLFWwindow *w, int width, int height)
{
  GLRenderer *wd = (GLRenderer *)glfwGetWindowUserPointer(w);
  wd->resizeGL(width, height);
}

static
cv::Mat
makeOpenGLImage(const cv::Mat &m)
{
  cv::Mat img;
  if (m.channels() == 1) {
    cv::cvtColor(m, img, cv::COLOR_GRAY2BGRA);
  }
  else if (m.channels() == 3) {
    cv::cvtColor(m, img, cv::COLOR_BGR2BGRA);
  }
  else if (m.channels() == 4) {
    img = m.clone();
  }
  else {
    //TODO
    std::cerr<<"***************************** image format is not handled\n";
    assert(false);
  }
  cv::flip(img, img, 0);
  return img;
}


GLRenderer::GLRenderer(int width, int height) : 
  m_camera(),
  m_camPhy(DEFAULT_CAMERA_PHY),
  m_camTheta(DEFAULT_CAMERA_THETA),
  m_camDist(DEFAULT_CAMERA_DIST),
  m_camLookAt(Eigen::Vector3f::Zero()),
  m_camFov(DEFAULT_CAMERA_FOV),
  m_object(nullptr),
  m_meshGL(nullptr),
  m_program(),
  m_textureId(0),
  m_samplerId(0),
  m_minFilter(GL_NEAREST), m_magFilter(GL_NEAREST),
  m_tex_width(0), m_tex_height(0),
  m_mesh(),
  m_useBackground(false),
  m_useBackgroundTexture(false),
  m_backgroundMesh(),
  m_backgroundObject(nullptr),
  m_backgroundMeshGL(nullptr),
  m_backgroundProgram(),
  m_backgroundTextureId(0),
  m_backgroundSamplerId(0),
  m_backgroundMinFilter(GL_NEAREST),
  m_backgroundMagFilter(GL_NEAREST),
  m_channels(3)
{
  if (ms_initialized == 0) {
    glfwSetErrorCallback(error_callback);

    if (! glfwInit()) {
      std::cerr<<"ERROR: glfwInit() failed\n";
      exit(EXIT_FAILURE);
    }
  }
  ms_initialized += 1;

  int window_width = width;
  int window_height = height;
  const int MAX_SIZE = 8192;
  if (window_width > MAX_SIZE || window_height > MAX_SIZE) {
    if (window_height > window_width) {
      window_width = (window_width*MAX_SIZE)/window_height;
      window_height = MAX_SIZE;
    }
    else {
      window_height = (window_height*MAX_SIZE)/window_width;
      window_width = MAX_SIZE;
    }
  }
  

  /*
    http://antongerdelan.net/opengl/glcontext2.html

    On Mac "only a limited set of OpenGL implementations are available; 4.1 and 3.3 on Mavericks, and 3.2 on pre-Mavericks. These are also limited to a "forward-compatible, core profile" context - the most conservative set of features with no backwards-compatibility support for features that have been made obsolete. "

    "The "forward compatible" profile disables all of the functionality from previous versions of OpenGL that has been flagged for removal in the future. If you get the OpenGL 3.2 or 4.x Quick Reference Card, then this means that all of the functions in blue font are not available. This future-proofs our code, and there's no other option on Mac. The "compatibility" profile doesn't flag any functions as deprecated, so for this to work we also must enable the "core" profile, which does mark deprecation. "

  */

  //for OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  //for forward-compatible core profile
  ////glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  /*
    With GLFW 3.3.2 with OSMesa
    if core profile is not requested
    it seems that the most recent supported OpenGL version is 3.1.
    If a more recent version is requested (without core profile), context creation will fail.

    Then, GLSL supported versions are only:
    1.10, 1.20, 1.30, 1.40, 1.00 ES, and 3.00 ES

    With core profile, context creation is ok for OpenGL 3.3 and GLSL 3.30 is supported.
  */

  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); //To have an invisible window
  
  //for anti-aliasing
  glfwWindowHint(GLFW_SAMPLES, 4);

  m_window = glfwCreateWindow(window_width, window_height, "GLRenderer", NULL, NULL);
  if (!m_window) {
    std::cerr<<"ERROR: unable to create window\n";
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetWindowUserPointer(m_window, this); //for framebufferSize_callback
  
  glfwMakeContextCurrent(m_window);
  gladLoadGL(glfwGetProcAddress); //B:without this, unable to call GL functions (ex: glGetString(GL_VERSION) fails)

  glfwSetFramebufferSizeCallback(m_window, framebufferSize_callback); 

  assert (glGetError () == GL_NO_ERROR);

  
  //B: framebufferSize_callback not called once ?
  //B: here ? 
  int fbWidth, fbHeight;
  glfwGetFramebufferSize(m_window, &fbWidth, &fbHeight);
  std::cerr<<"get framebuffer of size: w="<<fbWidth<<" h="<<fbHeight<<"\n";
  resizeGL(fbWidth, fbHeight);
  

  assert (glGetError () == GL_NO_ERROR);

  
  //B:TODO: initialization list 
  
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

  m_backgroundTexMat = Eigen::Matrix3f::Identity();

  assert (glGetError () == GL_NO_ERROR);

  initializeGL();

  assert (glGetError () == GL_NO_ERROR);



   
}

GLRenderer::~GLRenderer()
{
  delete m_object;
  delete m_meshGL;
  
  glfwDestroyWindow(m_window);

  assert(ms_initialized > 0);
  ms_initialized-=1;
  if (ms_initialized == 0) {
    glfwTerminate();
  }
}



void GLRenderer::printGLInfos()
{
  std::cout<<" GLFW version: \""<<glfwGetVersionString()<<"\"\n";
  std::cout<<" OpenGL version: \"" << glGetString(GL_VERSION)<< "\""<<std::endl;
  /*
  std::cout << "OpenGL context: " << context()->format().majorVersion()
            << "." << context()->format().minorVersion()
            << " " << ((context()->format().profile() == QGLFormat::CoreProfile)? "Core":
                       (context()->format().profile() == QGLFormat::CompatibilityProfile)? "Compatibility":
                       "No profile")
            << "\n";
  */

  int maxTexSize = 0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
  std::cout<<" GL_MAX_TEXTURE_SIZE="<<maxTexSize<<"\n";

  int dims[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);
  std::cout<<" GL_MAX_VIEWPORT_DIMS="<<dims[0]<<"x"<<dims[1]<<"\n";
  
}

//#define USE_SHADER_FILES

/*
#ifndef USE_SHADER_FILES
#include "shader_simple.hpp"
#include "shader_background.hpp"
#endif
*/

void GLRenderer::initializeGL()
{
  printGLInfos();

  glClearColor(0., 0., 0., 1.);

  glEnable(GL_DEPTH_TEST);

  GL_CHECK_ERROR_ALWAYS();
  
#ifdef USE_SHADER_FILES
  const bool ok = m_program.loadFromFiles(SRC_DIR"/shaders/simple.vert",
					  SRC_DIR"/shaders/simple.frag");
#else
#include "shader_simple.hpp"
  const bool ok = m_program.loadFromStrings(vs_simple, fs_simple);
#endif
  if (! ok) {
    std::cout<<"ERROR: unable to load shaders\n";
    exit(10);
  }  

  GL_CHECK_ERROR_ALWAYS();


  //B:TODO:OPTIM: no need for all that if we don't load a background image ????

  //For background :
#ifdef USE_SHADER_FILES
  const bool okP = m_backgroundProgram.loadFromFiles(SRC_DIR"/shaders/background.vert",
					       SRC_DIR"/shaders/background.frag");
#else
#include "shader_background.hpp"
  const bool okP = m_backgroundProgram.loadFromStrings(vs_background, fs_background);
#endif
  if (! okP) {
    std::cout<<"ERROR: unable to load (background) shaders\n";
    exit(10);
  }  

  m_backgroundMesh = makePlane();

  m_backgroundMeshGL = new GLMesh;
  m_backgroundMeshGL->init(m_backgroundMesh);

  m_backgroundObject = new GLObject;
  m_backgroundObject->attachShader(&m_backgroundProgram);

  m_backgroundObject->attachMesh(m_backgroundMeshGL);


  GL_CHECK_ERROR_ALWAYS();
}

bool
GLRenderer::loadMesh(const std::string &meshFilename)
{
  bool result = false;

  //std::cout<<"loadMesh "<<meshFilename<<"\n";

  const std::string filename = meshFilename;

  Mesh &m = m_mesh;

  bool checkAndOptimize = false;

  if (isBRSFile(filename)) {
    result = readBRS(filename, m);
    if (!result) {
      std::cerr << "ERROR: unable to read BRS file: " << filename << "\n";
    }
    checkAndOptimize = false;
    //we consider that BRS files are already optimized !
  }
  else if (isOBJFile(filename)) {
    result = readOBJ(filename, m);
    if (! result) {
      std::cout<<"ERROR: unable to read OBJ file: "<<filename<<"\n";
    }
    checkAndOptimize = true;
  }
  else {
    std::cout<<"ERROR: unhandled file format for: "<<filename<<"\n";
    result = false;
  }

  if (! m_mesh.isValid()) {
    std::cout<<"Mesh is not valid !\n";
    result = false;
  }

  if (result == false)
    return false;

  if (checkAndOptimize) {

    if (! check_EdgesH(m_mesh)) {
      std::cout<<"Unable to handle this (non 2-manifold) mesh \n";
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


  if (! m.hasNormals()) {
    m.computeNormals();
  }
  else {
    m.normalizeNormals();
  }
  
  //TODO: check if there is texcoords & compute them if not !!!
  
  initGLForMesh();

  updateCameraLookAt();


  //std::cout<<"Mesh has texcoords? "<<m_mesh.hasTexCoords()<<"\n";

  //update();

  return result;
}

void
GLRenderer::initGLForMesh()
{
  assert(m_mesh.isValid());

  m_mesh.unitize();



  GL_CHECK_ERROR_ALWAYS();

  m_meshGL = new GLMesh;

  GL_CHECK_ERROR_ALWAYS();

  m_meshGL->init(m_mesh);

  GL_CHECK_ERROR_ALWAYS();

  m_object = new GLObject;
  m_object->attachShader(&m_program);
  m_object->attachMesh(m_meshGL);
  //m_object->setTransformation(Matrix4f::Identity());

  GL_CHECK_ERROR_ALWAYS();
  
}

bool
GLRenderer::loadTexture(const std::string &imageFilename)
{
  bool result = false;
  
  cv::Mat image = cv::imread(imageFilename);
  result = (! image.empty());

  std::cout<<"GLRenderer::loadTexture result="<<result<<" image.empty()="<<image.empty()<<"\n";
  
  if (! image.empty()) {
    setTexture(image);
  }

  return result;
}

void
GLRenderer::setTexture(const cv::Mat &image)
{
  if (image.empty()) {
    std::cout<<"GLRenderer::setTexture() image.empty() !\n";
    return;
  }

  cv::Mat img = makeOpenGLImage(image);

  GL_CHECK_ERROR_ALWAYS();

  glGenTextures(1, &m_textureId);
  glBindTexture(GL_TEXTURE_2D, m_textureId);

  m_program.activate();

  GL_CHECK_ERROR_ALWAYS();
  
  glGenSamplers(1, &m_samplerId);
  glSamplerParameteri(m_samplerId, GL_TEXTURE_MIN_FILTER, m_minFilter);
  glSamplerParameteri(m_samplerId, GL_TEXTURE_MAG_FILTER, m_magFilter);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.cols, img.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, img.data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GL_CHECK_ERROR_ALWAYS();

  //glGenerateMipmap(GL_TEXTURE_2D); 

  GL_CHECK_ERROR_ALWAYS();

  m_tex_width = img.cols;
  m_tex_height = img.rows;

  m_channels = image.channels();
  setUseTexture(true);
  //update();
}



void
GLRenderer::resizeGL(int w, int h)
{  
  std::cout<<"resizeGL w="<<w<<" h="<<h<<"\n";

  glViewport(0, 0, w, h);

  assert(h > 0);
  m_camera.setPerspective(m_camFov, w/(float)h, .01, 10., 2.);

  GL_CHECK_ERROR_ALWAYS();
}

void
GLRenderer::updateCameraLookAt()
{
  Eigen::Vector3f camPos = m_camLookAt + m_camDist * Eigen::Vector3f(
								     std::sin(m_camPhy)*std::cos(m_camTheta),
								     std::sin(m_camTheta),
								     std::cos(m_camPhy)*std::cos(m_camTheta));
  
  m_camera.lookAt(camPos, m_camLookAt, Eigen::Vector3f::UnitY());

  
  //std::cout<<"camera.viewMatrix:\n"<<m_camera.getViewMatrix()<<"\n";
}

static float
P_bounded_rand(float min, float max)
{
  return (min + (static_cast<float>(rand()) / RAND_MAX * (max - min)));
}


void GLRenderer::paintGL(bool random)
{

  glfwMakeContextCurrent(m_window);
  std::cout<<"--------- paintGL("<<random<<")\n";

  if (random) {
    
    m_lightPos(0) = P_bounded_rand(DEFAULT_RANDOM_MIN_LIGHT_X, DEFAULT_RANDOM_MAX_LIGHT_X);
    m_lightPos(1) = P_bounded_rand(DEFAULT_RANDOM_MIN_LIGHT_Y, DEFAULT_RANDOM_MAX_LIGHT_Y);
    m_lightPos(2) = P_bounded_rand(DEFAULT_RANDOM_MIN_LIGHT_Z, DEFAULT_RANDOM_MAX_LIGHT_Z);
    m_ambient = Eigen::Vector3f::Constant(P_bounded_rand(DEFAULT_RANDOM_MIN_AMBIENT, DEFAULT_RANDOM_MAX_AMBIENT));
    m_diffuse = Eigen::Vector3f::Constant(P_bounded_rand(DEFAULT_RANDOM_MIN_DIFFUSE, DEFAULT_RANDOM_MAX_DIFFUSE));
    m_specular = Eigen::Vector3f::Constant(P_bounded_rand(DEFAULT_RANDOM_MIN_SPECULAR, DEFAULT_RANDOM_MAX_SPECULAR));
    //m_specularExponent = DEFAULT_SPECULAR_EXPONENT;

    m_camPhy = P_bounded_rand(DEFAULT_RANDOM_MIN_PHY, DEFAULT_RANDOM_MAX_PHY);
    m_camTheta = P_bounded_rand(DEFAULT_RANDOM_MIN_THETA, DEFAULT_RANDOM_MAX_THETA);
    updateCameraLookAt();
  }

  
  GL_CHECK_ERROR_ALWAYS();

  GL_CHECK_ERROR();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  GL_CHECK_ERROR_ALWAYS();


  if (m_useBackground) {
    
    m_backgroundProgram.activate();
    
    if (m_backgroundTextureId && m_useBackgroundTexture) {
      GL_CHECK_ERROR_ALWAYS();

      glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

      glBindSampler(0, m_backgroundSamplerId);
      glSamplerParameteri(m_backgroundSamplerId, GL_TEXTURE_MIN_FILTER, m_backgroundMinFilter);
      glSamplerParameteri(m_backgroundSamplerId, GL_TEXTURE_MAG_FILTER, m_backgroundMagFilter);


      glActiveTexture(GL_TEXTURE0 + 0);
      glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

      const GLint sampler = m_backgroundProgram.getUniformLocation("colorMap");

      //std::cout<<"background sampler="<<sampler<<"\n";
      
      glUniform1i(sampler, 0);

      GL_CHECK_ERROR_ALWAYS();

    }
    //else {
    //glBindTexture(GL_TEXTURE_2D, 0);
    //}

    const int useTextureLoc = m_backgroundProgram.getUniformLocation("use_texture");
    if (useTextureLoc >= 0)
      glUniform1i(useTextureLoc, m_useBackgroundTexture);

    if (m_useTexture) {
      const int texMatLoc = m_backgroundProgram.getUniformLocation("tex_matrix");
      if (texMatLoc >= 0)
	glUniformMatrix3fv(texMatLoc, 1, GL_FALSE, m_backgroundTexMat.data());
    }


    assert(m_backgroundObject);
    m_backgroundObject->draw(m_camera);

  }




  if (m_object) {

    m_program.activate();

    if (m_textureId != 0) {
      
      //B:TODO:DESIGN: not sure m_program is active !!?! 
      // It will fail if we have several programs !
      
      //std::cout<<"paintGL m_textureId="<<m_textureId<<"\n";
      
      GL_CHECK_ERROR_ALWAYS();
      
      glBindTexture(GL_TEXTURE_2D, m_textureId);

      glBindSampler(0, m_samplerId);
      glSamplerParameteri(m_samplerId, GL_TEXTURE_MIN_FILTER, m_minFilter);
      glSamplerParameteri(m_samplerId, GL_TEXTURE_MAG_FILTER, m_magFilter);
      
      glActiveTexture(GL_TEXTURE0 + 0);
      glBindTexture(GL_TEXTURE_2D, m_textureId);
      
      const GLint sampler = m_program.getUniformLocation("colorMap");
      
      glUniform1i(sampler, 0);

      //std::cout<<"texture sampler="<<sampler<<"\n";
      
      GL_CHECK_ERROR_ALWAYS();
    }
    
    {
      const int lightPosViewLoc = m_program.getUniformLocation("light_position_view");
      if (lightPosViewLoc >= 0) {
	Eigen::Vector4f lightPosView = m_camera.getViewMatrix() * m_lightPos;
	//std::cout<<"lightPosView: "<<lightPosView(0)<<", "<<lightPosView(1)<<", "<<lightPosView(2)<<"\n";
	glUniform3f(lightPosViewLoc, lightPosView(0), lightPosView(1), lightPosView(2));
      }
      
      const int lightAmbientLoc = m_program.getUniformLocation("light_ambient");
      if (lightAmbientLoc >= 0) {
	//std::cout<<"m_ambient: "<<m_ambient(0)<<", "<<m_ambient(1)<<", "<<m_ambient(2)<<"\n";
	glUniform3fv(lightAmbientLoc, 1, m_ambient.data());
      }
      const int lightDiffuseLoc = m_program.getUniformLocation("light_diffuse");
      if (lightDiffuseLoc >= 0) {
	//std::cout<<"m_diffuse: "<<m_diffuse(0)<<", "<<m_diffuse(1)<<", "<<m_diffuse(2)<<"\n";
	glUniform3fv(lightDiffuseLoc, 1, m_diffuse.data());
      }
      const int lightSpecularLoc = m_program.getUniformLocation("light_specular");
      if (lightSpecularLoc >= 0) {
	//std::cout<<"m_specular: "<<m_specular(0)<<", "<<m_specular(1)<<", "<<m_specular(2)<<"\n";
	glUniform3fv(lightSpecularLoc, 1, m_specular.data());
      }
      
      const int specularExpLoc = m_program.getUniformLocation("specular_exponent");
      if (specularExpLoc >= 0) {
	//std::cout<<"m_specularExponent: "<<m_specularExponent<<"\n";
	glUniform1f(specularExpLoc, m_specularExponent);
      }
      
      const int useTextureLoc = m_program.getUniformLocation("use_texture");
      if (useTextureLoc >= 0)
	glUniform1i(useTextureLoc, m_useTexture);
      
      if (m_useTexture) {
	const int texMatLoc = m_program.getUniformLocation("tex_matrix");
	if (texMatLoc >= 0) 
	  glUniformMatrix3fv(texMatLoc, 1, GL_FALSE, m_texMat.data());
      }

    }
    
    
    m_object->draw(m_camera);
  }


}


cv::Mat
GLRenderer::render(bool random)
{
  paintGL(random);

  assert (glGetError () == GL_NO_ERROR);
  glFlush();
  assert (glGetError () == GL_NO_ERROR);

  int width, height;
  glfwGetFramebufferSize(m_window, &width, &height);

  char* buffer;
#if USE_NATIVE_OSMESA
  glfwGetOSMesaColorBuffer(m_window, &width, &height, NULL, (void**) &buffer);
#else
  buffer = calloc(4, width * height);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
#endif

  // Output image Y-flipped because OpenGL
  cv::Mat out(height, width, CV_8UC4, buffer);
  cv::flip(out, out, 0);

  if (m_channels == 1) {
    cv::cvtColor(out, out, cv::COLOR_RGBA2GRAY);
  }
  else if (m_channels == 3) {
    cv::cvtColor(out, out, cv::COLOR_RGBA2BGR);
  }
  else if (m_channels == 4) {

    //Warning: here, out may still point to "buffer"
    // (that will be deleted when the context is destroyed)
    // Thus we need to allocate a new image.

    cv::Mat out2(out.size(), CV_8UC4);
    cv::cvtColor(out, out2, cv::COLOR_RGBA2BGRA);

    return out2;
  }

  return out;
}


void GLRenderer::update()
{
  //std::cout<<"-- update()\n";
  
  paintGL();
}



void
GLRenderer::updateMeshGL()
{
  //for now, we delete & rebuild
  //TODO:OPTIM: do not re-allocate VBO if same size...

  delete m_meshGL;
  m_meshGL = new GLMesh;
  GL_CHECK_ERROR_ALWAYS();
  m_meshGL->init(m_mesh);
  GL_CHECK_ERROR_ALWAYS();
  assert(m_object);
  m_object->attachMesh(m_meshGL);
}




void
GLRenderer::setLightPos(float x, float y, float z)
{
  m_lightPos(0) = x;
  m_lightPos(1) = y;
  m_lightPos(2) = z;
  m_lightPos(3) = 1;
  
  //update();
}

void
GLRenderer::setAmbientDiffuseSpecular(Eigen::Vector3f a, Eigen::Vector3f d, Eigen::Vector3f s)
{
  m_ambient = a;
  m_diffuse = d;
  m_specular = s;

  //update();
}

void
GLRenderer::setSpecularExponent(float se)
{
  m_specularExponent = se;

  //update();
}

void
GLRenderer::setUseTexture(bool onoff)
{
  m_useTexture = onoff;

  //update();
}

void
GLRenderer::setTextureMatrix(const Eigen::Matrix3f &texMat)
{
  m_texMat = texMat;

  if (m_useTexture)
    update();
}

void
GLRenderer::applyTextureMatrix()
{
  if (m_mesh.hasTexCoords()) {

    for (uint32_t i=0; i<m_mesh.numVertices; ++i) {
      Eigen::Vector3f v(m_mesh.texCoords[2*i+0], m_mesh.texCoords[2*i+1], 1);
      v = m_texMat * v;
      m_mesh.texCoords[2*i+0] = v[0];
      m_mesh.texCoords[2*i+1] = v[1];
      //TODO: divide by v[2] ????
    }

    updateMeshGL();
  
  }
  m_texMat = Eigen::Matrix3f::Identity();

  //update();
}

void
GLRenderer::applyViewMatrix()
{
  const Eigen::Matrix4f &viewMat = m_camera.getViewMatrix();
  for (uint32_t i=0; i<m_mesh.numVertices; ++i) {
    Eigen::Vector4f v(m_mesh.vertices[3*i+0], m_mesh.vertices[3*i+1], m_mesh.vertices[3*i+2], 1);
    v = viewMat * v;
    m_mesh.vertices[3*i+0] = v[0];
    m_mesh.vertices[3*i+1] = v[1];
    m_mesh.vertices[3*i+2] = v[2];
    //TODO: divide by v[3] ???
  }

  updateMeshGL();

  
  resetViewMatrix();
}

void
GLRenderer::resetViewMatrix()
{
  m_camPhy = DEFAULT_CAMERA_PHY;
  m_camTheta = DEFAULT_CAMERA_THETA;
  m_camDist = DEFAULT_CAMERA_DIST;
  m_camLookAt = Eigen::Vector3f::Zero();
  m_camFov = DEFAULT_CAMERA_FOV;
  updateCameraLookAt();

  //update();
}

void
GLRenderer::setUseBackgroundTexture(bool onoff)
{
  m_useBackgroundTexture = onoff;
  m_useBackground = onoff;  //For now, setUSeTexture control both backgroundtexture & background presence
  
  if (m_mesh.isValid() && m_useBackground) {

    float minB[3], maxB[3];
    m_mesh.getAABB(minB, maxB);

    float s = 3;
    float sx = s*(maxB[0]-minB[0]);
    float sy = s*(maxB[1]-minB[1]);

    Eigen::Matrix4f t = Eigen::Matrix4f::Identity();
    //scale on X & Y
    t(0, 0) = sx; 
    t(1, 1) = sy; 
    //translation on Z 
    t(2, 3) = minB[2]; 
    m_backgroundObject->setTransformation(t);
  }


  //update();
}

void
GLRenderer::setBackgroundTexture(const cv::Mat &image)
{
  if (image.empty()) {
    std::cout<<"GLRenderer::setBackgroundTexture image.empty() !\n";
    return;
  }

  cv::Mat img = makeOpenGLImage(image);

  GL_CHECK_ERROR_ALWAYS();

  glGenTextures(1, &m_backgroundTextureId);
  glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

  m_backgroundProgram.activate();

  GL_CHECK_ERROR_ALWAYS();

  glGenSamplers(1, &m_backgroundSamplerId);
  glSamplerParameteri(m_backgroundSamplerId, GL_TEXTURE_MIN_FILTER, m_backgroundMinFilter);
  glSamplerParameteri(m_backgroundSamplerId, GL_TEXTURE_MAG_FILTER, m_backgroundMagFilter);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.cols, img.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, img.data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GL_CHECK_ERROR_ALWAYS();

  glGenerateMipmap(GL_TEXTURE_2D);

  GL_CHECK_ERROR_ALWAYS();

  m_channels = std::max(m_channels, image.channels());
  setUseBackgroundTexture(true);
  //update();

}

void
GLRenderer::setBackgroundTextureMatrix(const Eigen::Matrix3f &texMat)
{
  m_backgroundTexMat = texMat;
  
  if (m_useBackgroundTexture)
    update();
}
