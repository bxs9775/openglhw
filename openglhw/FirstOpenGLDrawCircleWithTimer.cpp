// This example is from Angel's 6th edition graphics book.
// It kind of cheats by using an old shader style so that
// all the matrices don't need to be defined up front.

#include "stdafx.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"

#include "Camera.h"
#include "iostream"

#pragma comment ( lib, "opengl32.lib" )
#pragma comment ( lib, "lib/glew32.lib" )
#pragma comment ( lib, "lib/glfw3.lib" )

/// Define a helpful macro for handling offsets into buffer objects
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

#pragma region fields
// global variables

// prototype function as it's in a separate file
GLuint InitShader(const char* vShaderFile, const char* fShaderFile);

//constants
const GLuint vec4_size = sizeof(glm::vec4);
const GLuint GLint_size = sizeof(GLuint);
const GLuint GLfloat_size = sizeof(GLfloat);
const GLuint mat4_size = sizeof(glm::mat4);
//Number of elements in a single vertex
const GLuint vertexElements = 6;
const GLfloat PI = 3.14159265359;

/// vector 2 structure
struct vec2 {
	GLfloat  x;
	GLfloat  y;
	//  --- Constructors and Destructors ---
	vec2(GLfloat s = GLfloat(0.0)) :
		x(s), y(s) {}
	vec2(GLfloat x, GLfloat y) :
		x(x), y(y) {}
	vec2(const vec2& v) {
		x = v.x;  y = v.y;
	}
	//  --- Indexing Operator ---
	GLfloat& operator [] (int i) { return *(&x + i); }
	const GLfloat operator [] (int i) const { return *(&x + i); }
};

/// vector 3 structure
struct vec3 {
	GLfloat  x;
	GLfloat  y;
	GLfloat  z;
	//  --- Constructors and Destructors ---
	vec3(GLfloat s = GLfloat(0.0)) :
		x(s), y(s), z(s) {}
	vec3(GLfloat x, GLfloat y, GLfloat z) :
		x(x), y(y), z(z) {}
	vec3(const vec3& v) {
		x = v.x;  y = v.y; z = v.z;
	}
	//  --- Indexing Operator ---
	GLfloat& operator [] (int i) { return *(&x + i); }
	const GLfloat operator [] (int i) const { return *(&x + i); }
};

///Stores all the data needed to be kept with a VAO
struct model {
	GLuint vao;
	GLuint numVerts;
	GLfloat *vertices;
	GLuint numInds;
	GLuint *indices;

	GLuint numObjects;
	glm::mat4 *transforms;

	GLuint matVBO;

	model() :model(0, 0, 0) {

	}

	model(GLuint verts, GLuint inds, GLuint numObjs) {
		numVerts = verts;
		vertices = new GLfloat[numVerts*vertexElements];
		numInds = inds;
		indices = new GLuint[numInds];

		numObjects = numObjs;
		transforms = new glm::mat4[numObjects];
	}

	~model() {
		delete vertices;
		delete indices;
		delete transforms;
	}
};

///Provides the transform details of an individual instance of a model.
struct instanceInfo {
	glm::vec3 scale;
	glm::vec3 rotationAxis;
	GLfloat rotationAngle;
	glm::vec3 translation;

	glm::quat rotateQuat;

