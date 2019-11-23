/**
 * Author:	Fahim Hasan Khan (UCID#30013525)
 * Based on codes by: Andrew Robert Owens
 * Date:	February, 2017
 * Course:	CPSC 687 Fundamental of Computer Animation
 * Organization: University of Calgary
 *
 *
 * File:	main.cpp
 *
 */

#include <iostream>
#include <cmath>
#include <chrono>
#include <limits>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "ShaderTools.h"
#include "Vec3f.h"
#include "Vec3f_FileIO.h"
#include "Mat4f.h"
#include "OpenGLMatrixTools.h"
#include "Camera.h"

//==================== GLOBAL VARIABLES ====================//
/*	Put here for simplicity. Feel free to restructure into
*	appropriate classes or abstractions.
*/

// Drawing Program
GLuint basicProgramID;
GLuint basicProgramID1;

// Data needed for Plane
GLuint Plane_vaoID;
GLuint Plane_vertBufferID;
Mat4f Plane_M;

// Data needed for Quad
GLuint vaoID;
GLuint vertBufferID;
Mat4f M;

// Data needed for Curve 
GLuint Curve_vaoID;
GLuint Curve_vertBufferID;
Mat4f Curve_M;

// Data needed for Curve 1
GLuint Curve_vaoID1;
GLuint Curve_vertBufferID1;
Mat4f Curve_M1;

// Only one camera so only one veiw and perspective matrix are needed.
Mat4f V;
Mat4f P;

// Only one thing is rendered at a time, so only need one MVP
// When drawing different objects, update M and MVP = M * V * P
Mat4f MVP;

// Camera and veiwing Stuff
Camera camera;
int g_moveUpDown = 0;
int g_moveLeftRight = 0;
int g_moveBackForward = 0;
int g_rotateLeftRight = 0;
int g_rotateUpDown = 0;
int g_rotateRoll = 0;
float g_rotationSpeed = 0.015625;
float g_panningSpeed = 0.25;
bool g_cursorLocked;
float g_cursorX, g_cursorY;

bool g_play = false;
bool g_pov = false;

int WIN_WIDTH = 1500, WIN_HEIGHT = 900;
int FB_WIDTH = 1500, FB_HEIGHT = 900;
float WIN_FOV = 100;
float WIN_NEAR = 0.01;
float WIN_FAR = 1000;

float temp = 0;
int count = 0;
float arcL = 0.0;

VectorContainerVec3f vecs;
VectorContainerVec3f vecs2;

Vec3f *pts;
Vec3f *tangent;
Vec3f *normal;
Vec3f *binormal;
float *arc;

//==================== FUNCTION DECLARATIONS ====================//
void displayFunc();
void resizeFunc();
void init();
void generateIDs();
void deleteIDs();
void setupVAO();
void readfiles();
void loadQuadGeometryToGPU();
void reloadProjectionMatrix();
void loadModelViewMatrix();
void setupModelViewProjectionTransform();

void windowSetSizeFunc();
void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods);
void windowMouseMotionFunc(GLFWwindow *window, double x, double y);
void windowSetSizeFunc(GLFWwindow *window, int width, int height);
void windowSetFramebufferSizeFunc(GLFWwindow *window, int width, int height);
void windowMouseButtonFunc(GLFWwindow *window, int button, int action,
                           int mods);
void windowMouseMotionFunc(GLFWwindow *window, double x, double y);
void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods);
void animateQuad(float t);
void animateCam(float t);
void moveCamera();
void reloadMVPUniform();
void reloadColorUniform(float r, float g, float b);
std::string GL_ERROR();
int main(int, char **);

//==================== FUNCTION DEFINITIONS ====================//

