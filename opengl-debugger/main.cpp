#include <stdio.h>
#include <iostream>

// #include <glad.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "shader.cpp"

using namespace std;

#define WINDOW_WIDTH 32
#define WINDOW_HEIGHT 32

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

    printf("+ EGL v%i.%i initialized.\n", maj, min);

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

/**
 * Create 2D texture with necessary parameters
 * @param textureId the id of the texture
 * @param data the data for the texture (can be NULL)
 */
void createTexture2DUI32(unsigned int textureId, uint32_t *data) {
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * View every pixel of the framebuffer (contains uint32_t values)
 */
void viewFrameBuffer() {
    // allocate data in memory
    uint32_t *exportData = (uint32_t*)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
    glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RED_INTEGER, GL_UNSIGNED_INT, exportData);

    for (int i = 0; i <WINDOW_WIDTH; ++i) {
        for (int j = 0; j < WINDOW_HEIGHT; ++j) {
            printf("%u ", exportData[i+j*WINDOW_WIDTH]);
        }
        printf("\n");
    }
}

void debugger() {
    // create random values to be populated in framebuffer via shader
    uint32_t rndX = (uint32_t)(rand() % 32);
    uint32_t rndY = (uint32_t)(rand() % 32);
    uint32_t result = rndX + rndY;
    printf("rnd(%u, %u), result=%u\n", rndX, rndY, result);


    /**
     * Overall view: Create a xy-plane covering rectangle in framebuffer to write results from the shader to.
     * 1. Initialize framebuffer with texture (to make it possible to read from it after GPU computation)
     * 2. Create xy-plane rectangle
     * 3. Compile shaders and create shader program
     * 4. Set uniforms (data texture and random coordinates)
     * 5. Draw
     * 6. Read from framebuffer
     */
    // generate textures
    GLuint textures[2];
    glGenTextures(2, (GLuint*)&textures);


    // allocate framebuffer with texture
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // set up framebuffer & attached texture
    unsigned int texColorBuffer = textures[0];
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

    // compile shaders and initialize shader program
    Shader shader(
            "/data/local/tmp/papa/shaders/tr.vs",
            "/data/local/tmp/papa/shaders/tr.fs");

    // initialize (rndX, rndY) as uniform
    int vertexRndLocation = glGetUniformLocation(shader.ID, "rnd");

    // create data texture
    unsigned int texData = textures[1];
    uint32_t *data = (uint32_t*) malloc(WINDOW_WIDTH*WINDOW_HEIGHT * sizeof(uint32_t));
    memset(data, 0x41, WINDOW_WIDTH*WINDOW_HEIGHT * sizeof(uint32_t));
    createTexture2DUI32(texData, data);
    // write special values to texture
    glBindTexture(GL_TEXTURE_2D, texData);
    glTexSubImage2D(GL_TEXTURE_2D, 0, rndX, rndY, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &result);
    glBindTexture(GL_TEXTURE_2D, 0);

    // execute
    shader.use();
    // set uniforms
    glUniform2ui(vertexRndLocation, rndX, rndY);
    glBindTexture(GL_TEXTURE_2D, texData);


    // draw rectangle
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    viewFrameBuffer();
}

int main() {
    egl_setup();
    debugger();
    return 0;
}