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
#include "counters.h"

using namespace std;

#define KB 1024
#define MB (1024 * 1024)

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

int numGroups;
GLuint *groups;
CounterInfo *counters;

void init_groups_counters(GLuint **groupsList, int *numGroups, CounterInfo **counterInfo) {
  GLint n;
  GLuint *groups;
  CounterInfo *counters;

  glGetPerfMonitorGroupsAMD(&n, 0, NULL);
  groups = (GLuint *)malloc(n * sizeof(GLuint));
  glGetPerfMonitorGroupsAMD(NULL, n, groups);
  *numGroups = n;

  *groupsList = groups;
  counters = (CounterInfo *)malloc(sizeof(CounterInfo) * n);
  for (int i = 0; i < n; i++)
  {
    glGetPerfMonitorCountersAMD(groups[i], &counters[i].numCounters,
                                &counters[i].maxActiveCounters, 0, NULL);

    counters[i].counterList = (GLuint *)malloc(counters[i].numCounters *
                                               sizeof(int));

    glGetPerfMonitorCountersAMD(groups[i], NULL, NULL,
                                counters[i].numCounters,
                                counters[i].counterList);
  }

  *counterInfo = counters;
}

void dump_counter_names(int verbose) {
  int i = 0;

  if (verbose)
  {
    for (i = 0; i < numGroups; i++)
    {
      char curGroupName[256];
      glGetPerfMonitorGroupStringAMD(groups[i], 256, NULL, curGroupName);
      printf("group[%d] = %s\n", i, curGroupName);
      for (int j = 0; j < counters[i].numCounters; j++)
      {
        char curCounterName[256];
        glGetPerfMonitorCounterStringAMD(groups[i], counters[i].counterList[j], 256, NULL, curCounterName);
        printf("\t counters[%d] = %s\n", j, curCounterName);
      }
    }
  }
  else
  {
    printf("+ numGroups = %d\n", numGroups);
    for (int i = 0; i < numGroups; i++)
    {
      printf("groups[%d] = [", groups[i]);
      for (int j = 0; j < counters[i].numCounters; j++)
      {
        printf("%d ,", j);
      }
      printf("]\n");
    }
  }
}

void init_perf_functions()
{
  glGetPerfMonitorGroupsAMD = (PFNGLGETPERFMONITORGROUPSAMDPROC)eglGetProcAddress("glGetPerfMonitorGroupsAMD");
  glGetPerfMonitorCountersAMD = (PFNGLGETPERFMONITORCOUNTERSAMDPROC)eglGetProcAddress("glGetPerfMonitorCountersAMD");
  glGetPerfMonitorGroupStringAMD = (PFNGLGETPERFMONITORGROUPSTRINGAMDPROC)eglGetProcAddress("glGetPerfMonitorGroupStringAMD");
  glGetPerfMonitorCounterStringAMD = (PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC)eglGetProcAddress("glGetPerfMonitorCounterStringAMD");
  glSelectPerfMonitorCountersAMD = (PFNGLSELECTPERFMONITORCOUNTERSAMDPROC)eglGetProcAddress("glSelectPerfMonitorCountersAMD");
  glBeginPerfMonitorAMD = (PFNGLBEGINPERFMONITORAMDPROC)eglGetProcAddress("glBeginPerfMonitorAMD");
  glEndPerfMonitorAMD = (PFNGLENDPERFMONITORAMDPROC)eglGetProcAddress("glEndPerfMonitorAMD");
  glGetPerfMonitorCounterDataAMD = (PFNGLGETPERFMONITORCOUNTERDATAAMDPROC)eglGetProcAddress("glGetPerfMonitorCounterDataAMD");
  glGetPerfMonitorCounterInfoAMD = (PFNGLGETPERFMONITORCOUNTERINFOAMDPROC)eglGetProcAddress("glGetPerfMonitorCounterInfoAMD");
  glGenPerfMonitorsAMD = (PFNGLGENPERFMONITORSAMDPROC)eglGetProcAddress("glGenPerfMonitorsAMD");
  glDeletePerfMonitorsAMD = (PFNGLDELETEPERFMONITORSAMDPROC)eglGetProcAddress("glDeletePerfMonitorsAMD");
}

void measure_counters(GLuint monitor, GLuint *target_groups, GLuint *target_counters, GLuint num_target_counters, int verbose) {
  GLuint *counterData;
  if (verbose)
    printf("+ measuring performance for %d counters.\n", num_target_counters);
  for (int i = 0; i < num_target_counters; i++)
  {
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
  while (!data_ready)
  {
    glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AVAILABLE_AMD, sizeof(GLuint), &data_ready, NULL);
    printf(".");
    usleep(1000);
  }
  printf("\n");

  // get the size of data.
  GLuint resultSize;
  glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_SIZE_AMD, sizeof(GLint), &resultSize, NULL);
  if (verbose)
    printf("+ resultSize = [%d]\n", resultSize);
  counterData = (GLuint *)malloc(resultSize);

  // read data
  GLint bytesWritten;
  glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AMD, resultSize, counterData, &bytesWritten);
  if (verbose)
    printf("+ bytesWritten = %d\n", bytesWritten);

  if (verbose)
  {
    // print raw data
    printf("++ Raw Result Data: ");
    for (int i = 0; i < bytesWritten / (sizeof(GLuint)); i++)
      printf("[%x] ", *(counterData + i));
    printf("\n");
  }

  // print parsed data
  GLsizei wordCount = 0;
  while ((4 * wordCount) < bytesWritten)
  {
    GLuint groupId = counterData[wordCount];
    GLuint counterId = counterData[wordCount + 1];

    // Determine the counter type
    GLuint counterType;
    glGetPerfMonitorCounterInfoAMD(groupId, counterId, GL_COUNTER_TYPE_AMD, &counterType);

    if (counterType == GL_UNSIGNED_INT64_AMD)
    {
      uint64_t counterResult = *(uint64_t *)(&counterData[wordCount + 2]);
      printf("+++ [groupId %d][counterId %d] data => %llu [%llu]\n", groupId, counterId, counterResult, counterResult/16);
      wordCount += 4;
    }
    else if (counterType == GL_FLOAT)
    {
      float counterResult = *(float *)(&counterData[wordCount + 2]);
      printf("+++ [groupId %d][counterId %d] data => %f\n", groupId, counterId, counterResult);
      wordCount += 3;
    }
    // else if ( ... ) check for other counter types
    //   (GL_UNSIGNED_INT and GL_PERCENTAGE_AMD)
  }
}

void perform_measurement(GLuint *target_groups, GLuint *target_counters, int num_target_counters)
{
  printf("\n\n######################################\n\n");

  // enable counters and monitor
  GLuint monitor;

  glGenPerfMonitorsAMD(1, &monitor);
  measure_counters(monitor, target_groups, target_counters, num_target_counters, 0);
  
  // cleanup
  glDeletePerfMonitorsAMD(1, &monitor);
}

void counters_init() {
  init_perf_functions();
  init_groups_counters(&groups, &numGroups, &counters);
}