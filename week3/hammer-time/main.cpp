#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include "size.h"
#include "mem_util.h"
#include "eglSetup.h"
#include "shader.cpp"
#include "allocator.h"

using namespace std;

Shader shader;
unsigned int framebuffer;

void init_opengl_setup() {
  // generate textures
  GLuint textures[1];
  glGenTextures(1, (GLuint *)&textures);
  unsigned int texData = textures[0];

  shader = Shader(
      "/data/local/tmp/papa/shaders/tr.vs",
      "/data/local/tmp/papa/shaders/tr.fs");

  // execute
  shader.use();

  // bind texture
  glBindTexture(GL_TEXTURE_2D, texData);
}

void view_texture(unsigned int textureId) {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  // attach it to currently bound framebuffer object
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  }

  // allocate data in memory
  auto *exportData = (unsigned char*)malloc(KB4);
  glReadPixels(0, 0, PAGE_TEXTURE_W, PAGE_TEXTURE_H, GL_RGBA, GL_UNSIGNED_BYTE, exportData);

  for (int i = 0; i <KB4; i++) {
    if (i % 32 == 0) {
       printf("\n");
     }
     printf("%u ", exportData[i]);
  }
  printf("\n");

  free(exportData);
}

void init_framebuffer() {
  // allocate framebuffer
  glGenFramebuffers(1, &framebuffer);
}

void fill_texture(unsigned int textureId, unsigned char value) {
  auto *data = (unsigned char*)malloc(KB4);
  memset((void *)data, value, KB4);

  // write special values to texture
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, PAGE_TEXTURE_W, PAGE_TEXTURE_H, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glBindTexture(GL_TEXTURE_2D, 0);

  free(data);
}

int main( int argc, char** argv ) {
  egl_setup();

  // initialize the opengl render and a framebuffer
  init_opengl_setup();

  // second parameter is ignored for now
  KGSLEntry cont_entries[48];
  allocate_cont(48, KB4, &cont_entries[0]);
  print_entries(cont_entries, 0, 4);

  // test reading and writing to textures
  init_framebuffer();

  view_texture(cont_entries[0].texture_id);
  fill_texture(cont_entries[0].texture_id, 0);
  view_texture(cont_entries[0].texture_id);

  view_texture(cont_entries[1].texture_id);
  fill_texture(cont_entries[1].texture_id, 1);
  view_texture(cont_entries[1].texture_id);

  // executes the shader 
  glDrawArrays(GL_POINTS, 0, 1);

  return 0;
}
