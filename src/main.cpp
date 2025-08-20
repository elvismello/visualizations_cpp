//#include <iostream>
//#include <vector>
#include <chrono>
#include <cmath>
#include <cstdlib>

#include <iostream>
#include <print>

#include "common.hpp"
#include "renderer.hpp"
#include "compute.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>


int main() {
    std::string kernelPath = "../kernels/fill_points.cl";

    // Getting shaders
    auto vertexSrc = loadKernelSource("../shaders/vertex_1.glsl");
    const char* vertexShader = vertexSrc.c_str();

    auto fragmentSrc = loadKernelSource("../shaders/fragment_1.glsl");
    const char* fragmentShader = fragmentSrc.c_str();

    // Inicialize GLFW e GLEW /////////////////////////////////////////////////
    checkOrExit(glfwInit(), "Failed while initializing GLFW");


    auto* win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenCL + OpenGL GPU copy", nullptr, nullptr);
    checkOrExit(win != nullptr, "Failed while creating window");
    glfwMakeContextCurrent(win);
    glfwSwapInterval(0); // vsync
    checkOrExit(glewInit() == GLEW_OK, "Failed while initializing GLEW");


    // opencl compute class
    OpenCLCompute compute(DEFAULT_POINT_COUNT, kernelPath);

    // buffers opengl
    GLuint vao = 0, vbo = 0;

    // vertex array object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // vertex buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, DEFAULT_POINT_COUNT * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    GLuint glProgram = makeProgram(vertexShader, fragmentShader);
    glUseProgram(glProgram);

    // Buffer OpenCL CPU //////////////////////////////////////////////////////
    
    bool draw_lines = false;
    std::vector<cl_float2> tempBuffer (compute.getPointCount());
    auto initialTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration<float>(std::chrono::steady_clock::now() - initialTime).count();
    auto lastTime = initialTime;
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        auto currentTime = std::chrono::steady_clock::now();
        auto deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = std::chrono::steady_clock::now();
        
        elapsedTime = std::chrono::duration<float>(currentTime - initialTime).count();

        std::print("{:6.4f}  {:6.4f}  {:6.4}\r", elapsedTime, deltaTime, 1/deltaTime);
        std::fflush(stdout);

        // Updating buffers
        compute.updateBufferData(elapsedTime, &tempBuffer);
        //glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * compute.getPointCount() * 2, tempBuffer.data());

        // drawing calls
        glClear(GL_COLOR_BUFFER_BIT);

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
