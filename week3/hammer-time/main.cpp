#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <math.h>

#include "size.h"
#include "mem_util.h"
#include "eglSetup.h"
#include "shader.cpp"
#include "allocator.h"
#include "counters.h"

using namespace std;

Shader shader;
unsigned int framebuffer;
KGSLEntry cont_entries[64];

inline double log2(const double x)
{
  return log(x) * M_LOG2E;
}

//assumes little endian
void printBits(size_t const size, void const *const ptr) {
  unsigned char *b = (unsigned char *)ptr;
  unsigned char byte;
  int i, j;
  printf("+++ Bit-value: ");
  for (i = size - 1; i >= 0; i--)
  {
    for (j = 7; j >= 0; j--)
    {
      byte = (b[i] >> j) & 1;
      printf("%u", byte);
    }
  }
  puts("");
}

void init_opengl_setup() {
  shader = Shader(
      "/data/local/tmp/papa/shaders/tr.vs",
      "/data/local/tmp/papa/shaders/tr.fs");

  // execute
  shader.use();

}

void view_texture(unsigned int textureId, int kgsl_index) {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  // attach it to currently bound framebuffer object
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  }

  // allocate data in memory
  auto *exportData = (unsigned char*)malloc(KB4);
  glReadPixels(0, 0, PAGE_TEXTURE_W, PAGE_TEXTURE_H, GL_RGBA, GL_UNSIGNED_BYTE, exportData);
  int flip = 0;
  for (int i = 0; i < KB4; i++)
  {
    if (exportData[i] != 0xff)
    {
      printf("++++ BIT FLIP IDENTIFIED\n");
      printf("+++ INFO: [Texture-Id: %u][Bit-Index: %d][Byte-Index: %d][ByteValue: %x][8-byte-offset: %d]\n",
             cont_entries[kgsl_index].texture_id, (int)log2(exportData[i] ^ 0xff), i, exportData[i], i / 8);
      printBits(1, &exportData[i]);
      print_entries(cont_entries, kgsl_index, 1);
      // exit(0);
      flip = 1;
      for (int ii = 0; ii <KB4; ii++) {
        if (ii % 32 == 0) {
           printf("\n");
         }
         if ( ii % 4 == 0 ) {
           printf("  ");
         }
         printf("%x,", exportData[ii]);
      }
      printf("\n");
    }
  }
  if (!flip) {
    // printf("---- [%u] No Flip For You!\n", textureId);
  }

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
    if (i % 32 == 0)
    {
      printf("\n");
    }
    if (i % 4 == 0)
    {
      printf("  ");
    }
    printf("%u,", exportData[i]);
  }
  printf("\n");
  free(exportData);
}


void bind_texture(unsigned int textureId, int i, char type, int offset) {
  char temp[20];
  sprintf(temp, "%cTex0%d", type, i);
  // printf("++ Binding [%s] %d=[%d] -- offset: %d\n", temp, textureId, textureId%256, offset);

  GLuint uUniform = glGetUniformLocation(shader.ID, temp);
  glActiveTexture(GL_TEXTURE0 + i);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glUniform1i(uUniform, i);
}

