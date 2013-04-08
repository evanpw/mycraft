#version 330 core

uniform mat4 mvpMatrix;

in vec3 position;
in vec2 vertexUv;

out vec3 color;
out vec2 uv;

void main()
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	uv = vertexUv;
}

