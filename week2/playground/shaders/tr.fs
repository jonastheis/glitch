#version 300 es

#define MAX 32 
#define STRIDE 1
#define TEXTURE_SIZE 64.0

out uvec4 FragColor;

uniform usampler2D dataTexture;
vec2 texCoord; 


float normalize(int v) {
	return float(float(v) / (TEXTURE_SIZE/2.0)) - 1.0;
}

vec2 normalize(vec2 v) {
	return vec2(normalize(v.x), normalize(v.y));
}

void main(){

	uint val = 0u;
	//for (int x=0; x < MAX; x += 1) { 
	//	texCoord = vec2(x/MAX, y/MAX);
	//	val += texture(dataTexture, texCoord); 
	//}

	// val += texture(dataTexture, vec2(-1, -1));
	//val += texture(dataTexture, vec2(0, 0));
	//val += texture(dataTexture, vec2(1, 1));
	

	// access pattern for 64 by 64
	/*for (int i=0; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 0)));
	}
	for (int i=3; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 7)));
	}
	for (int i=18; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 10)));
	}
	for (int i=17; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 13)));
	}
	for (int i=16; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 16)));
	}
	for (int i=3; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 23)));
	}
	for (int i=18; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 26)));
	}
	for (int i=17; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 29)));
	}
	for (int i=16; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 32)));
	}
	for (int i=3; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 39)));
	}
	for (int i=18; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 42)));
	}
	for (int i=17; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 45)));
	}
	for (int i=16; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 48)));
	}
	for (int i=3; i < 64; i+= 4) {
		val += texture(dataTexture, normalize(vec2(i, 55)));
	}*/


	/*
	// access patterns for 32 by 32 texture
	val += texture(dataTexture, normalize(vec2(0, 0)));
	val += texture(dataTexture, normalize(vec2(4, 0)));
	val += texture(dataTexture, normalize(vec2(8, 0)));
	val += texture(dataTexture, normalize(vec2(12, 0)));
	val += texture(dataTexture, normalize(vec2(16, 0)));
	val += texture(dataTexture, normalize(vec2(20, 0)));
	val += texture(dataTexture, normalize(vec2(24, 0)));
	val += texture(dataTexture, normalize(vec2(28, 0)));

	val += texture(dataTexture, normalize(vec2(3, 7)));
	val += texture(dataTexture, normalize(vec2(7, 7)));
	val += texture(dataTexture, normalize(vec2(11, 7)));
	val += texture(dataTexture, normalize(vec2(15, 7)));
	val += texture(dataTexture, normalize(vec2(19, 7)));
	val += texture(dataTexture, normalize(vec2(23, 7)));

	val += texture(dataTexture, normalize(vec2(2, 10)));
	val += texture(dataTexture, normalize(vec2(6, 10)));
	val += texture(dataTexture, normalize(vec2(10, 10)));
	val += texture(dataTexture, normalize(vec2(14, 10)));
	val += texture(dataTexture, normalize(vec2(18, 10)));
	val += texture(dataTexture, normalize(vec2(22, 10)));

	val += texture(dataTexture, normalize(vec2(1, 13)));
	val += texture(dataTexture, normalize(vec2(5, 13)));
	val += texture(dataTexture, normalize(vec2(9, 13)));
	val += texture(dataTexture, normalize(vec2(13, 13)));
	val += texture(dataTexture, normalize(vec2(17, 13)));
	val += texture(dataTexture, normalize(vec2(21, 13)));

	val += texture(dataTexture, normalize(vec2(0, 16)));
	val += texture(dataTexture, normalize(vec2(4, 16)));
	val += texture(dataTexture, normalize(vec2(8, 16)));
	val += texture(dataTexture, normalize(vec2(12, 16)));
	val += texture(dataTexture, normalize(vec2(16, 16)));
	val += texture(dataTexture, normalize(vec2(20, 16)));
	val += texture(dataTexture, normalize(vec2(24, 16)));
	val += texture(dataTexture, normalize(vec2(28, 16)));

	val += texture(dataTexture, normalize(vec2(3 , 23)));
	val += texture(dataTexture, normalize(vec2(7 , 23)));
	val += texture(dataTexture, normalize(vec2(11, 23)));
	val += texture(dataTexture, normalize(vec2(15, 23)));
	val += texture(dataTexture, normalize(vec2(19, 23)));
	val += texture(dataTexture, normalize(vec2(23, 23)));

	val += texture(dataTexture, normalize(vec2(2 , 26)));
	val += texture(dataTexture, normalize(vec2(6 , 26)));
	val += texture(dataTexture, normalize(vec2(10, 26)));
	val += texture(dataTexture, normalize(vec2(14, 26)));
	val += texture(dataTexture, normalize(vec2(18, 26)));
	val += texture(dataTexture, normalize(vec2(22, 26)));

	val += texture(dataTexture, normalize(vec2(1 , 29)));
	val += texture(dataTexture, normalize(vec2(5 , 29)));
	val += texture(dataTexture, normalize(vec2(9 , 29)));
	val += texture(dataTexture, normalize(vec2(13, 29)));
	val += texture(dataTexture, normalize(vec2(17, 29)));
	val += texture(dataTexture, normalize(vec2(21, 29)));
	val += texture(dataTexture, normalize(vec2(25, 29)));
	val += texture(dataTexture, normalize(vec2(29, 29)));
	*/

	FragColor = uvec4(val, vec3(0));
}