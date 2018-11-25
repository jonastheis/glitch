#version 300 es
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0

uniform sampler2D dataTexture;

void main()
{
	vec4 val;

	val += texture(dataTexture, vec2(-1, -1));
	val += texture(dataTexture, vec2(0, 0));
	val += texture(dataTexture, vec2(1, 1));

    gl_Position = val;
}