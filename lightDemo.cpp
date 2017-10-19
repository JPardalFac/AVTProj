//
// AVT demo light 
// based on demos from GLSL Core Tutorial in Lighthouse3D.com   
//
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
//

#include <math.h>
#include <iostream>
#include <sstream>

#include <string>
#include <math.h> 
// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>

// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "basic_geometry.h"
#include "camera.h"
#include "Object.h"

#define CAPTION "AVT Light Demo"
#define PI 3.1415926

int WindowHandle = 0;
int WinX = 640, WinY = 480;

unsigned int FrameCount = 0;

float ratio = 0;
float* pratio = &ratio;
camera* cam = new camera();
//Object track[NUM_OF_];

VSShaderLib shader;

const int numObjs = 10;
struct MyMesh mesh[numObjs];
int objId = 0; //id of the object mesh - to be used as index of mesh: mesh[objID] means the current mesh

//wheels deviations from car body
float dev = 0.5f;

float xpos[4] = { 0.5f ,0.5f ,-0.5f,-0.5f  };
float ypos[4] = { 0.5f ,-0.5f ,0.5f,-0.5f  };

//car pos and rot
float currentPos[3] = {.5f, .5f, -.5f};
float currentRot = 0;
float rotInc = 3;
float currentRotAxis[3] = {0.0f,1.0f,0.0f};

enum direction { back, forward, left, right };
float speed = 0.5f;
bool move_forward = false, move_back = false, rot_left = false, rot_right = false;

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint lPos_uniformId;
GLint directionalLightOn_uniformId;

// Cameras Position
float camX, camY, camZ;
float FOcamX, FOcamY, FOcamZ;
float FPcamX, FPcamY, FPcamZ;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 10.0f;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];
bool isDirLightOn = true;
float lightPos[4] = { 4.0f, 6.0f, 2.0f, 0.0f };//switched from the default pointlight(w = 1) to a directional light (w = 0)

//test variables
int angle = 0;

void Timer(int value)
{
	angle += 2;
	glutPostRedisplay();
	glutTimerFunc(100, Timer, 0);
}

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;
	glutTimerFunc(1000, timer, 0);
}

void refresh(int value)
{
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, refresh, 0);
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	*pratio = (1.0f * w) / h;
	loadIdentity(PROJECTION);
	cam->updateProjection(pratio);
}


// ------------------------------------------------------------
//
// Render stufff
//

void moveCar(int direction) {
	float dx, dy;
	switch (direction) {
	case back:
		dy = (double)(cos(currentRot * PI / 180.0) * speed);
		dx = (double)(sin(currentRot * PI / 180.0) * speed);
		currentPos[0] += dx;
		currentPos[2] += dy;
		break;
	case forward:
		dy = (double)(cos(currentRot * PI / 180.0) * speed);
		dx = (double)(sin(currentRot * PI / 180.0) * speed);
		currentPos[0] -= dx;
		currentPos[2] -= dy;
		break;
	case left:
		currentRot += rotInc;
		if (currentRot > 360) 
		{
			currentRot = 0;
		}
		break;
	case right:
		currentRot -= rotInc; 
		if (currentRot < 0)
		{
			currentRot = 360;
		}
		break;
	}
}

void checkMovements() {
	if (move_forward) moveCar(forward);
	else if (move_back) moveCar(back);
	if (rot_left) moveCar(left);
	else if (rot_right) moveCar(right);

}

void sendMaterial(int index) {
	GLint loc;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[index].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[index].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[index].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[index].mat.shininess);
}

