#version 330 core

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;

in vec3 position;
in vec2 vertexUv;
in vec3 normal;

out float shading;
out vec2 uv;
out vec3 originalPosition;

void main()
{
	// Pass to the fragment shader
	originalPosition = position;

	vec3 n = normalize((modelMatrix * vec4(normal, 0.0)).xyz);
	vec3 l = normalize(vec3(-4.0, 2.0, 1.0));
	float diffuse = clamp(dot(n, l), 0.0, 1.0);
	float ambient = 0.2;
	shading = clamp(diffuse + ambient, 0.0, 1.0);

	gl_Position = mvpMatrix * vec4(position, 1.0);
	uv = vertexUv;
}