	instanceInfo() {
		scale = glm::vec3(1.0f, 1.0f, 1.0f);
		rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
		rotationAngle = 0;
		translation = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	instanceInfo(glm::vec3 scaleVec, glm::vec3 rotAxis, float angle, glm::vec3 transVec) {
		scale = scaleVec;
		rotationAxis = rotAxis;
		rotationAngle = angle;
		translation = transVec;
	}
};

const int NUM = 100;
const GLfloat RADIUS = 0.5;
GLuint vao;
GLuint vertBuffer;
GLuint indBuffer;
GLuint program;

GLuint transformLoc;
GLuint colorLocation;
vec3 color;

GLFWwindow* window;

double timeSinceColorChange = 0;

//transform info
glm::vec3 scale;
glm::vec3 rotationAxis;
float rotationAngle;
glm::vec3 translation;

glm::quat rotateQuat;

//Model specific matrixes
glm::mat4x4 identityMat = glm::mat4(1.0f);
glm::mat4 scaleMat;
glm::mat4 rotateMat;
glm::mat4 translateMat;

//world and model transforms
Camera camera = NULL;

glm::mat4 modelMatrix;
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 worldMatrix;

//Models
model* cube;
model* pyramid;
model* octahedron;

//Player
instanceInfo ship = instanceInfo(glm::vec3(0.5, 0.5, 0.7), glm::vec3(0, 0, 1.0), 0, glm::vec3(0, 0, 0));
#pragma endregion

///Creates a model matrix from the date of a given instance of a model
glm::mat4 getModelMatrix(instanceInfo instance) {
	
	glm::mat4 modelMat = glm::mat4();

	///Scale
	scaleMat = glm::scale(identityMat, instance.scale);

	///Rotate
	instance.rotateQuat = glm::angleAxis(instance.rotationAngle, glm::normalize(instance.rotationAxis));
	rotateMat = instance.rotateQuat.operator glm::tmat4x4<float, glm::packed_highp>();

	///Transform
	translateMat = glm::translate(identityMat, instance.translation);

	///Final matrix
	modelMat = translateMat*rotateMat*scaleMat;
	return modelMat;
}

#pragma region init
///Initializes the instanced arrays for a model
void initModelVAO(model *m) {
	// Create a vertex array object
	glGenVertexArrays(1, &(m->vao));
	glBindVertexArray(m->vao);

	GLuint vBuffer;
	// Create and initialize a buffer object
	glGenBuffers(1, &vBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
	glBufferData(GL_ARRAY_BUFFER, (GLfloat_size*m->numVerts*vertexElements), (void*)m->vertices, GL_STATIC_DRAW);
	GLuint iBuffer;
	glGenBuffers(1, &iBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLint_size*m->numInds), (void*)m->indices, GL_STATIC_DRAW);
	
	/**/
	const char* vShader = "vshader.glsl";
	const char* fShader = "fshader.glsl";
	GLuint program = InitShader(vShader, fShader);
	glUseProgram(program);

	// Initialize the vertex position attribute from the vertex shader
	GLuint loc = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, vertexElements * GLfloat_size, (GLvoid*)0);
	GLuint colorIndex = glGetAttribLocation(program, "color");
	glEnableVertexAttribArray(colorIndex);
	glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, vertexElements * GLfloat_size, (GLvoid*)(3 * GLfloat_size));

	//GLuint matVBO;
	glGenBuffers(1, &(m->matVBO));
	glBindBuffer(GL_ARRAY_BUFFER, m->matVBO);
	glBufferData(GL_ARRAY_BUFFER, m->numObjects * mat4_size, &(m->transforms[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint localTransLoc = glGetAttribLocation(program, "localTransform");
	glBindBuffer(GL_ARRAY_BUFFER, m->matVBO);
	glEnableVertexAttribArray(localTransLoc);
	glVertexAttribPointer(localTransLoc, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (GLvoid*)0);
	glEnableVertexAttribArray(localTransLoc + 1);
	glVertexAttribPointer(localTransLoc + 1, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (GLvoid*)(vec4_size));
	glEnableVertexAttribArray(localTransLoc + 2);
	glVertexAttribPointer(localTransLoc + 2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (GLvoid*)(2 * vec4_size));
	glEnableVertexAttribArray(localTransLoc + 3);
	glVertexAttribPointer(localTransLoc + 3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (GLvoid*)(3 * vec4_size));

	glVertexAttribDivisor(localTransLoc, 1);
	glVertexAttribDivisor(localTransLoc + 1, 1);
	glVertexAttribDivisor(localTransLoc + 2, 1);
	glVertexAttribDivisor(localTransLoc + 3, 1);


	//glm transform
	camera.computeMatricesFromInputs();
	projectionMatrix = camera.getProjectionMatrix();
	viewMatrix = camera.getViewMatrix();
	worldMatrix = projectionMatrix*viewMatrix;

	transformLoc = glGetUniformLocation(program, "worldTransform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(worldMatrix));

	glUseProgram(program);

	// Unbind the buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// initialize all graphics
void init(void)
{
	glEnable(GL_DEPTH_TEST);

	//starting color vector
	color.x = 1.0;
	color.y = 1.0;
	color.z = 1.0;
	
	//Creating models

	//Box
	GLfloat cubeVertices[] = {
		// Positions          // Colors           
		0.5f,  0.5f, 0.5f,    1.0f, 0.0f, 0.0f,   // Front   Top      Right
		0.5f, -0.5f, 0.5f,    1.0f, 0.0f, 1.0f,   // Front   Bottom   Right
		-0.5f, -0.5f, 0.5f,   1.0f, 0.0f, 0.0f,  // Front   Bottom   Left
		-0.5f,  0.5f, 0.5f,   1.0f, 0.0f, 1.0f,  // Front   Top      Left
		0.5f,  0.5f, -0.5f,    1.0f, 0.0f, 0.0f, // Back    Top      Right
		0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 1.0f, // Back    Bottom   Right
		-0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f, // Back    Bottom   Left
		-0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 1.0f, // Back    Top      Left 
	};
	GLuint cubeIndices[] = {  // Note that we start from 0!
		0, 1, 3,  // Front	  Triangle 1
		1, 2, 3,  // Front	  Triangle 2
		4, 5, 1,  // Right	  Triangle 1
		0, 4, 1,  // Right	  Triangle 2
		4, 5, 6,  // Back	  Triangle 1
		7, 4, 6,  // Back	  Triangle 2
		4, 0, 3,  // Top	  Triangle 1
		7, 4, 3,  // Top	  Triangle 2
		3, 7, 2,  // Left	  Triangle 1
		7, 6, 2,  // Left	  Triangle 2
		5, 6, 1,  // Bottom	  Triangle 1
		2, 6, 1   // Bottom	  Triangle 2
	};

	//instances
	int index = 0;
	instanceInfo cubeInstances[] = {
		instanceInfo(glm::vec3(0.9, 0.9, 0.9), glm::vec3(0, 0, 1.0), 0, glm::vec3(3,1,4)),
		instanceInfo(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0, 1.0, 0), 3, glm::vec3(-1,5,9)),
		instanceInfo(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0, 0, 1.0), 5, glm::vec3(2,-6,5)),
		instanceInfo(glm::vec3(0.9, 0.9, 0.9), glm::vec3(1.0, 0, 0), 4, glm::vec3(3,5,-8)),
		instanceInfo(glm::vec3(1.4, 1.4, 1.4), glm::vec3(1.0, 1.0, 0), 2, glm::vec3(-9,-7,9)),
		instanceInfo(glm::vec3(1.0, 1.0, 1.0), glm::vec3(1.0, 0, 1.0), 7, glm::vec3(-3,2,-3)),
		instanceInfo(glm::vec3(0.5, 0.5, 0.5), glm::vec3(0, 0, 1.0), 1.5, glm::vec3(8,-4,-6)),
		instanceInfo(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0, 1.0, 1.0), 9, glm::vec3(-2,-6,-4)),
		instanceInfo(glm::vec3(1.0, 1.0, 1.0), glm::vec3(1.0, 1.0, 1.0), 1, glm::vec3(3,3,8))
	};
	glm::mat4 cubeTransforms[] = {
		getModelMatrix(cubeInstances[0]),
		getModelMatrix(cubeInstances[1]),
		getModelMatrix(cubeInstances[2]),
		getModelMatrix(cubeInstances[3]),
		getModelMatrix(cubeInstances[4]),
		getModelMatrix(cubeInstances[5]),
		getModelMatrix(cubeInstances[6]),
		getModelMatrix(cubeInstances[7]),
		getModelMatrix(cubeInstances[8])
	};

	cube = new model(8, 36, 9);
	memcpy_s(cube->vertices, cube->numVerts*GLint_size*vertexElements, (void*)cubeVertices, cube->numVerts*GLint_size*vertexElements);
	memcpy_s(cube->indices, cube->numInds*GLfloat_size, (void*)cubeIndices, cube->numInds*GLfloat_size);
	memcpy_s(cube->transforms, cube->numObjects*mat4_size, (void*)cubeTransforms, cube->numObjects*mat4_size);
	
	//pyramid
	GLfloat pyramidVertices[] = {
		// Positions          // Colors           
		0.5f,  0.5f, 0.5f,    0.0f, 0.0f, 1.0f,   // Front   Top      Right
		0.5f, -0.5f, 0.5f,    0.0f, 0.0f, 1.0f,   // Front   Bottom   Right
		-0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   // Front   Bottom   Left
		-0.5f,  0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   // Front   Top      Left
		   0,     0, -0.5f,    0.0f, 1.0f, 1.0f,  // Back    Point
	};
	GLuint pyramidIndices[] = {  // Note that we start from 0!
		0, 1, 3,  // Front	  Triangle 1
		1, 2, 3,  // Front	  Triangle 2
		1, 0, 4,
		2, 1, 4,
		3, 2, 4,
		0, 3, 4
	};
	instanceInfo pyramidInstances[] = {
		ship,
		instanceInfo(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0, 0, 1.0), 2, glm::vec3(7,-1,8)),
		instanceInfo(glm::vec3(2.0, 2.0, 2.0), glm::vec3(1.0, 0, 0), 2, glm::vec3(8,2,-8)),
		instanceInfo(glm::vec3(0.7, 0.7, 0.7), glm::vec3(0, 1.0, 1.0), 2, glm::vec3(-8,4,6))
	};
	glm::mat4 pyramidTransforms[] = {
		getModelMatrix(pyramidInstances[0]),
		getModelMatrix(pyramidInstances[1]),
		getModelMatrix(pyramidInstances[2]),
		getModelMatrix(pyramidInstances[3])
	};

	pyramid = new model(5, 18, 4);
	memcpy_s(pyramid->vertices, pyramid->numVerts*GLint_size*vertexElements, (void*)pyramidVertices, pyramid->numVerts*GLint_size*vertexElements);
	memcpy_s(pyramid->indices, pyramid->numInds*GLfloat_size, (void*)pyramidIndices, pyramid->numInds*GLfloat_size);
	memcpy_s(pyramid->transforms, pyramid->numObjects*mat4_size, (void*)pyramidTransforms, pyramid->numObjects*mat4_size);

	//octahedron
	GLfloat octahedronVertices[] = {
		// Positions          // Colors           
		0.5f,  0.5f, 0,    0.0f, 1.0f, 0.0f,   // Top      Right
		0.5f, -0.5f, 0,    0.0f, 1.0f, 0.0f,   // Bottom   Right
		-0.5f, -0.5f, 0,   0.0f, 1.0f, 0.0f,   // Bottom   Left
		-0.5f,  0.5f, 0,   1.0f, 1.0f, 0.0f,   // Top      Left
		0,     0, -0.5f,    1.0f, 1.0f, 0,  // Back    Point
		0,     0, 0.5f,    1.0f, 1.0f, 0,  // Front   Point
	};
	GLuint octahedronIndices[] = { 
		1, 0, 4,
		2, 1, 4,
		3, 2, 4,
		0, 3, 4,
		1, 0, 5,
		2, 1, 5,
		3, 2, 5,
		0, 3, 5
	};
	instanceInfo octahedronInstances[] = {
		instanceInfo(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0, 0, 1.0), 0, glm::vec3(-1,-1,2)),
		instanceInfo(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0, 1.0, 0), 3, glm::vec3(-5,8,-1)),
		instanceInfo(glm::vec3(1.0, 1.3, 1.0), glm::vec3(1.0, 0, 0), 3, glm::vec3(2,-1,-3)),
		instanceInfo(glm::vec3(0.9, 0.7, 0.9), glm::vec3(0, 1.0, 1.0), 4, glm::vec3(-5,-5,-8)),
		instanceInfo(glm::vec3(1.0, 1.0, 1.0), glm::vec3(1.0, 0, 1.0), 9, glm::vec3(1,4,4))
	};
	glm::mat4 octahedronTransforms[] = {
		getModelMatrix(octahedronInstances[0]),
		getModelMatrix(octahedronInstances[1]),
		getModelMatrix(octahedronInstances[2]),
		getModelMatrix(octahedronInstances[3]),
		getModelMatrix(octahedronInstances[4])
	};

	octahedron = new model(6, 24, 5);
	memcpy_s(octahedron->vertices, octahedron->numVerts*GLint_size*vertexElements, (void*)octahedronVertices, octahedron->numVerts*GLint_size*vertexElements);
	memcpy_s(octahedron->indices, octahedron->numInds*GLfloat_size, (void*)octahedronIndices, octahedron->numInds*GLfloat_size);
	memcpy_s(octahedron->transforms, octahedron->numObjects*mat4_size, (void*)octahedronTransforms, octahedron->numObjects*mat4_size);

	//Initializeing models
	initModelVAO(cube);
	initModelVAO(pyramid);
	initModelVAO(octahedron);

	srand(glfwGetTime());

}
#pragma endregion