void displayFunc() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Use our shader
  glUseProgram(basicProgramID);

  // ===== DRAW PLANE ====== //
  MVP = P * V * Plane_M;
  reloadMVPUniform();
  reloadColorUniform(0, 1, 0);

  // Use VAO that holds buffer bindings
  // and attribute config of buffers
  glBindVertexArray(Plane_vaoID);
  // Draw Box
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // ===== DRAW CART ====== //
  MVP = P * V * M;
  reloadMVPUniform();
  reloadColorUniform(1, 0, 1);

  // Use VAO that holds buffer bindings
  // and attribute config of buffers
  glBindVertexArray(vaoID);
  // Draw Box
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 36);


  // ==== DRAW Curve ===== //
  MVP = P * V * Curve_M;
  reloadMVPUniform();

  reloadColorUniform(0, 0, 1);

  // Use VAO that holds buffer bindings
  // and attribute config of buffers
  glBindVertexArray(Curve_vaoID);
  // Draw lines
  glDrawArrays(GL_LINE_LOOP, 0, count-1);

  // ==== DRAW Curve 2===== //
  MVP = P * V * Curve_M1;
  reloadMVPUniform();

  reloadColorUniform(0, 0, 1);

  // Use VAO that holds buffer bindings
  // and attribute config of buffers
  glBindVertexArray(Curve_vaoID1);
  // Draw lines
  glDrawArrays(GL_LINE_LOOP, 0, count-1);
  
}

void readfiles(){
	std::string file("track.txt");
	loadVec3fFromFile(vecs, file);

	std::string file1("car.txt");
	loadVec3fFromFile(vecs2, file1);
}


Vec3f Bezier(Vec3f A, Vec3f B, Vec3f C, Vec3f D, double t) {
	Vec3f P;

	P.x() = pow((1 - t), 3) * A.x() + 3 * t * pow((1 - t), 2) * B.x() + 3 * (1 - t) * pow(t, 2)* C.x() + pow(t, 3)* D.x();
	P.y() = pow((1 - t), 3) * A.y() + 3 * t * pow((1 - t), 2) * B.y() + 3 * (1 - t) * pow(t, 2)* C.y() + pow(t, 3)* D.y();
	P.z() = pow((1 - t), 3) * A.z() + 3 * t * pow((1 - t), 2) * B.z() + 3 * (1 - t) * pow(t, 2)* C.z() + pow(t, 3)* D.z();
	return P;
}

void animateQuad(float t) {
  int l = (int)t;
  //float angle = acos(pts[l].dotProduct(pts[l + 1]) / (pts[l].length() * pts[l + 1].length()));

  M = RotateAboutXMatrix(normal[l].x()*(180.0 / 3.1416));
  M = RotateAboutYMatrix(normal[l].y()*(180.0 / 3.1416));
  M = RotateAboutZMatrix(normal[l].z()*(180.0 / 3.1416));
  
  
  M = TranslateMatrix(pts[l].x(), pts[l].y(), pts[l].z()) * M;
  printf("\n t = %f", t);

  setupModelViewProjectionTransform();
  reloadMVPUniform();
  //temp = s;
}


void loadQuadGeometryToGPU() {
  // Just basic layout of floats, for a quad
  // 3 floats per vertex, 4 vertices
  std::vector<Vec3f> verts;

  for (auto & v : vecs2)
  {
	  verts.push_back(v);
	  std::cout << v << std::endl;
  }

  
  glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(Vec3f) * 36, // byte size of Vec3f, 4 of them
               verts.data(),      // pointer (Vec3f*) to contents of verts
               GL_STATIC_DRAW);   // Usage pattern of GPU buffer
}

void loadPlaneGeometryToGPU() {
	// Just basic layout of floats, for a quad
	// 3 floats per vertex, 4 vertices
	std::vector<Vec3f> verts;
	verts.push_back(Vec3f(-30, -5, -10));
	verts.push_back(Vec3f(30, -5, -10));
	verts.push_back(Vec3f(-30, -5, 10));
	verts.push_back(Vec3f(30, -5, 10));

	glBindBuffer(GL_ARRAY_BUFFER, Plane_vertBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Vec3f)* 4, // byte size of Vec3f, 4 of them
		verts.data(),      // pointer (Vec3f*) to contents of verts
		GL_STATIC_DRAW);   // Usage pattern of GPU buffer
}


