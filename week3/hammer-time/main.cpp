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

int main( int argc, char** argv ) {
  egl_setup();

  // initialize the opengl render and a framebuffer
  init_opengl_setup();

  // second parameter is ignored for now
  KGSLEntry cont_entries[48];
  allocate_cont(48, KB4, &cont_entries[0]);
  print_entries(cont_entries, 0, 4);

  glBindTexture(GL_TEXTURE_2D, 5);
  GLuint *tData = (GLuint *)malloc(PAGE_TEXTURE_H * PAGE_TEXTURE_W * sizeof(GLuint));
  memset((void *)tData, 0, PAGE_TEXTURE_H * PAGE_TEXTURE_W * sizeof(GLuint));
  glReadPixels(0, 0, PAGE_TEXTURE_W, PAGE_TEXTURE_H, GL_RED_INTEGER, GL_UNSIGNED_INT, tData);
  printf("+++ probe value => %u\n", tData[0]);

  // executes the shader 
  glDrawArrays(GL_POINTS, 0, 1);

  return 0;
}
