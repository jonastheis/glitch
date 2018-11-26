#version 300 es

#define TEXTURE_SIZE 32
#define STRIDE 4
#define TILE 4

layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
uniform sampler2D dataTexture;

// float _normalize(int v) {
// 	return float(float(v) / (TEXTURE_SIZE/2.0)) - 1.0;
// }
// float normalize(int v) {
// 	return float(float(v) / TEXTURE_SIZE);
// }

// vec2 normalize(vec2 v) {
// 	return vec2(normalize(v.x), normalize(v.y));
// }


// def offToPix(t):
// 	tile_number = int(t / (TILE*TILE))
// 	tile_row = int(tile_number / ((TW/TILE))) * TILE
// 	tile_col = int(tile_number % (TW/TILE)) * TILE

// 	local_x = t % TILE
// 	local_y = int((t-(tile_number*TILE*TILE)) / TILE)
	
// 	global_x = local_x + tile_col 
// 	global_y = local_y + tile_row 

// 	return (global_x, global_y)

ivec2 offToPix(int t) {
		int tile_number = int(t / (TILE*TILE));
		int tile_row = int(tile_number / ((TEXTURE_SIZE/TILE))) * TILE;
		int tile_col = int(tile_number % (TEXTURE_SIZE/TILE)) * TILE;

		int local_x = t % TILE;
		int local_y = int((t-(tile_number*TILE*TILE)) / TILE);
		
		int global_x = local_x + tile_col;
		int global_y = local_y + tile_row; 

		return ivec2(global_x, global_y);
}

void main()
{
	
	vec4 val;

	// STEP1: this is enough to infer the cache line size. no mis for (0,1,3) and a miss for (4,0)
	// cache line size = 4texel => 16 bytes 
	int x = 0;
	val += texelFetch(dataTexture, ivec2(x,0), 0);


	// STEP2: works with 256, breaks on 256+16 => cache size = 256 texels = 1024 KB
	for (int i = 0; i < 2 ; i++) {
		for (int t = 0; t < 256; t += STRIDE) {
			// val += texelFetch(dataTexture, ivec2(t, 0), 0);
			val += texelFetch(dataTexture, offToPix(t), 0);
		}
	}
	

    gl_Position = vec4(aPos, val);
}