void loadCurveGeometryToGPU() {
	// Just basic layout of floats, for a quad
	// 3 floats per vertex, 4 vertices
	std::vector<Vec3f> verts;
	std::vector<Vec3f> verts1;

	Vec3f abc[100];
	
	int i = 0;
	for (auto & v : vecs)
	{
		std::cout << v << std::endl;
		abc[i] = v;
		i++;
	}

	int m = i * 33;

	pts = (Vec3f *)malloc(m*sizeof(Vec3f));
	arc = (float *)malloc(m*sizeof(float));
	tangent = (Vec3f *)malloc(m*sizeof(Vec3f));
	normal = (Vec3f *)malloc(m*sizeof(Vec3f));
	binormal = (Vec3f *)malloc(m*sizeof(Vec3f));

	
	for (int j = 0; j < i-2; j=j+3){
		for (double t = 0.0; t <= 1.0; t += 0.01) {
			Vec3f P = Bezier(abc[j], abc[j+1], abc[j+2], abc[j+3], t);
			//verts.push_back(P);
			
			//verts1.push_back(Vec3f(P.x(), P.y(), P.z()-0.5));
			pts[count] = P;
			if (count>0){

				float length = P.distance(pts[count - 1]);
				//printf("\nArcLength %f", arcL);
				arcL = arcL + length;
				arc[count] = length;
			}	
			count++;
		}
	}

	for (int k = 1; k < count; k++){
		Vec3f tmp = pts[k+1] - pts[k];
		tangent[k] = tmp.normalized();
		printf("\nTangent %f,%f,%f", tangent[k].x(), tangent[k].y(), tangent[k].z());

		tmp = pts[k+1] - 2 * pts[k] + pts[k-1];
		normal[k] = tmp.normalized();
		printf("\nNormal %f,%f,%f", normal[k].x(), normal[k].y(), normal[k].z());

		tmp = tangent[k].crossProduct(normal[k]);
		binormal[k] = tmp.normalized();
		float u = sqrt(binormal[k].x()*binormal[k].x() + binormal[k].y()*binormal[k].y() + binormal[k].z()*binormal[k].z());
		printf("\nBinormal %f,%f,%f,%f", binormal[k].x(), binormal[k].y(), binormal[k].z(), u);

		verts.push_back(pts[k] - 0.5*binormal[k]);
		verts1.push_back(pts[k] + 0.5*binormal[k]);
		//verts.push_back(Vec3f(pts[k].x(), pts[k].y(), pts[k].z()+0.5));
		//verts1.push_back(Vec3f(pts[k].x(), pts[k].y(), pts[k].z()-0.5));
	}


	glBindBuffer(GL_ARRAY_BUFFER, Curve_vertBufferID);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Vec3f)* count, // byte size of Vec3f, 4 of them
		verts.data(),      // pointer (Vec3f*) to contents of verts
		GL_STATIC_DRAW);   // Usage pattern of GPU buffer

	glBindBuffer(GL_ARRAY_BUFFER, Curve_vertBufferID1);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Vec3f)* count, // byte size of Vec3f, 4 of them
		verts1.data(),      // pointer (Vec3f*) to contents of verts
		GL_STATIC_DRAW);   // Usage pattern of GPU buffer
}

void setupVAO() {
  glBindVertexArray(vaoID);

  glEnableVertexAttribArray(0); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
  glVertexAttribPointer(0,        // attribute layout # above
                        3,        // # of components (ie XYZ )
                        GL_FLOAT, // type of components
                        GL_FALSE, // need to be normalized?
                        0,        // stride
                        (void *)0 // array buffer offset
                        );

  glBindVertexArray(Plane_vaoID);

  glEnableVertexAttribArray(0); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, Plane_vertBufferID);
  glVertexAttribPointer(0,        // attribute layout # above
	  3,        // # of components (ie XYZ )
	  GL_FLOAT, // type of components
	  GL_FALSE, // need to be normalized?
	  0,        // stride
	  (void *)0 // array buffer offset
	  );


  glBindVertexArray(Curve_vaoID);

  glEnableVertexAttribArray(0); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, Curve_vertBufferID);
  glVertexAttribPointer(0,        // attribute layout # above
						3,        // # of components (ie XYZ )
						GL_FLOAT, // type of components
						GL_FALSE, // need to be normalized?
						0,        // stride
						(void *)0 // array buffer offset
						);

  glBindVertexArray(Curve_vaoID1);

  glEnableVertexAttribArray(0); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, Curve_vertBufferID1);
  glVertexAttribPointer(0,        // attribute layout # above
	  3,        // # of components (ie XYZ )
	  GL_FLOAT, // type of components
	  GL_FALSE, // need to be normalized?
	  0,        // stride
	  (void *)0 // array buffer offset
	  );

  glBindVertexArray(0); // reset to default
}

