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
#define WINDOW_WIDTH 1
#define WINDOW_HEIGHT 1

#define TEXTURE_WIDTH 128
#define TEXTURE_HEIGHT 128


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

void init_groups_counters(GLuint **groupsList, int *numGroups, CounterInfo **counterInfo) {
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

void dump_counter_names(int verbose) {
  int          i = 0;

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

void init_perf_functions() {
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
    auto *data = (uint32_t*) malloc(TEXTURE_WIDTH*TEXTURE_HEIGHT * sizeof(uint32_t));
    memset(data, 0x01, TEXTURE_HEIGHT*TEXTURE_WIDTH * sizeof(uint32_t));
    createTexture2DUI32(texData, data);


    // execute
    shader.use();
    // set uniforms
    // glUniform2i(vertexRndLocation, rndX, rndY);
    glBindTexture(GL_TEXTURE_2D, texData);
}

void measure_counters (GLuint monitor, GLuint* target_groups, GLuint* target_counters, GLuint num_target_counters) {
  GLuint *counterData;
  printf("+ measuring performance for %d counters for [%dx%d = %d] shader calls \n", num_target_counters, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_HEIGHT*WINDOW_WIDTH);
  for (int i = 0; i < num_target_counters; i++) {
    glSelectPerfMonitorCountersAMD(monitor, GL_TRUE, target_groups[i], 1, &target_counters[i]);  
  }

  glBeginPerfMonitorAMD(monitor);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  viewFrameBuffer();
  // glDrawArrays(GL_TR, 0, 1);
  glEndPerfMonitorAMD(monitor);

  // check if there is anything ready? 
  GLuint data_ready = 0; 
  printf("+ waiting on data to become ready");
  int count = 0;
  while ( !data_ready ) {
    glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AVAILABLE_AMD, sizeof(GLuint), &data_ready, NULL);
    printf(".");
    usleep(1000);
  }
  printf("\n");

  // get the size of data.
  GLuint resultSize;
  glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_SIZE_AMD, sizeof(GLint), &resultSize, NULL);
  printf("+ resultSize = [%d]\n", resultSize);
  counterData = (GLuint*) malloc(resultSize);

  // read data
  GLint bytesWritten;
  glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AMD, resultSize, counterData, &bytesWritten);
  printf("+ bytesWritten = %d\n", bytesWritten);

  // print raw data
  printf("++ Raw Result Data: ");
  for (int i = 0; i < bytesWritten/(sizeof(GLuint)); i++)
    printf("[%x] ", *(counterData+i));
  printf("\n");

  // print parsed data
  GLsizei wordCount = 0;
  while ( (4 * wordCount) < bytesWritten )
  {
    GLuint groupId = counterData[wordCount];
    GLuint counterId = counterData[wordCount + 1];

    // Determine the counter type
    GLuint counterType;
    glGetPerfMonitorCounterInfoAMD(groupId, counterId, GL_COUNTER_TYPE_AMD, &counterType);

    if ( counterType == GL_UNSIGNED_INT64_AMD ) {
        uint64_t counterResult = *(uint64_t*)(&counterData[wordCount + 2]);
        printf("+++ [groupId %d][counterId %d] data => %llu\n", groupId, counterId, counterResult);
        wordCount += 4;
    }
    else if ( counterType == GL_FLOAT ) {
        float counterResult = *(float*)(&counterData[wordCount + 2]);
        printf("+++ [groupId %d][counterId %d] data => %f\n", groupId, counterId, counterResult);
        wordCount += 3;
    } 
    // else if ( ... ) check for other counter types 
    //   (GL_UNSIGNED_INT and GL_PERCENTAGE_AMD)
  }
}

int main( int argc, char** argv ) {
  egl_setup(); 

  // initialize counters and functions 
  init_perf_functions();
  init_groups_counters(&groups, &numGroups, &counters);

  // dump the list of counters. use `1` as parameter to get full names
  dump_counter_names(1);

  // initialize the opengl render part. creates 5x5 rectangle. 
  init_frame_render();

  // determine the counters to be used 
  /* 
  group[9] = TP
       counters[0] = TPL1_TPPERF_L1_REQUESTS
       counters[1] = TPL1_TPPERF_TP0_L1_REQUESTS
       counters[2] = TPL1_TPPERF_TP0_L1_MISSES
       counters[3] = TPL1_TPPERF_TP1_L1_REQUESTS
       counters[4] = TPL1_TPPERF_TP1_L1_MISSES
       counters[5] = TPL1_TPPERF_TP2_L1_REQUESTS
       counters[6] = TPL1_TPPERF_TP2_L1_MISSES
       counters[7] = TPL1_TPPERF_TP3_L1_REQUESTS
       counters[8] = TPL1_TPPERF_TP3_L1_MISSES
  */
  GLuint target_groups[]   = {9, 9, 9, 9, 9, 9}; 
  GLuint target_counters[] = {3, 4, 5, 6, 7, 8}; 
  GLuint num_target_counters = 6;

  // enable ounters and monitor
  GLuint monitor;
  glGenPerfMonitorsAMD(1, &monitor);

  measure_counters(monitor, target_groups, target_counters, num_target_counters);

  // cleanup
  glDeletePerfMonitorsAMD(1, &monitor);
  return 0;
}