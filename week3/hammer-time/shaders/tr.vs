#version 300 es
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0

#define TEXTURE_SIZE 32
#define TILE 4
#define HAMMER 16000000

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

float hammerIt() {
    float val;

    // these 7 will fill the UCHE cache set
    val += texelFetch(HTex00, ivec2(0,0), 0).r;
    val += texelFetch(HTex01, ivec2(0,0), 0).r;
    val += texelFetch(HTex02, ivec2(0,0), 0).r;
    val += texelFetch(HTex03, ivec2(0,0), 0).r;

    val += texelFetch(HTex04, ivec2(0,0), 0).r;
    val += texelFetch(HTex05, ivec2(0,0), 0).r;
    val += texelFetch(HTex06, ivec2(0,0), 0).r;
    val += texelFetch(HTex07, ivec2(0,0), 0).r;
    // // this will kick out HText00 from UCHE
    val += texelFetch(HTex08, ivec2(0,0), 0).r;
    
    // These 7 will fill the rest of the L1 set.
    // NOTE: evicting L1 is already a new access to DRAM. 
    val += texelFetch(HTex00, ivec2(0,2), 0).r;
    val += texelFetch(HTex01, ivec2(0,2), 0).r;
    val += texelFetch(HTex02, ivec2(0,2), 0).r;
    val += texelFetch(HTex03, ivec2(0,2), 0).r;

    val += texelFetch(HTex04, ivec2(0,2), 0).r;
    val += texelFetch(HTex05, ivec2(0,2), 0).r;
    val += texelFetch(HTex06, ivec2(0,2), 0).r;
    val += texelFetch(HTex07, ivec2(0,2), 0).r;
    // // this will kick out HText00 from L1
    val += texelFetch(HTex08, ivec2(0,2), 0).r;

    // // // finally, we access this again and it will be treated as a new read from DRAM
    // for (int i = 0; i < 1; i++) {
    //     val += texelFetch(HTex00, ivec2(0,0), 0).r;
    // }
    return val;
}

void main() {   
    
    // -------------------- POC for the efficient cache eviction     

    // final hammer loop 
    float val = 0.0;
    for (int h = 0; h < HAMMER; h++ ) {

        // these 7 will fill the UCHE cache set
        val += texelFetch(HTex00, ivec2(0,0), 0).r;
        val += texelFetch(HTex01, ivec2(0,0), 0).r;
        val += texelFetch(HTex02, ivec2(0,0), 0).r;
        val += texelFetch(HTex03, ivec2(0,0), 0).r;

        val += texelFetch(HTex04, ivec2(0,0), 0).r;
        val += texelFetch(HTex05, ivec2(0,0), 0).r;
        val += texelFetch(HTex06, ivec2(0,0), 0).r;
        val += texelFetch(HTex07, ivec2(0,0), 0).r;
        // // this will kick out HText00 from UCHE
        val += texelFetch(HTex08, ivec2(0,0), 0).r;
        
        // These 7 will fill the rest of the L1 set.
        // NOTE: evicting L1 is already a new access to DRAM. 
        val += texelFetch(HTex00, ivec2(0,2), 0).r;
        val += texelFetch(HTex01, ivec2(0,2), 0).r;
        val += texelFetch(HTex02, ivec2(0,2), 0).r;
        val += texelFetch(HTex03, ivec2(0,2), 0).r;

        val += texelFetch(HTex04, ivec2(0,2), 0).r;
        val += texelFetch(HTex05, ivec2(0,2), 0).r;
        val += texelFetch(HTex06, ivec2(0,2), 0).r;
        val += texelFetch(HTex07, ivec2(0,2), 0).r;
        // // this will kick out HText00 from L1
        val += texelFetch(HTex08, ivec2(0,2), 0).r;

        // // // finally, we access this again and it will be treated as a new read from DRAM
        // for (int i = 0; i < 1; i++) {
        //     val += texelFetch(HTex00, ivec2(0,0), 0).r;
        // }
    }

    // single hammer
    // float val = hammerIt();
    // temp += val;

    gl_Position = vec4(aPos, val);
}