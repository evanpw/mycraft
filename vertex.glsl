#version 330 core

in vec3 position;

void main()
{
	gl_Position.xyz = 0.5 * position;
    gl_Position.w = 1.0;
}

