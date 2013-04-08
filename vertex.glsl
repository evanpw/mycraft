#version 330 core

uniform mat4 mvpMatrix;
uniform uint cubeType;

in vec3 position;
//in vec2 vertexUv;

out vec3 color;
out vec2 uv;

void main()
{
	if (cubeType == 0u)
	{
		color = vec3(0.0, position.y, 0.0);
	}
	else if (cubeType == 1u)
	{
		color = vec3(position.y, position.y, position.y);
	}

	gl_Position = mvpMatrix * vec4(position, 1.0);

	//uv = vertexUv;
	uv = position.xy;
	if (position.z == 0.0) uv.x = (1 - uv.x);
}

