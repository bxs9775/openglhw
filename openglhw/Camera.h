#pragma once
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

#pragma comment ( lib, "opengl32.lib" )
#pragma comment ( lib, "lib/glew32.lib" )
#pragma comment ( lib, "lib/glfw3.lib" )

class Camera
{
public:
	Camera(GLFWwindow* window);
	~Camera();

	void computeMatricesFromInputs();
	
	glm::vec3 getPosition();
	float getHorizontalAngle();
	float getVerticalAngle();

	glm::vec3 getForward();
	
	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix();
private:
	glm::mat4 projectionMat;
	glm::mat4 viewMat;

	GLFWwindow* window;

	//Camera values (based on tutorial
	glm::vec3 position;
	//Horisontal position of the camera
	float horizontalAngle;
	//Vertical position of the camera
	float verticalAngle;
	//The initial field of view
	float initialFoV;
	float FoV;

	float moveSpeed;
	float mouseSpeed;

	float deltaTime;

	//Direction vectors
	glm::vec3 forward;
	glm::vec3 up;
	glm::vec3 right;
};

