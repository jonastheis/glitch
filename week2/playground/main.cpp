#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
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

int getCounterByName(char *groupName, char *counterName, GLuint *groupID, GLuint *counterID) {
    int          numGroups;
    GLuint       *groups;
    CounterInfo  *counters;
    int          i = 0;

    if (!countersInitialized) {
        getGroupAndCounterList(&groups, &numGroups, &counters);
        countersInitialized = 1;
    }

    for ( i = 0; i < numGroups; i++ ) {
       char curGroupName[256];
       glGetPerfMonitorGroupStringAMD(groups[i], 256, NULL, curGroupName);
       if (strcmp(groupName, curGroupName) == 0)
       {
           *groupID = groups[i];
           break;
       }
   }

   if ( i == numGroups )
        return -1;           // error - could not find the group name

    for ( int j = 0; j < counters[i].numCounters; j++ )
    {
        char curCounterName[256];
        
        glGetPerfMonitorCounterStringAMD(groups[i],
         counters[i].counterList[j], 
         256, NULL, curCounterName);
        if (strcmp(counterName, curCounterName) == 0)
        {
            *counterID = counters[i].counterList[j];
            return 0;
        }
    }

    return -1;           // error - could not find the counter name
}

int main( int argc, char** argv ) {
    egl_setup();



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
    "/data/exec-test/tr.vs",
    "/data/exec-test/tr.fs");


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