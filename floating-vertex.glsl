#version 150

uniform mat4 projection;

in vec3 position;
in vec3 texCoord;

out vec3 fragTexCoord;

void main()
{
	gl_Position = projection * vec4(position, 1.0);
	gl_Position.xy += vec2(0.75) * gl_Position.w;

	fragTexCoord = texCoord;
}