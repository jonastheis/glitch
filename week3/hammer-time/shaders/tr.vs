#version 300 es
#pragma optimize (off)
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0

#define TEXTURE_SIZE 32
#define TILE 4

uniform usampler2D HTex00;
uniform usampler2D HTex01;
uniform usampler2D HTex02;
uniform usampler2D HTex03;
uniform usampler2D HTex04;
uniform usampler2D HTex05;
uniform usampler2D HTex06;
uniform usampler2D HTex07;
uniform usampler2D HTex08;

// bind dummy texture last to prevent last texture error - do not use
uniform sampler2D DTex09;

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

void main() {   
    
    // -------------------- POC for the efficient cache eviction     

    // final hammer loop 
    uint val = 0u;
    ivec2 base = ivec2(0,0);
    ivec2 base32 = ivec2(0,2);
    for (uint i = 0u; i < 1300000u; i++) {
        val += texelFetch(HTex00, base, 0).r;
        val += texelFetch(HTex01, base, 0).r;
        val += texelFetch(HTex02, base, 0).r;
        val += texelFetch(HTex03, base, 0).r;

        val += texelFetch(HTex04, base, 0).r;
        val += texelFetch(HTex05, base, 0).r;
        val += texelFetch(HTex06, base, 0).r;
        val += texelFetch(HTex07, base, 0).r;
        // // this will kick out HText00 from UCHE
        val += texelFetch(HTex08, base, 0).r;
        
        // These 7 will fill the rest of the L1 set.
        // NOTE: evicting L1 is already a new access to DRAM. 
        val += texelFetch(HTex00, base32, 0).r;
        val += texelFetch(HTex01, base32, 0).r;
        val += texelFetch(HTex02, base32, 0).r;
        val += texelFetch(HTex03, base32, 0).r;

        val += texelFetch(HTex04, base32, 0).r;
        val += texelFetch(HTex05, base32, 0).r;
        val += texelFetch(HTex06, base32, 0).r;
        val += texelFetch(HTex07, base32, 0).r;
        // // this will kick out HText00 from L1
        val += texelFetch(HTex08, base32, 0).r;
    } 


    // gl_Position = vec4(aPos, val);
    gl_Position = vec4(aPos, val);
}