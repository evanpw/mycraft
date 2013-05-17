#version 400 core

uniform mat4 vpMatrix;
uniform vec3 sunPosition;
uniform float brightness;

in vec3 position;
in vec4 texCoord;
in float lighting;

out float fragLighting;
out float fogFactor;
out vec4 fragTexCoord;

void main()
{
	fragTexCoord = texCoord;
	fragLighting = lighting;

	gl_Position = vpMatrix * vec4(position, 1.0);
	fogFactor = clamp((length(gl_Position) - 90.0) / 90.0, 0.0, 1.0);
}

