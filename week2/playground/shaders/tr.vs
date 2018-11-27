#version 300 es

#define TEXTURE_SIZE 256
#define STRIDE 4
#define STRIDE_UCHE 16
#define TILE 4

layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
uniform sampler2D dataTexture;
uniform int max;

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

	// ------------------------------ L1 Experiments 
	// STEP1: this is enough to infer the cache line size. no mis for (0,1,3) and a miss for (4,0)
	// cache line size = 4texel => 16 bytes 
	// int x = 0;
	// val += texelFetch(dataTexture, ivec2(x,0), 0);


	// STEP2: works with 256, 1/2 ratio breaks on 256+16 => cache size = 256 texels = 1024 KB
	// for (int i = 0; i < 2 ; i++) {
	// 	for (int t = 0; t < max; t += STRIDE) {
	// 		// val += texelFetch(dataTexture, ivec2(t, 0), 0);
	// 		val += texelFetch(dataTexture, offToPix(t), 0);
	// 	}
	// }

	// ------------------------------ UCHE Experiments 
	// STEP1: Infer the cache line size/

	// FIRST: run just this. you will see 16 UCHE_UCHEPERF_VBIF_READ_BEATS_TP. This translates to 4 fetches from UCHE bc.
	// there are 4 TP units and each will send 4 requests (we guess).
	// val += texelFetch(dataTexture, ivec2(0,0), 0);

	// THEN: run this: Nothing changes in [groupId 8][counterId 0]. This is because the extra three ones (x = 1,2,3) is not even reacing 
	// UCHE and will be resolved in L1
	// val += texelFetch(dataTexture, ivec2(0,0), 0);
	// val += texelFetch(dataTexture, ivec2(1,0), 0);

	// val += texelFetch(dataTexture, ivec2(5,0), 0);
	// val += texelFetch(dataTexture, ivec2(6,0), 0);

	// val += texelFetch(dataTexture, ivec2(6,0), 0);
	// val += texelFetch(dataTexture, ivec2(7,0), 0);

	// THEN: run this
	// for (int i = 0; i < 4; i++) {
	// 	val += texelFetch(dataTexture, ivec2(i*STRIDE_UCHE,0), 0);
	// }

	// val += texelFetch(dataTexture, ivec2(x+1,0), 0);
	// val += texelFetch(dataTexture, ivec2(x+2,0), 0);
	// val += texelFetch(dataTexture, ivec2(x+3,0), 0);
	// val += texelFetch(dataTexture, ivec2(x+4,0), 0);
	// val += texelFetch(dataTexture, ivec2(x+5,0), 0);
	// val += texelFetch(dataTexture, ivec2(x+6,0), 0);
	// val += texelFetch(dataTexture, ivec2(x+7,0), 0);

	// STEP2: with known L2 cache size 16, touch texels 16 apart, the number of L2 misses ([groupId 8][counterId 0]) is ALWAYS the same 
	// as the number of texels fetched, regardless of the outer loop. Any value above 8K as the upper bound will break this ratio. 
	for (int i = 0; i < 2; i++) {
		for (int t = 0; t < max; t+=STRIDE_UCHE) {
			val += texelFetch(dataTexture, offToPix(t), 0);
		}
	}


	

    gl_Position = vec4(aPos, val);
}