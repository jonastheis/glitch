#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
// TODO: why the fuck was I forced to import this?
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdio.h>
#include <iostream>

#include "eglSetup.h"
#include "shader.cpp"


using namespace std;

#define KB 1024
#define MB (1024*1024)
#define WINDOW_WIDTH 32
#define WINDOW_HEIGHT 32


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

typedef struct 
{
  GLuint       *counterList;
  int         numCounters;
  int         maxActiveCounters;
} CounterInfo;

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
  for (int i = 0 ; i < n; i++ )
  {
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

void getCounterNames() {
  int          numGroups;
  GLuint       *groups;
  CounterInfo  *counters;
  int          i = 0;

  getGroupAndCounterList(&groups, &numGroups, &counters);

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
}

  int main( int argc, char** argv ) {
    egl_setup(); 
    init_functions();
    getCounterNames();

    // enable the counters
    int group[2] = {9, 9}; 
    GLuint counter[2] = {0, 1}; 
    GLuint monitor;
    GLuint *counterData;

    glSelectPerfMonitorCountersAMD(monitor, GL_TRUE, group[0], 1,
                                   &counter[0]);
    glSelectPerfMonitorCountersAMD(monitor, GL_TRUE, group[1], 1, 
                                   &counter[1]);

    glBeginPerfMonitorAMD(monitor);

    // RENDER FRAME HERE
    // ...
    
    glEndPerfMonitorAMD(monitor);

    // read the counters
    GLuint resultSize;
    glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_SIZE_AMD, 
                                   sizeof(GLint), &resultSize, NULL);

    counterData = (GLuint*) malloc(resultSize);

    GLsizei bytesWritten;
    glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AMD,  
                                   resultSize, counterData, &bytesWritten);

    printf("+++ ");
    for (int i = 0; i < bytesWritten; i++)
      printf("%u", *(counterData+i));

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

      if ( counterType == GL_UNSIGNED_INT64_AMD )
      {
          uint64_t counterResult = *(uint64_t*)(&counterData[wordCount + 2]);
          printf("[groupId %d][counterId %d] data => %llu\n", groupId, counterId, counterResult);

          wordCount += 4;
      }
      else if ( counterType == GL_FLOAT )
      {
          float counterResult = *(float*)(&counterData[wordCount + 2]);
          printf("[groupId %d][counterId %d] data => %f\n", groupId, counterId, counterResult);

          wordCount += 3;
      } 
      // else if ( ... ) check for other counter types 
      //   (GL_UNSIGNED_INT and GL_PERCENTAGE_AMD)
    }
 






    /////////////////////////////////////////////////////////////////////////////////


    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLfloat vertex_buffer_data[] = {
     0.5f,  0.5f, 0.0f,
   };

    // vertex_buffer id
   GLuint vertex_buffer;
    // Generate 1 buffer
   glGenBuffers(1, &vertex_buffer);
    // The following commands will talk about our 'vertex_buffer' buffer
   glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    // Give our vertices to OpenGL.
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);


    // create shaders 
   Shader shader(
    "/data/local/tmp/papa/shaders/tr.vs",
    "/data/local/tmp/papa/shaders/tr.fs");


    // draw. Could go inside a loop? 
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use(); // equivalent of glUseProgram

    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(
       0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
       1,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
       );
    glDrawArrays(GL_POINTS, 0, 1); // Starting from vertex 0; 3 vertices total -> 1 triangle
    glDisableVertexAttribArray(0);

    return 0;
  }