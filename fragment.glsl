#version 330 core

in vec3 color;

void main()
{
	gl_FragColor = vec4(color, 1.0);
	//gl_FragColor = vec4(1, 0, 0, 1);
	//gl_FragColor = vec4(mod(gl_FragCoord.x, 50) / 50.0, mod(gl_FragCoord.y, 50) / 5.0, 0.0, 1.0);
}