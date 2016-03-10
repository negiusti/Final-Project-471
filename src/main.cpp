/* P3 materials example - see SetMaterial and fragment shader
CPE 471 Cal Poly Z. Wood + S. Sueda
*/
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Math.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "BoundingSphere.h"
#include "Texture.h"

#define NUM_SHAPES 2

using namespace std;
using namespace Eigen;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
shared_ptr<Program> prog, floorprog;
shared_ptr<Shape> bathing, mars, shape1, shape2, flashlightShape;

Texture floortexture;
GLint h_floortexture;

int g_GiboLen;
int g_width, g_height, changeDir;
int gMat = 0;
int mode = 1;
float zOffset = 0;
float xOffset = 0;
float phi = 0;
float theta = -M_PI_2;
float eyeX, eyeY, eyeZ, targetX, targetY, targetZ; 
float radius = 2.0;
float marsX = -4;
float marsZ = 2;
float bathingX = 2;
float bathingZ = -4;
BoundingSphere spheres [NUM_SHAPES];
Vector3f u, v, w, gaze, target, eye, flashlight, targetDir, targetTrans;
GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

bool hasCollisions(Vector3f eye) {
	for (int i = 0; i < NUM_SHAPES; i++) {
		if(spheres[i].willCollide(eye)) {
			return true;
		}
	}
	return false;
}

bool wallCollide(Vector3f eye) {
	if (eye[0] < -35 || eye[0] > 35 
		|| eye[1] < -35 || eye[1] > 35
		|| eye[2] < -35 || eye[2] > 35) {
		return true;
	}
	return false;
}

