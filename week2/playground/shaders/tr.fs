#version 300 es

#define MAX 32 
#define TEXTURE_SIZE 128.0

out uvec4 FragColor;

uniform usampler2D dataTexture;
vec2 texCoord; 
uint val;

float normalize(int v) {
	return float(float(v) / (TEXTURE_SIZE/2.0)) - 1.0;
}

vec2 normalize(vec2 v) {
	return vec2(normalize(v.x), normalize(v.y));
}

void main(){
	//for (int x=0; x < MAX; x += 1) { 
	//	texCoord = vec2(x/MAX, y/MAX);
	//	val += texture(dataTexture, texCoord); 
	//}
	val += texture(dataTexture, normalize(vec2(4, 0)));
	val += texture(dataTexture, normalize(vec2(8, 0)));
	val += texture(dataTexture, normalize(vec2(12, 0)));
	val += texture(dataTexture, normalize(vec2(16, 0)));
	val += texture(dataTexture, normalize(vec2(20, 0)));
	val += texture(dataTexture, normalize(vec2(24, 0)));
	val += texture(dataTexture, normalize(vec2(28, 0)));
	val += texture(dataTexture, normalize(vec2(32, 0)));
	val += texture(dataTexture, normalize(vec2(36, 0)));

	FragColor = uvec4(val, vec3(0));
}