void reloadProjectionMatrix() {
  // Perspective Only

  // field of view angle 60 degrees
  // window aspect ratio
  // near Z plane > 0
  // far Z plane

  P = PerspectiveProjection(WIN_FOV, // FOV
                            static_cast<float>(WIN_WIDTH) /
                                WIN_HEIGHT, // Aspect
                            WIN_NEAR,       // near plane
                            WIN_FAR);       // far plane depth
}

void loadModelViewMatrix() {
  M = IdentityMatrix();
  Plane_M = IdentityMatrix();
  Curve_M = IdentityMatrix();
  Curve_M1 = IdentityMatrix();
  // view doesn't change, but if it did you would use this
  V = camera.lookatMatrix();
}

void reloadViewMatrix() { V = camera.lookatMatrix(); }

void setupModelViewProjectionTransform() {
  MVP = P * V * M; // transforms vertices from right to left (odd huh?)
}


void reloadMVPUniform() {
  GLint id = glGetUniformLocation(basicProgramID, "MVP");

  glUseProgram(basicProgramID);
  glUniformMatrix4fv(id,        // ID
                     1,         // only 1 matrix
                     GL_TRUE,   // transpose matrix, Mat4f is row major
                     MVP.data() // pointer to data in Mat4f
                     );
}

void reloadColorUniform(float r, float g, float b) {
  GLint id = glGetUniformLocation(basicProgramID, "inputColor");

  glUseProgram(basicProgramID);
  glUniform3f(id, // ID in basic_vs.glsl
              r, g, b);
}

void generateIDs() {
  // shader ID from OpenGL
  std::string vsSource = loadShaderStringfromFile("./shaders/basic_vs.glsl");
  std::string gsSource = loadShaderStringfromFile("./shaders/basic_gs.glsl");
  std::string fsSource = loadShaderStringfromFile("./shaders/basic_fs.glsl");
  basicProgramID = CreateShaderProgram(vsSource, fsSource);
  //basicProgramID = CreateShaderProgram(vsSource, gsSource, fsSource);

  // VAO and buffer IDs given from OpenGL
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vertBufferID);
  glGenVertexArrays(1, &Plane_vaoID);
  glGenBuffers(1, &Plane_vertBufferID);
  glGenVertexArrays(1, &Curve_vaoID);
  glGenBuffers(1, &Curve_vertBufferID);
  glGenVertexArrays(1, &Curve_vaoID1);
  glGenBuffers(1, &Curve_vertBufferID1);
}

void deleteIDs() {
  glDeleteProgram(basicProgramID);

  glDeleteVertexArrays(1, &vaoID);
  glDeleteBuffers(1, &vertBufferID);

  glDeleteVertexArrays(1, &Plane_vaoID);
  glDeleteBuffers(1, &Plane_vertBufferID);

  glDeleteVertexArrays(1, &Curve_vaoID);
  glDeleteBuffers(1, &Curve_vertBufferID);

  glDeleteVertexArrays(1, &Curve_vaoID1);
  glDeleteBuffers(1, &Curve_vertBufferID1);
}

void init() {
  glEnable(GL_DEPTH_TEST);
  glPointSize(50);

  camera = Camera(Vec3f{ 0, 0, 20 }, Vec3f{ 0, 1, -2 }, Vec3f{ 0, 1, 1 });
  //camera = Camera(Vec3f{0, 0, 15}, Vec3f{0, 0, -1}, Vec3f{0, 1, 0}); //Side View
  //camera = Camera(Vec3f{ 0, 10, 0 }, Vec3f{ 0, -1, 0 }, Vec3f{ 0, 0, 1 }); //Top View
  

  // SETUP SHADERS, BUFFERS, VAOs
  readfiles();
  generateIDs();
  setupVAO();
  loadQuadGeometryToGPU();
  loadCurveGeometryToGPU();
  loadPlaneGeometryToGPU();

  loadModelViewMatrix();
  reloadProjectionMatrix();
  setupModelViewProjectionTransform();
  reloadMVPUniform();
}