#pragma region Update/Draw
// From the following website:
// http://r3dux.org/2012/07/a-simple-glfw-fps-counter/
double calcFPS(double timeInterval = 1.0, std::string windowTitle = "NONE")
{
	// Static values which only get initialised the first time the function runs
	static double startTime = glfwGetTime(); // Set the initial time to now
	static double fps = 0.0;           // Set the initial FPS value to 0.0

									   // Set the initial frame count to -1.0 (it gets set to 0.0 on the next line). Because
									   // we don't have a start time we simply cannot get an accurate FPS value on our very
									   // first read if the time interval is zero, so we'll settle for an FPS value of zero instead.
	static double frameCount = -1.0;

	// Here again? Increment the frame count
	frameCount++;

	// Ensure the time interval between FPS checks is sane (low cap = 0.0 i.e. every frame, high cap = 10.0s)
	if (timeInterval < 0.0)
	{
		timeInterval = 0.0;
	}
	else if (timeInterval > 10.0)
	{
		timeInterval = 10.0;
	}

	// Get the duration in seconds since the last FPS reporting interval elapsed
	// as the current time minus the interval start time
	double duration = glfwGetTime() - startTime;

	// If the time interval has elapsed...
	if (duration > timeInterval)
	{
		// Calculate the FPS as the number of frames divided by the duration in seconds
		fps = frameCount / duration;

		// If the user specified a window title to append the FPS value to...
		if (windowTitle != "NONE")
		{
			// Convert the fps value into a string using an output stringstream
			std::ostringstream stream;
			stream << fps;
			std::string fpsString = stream.str();

			// Append the FPS value to the window title details
			windowTitle += " | FPS: " + fpsString;

			// Convert the new window title to a c_str and set it
			const char* pszConstString = windowTitle.c_str();
			glfwSetWindowTitle(window, pszConstString);
		}
		else // If the user didn't specify a window to append the FPS to then output the FPS to the console
		{
			std::cout << "FPS: " << fps << std::endl;
		}

		// Reset the frame count to zero and set the initial time to be now
		frameCount = 0.0;
		startTime = glfwGetTime();
	}

	// Return the current FPS - doesn't have to be used if you don't want it!
	return fps;
}

