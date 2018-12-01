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
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
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

/**
 * Create plain 2D full coordinate system width/height (full screen) rectangle
 */
void createRectangle() {
    // prepare rectangle
    float vertices[] = {
            1,  1, 0.0f,  // top right
            1, -1, 0.0f,  // bottom right
            -1, -1, 0.0f,  // bottom left
            -1,  1, 0.0f   // top left
    };
    unsigned int indices[] = {  // note that we start from 0!
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
    };
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // binding one time is sufficient, because there is only 1 object to draw
    glBindVertexArray(VAO);
}

void init_debug() {
  GLuint texture;
  glGenTextures(1, (GLuint*)&texture);

  init_framebuffer();

  createTexture2DRGBA(texture, NULL, PAGE_TEXTURE_W, PAGE_TEXTURE_H);
  // attach it to currently bound framebuffer object
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
  // Render to our framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  }

  // create rectangle
  createRectangle();
}

/**
 * View every pixel of the framebuffer 
 */
void view_framebuffer() {
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
}

int main( int argc, char** argv ) {
  egl_setup();

  // initialize the opengl render and a framebuffer
  init_opengl_setup();

  // second parameter is ignored for now
  // KGSLEntry cont_entries[48];
  // allocate_cont(48, KB4, &cont_entries[0]);
  // print_entries(cont_entries, 0, 4);

  // test reading and writing to textures
  // init_framebuffer();

  // view_texture(cont_entries[0].texture_id);
  // fill_texture(cont_entries[0].texture_id, 0);
  // view_texture(cont_entries[0].texture_id);

  // view_texture(cont_entries[1].texture_id);
  // fill_texture(cont_entries[1].texture_id, 1);
  // view_texture(cont_entries[1].texture_id);

  // executes the shader 
  // glDrawArrays(GL_POINTS, 0, 1);
  init_debug();
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  view_framebuffer();
  return 0;
}
