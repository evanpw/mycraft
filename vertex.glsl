#version 330 core

uniform mat4 mvpMatrix;
in vec3 position;
out vec3 color;

void main()
{
	color = vec3(position.x * 0.5 + 0.5, position.y * 0.5 + 0.5, position.z * 0.5 + 0.5);
	gl_Position = mvpMatrix * vec4(position, 1.0);
}

