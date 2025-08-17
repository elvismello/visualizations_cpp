#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <cstdlib>

#include "common.hpp"
#include "renderer.hpp"
#include "compute.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>


int main() {

    // Getting kernels
    // auto kernelSrc = loadKernelSource("../kernels/fill_points.cl");
    // const char* kernelOpenCL = kernelSrc.c_str();

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


    // compute class

    OpenCLCompute compute(N);
    compute.initSharedBuffer();

    // Buffers OpenGL
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, compute.getVBO());
    // glBufferData(GL_ARRAY_BUFFER, N * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    GLuint glProgram = makeProgram(vertexShader, fragmentShader);

    // Buffer OpenCL CPU //////////////////////////////////////////////////////
    bool draw_lines = false;
    auto t0 = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        auto t = std::chrono::duration<float>(std::chrono::steady_clock::now() - t0).count();

        // Updating buffers
        compute.updateBufferData(t);

        // drawing calls
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(glProgram);
        glBindVertexArray(vao);
        if (draw_lines)
        {
            glLineWidth(2.0f);
            glDrawArrays(GL_LINE_STRIP, 0, N);
        }
        else
        {
            glDrawArrays(GL_POINTS, 0, N);
        }
        
        glfwSwapBuffers(win);
        //clEnqueueUnmapMemObject(clQueue, clbuf, mapped, 0, nullptr, nullptr);
    }
    // cleaning buffer
    
    return 0;
}