int main(int argc, char **argv) {
  GLFWwindow *window;

  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window =
      glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "CPSC 687 A1", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetWindowSizeCallback(window, windowSetSizeFunc);
  glfwSetFramebufferSizeCallback(window, windowSetFramebufferSizeFunc);
  glfwSetKeyCallback(window, windowKeyFunc);
  glfwSetCursorPosCallback(window, windowMouseMotionFunc);
  glfwSetMouseButtonCallback(window, windowMouseButtonFunc);

  glfwGetFramebufferSize(window, &WIN_WIDTH, &WIN_HEIGHT);

  // Initialize glad
  if (!gladLoadGL()) {
    std::cerr << "Failed to initialise GLAD" << std::endl;
    return -1;
  }

  std::cout << "GL Version: :" << glGetString(GL_VERSION) << std::endl;
  std::cout << GL_ERROR() << std::endl;

  // Initialize all the geometry, and load it once to the GPU
  init(); // our own initialize stuff func

  float t = 0;
  float dt = 0;
  float H = 16.200920;
  float speed = 0;
  int stage = 0;
  float mh = 0;
  float AM = 0.1;

  animateQuad(t);

  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         !glfwWindowShouldClose(window)) {

    if (g_play) {

	 float h=pts[(int)t].y();
	 if (h > mh) mh = h;
	 printf("\nH = %f", mh);

	 if (H == h) stage = 1;
	 if (t>(count-3)) stage = 2;

	 if (stage == 0){
		 speed = 5;
		 dt = (speed*AM);
	 }
	 if (stage == 1){
		 speed = sqrt(2 * 9.8*((H+2.393216) - h));
		 //dt = (AM*arc[(int)t])/speed;
		 dt = (speed*AM);
	 }
	 if (stage == 2){
		 speed = AM*speed*((count - t) / 3);
		 dt = speed;
	 }

	 printf("\nStage = %d, Speed = %f, n = %f", stage, dt, normal[(int)t].length());
      
	 t += dt;
      animateQuad(t);
	  
	  
	  if (t > (count-1)){
		  t = 0;
		  speed = 0;
		  stage = 0;
		  dt = 1;
		  g_play = false;
		  animateQuad(t);
	  }
    }

	if (g_pov)
		animateCam(t);

    displayFunc();
    moveCamera();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // clean up after loop
  deleteIDs();

  return 0;
}

//==================== CALLBACK FUNCTIONS ====================//

void windowSetSizeFunc(GLFWwindow *window, int width, int height) {
  WIN_WIDTH = width;
  WIN_HEIGHT = height;

  reloadProjectionMatrix();
  setupModelViewProjectionTransform();
  reloadMVPUniform();
}

void windowSetFramebufferSizeFunc(GLFWwindow *window, int width, int height) {
  FB_WIDTH = width;
  FB_HEIGHT = height;

  glViewport(0, 0, FB_WIDTH, FB_HEIGHT);
}

void windowMouseButtonFunc(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      g_cursorLocked = GL_TRUE;
    } else {
      g_cursorLocked = GL_FALSE;
    }
  }
}

void windowMouseMotionFunc(GLFWwindow *window, double x, double y) {
  if (g_cursorLocked) {
    float deltaX = (x - g_cursorX) * 0.01;
    float deltaY = (y - g_cursorY) * 0.01;
    camera.rotateAroundFocus(deltaX, deltaY);

    reloadViewMatrix();
    setupModelViewProjectionTransform();
    reloadMVPUniform();
  }

  g_cursorX = x;
  g_cursorY = y;
}

