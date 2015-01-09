#version 150

uniform sampler2DArray textureSampler;
//uniform bool highlight;
uniform vec2 resolution;
uniform float brightness;

in float fogFactor;
in float fragLighting;
in vec3 fragTexCoord;
in vec4 gl_FragCoord;

out vec4 fragColor;

void main()
{
	fragColor = vec4(vec3(brightness * fragLighting), 1.0) * texture(textureSampler, fragTexCoord);

	vec3 skyColor = brightness * vec3(0.6f, 0.6f, 1.0f);
	fragColor = mix(fragColor, vec4(skyColor, 1.0), fogFactor);

	/*
	if (highlight)
		fragColor = mix(fragColor, vec4(1.0, 0.0, 0.0, 1.0), 0.1);
	*/

	vec2 screenCoord = gl_FragCoord.xy - (resolution / 2.0);

	if ((abs(screenCoord.x) < 1 && abs(screenCoord.y) < 7) ||
	 	(abs(screenCoord.y) < 1 && abs(screenCoord.x) < 7))
	{
		fragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
}