/* code to define the ground plane */
static void initGeom() {

   float g_groundSize = 1000;
   float g_groundY = -0.5;

  // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
    float GrndPos[] = {
    -g_groundSize, g_groundY, -g_groundSize,
    -g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY, -g_groundSize
    };

    float GrndNorm[] = {
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0
    };


  static GLfloat GrndTex[] = {
      0,0,
      0,400,
      400,400,
      400,0
  };

    unsigned short idx[] = {0, 1, 2, 0, 2, 3};


   GLuint VertexArrayID;
	//generate the VAO
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

    g_GiboLen = 6;
    glGenBuffers(1, &GrndBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

    glGenBuffers(1, &GrndNorBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);
    
	glGenBuffers(1, &GrndTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

    glGenBuffers(1, &GIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	// don't allow camera to "fly" or "dig into ground"
	Vector3f newW;
	newW[0] = w[0];
	newW[2] = w[2];
	newW[1] = w[1] - (w.dot(Vector3f(0,1,0)));
	printf("eye %f %f %f\n", eye[0], eye[1], eye[2]);
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	else if (key == GLFW_KEY_W && !hasCollisions(eye - .7*newW) && !wallCollide(eye - .7*newW)) {
		eye = eye - .7*newW;
		targetTrans = targetTrans - .7*newW;
		flashlight = flashlight - .7*newW;
	}
	else if (key == GLFW_KEY_S && !hasCollisions(eye + .7*newW) && !wallCollide(eye + .7*newW)) {
		eye = eye + .7*newW;
		targetTrans = targetTrans + .7*newW;
		flashlight = flashlight + .7*newW;
	}
	else if (key == GLFW_KEY_A && !hasCollisions(eye - .7*u) && !wallCollide(eye - .7*u)) {
		eye = eye - .7*u;
		targetTrans = targetTrans - .7*u;
		flashlight = flashlight - .7*u;
	}
	else if (key == GLFW_KEY_D && !hasCollisions(eye + .7*u) && !wallCollide(eye + .7*u)) {
		eye = eye + .7*u;
		targetTrans = targetTrans + .7*u;
		flashlight = flashlight + .7*u;
	}

}

static void scroll_fun(GLFWwindow *window, double dX, double dY) {
	glfwGetWindowSize (window, &g_width, &g_height);
	phi -= dY*M_PI/g_height;
    theta -= dX*M_PI/g_width;
    targetDir = Vector3f(cos(phi)*cos(theta), sin(phi), cos(phi)*cos(M_PI_2-theta));
}

static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
   double posX, posY;
   if (action == GLFW_PRESS) {
      glfwGetCursorPos(window, &posX, &posY);
      cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
	}
}

static void resize_callback(GLFWwindow *window, int width, int height) {
   g_width = width;
   g_height = height;
   glViewport(0, 0, width, height);
}

// helper function to set materials
void SetMaterial(int i, shared_ptr<Program> prog) {
  switch (i) {
    case 0: // blue petals
 		glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);
 		glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 2.9);
 		glUniform3f(prog->getUniform("MatSpec"), 0.14, 2.2, 1.8);
 		glUniform1f(prog->getUniform("Shine"), 0.1);
        break;
    case 1: // red petals
 		glUniform3f(prog->getUniform("MatAmb"), 4.53, 0.13, 0.14);
 		glUniform3f(prog->getUniform("MatDif"), 4.4, 0.1, 0.1);
 		glUniform3f(prog->getUniform("MatSpec"), 4.3, 0.7, 0.4);
 		glUniform1f(prog->getUniform("Shine"), 2.1);
      break;
    case 2: // yellow pollen/petals
 		glUniform3f(prog->getUniform("MatAmb"), 4.3294, 4.2235, 0.02745);
 		glUniform3f(prog->getUniform("MatDif"), 2.7804, 2.5686, 0.11373);
 		glUniform3f(prog->getUniform("MatSpec"), 4.9922, 4.941176, 0.80784);
 		glUniform1f(prog->getUniform("Shine"), 3.1);
        break;
	 case 3: // brown for the doggies
 		glUniform3f(prog->getUniform("MatAmb"), 0.1913, 0.0735, 0.0225);
 		glUniform3f(prog->getUniform("MatDif"), 0.7038, 0.27048, 0.0828);
 		glUniform3f(prog->getUniform("MatSpec"), 0.257, 0.1376, 0.08601);
 		glUniform1f(prog->getUniform("Shine"), 1.2);
        break;
     case 4: // pink petals
 		glUniform3f(prog->getUniform("MatAmb"), 0.55, 0.13, 0.24);
 		glUniform3f(prog->getUniform("MatDif"), 0.8804, 0.6686, 0.66373);
 		glUniform3f(prog->getUniform("MatSpec"), 0.09922, 0.0941176, 0.080784);
 		glUniform1f(prog->getUniform("Shine"), 0.1);
        break;
     case 5: // floor
 		glUniform3f(prog->getUniform("MatAmb"), 0.05, 0.11, 0.04);
 		glUniform3f(prog->getUniform("MatDif"), 0.01, 0.13, 0.01);
 		glUniform3f(prog->getUniform("MatSpec"), 0.09922, 0.0941176, 0.080784);
 		glUniform1f(prog->getUniform("Shine"), 0.01);
        break;
     case 6: // marble
 		glUniform3f(prog->getUniform("MatAmb"), 0.05, 0.11, 0.04);
 		glUniform3f(prog->getUniform("MatDif"), 1.01, 1.13, 1.01);
 		glUniform3f(prog->getUniform("MatSpec"), 0.09922, 0.0941176, 0.080784);
 		glUniform1f(prog->getUniform("Shine"), 2.01);
        break;
	}
}