void _prepare_hammer_time() {
  // example for hammering first bank x=hammer, .=eviction
  //  0001 0203 0405 0607 0809 1011 1213 1415
  // |xxxx|----|----|----|----|----|----|----|
  //  1617 1819 2021 2223 2425 2627 2829 3031
  // |----|----|----|----|----|----|----|----|
  //  3233 3435 3637 3839 4041 4243 4445 4647
  // |xxxx|----|----|----|----|----|----|----|
  //  4849 5051 5253 5455 5657 5859 6061 6263
  // |----|----|----|----|----|--..|....|....|

  for (int row = 0; row <= 16; row += 16) {
    printf("++ Prepare row [%d]\n", row/16);

    // fill textures in row n-1, n+1 with 0
    for (int localOffset = 0; localOffset < 16; localOffset += 2) {
      int offset = row + localOffset;
      // printf("++ Hammering [%d][%d]\n", offset, offset+1);

      fill_texture(cont_entries[offset + 0].texture_id, 0x00);
      fill_texture(cont_entries[offset + 1].texture_id, 0x00);

      fill_texture(cont_entries[offset + 32].texture_id, 0x00);
      fill_texture(cont_entries[offset + 33].texture_id, 0x00);

      // fill textures in row n with 1
      fill_texture(cont_entries[offset + 16].texture_id, 0xFF);
      fill_texture(cont_entries[offset + 17].texture_id, 0xFF);

      // pass hammer textures according to hammer pattern: jump to differnet row to trigger row buffer when hammering
      bind_texture(cont_entries[offset + 0].texture_id, 0, 'H', offset + 0);
      bind_texture(cont_entries[offset + 32].texture_id, 2, 'H', offset + 32);
      bind_texture(cont_entries[offset + 1].texture_id, 4, 'H', offset + 1);
      bind_texture(cont_entries[offset + 33].texture_id, 6, 'H', offset + 33);

      // select 5 textures for eviction
      if (row >= 16) {
        if (localOffset < 8) {
          // take 5 textures from end of first row: 11,12,13,14,15
          for (int i = 0; i < 5; i++) {
            bind_texture(cont_entries[11 + i].texture_id, i == 4 ? 8 : i*2+1, 'H', 11 + i);
          }
        } else {
          // take 5 textures from beginning of first row: 0,1,2,3,4
          for (int i = 0; i < 5; i++) {
            bind_texture(cont_entries[i].texture_id, i == 4 ? 8 : i*2+1, 'H', i);
          }
        }
      } else {
        if (localOffset < 8) {
          // take 5 textures from end of last row: 59,60,61,62,63
          for (int i = 0; i < 5; i++) {
            bind_texture(cont_entries[59 + i].texture_id, i == 4 ? 8 : i*2+1, 'H', 59 + i);
          }
        } else {
          // take 5 textures from beginning of last row: 48,49,50,51,52
          for (int i = 0; i < 5; i++) {
            bind_texture(cont_entries[48 + i].texture_id, i == 4 ? 8 : i*2+1, 'H', 48 + i);
          }
        }
      }
      
      // important: bind dummy texture last to prevent last texture error
      bind_texture(cont_entries[15].texture_id, 9, 'D', 15);

      glDrawArrays(GL_POINTS, 0, 1);

      // check hammered textures for bit flip
      // printf("+++ Checking [%d][%d]\n", offset + 16, offset + 17);
      view_texture(cont_entries[offset + 16].texture_id, offset+16);
      view_texture(cont_entries[offset + 17].texture_id, offset+17);
    }

  }
}

void prepare_hammer_time(){
  for (int i = 0; i < 400000; i++)
  {
    allocate_cont(64, KB4, &cont_entries[0]);
    // print_entries(cont_entries, 0, 4);
    _prepare_hammer_time();
  }
}

int main( int argc, char** argv ) {
  egl_setup();
  init_opengl_setup();
  init_framebuffer();


  // ---------------------- Main Program: Run bitflip test ----------------------
  prepare_hammer_time();


  // ---------------------- Testing Setups ----------------------
  // Option1) debug shader output with framebuffer
  // init_debug();
  // allocate_cont(64, KB4, &cont_entries[0]);
  // print_entries(cont_entries, 0, 4);
  // _prepare_hammer_time();
  // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  // view_framebuffer();

  // Option2) Execute 1 pixel
  // glDrawArrays(GL_POINTS, 0, 1);

  // Option3) Execute 1 pixel with counters 
  // counters_init();
  // GLuint group_UCHE[]   = {8, 9, 9};
  // GLuint counter_UCHE[] = {0, 1, 2};
  // GLuint num_target_counters = 3;
  // allocate_cont(64, KB4, &cont_entries[0]);
  // print_entries(cont_entries, 0, 4);
  // _prepare_hammer_time();
  // perform_measurement(group_UCHE, counter_UCHE, num_target_counters);

  return 0;
}