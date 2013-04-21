#version 330 core

uniform samplerCube textureSampler;
uniform bool highlight;
uniform vec2 resolution;

in float fogFactor;
in float shading;
in vec3 fragTexCoord;
in vec4 gl_FragCoord;
in vec3 modelCoordinates;

out vec4 fragColor;

void main()
{
	fragColor = vec4(vec3(shading), 1.0) * texture(textureSampler, modelCoordinates - vec3(0.5));
	fragColor = mix(fragColor, vec4(0.8f, 0.8f, 1.0f, 1.0f), fogFactor);

	if (highlight)
		fragColor = mix(fragColor, vec4(1.0, 0.0, 0.0, 1.0), 0.1);

	vec2 screenCoord = gl_FragCoord.xy - (resolution / 2.0);

	if ((abs(screenCoord.x) < 1 && abs(screenCoord.y) < 7) ||
	 	(abs(screenCoord.y) < 1 && abs(screenCoord.x) < 7))
	{
		fragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
}