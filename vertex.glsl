#version 330 core

uniform mat4 vpMatrix;

in vec3 position;
in vec3 texCoord;
in vec3 normal;

out float shading;
out float fogFactor;
out vec3 fragTexCoord;

void main()
{
	fragTexCoord = texCoord;

	vec3 n = normalize(normal);
	vec3 l = normalize(vec3(-4.0, 2.0, 1.0));
	float diffuse = clamp(dot(n, l), 0.0, 1.0);
	float ambient = 0.2;
	shading = clamp(diffuse + ambient, 0.0, 1.0);

	gl_Position = vpMatrix * vec4(position, 1.0);
	fogFactor = clamp((length(gl_Position) - 100.0) / 128.0, 0.0, 1.0);
}

