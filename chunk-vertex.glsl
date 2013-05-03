#version 400 core

uniform mat4 vpMatrix;
uniform vec3 sunPosition;
uniform float brightness;

in vec3 position;
in vec4 texCoord;
in vec3 normal;

out float shading;
out float fogFactor;
out vec4 fragTexCoord;

void main()
{
	fragTexCoord = texCoord;

	vec3 n = normalize(normal);
	vec3 l = normalize(sunPosition);
	float diffuse = clamp(0.7 * dot(n, l), 0.0, 1.0);
	diffuse *= brightness;

	float ambient = 0.3;
	shading = clamp(diffuse + ambient, 0.0, 1.0);

	gl_Position = vpMatrix * vec4(position, 1.0);
	fogFactor = clamp((length(gl_Position) - 90.0) / 90.0, 0.0, 1.0);
}

