#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in mat4 localTransform;

out vec3 fragmentColor;
uniform mat4 worldTransform;
void main()
{
	gl_Position = worldTransform * localTransform * vec4(position, 1.0);
	fragmentColor = color;
}