void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods) {
  bool set = action != GLFW_RELEASE && GLFW_REPEAT;
  switch (key) {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GL_TRUE);
    break;
  case GLFW_KEY_W:
    g_moveBackForward = set ? 1 : 0;
    break;
  case GLFW_KEY_S:
    g_moveBackForward = set ? -1 : 0;
    break;
  case GLFW_KEY_A:
    g_moveLeftRight = set ? 1 : 0;
    break;
  case GLFW_KEY_D:
    g_moveLeftRight = set ? -1 : 0;
    break;
  case GLFW_KEY_Q:
    g_moveUpDown = set ? -1 : 0;
    break;
  case GLFW_KEY_E:
    g_moveUpDown = set ? 1 : 0;
    break;
  case GLFW_KEY_UP:
    g_rotateUpDown = set ? -1 : 0;
    break;
  case GLFW_KEY_DOWN:
    g_rotateUpDown = set ? 1 : 0;
    break;
  case GLFW_KEY_LEFT:
    if (mods == GLFW_MOD_SHIFT)
      g_rotateRoll = set ? -1 : 0;
    else
      g_rotateLeftRight = set ? 1 : 0;
    break;
  case GLFW_KEY_RIGHT:
    if (mods == GLFW_MOD_SHIFT)
      g_rotateRoll = set ? 1 : 0;
    else
      g_rotateLeftRight = set ? -1 : 0;
    break;
  case GLFW_KEY_SPACE:
    g_play = set ? !g_play : g_play;
    break;
  case GLFW_KEY_C:
	  g_pov = set ? !g_pov : g_pov;
	  break;
  case GLFW_KEY_LEFT_BRACKET:
    if (mods == GLFW_MOD_SHIFT) {
      g_rotationSpeed *= 0.5;
    } else {
      g_panningSpeed *= 0.5;
    }
    break;
  case GLFW_KEY_RIGHT_BRACKET:
    if (mods == GLFW_MOD_SHIFT) {
      g_rotationSpeed *= 1.5;
    } else {
      g_panningSpeed *= 1.5;
    }
    break;
  default:
    break;
  }
}

//==================== OPENGL HELPER FUNCTIONS ====================//

void moveCamera() {
  Vec3f dir;

  if (g_moveBackForward) {
    dir += Vec3f(0, 0, g_moveBackForward * g_panningSpeed);
  }
  if (g_moveLeftRight) {
    dir += Vec3f(g_moveLeftRight * g_panningSpeed, 0, 0);
  }
  if (g_moveUpDown) {
    dir += Vec3f(0, g_moveUpDown * g_panningSpeed, 0);
  }

  if (g_rotateUpDown) {
    camera.rotateUpDown(g_rotateUpDown * g_rotationSpeed);
  }
  if (g_rotateLeftRight) {
    camera.rotateLeftRight(g_rotateLeftRight * g_rotationSpeed);
  }
  if (g_rotateRoll) {
    camera.rotateRoll(g_rotateRoll * g_rotationSpeed);
  }

  if (g_moveUpDown || g_moveLeftRight || g_moveBackForward ||
      g_rotateLeftRight || g_rotateUpDown || g_rotateRoll) {
    camera.move(dir);
    reloadViewMatrix();
    setupModelViewProjectionTransform();
    reloadMVPUniform();
  }
}

void animateCam(float t) {

	int l = (int)t;
	Vec3f dir;
	//camera = Camera(Vec3f(pts[l].x() - 5, pts[l].y() + 5, pts[l].z()), pts[l+1]-pts[l], Vec3f{ 0, 1, 0 });
	camera = Camera(Vec3f(pts[l].x() - 1, pts[l].y() + 2, pts[l].z()), tangent[l], Vec3f{ 0, 1, 0 });

	reloadViewMatrix();
	setupModelViewProjectionTransform();
	reloadMVPUniform();
}


std::string GL_ERROR() {
  GLenum code = glGetError();

  switch (code) {
  case GL_NO_ERROR:
    return "GL_NO_ERROR";
  case GL_INVALID_ENUM:
    return "GL_INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "GL_INVALID_VALUE";
  case GL_INVALID_OPERATION:
    return "GL_INVALID_OPERATION";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  case GL_OUT_OF_MEMORY:
    return "GL_OUT_OF_MEMORY";
  default:
    return "Non Valid Error Code";
  }
}
