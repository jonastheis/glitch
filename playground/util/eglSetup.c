/*************************************************************

Function needed to setup the EGL context. This specific setup 
is reuqired to perform offscreen rendering

This file is just meant as a directive to provide you with 
the required configurations. 
You can insert this function in your program however you want. 

***************************************************************/


#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>


EGLDisplay display;
EGLSurface pBuffer;
EGLContext ctx;

void egl_setup() {
    int maj, min;
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (display == EGL_NO_DISPLAY) {
        printf("EGL_NO_DISPLAY");
        exit(-1);
    }

    if (!eglInitialize(display, &maj, &min)) {
        printf("eglinfo: eglInitialize failed\n");
        exit(-1);
    }

    printf("EGL v%i.%i initialized\n", maj, min);

    EGLint attribs[] =
    {
        EGL_NONE, 
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        
    };
    
    EGLint pBuffer_attribs[] =
    {
        EGL_WIDTH, 128,
        EGL_HEIGHT, 1024,
        EGL_TEXTURE_TARGET, EGL_NO_TEXTURE,
        EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE,
        EGL_NONE
    };

    EGLint ctx_attribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    EGLint num_configs;
    EGLConfig config;

    if (!eglChooseConfig(display, attribs, &config, 1, &num_configs) || (num_configs < 1))
    {
        printf("Could not find config for %s (perhaps this API is unsupported?)\n", "GLES3");
    }
    EGLint vid;
    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &vid))
    {
        printf("Could not get native visual ID from chosen config\n");
    }
    eglBindAPI(EGL_OPENGL_ES_API);

    pBuffer = eglCreatePbufferSurface(display, config, pBuffer_attribs);
    ctx = eglCreateContext(display, config, EGL_NO_CONTEXT, ctx_attribs);
    if (ctx == EGL_NO_CONTEXT) {
        printf("Context not created!\n");
    }

    if(!eglMakeCurrent(display, pBuffer, pBuffer, ctx))
    { 
        printf("eglMakeCurrent() failed\n");

    }

}