///Updates the ship's transform and sends it to the graphics screen
void updateShip() {
	ship.translation = camera.getPosition() + (camera.getForward()*(GLfloat)2);

	///Scale
	scaleMat = glm::scale(identityMat, ship.scale);

	///Rotate
	static glm::vec3 upVector = glm::vec3(0, 1.0f, 0);
	static glm::vec3 rightVector = glm::vec3(1.0f, 0, 0);

	float horizontalAngle = camera.getHorizontalAngle()+PI;
	float verticalAngle = camera.getVerticalAngle();

	glm::mat4 rotate1;
	ship.rotateQuat = glm::angleAxis(horizontalAngle, glm::normalize(upVector));
	rotate1 = ship.rotateQuat.operator glm::tmat4x4<float, glm::packed_highp>();


	glm::mat4 rotate2;
	ship.rotateQuat = glm::angleAxis(verticalAngle, glm::normalize(glm::vec3(rotate1*glm::vec4(rightVector,1))));
	rotate2 = ship.rotateQuat.operator glm::tmat4x4<float, glm::packed_highp>();

	rotateMat = rotate2*rotate1;

	///Transform
	translateMat = glm::translate(identityMat, ship.translation);

	///Final matrix
	glm::mat4 transformMat = translateMat*rotateMat*scaleMat;

	glBindVertexArray(pyramid->vao);
	glBindBuffer(GL_ARRAY_BUFFER, pyramid->matVBO);

	glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * vec4_size, &transformMat);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

