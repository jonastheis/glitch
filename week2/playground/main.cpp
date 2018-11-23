#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
// #include <GLES3/gl3ext.h>
// TODO: why the fuck was I forced to import this?
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include "eglSetup.h"
#include "shader.cpp"


using namespace std;

#define KB 1024
#define MB (1024*1024)
#define WINDOW_WIDTH 4
#define WINDOW_HEIGHT 4


// import global variables from setup phase
extern EGLDisplay display; // eglGetDisplay — return an EGL display connection
extern EGLSurface pBuffer; // eglCreatePbufferSurface — create a new EGL pixel buffer surface
extern EGLContext ctx;

PFNGLGETPERFMONITORGROUPSAMDPROC glGetPerfMonitorGroupsAMD;
PFNGLGETPERFMONITORCOUNTERSAMDPROC glGetPerfMonitorCountersAMD;
PFNGLGETPERFMONITORGROUPSTRINGAMDPROC glGetPerfMonitorGroupStringAMD;
PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC glGetPerfMonitorCounterStringAMD;
PFNGLSELECTPERFMONITORCOUNTERSAMDPROC glSelectPerfMonitorCountersAMD;
PFNGLBEGINPERFMONITORAMDPROC glBeginPerfMonitorAMD;
PFNGLENDPERFMONITORAMDPROC glEndPerfMonitorAMD;
PFNGLGETPERFMONITORCOUNTERDATAAMDPROC glGetPerfMonitorCounterDataAMD;
PFNGLGETPERFMONITORCOUNTERINFOAMDPROC glGetPerfMonitorCounterInfoAMD;
PFNGLGENPERFMONITORSAMDPROC glGenPerfMonitorsAMD;
PFNGLDELETEPERFMONITORSAMDPROC glDeletePerfMonitorsAMD;

typedef struct {
  GLuint       *counterList;
  int         numCounters;
  int         maxActiveCounters;
} CounterInfo;

int          numGroups;
GLuint       *groups;
CounterInfo  *counters;

void getGroupAndCounterList(GLuint **groupsList, int *numGroups, CounterInfo **counterInfo) {
  GLint          n;
  GLuint        *groups;
  CounterInfo   *counters;

  glGetPerfMonitorGroupsAMD(&n, 0, NULL);
  groups = (GLuint*) malloc(n * sizeof(GLuint));
  glGetPerfMonitorGroupsAMD(NULL, n, groups);
  *numGroups = n;

  *groupsList = groups;
  counters = (CounterInfo*) malloc(sizeof(CounterInfo) * n);
  for (int i = 0 ; i < n; i++ ) {
    glGetPerfMonitorCountersAMD(groups[i], &counters[i].numCounters,
     &counters[i].maxActiveCounters, 0, NULL);

    counters[i].counterList = (GLuint*)malloc(counters[i].numCounters * 
      sizeof(int));

    glGetPerfMonitorCountersAMD(groups[i], NULL, NULL,
      counters[i].numCounters, 
      counters[i].counterList);
  }

  *counterInfo = counters;
}

void getCounterNames(int verbose) {
  int          i = 0;

  getGroupAndCounterList(&groups, &numGroups, &counters);

  if ( verbose ) {
    for ( i = 0; i < numGroups; i++ ) {
      char curGroupName[256];
      glGetPerfMonitorGroupStringAMD(groups[i], 256, NULL, curGroupName);
      printf("group[%d] = %s\n",i, curGroupName );
      for ( int j = 0; j < counters[i].numCounters; j++ ) {
        char curCounterName[256];
        glGetPerfMonitorCounterStringAMD(groups[i], counters[i].counterList[j], 256, NULL, curCounterName);
        printf("\t counters[%d] = %s\n", j, curCounterName);
      }
    }
  }
  else {
    printf("+ numGroups = %d\n", numGroups);
    for (int i = 0; i < numGroups; i++) {
      printf("groups[%d] = [", groups[i]);
      for (int j = 0; j < counters[i].numCounters; j++) {
        printf("%d ,", j);
      } 
      printf("]\n");
    }
  }
}

void init_functions() {
  glGetPerfMonitorGroupsAMD = (PFNGLGETPERFMONITORGROUPSAMDPROC) eglGetProcAddress("glGetPerfMonitorGroupsAMD");
  glGetPerfMonitorCountersAMD = (PFNGLGETPERFMONITORCOUNTERSAMDPROC) eglGetProcAddress("glGetPerfMonitorCountersAMD");
  glGetPerfMonitorGroupStringAMD = (PFNGLGETPERFMONITORGROUPSTRINGAMDPROC) eglGetProcAddress("glGetPerfMonitorGroupStringAMD");
  glGetPerfMonitorCounterStringAMD = (PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC) eglGetProcAddress("glGetPerfMonitorCounterStringAMD");
  glSelectPerfMonitorCountersAMD = (PFNGLSELECTPERFMONITORCOUNTERSAMDPROC) eglGetProcAddress("glSelectPerfMonitorCountersAMD");
  glBeginPerfMonitorAMD = (PFNGLBEGINPERFMONITORAMDPROC) eglGetProcAddress("glBeginPerfMonitorAMD");
  glEndPerfMonitorAMD = (PFNGLENDPERFMONITORAMDPROC) eglGetProcAddress("glEndPerfMonitorAMD");
  glGetPerfMonitorCounterDataAMD = (PFNGLGETPERFMONITORCOUNTERDATAAMDPROC) eglGetProcAddress("glGetPerfMonitorCounterDataAMD");
  glGetPerfMonitorCounterInfoAMD = (PFNGLGETPERFMONITORCOUNTERINFOAMDPROC) eglGetProcAddress("glGetPerfMonitorCounterInfoAMD");
  glGenPerfMonitorsAMD = (PFNGLGENPERFMONITORSAMDPROC) eglGetProcAddress("glGenPerfMonitorsAMD");
  glDeletePerfMonitorsAMD = (PFNGLDELETEPERFMONITORSAMDPROC) eglGetProcAddress("glDeletePerfMonitorsAMD");
}

