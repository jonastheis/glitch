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
#define TARGET_PFNS 48
#define NUM_TEXTURES (32*KB)

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

int sort_by_id (const void * v1, const void * v2) {
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
        printf("+ KGSLEntry[%d+%d=%d] = {id: %d, useraddr: %x, pfn: %jx, alloc_order: %d}\n", start_idx, i, start_idx+i, 
            entries[start_idx+i].id, entries[start_idx+i].useraddr, entries[start_idx+i].pfn, entries[start_idx+i].alloc_order);
    }
}

void dump_entries_csv(FILE* fp, KGSLEntry entries[], int start_idx, int num_to_dump) {
    for (int i = 0; i < num_to_dump; i++) {
        fprintf(fp, "papa,%d,%x,%jx,%d\n", 
            entries[start_idx+i].id, entries[start_idx+i].useraddr, 
            entries[start_idx+i].pfn, entries[start_idx+i].alloc_order);
    }
}

void dump_pagemap(FILE *fp) {
    int  a;
    FILE *pagemap_fp = fopen("/proc/self/pagemap", "r");
    while(1)
    {
        a = fgetc(pagemap_fp);

        if(!feof(pagemap_fp))
            fputc(a, fp);
        else
            break;
    }
    fclose(pagemap_fp);
}

int main( int argc, char** argv ) {
    egl_setup();

    #ifdef DUMP
    printf("+ DUMP enabled.\n");
    #endif

    printf("+ Generating %d textures...\n", NUM_TEXTURES);
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
    int *target_entries_idxs = malloc(50 * sizeof(int));

    int num_kgsl_entries = 0;
    int num_target_kgsl_entries = 0;

    pid_t pid = getpid();
    char tmp[128];
    snprintf(tmp, sizeof(tmp), "/d/kgsl/proc/%d/mem", pid);
    char line[1024];
    
    printf("+ Attempting to open file %s \n", tmp);

    FILE *fp = fopen(tmp, "r");

    #ifdef DUMP
    FILE *kgsl_dump_fp = fopen("./dumps/kgsl.dump", "w");
    if ( fp == 0) {
        printf("Unable to open kgsl dump file\n");
        exit(1);
    }
    #endif 

    if ( fp == 0) {
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

        #ifdef DUMP 
        fprintf(kgsl_dump_fp, "%s", line);
        #endif 

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
            strncpy(entry.usage, usage, 20);
            entry.useraddr = useraddr;
            entry.pfn = texture_pagemap.pfn;
            entry.alloc_order = 0;
            entries[num_kgsl_entries] = entry;
            num_kgsl_entries++;

            if ( (num_kgsl_entries+1) % 1000 == 0 ) {
                print_entries(entries, num_kgsl_entries-1, 1);
            }
        }
    }

    // sort them by id
    qsort(entries, num_kgsl_entries, sizeof(KGSLEntry), sort_by_id);
    printf("++ Created %d textures and found %d in the file. Approximate size %d MB\n", NUM_TEXTURES, num_kgsl_entries, (NUM_TEXTURES*4*KB)/MB);

    // Check for consequtive pfns 
    for (int i = 0 ; i < num_kgsl_entries - TARGET_PFNS; i++) {
        uint64_t base_pfn = entries[i].pfn;
        int j;
        for (j = 1; j < num_kgsl_entries-i ; j++) {
            uint64_t current_pfn = entries[i+j].pfn;
            if ( base_pfn+j != current_pfn) {
                break;
            }
        }
        entries[i].alloc_order = j;
    }

    for (int i = 0; i < num_kgsl_entries; i++) {
        if ( entries[i].alloc_order > TARGET_PFNS ) {
            target_entries_idxs[num_target_kgsl_entries] = i;
            num_target_kgsl_entries++;
            i += entries[i].alloc_order;
        }
    }

    printf("++ Found %d order 6 or more KGSL entries \n", num_target_kgsl_entries);

    #ifdef DUMP
    FILE* fp_array = fopen("dumps/array.dump", "w");
    fprintf(fp_array, "group_id,id,useraddr,pfn,alloc_order\n");
    #endif 

    for (int i = 0; i < num_target_kgsl_entries; i++)  {
        print_entries(entries, target_entries_idxs[i], entries[target_entries_idxs[i]].alloc_order);
        #ifdef DUMP
        dump_entries_csv(fp_array, entries, target_entries_idxs[i], entries[target_entries_idxs[i]].alloc_order);
        #endif 
    }

    #ifdef DUMP 
    printf("++ Dumping Pagemap file\n");
    FILE *pagemap_fp = fopen("./dumps/pagemap.dump", "wr");
    dump_pagemap(pagemap_fp);
    #endif 

    fclose(fp);
    free(entries);
    free(target_entries_idxs);

    #ifdef DUMP
    fclose(kgsl_dump_fp);
    fclose(pagemap_fp);
    #endif

    return 0;
}