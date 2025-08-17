//#include <iostream>
//#include <vector>
#include <chrono>
#include <cmath>
#include <cstdlib>

#include "common.hpp"
#include "renderer.hpp"
#include "compute.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>


int main() {
    // Getting shaders
    auto vertexSrc = loadKernelSource("../shaders/vertex_1.glsl");
    const char* vertexShader = vertexSrc.c_str();

    auto fragmentSrc = loadKernelSource("../shaders/fragment_1.glsl");
    const char* fragmentShader = fragmentSrc.c_str();

    // Inicialize GLFW e GLEW /////////////////////////////////////////////////
    checkOrExit(glfwInit(), "Failed while initializing GLFW");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    auto* win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenCL + OpenGL GPU copy", nullptr, nullptr);
    checkOrExit(win != nullptr, "Failed while creating window");
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1); // vsync
    checkOrExit(glewInit() == GLEW_OK, "Failed while initializing GLEW");


    // opencl compute class
    OpenCLCompute compute(DEFAULT_POINT_COUNT);

    // Buffers OpenGL
    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, DEFAULT_POINT_COUNT * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    GLuint glProgram = makeProgram(vertexShader, fragmentShader);

    // Buffer OpenCL CPU //////////////////////////////////////////////////////
    
    bool draw_lines = false;
    std::vector<cl_float2> tempBuffer (compute.getPointCount());
    auto t0 = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        auto t = std::chrono::duration<float>(std::chrono::steady_clock::now() - t0).count();

        // Updating buffers
        compute.updateBufferData(t, &tempBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * compute.getPointCount() * 2, tempBuffer.data());

        // drawing calls
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(glProgram);
        glBindVertexArray(vao);
        if (draw_lines)
        {
            glLineWidth(2.0f);
            glDrawArrays(GL_LINE_STRIP, 0, compute.getPointCount());
        }
        else
        {
            glDrawArrays(GL_POINTS, 0, compute.getPointCount());
        }
        
        glfwSwapBuffers(win);
    }
    // cleaning buffer
    if (vbo) glDeleteBuffers(1, &vbo);

    return 0;
}
