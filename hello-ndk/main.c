#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "eglSetup.h"
#include "mem_util.h"

#define KB 1024
#define MB (1024*1024)
#define NUM_TEXTURES (32*KB)
#define TARGET_PFNS 48

#define WIDHT 32
#define HEIGHT 32

// import global variables from setup phase
extern EGLDisplay display; // eglGetDisplay — return an EGL display connection
extern EGLSurface pBuffer; // eglCreatePbufferSurface — create a new EGL pixel buffer surface
extern EGLContext ctx;

void check_shader_error(int shader) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("ERROR::SHADER::COMPILATION_FAILED [%s]\n", infoLog);
    }
}

void check_program_error(int p) {
    int success;
    char infoLog[512];
    glGetProgramiv(p, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(p, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED [%s]\n", infoLog);
    }
}

int cmpfunc (const void * v1, const void * v2) {
    const KGSLEntry *p1 = (KGSLEntry *)v1;
    const KGSLEntry *p2 = (KGSLEntry *)v2;
    if (p1->id < p2->id)
        return -1;
    else if (p1->id > p2->id)
        return +1;
    else
        return 0;
}

void print_entries(KGSLEntry entries[], int start_idx, int num_to_print) {
    for (int i = 0; i < num_to_print; i++) {
        printf("+ KGSLEntry[%d] = {id: %d, useraddr: %x, pfn: %jx}\n", start_idx+i, 
            entries[start_idx+i].id, entries[start_idx+i].useraddr, entries[start_idx+i].pfn);
    }
}

int main( int argc, char** argv ) {
    egl_setup();
    printf("+ EGL_SETUP Success\n");

    // Generate a lot of textures 
    GLuint textures[NUM_TEXTURES];
    glGenTextures(NUM_TEXTURES, textures);
    for (int i = 0; i < NUM_TEXTURES; i++) {
        GLuint *tData = malloc(WIDHT*HEIGHT*sizeof(GLuint));
        memset((void *)tData, 0x41, WIDHT*HEIGHT*sizeof(GLuint));
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, WIDHT, HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, tData);
    }
    
    // read kgsl file to filter the textures 
    // relax a bit for now: the first NUM_TEXTURES textures are the ones that we are interested in
    KGSLEntry *entries = malloc((NUM_TEXTURES+4)*sizeof(KGSLEntry));

    int num_kgsl_entries = 0;

    pid_t pid = getpid();
    char tmp[128];
    snprintf(tmp, sizeof(tmp), "/d/kgsl/proc/%d/mem", pid);
    printf("+ Attempting to open file %s \n", tmp);
    FILE *fp = fopen(tmp, "r");
    char line[1024];
    if ( fp == 0 ) {
        printf("Unable to open kgsl file\n");
        exit(1);
    }
    while (1) {
        char usage[20];
        uint32_t useraddr; 
        int id; 
        int ret;
        if (fgets(line, sizeof(line), fp) == NULL) {
            break;
        }

         /*
         *  gpuaddr  useraddr     size     id   flags       type        usage           sglen
         *  545ba000 545ba000     4096     1    -----pY     gpumem      arraybuffer     1
         */
        ret = sscanf(line, "%*s %x %*s %d %*s %*s %s %*s\n", &useraddr, &id, usage);
        if ( ret != 3 ) { continue; }
        if (strcmp(usage, "texture") == 0) {
            KGSLEntry entry; 
            PagemapEntry texture_pagemap;
            pagemap_get_entry(&texture_pagemap, (uintptr_t)useraddr);
            entry.id = id;
            // entry.usage = usage;
            strncpy(entry.usage, usage, 20);
            entry.useraddr = useraddr;
            entry.pfn = texture_pagemap.pfn;
            entries[num_kgsl_entries] =  entry;
            num_kgsl_entries++;
        }
    }

    // sort them by id
    qsort(entries, num_kgsl_entries, sizeof(KGSLEntry), cmpfunc);
    // print_entries(entries, 0, NUM_TEXTURES);
    printf("++ Created %d textures and found %d in the file. Approximate size %d MB\n", NUM_TEXTURES, num_kgsl_entries, (NUM_TEXTURES*4*KB)/MB);

    // Check for $(TARGET_PFNS) consequtive pfns 
    int cont_mem_kgsl_idx = 0;
    int max_order_found = 0;
    int found = 0;
    for (int i = 0 ; i < num_kgsl_entries - TARGET_PFNS; i++) {
        uint64_t base_pfn = entries[i].pfn;
        cont_mem_kgsl_idx = i;
        int j;
        found = 1;
        for (j = 1; j < TARGET_PFNS; j++) {
            uint64_t current_pfn = entries[i+j].pfn;
            if ( base_pfn+j != current_pfn) {
                found = 0;
                break;
            }
        }

        if ( found ) { break; }
    }

    if ( found ) {
        printf("+++ Found %d contiguous texture memories at index %d if entries[]\n", TARGET_PFNS, cont_mem_kgsl_idx);
    }
    else {
        printf("+++ Unable to find contiguous memory\n");
    }

    print_entries(entries, cont_mem_kgsl_idx, TARGET_PFNS);

    free(entries);
    return 0;
}