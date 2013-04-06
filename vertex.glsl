#version 330 core

uniform mat4 mvpMatrix;
uniform uint cubeType;

in vec3 position;

out vec3 color;

void main()
{
	if (cubeType == 0u)
	{
		color = vec3(0.0, 0.7, 0.0);
	}
	else if (cubeType == 1u)
	{
		color = vec3(0.7, 0.7, 0.7);
	}

	gl_Position = mvpMatrix * vec4(position, 1.0);
}