///Handles per model drawing with glDraw elements instanced
void drawModel(model* m) {
	glBindVertexArray(m->vao);

	glDrawElementsInstanced(GL_TRIANGLES, m->numInds, GL_UNSIGNED_INT, 0, m->numObjects);

	glBindVertexArray(0);
}

//Clears the screen and draws all the models
void display(void)
{
	// Clear the buffer  - black screen
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	drawModel(cube);
	drawModel(pyramid);
	drawModel(octahedron);
}
#pragma endregion

///Handles the main game code and update loop
int main(void)
{
	// Initialize the library 
	if (!glfwInit())
		return -1;

	// Create a windowed mode window and its OpenGL context 
	window = glfwCreateWindow(640, 480, "OpenGL Final Assignment", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// Make the window's context current 
	glfwMakeContextCurrent(window);

	// Init GLEW 
	glewInit();

	camera = Camera(window);

	// Set up the vert's to draw
	init();

	// Loop until the user closes the window 
	while (!glfwWindowShouldClose(window))
	{
		camera.computeMatricesFromInputs();
		projectionMatrix = camera.getProjectionMatrix();
		viewMatrix = camera.getViewMatrix();
		worldMatrix = projectionMatrix*viewMatrix;
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(worldMatrix));
		
		updateShip();

		// Render here
		display();
		double fps = calcFPS(3.0, "Game World: ");
		
		glFlush();

		// Swap front and back buffers 
		glfwSwapBuffers(window);

		// Poll for and process events 
		glfwPollEvents();
	}

	free(cube);
	free(pyramid);
	free(octahedron);

	glfwTerminate();
	return 0;
}