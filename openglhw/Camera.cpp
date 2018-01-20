#include "stdafx.h"
#include "Camera.h"


Camera::Camera(GLFWwindow* window)
{
	this->window = window;

	position = glm::vec3(0, 0, 5);
	horizontalAngle = 3.14f;
	verticalAngle = 0.0f;
	initialFoV = 45.0f;
	FoV = 0;
	moveSpeed = 6.0f;
	mouseSpeed = 0.01f;
	deltaTime = 0;
}


Camera::~Camera()
{
}

#pragma region Get Methods
///Gets the camera position
glm::vec3 Camera::getPosition()
{
	return position;
}

///Gets the horizontal angle of the camera
float Camera::getHorizontalAngle()
{
	return horizontalAngle;
}

///Gets the vertical angle of the camera
float Camera::getVerticalAngle()
{
	return verticalAngle;
}

///Gets the camera's forward vector
glm::vec3 Camera::getForward()
{
	return forward;
}

///Gets the projection matrix for the camera
glm::mat4 Camera::getProjectionMatrix()
{
	return projectionMat;
}

///Gets the view matrix for the camera
glm::mat4 Camera::getViewMatrix()
{
	return viewMat;
}

#pragma endregion

///Uses mouse and keyboard inputs to calculate the position, orientation, view matrix and projection matrix for the camera. Thses values can be retrieved using the associated getter methods.
///The program accepts both the arrow keys and WASD for camera movement. ESC exits the program.
void Camera::computeMatricesFromInputs()
{
	///////////Camera rotation///////////
	
	//Get mouse coordibnates and reset position
	static double lastTime = glfwGetTime();
	deltaTime = float(glfwGetTime() - lastTime);
	
	double xpos, ypos;
	glfwGetCursorPos(window,&xpos, &ypos);

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glfwSetCursorPos(window,width / 2, height / 2);

	//Calculate orientations
	horizontalAngle += mouseSpeed * deltaTime * float(float(width) / 2 - xpos);
	verticalAngle += mouseSpeed * deltaTime * float(float(height) / 2 - ypos);

	//Spherical angles to Cartesian coordinates
	glm::vec3 direction = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	forward = glm::normalize(direction);

	//Right vector
	right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	//Up vector
	up = glm::cross(right, direction);

	///////////Camera translation///////////
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += direction * deltaTime * moveSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= direction * deltaTime * moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * deltaTime * moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * deltaTime * moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	//FoV = initialFoV;
	projectionMat = glm::perspective(initialFoV, 4.0f / 3.0f, 0.1f, 100.0f);
	viewMat = glm::lookAt(position, position + direction, up);

	lastTime = glfwGetTime();
}