void createTexture2DUI32(unsigned int textureId, uint32_t *data) {
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

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

void viewFrameBuffer() {
  // allocate data in memory
  printf("+ framebuffer = \n");
    uint32_t *exportData = (uint32_t*)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
    glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RED_INTEGER, GL_UNSIGNED_INT, exportData);

    for (int i = 0; i <WINDOW_WIDTH; ++i) {
        for (int j = 0; j < WINDOW_HEIGHT; ++j) {
            printf("%u ", exportData[i+j*WINDOW_WIDTH]);
        }
        printf("\n");
    }
}

void init_frame_render() {
    // generate textures
    GLuint textures[2];
    glGenTextures(2, (GLuint*)&textures);
    unsigned int texColorBuffer = textures[0];
    unsigned int texData        = textures[1];

    // allocate framebuffer with texture
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // set up framebuffer & attached texture
    createTexture2DUI32(texColorBuffer, NULL);
    // attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);
    // Render to our framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }

    // create rectangle
    createRectangle();

    Shader shader(
      "/data/local/tmp/papa/shaders/tr.vs",
      "/data/local/tmp/papa/shaders/tr.fs");

    // create data texture
    auto *data = (uint32_t*) malloc(WINDOW_WIDTH*WINDOW_HEIGHT * sizeof(uint32_t));
    memset(data, 0x01, WINDOW_WIDTH*WINDOW_HEIGHT * sizeof(uint32_t));
    createTexture2DUI32(texData, data);


    // execute
    shader.use();
    // set uniforms
    // glUniform2i(vertexRndLocation, rndX, rndY);
    glBindTexture(GL_TEXTURE_2D, texData);
}

int main( int argc, char** argv ) {
  egl_setup(); 
  init_functions();
  init_frame_render();
  getCounterNames(0);

  // enable the counters
  GLuint target_groups[2] = {9, 9}; 
  GLuint target_counters[2] = {1, 2}; 
  GLuint monitor;
  GLuint *counterData;

  glGenPerfMonitorsAMD(1, &monitor);
  glSelectPerfMonitorCountersAMD(monitor, GL_TRUE, target_groups[0], 1, &target_counters[0]);
  glSelectPerfMonitorCountersAMD(monitor, GL_TRUE, target_groups[1], 1, &target_counters[1]);

  // -----------------------------------------------------------------------------------------
  glBeginPerfMonitorAMD(monitor);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  viewFrameBuffer();

  // glDrawArrays(GL_TR, 0, 1);
  
  glEndPerfMonitorAMD(monitor);
  // -----------------------------------------------------------------------------------------

  // check if there is anything ready? 
  GLuint data_ready = 0; 
  printf("+ waiting on data to become ready.\n");
  int count = 0;
  while (! data_ready ) {
    glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AVAILABLE_AMD, sizeof(GLuint), &data_ready, NULL);
    printf("data_ready = %d\n", data_ready);
    usleep(1000000);
    if ( ++count > 4 ) { break; }
  }

  printf("+ data_ready = %u\n", data_ready);

  // read the counters
  GLuint resultSize = 12;
  glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_SIZE_AMD, sizeof(GLint), &resultSize, NULL);

  printf("+ resultSize = [%d] [%u] [%x]\n", resultSize, resultSize, resultSize);
  counterData = (GLuint*) malloc(resultSize);

  GLint bytesWritten;
  glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AMD, resultSize, counterData, &bytesWritten);

  printf("+ bytesWritten = %d\n", bytesWritten);

  printf("+++ Raw Result Data: \n");
  fflush(stdout);
  
  for (int i = 0; i < bytesWritten; i++)
    printf("%x ", *(counterData+i));
  printf("\n");
  for (int i = 0; i < bytesWritten; i++)
    printf("%u ", *(counterData+i));
  printf("\n");
  // display or log counter info
  GLsizei wordCount = 0;

  while ( (4 * wordCount) < bytesWritten )
  {

    GLuint groupId = counterData[wordCount];
    GLuint counterId = counterData[wordCount + 1];

    // Determine the counter type
    GLuint counterType;
    glGetPerfMonitorCounterInfoAMD(groupId, counterId, 
                                   GL_COUNTER_TYPE_AMD, &counterType);

    if ( counterType == GL_UNSIGNED_INT64_AMD ) {
        uint64_t counterResult = *(uint64_t*)(&counterData[wordCount + 2]);
        printf("[groupId %d][counterId %d] data => %llu\n", groupId, counterId, counterResult);

        wordCount += 4;
    }
    else if ( counterType == GL_FLOAT ) {
        float counterResult = *(float*)(&counterData[wordCount + 2]);
        printf("[groupId %d][counterId %d] data => %f\n", groupId, counterId, counterResult);

        wordCount += 3;
    } 
    // else if ( ... ) check for other counter types 
    //   (GL_UNSIGNED_INT and GL_PERCENTAGE_AMD)
  }

  /////////////////////////////////////////////////////////////////////////////////

  printf("Fooooo \n");
  printf("Fooooo \n");
  printf("Fooooo \n");
  printf("Fooooo \n");
  printf("Fooooo \n");


  glDeletePerfMonitorsAMD(1, &monitor);
  return 0;
}