void drawObj(int index) {
	glBindVertexArray(mesh[index].vao);
	glDrawElements(mesh[index].type, mesh[index].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void sendMatrices() {
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);
}

//called when directional light is toggled on/off, sends the new value to the shader
void sendDirectionalLightToggle() 
{
	if (isDirLightOn)
		glUniform1i(directionalLightOn_uniformId, 0);
	else
		glUniform1i(directionalLightOn_uniformId, 1);
	isDirLightOn = !isDirLightOn;
}

void renderScene(void) {


	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt
	//lookAt(camX, camY, camZ, 0, 0, 0, 0, 1, 0);
	if (cam->activeCam == cam->MOVINGPERSPECTIVE)
		cam->sendCamCoords(camX, camY, camZ);
	cam->updateLookAt(currentPos);
	// use our shader
	glUseProgram(shader.getProgramIndex());

	//send the light position in eye coordinates

		//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 

	float res[4];
	multMatrixPoint(VIEW, lightPos, res);   //lightPos definido em World Coord so is converted to eye space
	glUniform4fv(lPos_uniformId, 1, res);

	checkMovements();

	//draw car body
	sendMaterial(0);
	pushMatrix(MODEL);
	translate(MODEL, currentPos[0] , currentPos[1], currentPos[2]);
	rotate(MODEL, currentRot, currentRotAxis[0], currentRotAxis[1], currentRotAxis[2]);
	//rotate(MODEL,90,0,90,0);
	sendMatrices();
	drawObj(0);

	//draw car wheels
	int x=0, y=0;
	for (int i = 4; i >= 1; i--) {

		pushMatrix(MODEL);
		sendMaterial(i);
		translate(MODEL, xpos[x], 0.0f, ypos[y]);
		rotate(MODEL, 90,0,0,1);
		sendMatrices();
		drawObj(i);

		popMatrix(MODEL);

		x++;
		y++;

	}
	//draw table
	popMatrix(MODEL);
	sendMaterial(5);

	//translate(MODEL, 2.0f, -0.1f, 2.0f);
	rotate(MODEL,-90,1,0,0);
	scale(MODEL, 10, 7, 1);
	sendMatrices();
	drawObj(5);
	glutSwapBuffers();
}



// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	switch (key) {

	case 27:
		glutLeaveMainLoop();
		break;

	case 'c':
		printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r);
		break;
	case 'm': glEnable(GL_MULTISAMPLE); break;
	//case 'n': glDisable(GL_MULTISAMPLE); break;
	case 'a':
		move_forward = false;
		move_back = true;
		break;
	case 'q':
		move_forward = true; 
		move_back = false;
		break;
	case 'o': 
		rot_left = true; 
		rot_right = false; 
		break;
	case 'p': 
		rot_left = false; 
		rot_right = true; 
		break; 
	case '1': 
		cam->setFixedOrtho();
		cam->setCameraType(cam->FIXEDORTHO);
		break;
	case '2': 
		cam->setFixedPerspective(pratio);
		cam->setCameraType(cam->FIXEDPERSPECTIVE); 
		break;
	case '3': 
		cam->setMovingPerspective(pratio);
		cam->setCameraType(cam->MOVINGPERSPECTIVE); 
		break;
	case 'n':
		sendDirectionalLightToggle();
		break;
	}

}

void processKeysUp(unsigned char key, int xx, int yy)
{
	switch (key) {

	case 'a':
		move_back = false;
		break;
	case 'q':
		move_forward = false;
		break;
	case 'o':
		rot_left = false;
		break;
	case 'p':
		rot_right = false;
		break;
	}

}


// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN) {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
			if (r < 0.1f)
				r = 0.1f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX = -xx + startX;
	deltaY = yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {
		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}

	camX = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camZ = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camY = rAux *   						       sin(betaAux * 3.14f / 180.0f);

	//  uncomment this if not using an idle func
		glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r *   						     sin(beta * 3.14f / 180.0f);

	//  uncomment this if not using an idle func
	//	glutPostRedisplay();
}

// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for models
	shader.init();
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0, "colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");
	directionalLightOn_uniformId = glGetUniformLocation(shader.getProgramIndex(), "directionalLightOn");

	glUniform1i(directionalLightOn_uniformId, true);

	printf("InfoLog for Hello World Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	return(shader.isProgramLinked());
}

void createCar() {

	float amb[] = { 0.3f, 0.0f, 0.0f, 1.0f };
	float diff[] = { 0.5f, 0.0f, 0.0f, 1.0f };
	float spec[] = { 0.7f, 0.6f, 0.6f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 32.0f;
	int texcount = 0;

	objId = 0;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createQuad(1,1);
	//createCube();

	//WHEELS//	
	objId = 1;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createTorus(0.1, 0.2, 15, 5);

	objId = 2;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createTorus(0.1, 0.2, 15, 5);

	objId = 3;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createTorus(0.1, 0.2, 15, 5);

	objId = 4;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createTorus(0.1, 0.2, 15, 5);
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

void init()
{
	// set the camera position based on its spherical coordinates
	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r *   						     sin(beta * 3.14f / 180.0f);


	float amb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	float diff[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float spec[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 32.0f;
	int texcount = 0;
	
	createCar();

	//table
	objId=5;
	memcpy(mesh[objId].mat.ambient, amb,4*sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff,4*sizeof(float));
	memcpy(mesh[objId].mat.specular, spec,4*sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive,4*sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createQuad(10,10);


	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char **argv) {

	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);

	//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	//glutIdleFunc(renderScene);

//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutKeyboardUpFunc(processKeysUp);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc(mouseWheel);
	glutTimerFunc(0, timer, 0);
	glutTimerFunc(0, refresh, 0);
	glutTimerFunc(0, Timer, 0);

	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);
}

