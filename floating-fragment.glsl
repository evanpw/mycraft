#version 150
#extension GL_ARB_texture_cube_map_array : enable

uniform samplerCubeArray textureSampler;
uniform int blockType;

in vec3 fragTexCoord;

out vec4 fragColor;

void main()
{
	vec4 stpq = vec4(fragTexCoord, blockType);
	fragColor = texture(textureSampler, stpq);
	fragColor.a = 0.85;
}