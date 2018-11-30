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

#include "mem_util.h"
#include "size.h"

KGSLEntry *entries;

int sort_by_id(const void *v1, const void *v2) {
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
  for (int i = 0; i < num_to_print; i++)
  {
    printf("+ KGSLEntry[%d+%d=%d] = {id: %d, texture_id: %d, useraddr: %x, pfn: %jx, alloc_order: %d}\n", start_idx, i, start_idx + i,
           entries[start_idx + i].id, entries[start_idx + i].texture_id, entries[start_idx + i].useraddr, entries[start_idx + i].pfn, entries[start_idx + i].alloc_order);
  }
}

void createTexture2DRGBA(unsigned int textureId, uint32_t *data, GLuint width, GLuint height)
{
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);
}

int get_first_index() {
  GLuint textures[1];
  glGenTextures(1, textures);
  GLuint *tData = (GLuint *)malloc(PAGE_TEXTURE_H * PAGE_TEXTURE_W * sizeof(GLuint));
  memset((void *)tData, 0x41, PAGE_TEXTURE_H * PAGE_TEXTURE_W * sizeof(GLuint));
  glBindTexture(GL_TEXTURE_2D, textures[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, PAGE_TEXTURE_W, PAGE_TEXTURE_H, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, tData);
  free(tData);

  pid_t pid = getpid();
  char tmp[128];
  snprintf(tmp, sizeof(tmp), "/d/kgsl/proc/%d/mem", pid);
  char line[1024];

  FILE *fp = fopen(tmp, "r");
  if (fp == 0)
  {
    printf("Unable to open kgsl file\n");
    exit(1);
  }
  int max_id = 0;
  while (1) {
    char usage[20];
    uint32_t useraddr;
    int id;
    int ret;

    if (fgets(line, sizeof(line), fp) == NULL) break;
    ret = sscanf(line, "%*s %x %*s %d %*s %*s %s %*s\n", &useraddr, &id, usage);
    if (ret != 3) continue;

    // printf("%s", line);

    if (strcmp(usage, "texture") == 0) {
      if (id > max_id ) {
        max_id = id;
      }
    }
  }
  return max_id - textures[0];
}


int _allocate_cont(int num_pages, int page_size, int NUM_BLANK_TEXTURES, int *num_created_before) {
  int return_index = 0;
  printf("[Allocator] + Exhusting memory with %d page-textures\n", NUM_BLANK_TEXTURES);
  // Generate a lot of textures, each 1 page
  GLuint textures[NUM_BLANK_TEXTURES];
  glGenTextures(NUM_BLANK_TEXTURES, textures);
  for (int i = 0; i < NUM_BLANK_TEXTURES; i++) {
    GLuint *tData = (GLuint*) malloc(PAGE_TEXTURE_H * PAGE_TEXTURE_W * sizeof(GLuint));
    memset((void *)tData, textures[i]%(0x100), PAGE_TEXTURE_H * PAGE_TEXTURE_W * sizeof(GLuint));
    createTexture2DRGBA(textures[i], tData, PAGE_TEXTURE_W, PAGE_TEXTURE_H);

    // TODO: is this needed or not? 
    free(tData);
  }

  // read kgsl file to filter the textures
  if ( *num_created_before == 0 ) {
    entries = (KGSLEntry*) malloc((NUM_BLANK_TEXTURES + 4) * sizeof(KGSLEntry));
  }
  else {
    entries = (KGSLEntry *)malloc( (NUM_BLANK_TEXTURES + *num_created_before) * sizeof(KGSLEntry));
  }
  int num_kgsl_entries = 0;
  

  pid_t pid = getpid();
  char tmp[128];
  snprintf(tmp, sizeof(tmp), "/d/kgsl/proc/%d/mem", pid);
  char line[1024];

  FILE *fp = fopen(tmp, "r");
  if (fp == 0) {
    printf("Unable to open kgsl file\n");
    exit(1);
  }

  while (1) {
    char usage[20];
    uint32_t useraddr;
    int id;
    int ret;

    if (fgets(line, sizeof(line), fp) == NULL) break;
    ret = sscanf(line, "%*s %x %*s %d %*s %*s %s %*s\n", &useraddr, &id, usage);
    if (ret != 3) continue;

    // printf("%s", line);

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
    }
  }

  // sort them by id
  qsort(entries, num_kgsl_entries, sizeof(KGSLEntry), sort_by_id);
  printf("[Allocator] + Created %d textures and found %d in the file. Approximate size %d MB\n", NUM_BLANK_TEXTURES, num_kgsl_entries, (NUM_BLANK_TEXTURES * KB4) / MB);

  // Check for consequtive pfns and assign alloc order 
  for (int i = 0; i < num_kgsl_entries - num_pages; i++)
  {
    uint64_t base_pfn = entries[i].pfn;
    int j;
    for (j = 1; j < num_kgsl_entries - i; j++)
    {
      uint64_t current_pfn = entries[i + j].pfn;
      if (base_pfn + j != current_pfn)
      {
        break;
      }
    }
    entries[i].alloc_order = j;
  }


  // check and filter allocation order 
  for (int i = 0; i < num_kgsl_entries; i++) {
    if (entries[i].alloc_order > num_pages)
    {
      return_index = i;
      break; 
    }
  }

  fclose(fp);

  if (!return_index) {
    *num_created_before = num_kgsl_entries;
    free(entries);
    return 0;
  }

  // printf("%d %d %d .... %d %d %d\n", textures[0], textures[1], textures[2], textures[NUM_BLANK_TEXTURES-3], textures[NUM_BLANK_TEXTURES-2], textures[NUM_BLANK_TEXTURES-1]);
  printf("[Allocator] + Target memory region found. \n");
  return return_index;
}

int allocate_cont(int num_pages, int page_size, KGSLEntry *ret_entries) {
  int NUM_BLANK_TEXTURES = KB4;
  int before = 0;

  int offset = get_first_index();
  
  int idx = _allocate_cont(num_pages, page_size, NUM_BLANK_TEXTURES, &before);
  while(!idx) {
    printf("--- Failed [%d created]\n", before);
    NUM_BLANK_TEXTURES *= 2;
    idx = _allocate_cont(num_pages, page_size, NUM_BLANK_TEXTURES, &before);
  }
  
  for (int i = 0; i < num_pages; i++) {
    entries[idx+i].texture_id = entries[idx+i].id - offset;
  }
  memcpy(ret_entries, &entries[idx], num_pages*sizeof(KGSLEntry));
  
  return 0;
}
