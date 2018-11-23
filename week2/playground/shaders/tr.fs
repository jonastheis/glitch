#version 300 es
out uvec4 FragColor;

uniform usampler2D dataTexture;

void main(){
	FragColor = uvec4(texelFetch(dataTexture, ivec2(0,0), 0));
}