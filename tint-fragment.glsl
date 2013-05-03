#version 130

uniform vec3 color;
out vec4 fragColor;

void main()
{
	fragColor = vec4(color, 0.2);
}