static void init()
{
	GLSL::checkVersion();

	// Set background color.
	glClearColor(0, 0, 0, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
	// Initialize mesh.

    flashlightShape = make_shared<Shape>();
	flashlightShape->loadMesh(RESOURCE_DIR + "Flashlight.obj");
	flashlightShape->resize();
	flashlightShape->init();

	shape1 = make_shared<Shape>();
	shape1->loadMesh(RESOURCE_DIR + "sphere.obj");
	shape1->resize();
	shape1->init();

	shape2 = make_shared<Shape>();
	shape2->loadMesh(RESOURCE_DIR + "cube.obj");
	shape2->resize();
	shape2->init();

	bathing = make_shared<Shape>();
	bathing->loadMesh(RESOURCE_DIR + "nymph1.obj");
	bathing->resize();
	bathing->init();

	mars = make_shared<Shape>();
	mars->loadMesh(RESOURCE_DIR + "mars.obj");
	mars->resize();
	mars->init();

	// Initialize the GLSL program.
	prog = make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
	prog->init();
	
	prog->addUniform("P");
	prog->addUniform("V");
	prog->addUniform("MV");
	prog->addUniform("MatAmb");
	prog->addUniform("MatDif");
	prog->addUniform("MatSpec");
 	prog->addUniform("Shine");
 	prog->addUniform("lightPos");
	prog->addAttribute("vertPos");
	prog->addAttribute("vertNor");
	prog->addAttribute("vertTex");

	floorprog = make_shared<Program>();
	floorprog->setVerbose(true);
	floorprog->setShaderNames(RESOURCE_DIR + "floor_vert.glsl", RESOURCE_DIR + "floor_frag.glsl");
	floorprog->init();

	floortexture.setFilename(RESOURCE_DIR + "white_cinder_blocks.bmp");
	floortexture.setUnit(0);
    floortexture.setName("FloorTexture");
  	floortexture.init();

	floorprog->addUniform("P");
	floorprog->addUniform("MV");
	floorprog->addUniform("V");
	floorprog->addUniform("lightPos");
	floorprog->addUniform("FloorTexture");
	floorprog->addAttribute("vertPos");
	floorprog->addAttribute("vertNor");
	floorprog->addAttribute("vertTex");
	floorprog->addTexture(&floortexture);

	//spheres[0] = BoundingSphere(Vector3f((bathing->maxX + bathing->minX)/2 + bathingX, 7, (bathing->maxZ + bathing->minZ)/2 + bathingZ), 
	//	(bathing->maxX - bathing->minX) * 5);
	spheres[0] = BoundingSphere(Vector3f(bathingX*5, 7, bathingZ*5), 
		(bathing->maxX - bathing->minX));
	
	spheres[1] = BoundingSphere(Vector3f(marsX*5, 7, marsZ*5), 
		(mars->maxX - mars->minX));
	//spheres[1] = BoundingSphere(Vector3f((mars->maxX + mars->minX)/2 + marsX, 7, (mars->maxZ + mars->minZ)/2 + marsZ), 
	//	(mars->maxX - mars->minX) * 5);

	targetDir = Vector3f(cos(phi)*cos(theta), sin(phi), cos(phi)*cos(M_PI_2-theta));
	targetTrans = Vector3f(0, 7, 0);
	eye = Vector3f(0, 7, 0);
	glfwSetScrollCallback (window, scroll_fun);
}

static void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   float aspect = width/(float)height;

   auto P = make_shared<MatrixStack>();
   auto MV = make_shared<MatrixStack>();
   auto V = make_shared<MatrixStack>();
   // Apply perspective projection.
   P->pushMatrix();
   P->perspective(45.0f, aspect, 0.01f, 100.0f);

	// Draw a stack of cubes with indiviudal transforms 
	prog->bind();
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
	
	target = targetTrans + targetDir;
	gaze = target - eye;
	w = (-gaze).normalized();
	u = (Vector3f(0,1,0).cross(w)).normalized();
	v = w.cross(u);

	V->loadIdentity();
	glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());

	V->lookAt(eye, target, Vector3f(0, 1, 0));
	glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
	glUniform3f(prog->getUniform("lightPos"), eye[0], eye[1], eye[2]);

// WALLS
	SetMaterial(3, prog);
	// WALL 1
	MV->pushMatrix();
    MV->loadIdentity();
    MV->scale(Vector3f(.2, 50, 100));
    MV->translate(Vector3f(-200, 0, 0));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
	shape2->draw(prog);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	MV->popMatrix();

	// WALL 2
	MV->pushMatrix();
    MV->loadIdentity();
    MV->scale(Vector3f(.2, 50, 100));
    MV->translate(Vector3f(200, 0, 0));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
	shape2->draw(prog);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	MV->popMatrix();

	// WALL 3
	MV->pushMatrix();
    MV->loadIdentity();
    MV->scale(Vector3f(100, 50, .2));
    MV->translate(Vector3f(0, 0, 200));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
	shape2->draw(prog);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	MV->popMatrix();

	// WALL 4
	MV->pushMatrix();
    MV->loadIdentity();
    MV->scale(Vector3f(100, 50, .2));
    MV->translate(Vector3f(0, 0, -200));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
	shape2->draw(prog);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	MV->popMatrix();
