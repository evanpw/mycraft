#version 330 core

uniform sampler2D textureSampler;

in vec2 uv;
in float shading;
in vec3 originalPosition;

out vec4 fragColor;

void main()
{
	fragColor = vec4(shading * vec3(texture(textureSampler, uv)), 1.0);

	//fragColor = vec4(color, 1.0) * texture(textureSampler, uv);
	//fragcolor = mix(vec4(color, 1.0), texture(textureSampler, uv), 0.5);
	//fragColor = vec4(color, 1.0);
}