#version 150

uniform sampler2DArray textureSampler;
uniform int blockType;

in vec3 fragTexCoord;

out vec4 fragColor;

void main()
{
    vec3 stp = fragTexCoord;
    stp.p = blockType * 6 + fragTexCoord.p;
    
	fragColor = texture(textureSampler, stp);
	fragColor.a = 0.85;
}