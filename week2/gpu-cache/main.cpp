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
#define WINDOW_WIDTH 5
#define WINDOW_HEIGHT 5

#define TEXTURE_WIDTH 256 
#define TEXTURE_HEIGHT 256


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
Shader shader;

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

void createTexture2DRGBA(unsigned int textureId, uint32_t *data, GLuint width, GLuint height) {
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void init_frame_render() {
    // generate textures
    GLuint textures[1];
    glGenTextures(1, (GLuint*)&textures);
    unsigned int texData = textures[0];

    shader = Shader(
      "/data/local/tmp/papa/shaders/tr.vs",
      "/data/local/tmp/papa/shaders/tr.fs");

    // create data texture
    auto *data = (uint32_t*) malloc(TEXTURE_WIDTH*TEXTURE_HEIGHT * sizeof(uint32_t));
    memset(data, 0x01, TEXTURE_HEIGHT*TEXTURE_WIDTH * sizeof(uint32_t));
    createTexture2DRGBA(texData, data, TEXTURE_WIDTH, TEXTURE_HEIGHT);

    // execute
    shader.use();

    // bind texture
    glBindTexture(GL_TEXTURE_2D, texData);
}

void measure_counters (GLuint monitor, GLuint* target_groups, GLuint* target_counters, GLuint num_target_counters, int verbose) {
  GLuint *counterData;
  if (verbose) printf("+ measuring performance for %d counters.\n", num_target_counters);
  for (int i = 0; i < num_target_counters; i++) {
    glSelectPerfMonitorCountersAMD(monitor, GL_TRUE, target_groups[i], 1, &target_counters[i]);  
  }

  glBeginPerfMonitorAMD(monitor);

  // draw only 1 point -> shaders get executed just once
  glDrawArrays(GL_POINTS, 0, 1);

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
  if (verbose) printf("+ resultSize = [%d]\n", resultSize);
  counterData = (GLuint*) malloc(resultSize);

  // read data
  GLint bytesWritten;
  glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AMD, resultSize, counterData, &bytesWritten);
  if (verbose) printf("+ bytesWritten = %d\n", bytesWritten);

  if (verbose) {
    // print raw data
    printf("++ Raw Result Data: ");
    for (int i = 0; i < bytesWritten/(sizeof(GLuint)); i++)
      printf("[%x] ", *(counterData+i));
    printf("\n");
  }

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

void perform_measurement(GLuint* target_groups, GLuint* target_counters, int num_target_counters, int texelLimit) {
  printf("\n\n######################################\n\n");
  
  printf("Texels: %d, Bytes: %d\n", texelLimit, texelLimit*4);
  // enable counters and monitor
  GLuint monitor;
  glGenPerfMonitorsAMD(1, &monitor);

  shader.setInt("max", texelLimit);
  measure_counters(monitor, target_groups, target_counters, num_target_counters, 0);

  // cleanup
  glDeletePerfMonitorsAMD(1, &monitor);
}

int main( int argc, char** argv ) {
  egl_setup(); 

  // initialize counters and functions 
  init_perf_functions();
  init_groups_counters(&groups, &numGroups, &counters);

  // dump the list of counters. use `1` as parameter to get full names
  // dump_counter_names(1);

  // initialize the opengl render part including texture
  init_frame_render();

  // ----------------------------- L1 -----------------------------
  GLuint group_L1[]   = {9, 9, 9, 9}; 
  GLuint counter_L1[] = {1, 2, 5, 6};
  GLuint num_target_counters = 2;

  // measure size of L1 -> 64*4B steps
  for (int i=64; i<=384; i+=64) {
    perform_measurement(group_L1, counter_L1, num_target_counters, i);
  }
  // investigate a bit more around 256 Texels == 1KB
  for (int i=256; i<=288; i+=4) {
    perform_measurement(group_L1, counter_L1, num_target_counters, i);
  }


  // ----------------------------- UCHE -----------------------------
  // GLuint group_UCHE[]   = {8, 9, 9, 8}; 
  // GLuint counter_UCHE[] = {0, 1, 2, 0};
  // GLuint num_target_counters = 3; 

  // // measure size of UCHE -> 1024*4B== 4KB steps
  // for (int i=1024; i<=10*KB; i+=1024) {
  //   perform_measurement(group_UCHE, counter_UCHE, num_target_counters, i);
  // }
  // // investigate a bit more around 8*KB texels
  // for (int i=8*KB; i<=9*KB; i+=256) {
  //   perform_measurement(group_UCHE, counter_UCHE, num_target_counters, i);
  // }

  return 0;
}