// END WALLS

// BEG STATUES
	// bathing lady
	SetMaterial(6, prog);
	MV->pushMatrix();
    MV->loadIdentity();
    MV->scale(Vector3f(5, 5, 5));
    MV->translate(Vector3f(bathingX, 1, bathingZ));
	MV->rotate(180, Vector3f(0, 0, 1));
	MV->rotate(180, Vector3f(0, 1, 0));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
	bathing->draw(prog);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	MV->popMatrix();
   
    // mars
	MV->pushMatrix();
    MV->loadIdentity();
    MV->scale(Vector3f(5, 5, 5));
    MV->translate(Vector3f(marsX, 1, marsZ));
	MV->rotate(180, Vector3f(0, 0, 1));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
	mars->draw(prog);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	MV->popMatrix();
// END STATUES
	
	// flashlight
	V->pushMatrix();
	V->loadIdentity();
	glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
	MV->pushMatrix();
    MV->loadIdentity();
    MV->translate(Vector3f(1, 6, -3));
    MV->rotate(targetDir[0], Vector3f(1, 0, 0));
    MV->rotate(targetDir[1], Vector3f(0, 1, 0));
    MV->rotate(targetDir[2], Vector3f(0, 0, 1));
    MV->rotate(180, Vector3f(0, 1, 0));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
	flashlightShape->draw(prog);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	MV->popMatrix();
	V->popMatrix();
prog->unbind();

floorprog->bind();
	/* draw floor */
   MV->pushMatrix();
     MV->loadIdentity();
     glUniformMatrix4fv(floorprog->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
	  	glUniformMatrix4fv(floorprog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
	

	glUniformMatrix4fv(floorprog->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());

	glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
   glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	 
	glEnableVertexAttribArray(2);
   glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

   // draw!
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
   glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	MV->popMatrix();
   floorprog->unbind();

	

   // Pop matrix stacks.
   P->popMatrix();
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
   //request the highest possible version of OGL - important for mac
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "Nicole Giusti", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	//weird bootstrap of glGetError
   glGetError();
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
   cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
   //set the mouse call back
   glfwSetMouseButtonCallback(window, mouse_callback);
   //set the window resize call back
   glfwSetFramebufferSizeCallback(window, resize_callback);

	// Initialize scene. Note geometry initialized in init now
	init();
	initGeom();

	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


/*

Cool Looking Vases (V)
http://www.turbosquid.com/3d-models/free-vase-turquoise-3d-model/571678
http://www.turbosquid.com/3d-models/free-ceramic-pottery-vase-3d-model/627797

Skeleton Statue (S)
http://www.turbosquid.com/3d-models/free-obj-mode-scan-ligier-richier/969915 

Greek/Roman Sculptures (R)
Mars
http://www.turbosquid.com/3d-models/scan-statue-mars-obj-free/942970

Mercury
http://www.turbosquid.com/3d-models/free-obj-mode-scan-statue-mercury/943904 

Hermes
http://www.turbosquid.com/3d-models/free-obj-mode-scan-statue-hermes/1006366 

Venus w/ Cupid
http://www.turbosquid.com/3d-models/free-obj-model-scan-statue-venus-kissing/955680 

Hunter w/ Dog
http://www.turbosquid.com/3d-models/free-scan-statue-hunter-dog-3d-model/791511

Marble Player
http://www.turbosquid.com/3d-models/scan-statue-marble-player-obj-free/943887

David by Michelangelo 
https://sketchfab.com/models/8f4827cf36964a17b90bad11f48298ac

*/