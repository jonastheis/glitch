#version 300 es

#define COLOR_MAX 255.0

out vec4 FragColor;

uniform sampler2D HTex00;
uniform sampler2D HTex01;
uniform sampler2D HTex02;
uniform sampler2D HTex03;
uniform sampler2D HTex04;
uniform sampler2D HTex05;
uniform sampler2D HTex06;
uniform sampler2D HTex07;
uniform sampler2D HTex08;
// bind dummy texture last to prevent last texture error - do not use
uniform sampler2D DTex09;

float normalize(int t) {
	return float(t)/COLOR_MAX;
}

float normalize(float t) {
	return t/COLOR_MAX;
}


void main(){
	// vec4 val = texelFetch(HTex00, ivec2(0,0), 0) ;
	// FragColor = vec4(.5,.5,.5,.5);

	FragColor = vec4(
		texelFetch(HTex00, ivec2(0,0), 0).r,
		texelFetch(HTex01, ivec2(0,0), 0).r,
		texelFetch(HTex02, ivec2(0,0), 0).r,
		texelFetch(HTex03, ivec2(0,0), 0).r
	);
}