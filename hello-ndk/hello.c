#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
//#include <kgsl_debugfs.h>

#include <stdio.h>
#include <stdlib.h>
#include "time.h"

#include "eglSetup.h"


// import global variables from setup phase
extern EGLDisplay display;
extern EGLSurface pBuffer;
extern EGLContext ctx;

void check_shader_error(int shader) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("ERROR::SHADER::COMPILATION_FAILED [%s]\n", infoLog);
    }
}

void check_program_error(int p) {
    int success;
    char infoLog[512];
    glGetProgramiv(p, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(p, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED [%s]\n", infoLog);
    }
}
void triangle() {
    // keep an eye on version number, also es must be used instead of core.
    // precision is mandatory in android
    const char *vertexShaderSource = "#version 300 es\n"
                                     "layout (location = 0) in vec3 aPos;\n"
                                     "void main()\n"
                                     "{\n"
                                     "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                     "}\0";
    const char *fragmentShaderSource = "#version 300 es\n"
                                       "precision mediump float;\n"
                                       "out vec4 FragColor;\n"
                                       "void main()\n"
                                       "{\n"
                                       "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                       "}\n\0";

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    check_shader_error(vertexShader);

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    check_shader_error(fragmentShader);

    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    check_program_error(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // set up vertex data
    float vertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    time_t start, end;
    double elapsed;  // seconds
    start = time(NULL);
    int terminate = 1;
    while(terminate)
    {
        end = time(NULL);
        elapsed = difftime(end, start);
        if (elapsed >= 5.0 )
            terminate = 0;

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time

        // swap buffers
        eglSwapBuffers(display, pBuffer);
    }

}

int main( int argc, char** argv ) {
    printf("Hello NDK!\n");

    egl_setup();
    printf("+ EGL_SETUP Success\n");

    const GLubyte *msg = glGetString(GL_VERSION);
    printf("+ GLVersion: %s\n", msg);

    msg = glGetString(GL_VENDOR);
    printf("+ GLVendor: %s\n", msg);

    msg = glGetString(GL_RENDERER);
    printf("+ GL_RENDERER: %s\n", msg);

    msg = glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("+ GL_SHADING_LANGUAGE_VERSION: %s\n", msg);

    msg = glGetString(GL_EXTENSIONS);
    printf("+ GL_EXTENSIONS: %s\n", msg);

    triangle();

    return 0;
}