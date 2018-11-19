#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <iostream>
#include "shader.cpp"

using namespace std;

#define WINDOW_WIDTH 32
#define WINDOW_HEIGHT 32

GLFWwindow* window;

void triangle() {
    // prepare triangle
    float vertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f,  0.5f, 0.0f
    };

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // 1. bind Vertex Array Object
    glBindVertexArray(VAO);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 3. then set our vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    Shader shader(
        "/Users/jonastheis/projects/vu/hwsec/glitch/playground/source/shaders/tr.vs",
        "/Users/jonastheis/projects/vu/hwsec/glitch/playground/source/shaders/tr.fs");
    while(!glfwWindowShouldClose(window))
    {
        // possibly evaluate inputs

        // render commands
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // check and call events and swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

}

void createRectangle(unsigned int *VAO) {
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
    unsigned int VBO, EBO;
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void rectangle() {
    unsigned int VAO;
    createRectangle(&VAO);

    Shader shader(
            "/Users/jonastheis/projects/vu/hwsec/glitch/playground/source/shaders/tr.vs",
            "/Users/jonastheis/projects/vu/hwsec/glitch/playground/source/shaders/tr.fs");
    while(!glfwWindowShouldClose(window))
    {
        // possibly evaluate inputs

        // render commands
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // check and call events and swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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

void debugger() {
    uint32_t rndX = (uint32_t)(rand() % 32);
    uint32_t rndY = (uint32_t)(rand() % 32);
    uint32_t result = rndX + rndY;
    printf("rnd(%u, %u), result=%u\n", rndX, rndY, result);


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
    unsigned int VAO;
    createRectangle(&VAO);


    // create data texture and pass as uniform to shader
    unsigned int texData = textures[1];
    uint32_t *data = (uint32_t*) malloc(WINDOW_WIDTH*WINDOW_HEIGHT * sizeof(uint32_t));
    memset(data, 0x41, WINDOW_WIDTH*WINDOW_HEIGHT * sizeof(uint32_t));
    createTexture2DUI32(texData, data);
    // write values to texture
    glBindTexture(GL_TEXTURE_2D, texData);
    glTexSubImage2D(GL_TEXTURE_2D, 0, rndX, rndY, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &result);
    glBindTexture(GL_TEXTURE_2D, 0);


    Shader shader(
            "/Users/jonastheis/projects/vu/hwsec/glitch/playground/source/shaders/tr.vs",
            "/Users/jonastheis/projects/vu/hwsec/glitch/playground/source/shaders/tr.fs");

    // initialize (rndX, rndY) as uniform
    int vertexRndLocation = glGetUniformLocation(shader.ID, "rnd");

    // execute
    shader.use();
    // set uniforms
    glUniform2ui(vertexRndLocation, rndX, rndY);
    glBindTexture(GL_TEXTURE_2D, texData);


    // draw rectangle
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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

int main(int argc, char* argv[]) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    debugger();


    glfwTerminate();
    return 0;
}
