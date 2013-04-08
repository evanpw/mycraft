#version 330 core

uniform sampler2D textureSampler;

in vec2 uv;
in vec3 color;

out vec4 fragColor;

void main()
{
	//fragColor = mix(vec4(color, 1.0), texture(textureSampler, uv), 0.5);
	fragColor = vec4(vec3(texture(textureSampler, uv)), 1.0);
	//fragColor = vec4(color, 0.5);
}