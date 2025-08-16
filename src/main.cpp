//#include <fstream>
#include <iostream>
//#include <sstream>
//#include <stdexcept>
// #include <stdexcept>
#include <threads.h>
#include <vector>
#include <chrono>
#include <cmath>
#include <cstdlib>

#include "common.hpp"
#include "renderer.hpp"
// #include "opencl_utils.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>



int main() {

    // Getting kernels
    auto kernelSrc = loadKernelSource("../kernels/fill_points.cl");
    const char* kernelOpenCL = kernelSrc.c_str();

    // Getting shaders
    auto vertexSrc = loadKernelSource("../shaders/vertex.glsl");
    const char* vertexShader = vertexSrc.c_str();

    auto fragmentSrc = loadKernelSource("../shaders/fragment.glsl");
    const char* fragmentShader = fragmentSrc.c_str();



    // Inicialize GLFW e GLEW /////////////////////////////////////////////////
    checkOrExit(glfwInit(), "Failed while initializing GLFW");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    auto* win = glfwCreateWindow(960, 720, "OpenCL + OpenGL CPU copy", nullptr, nullptr);
    checkOrExit(win != nullptr, "Failed while creating window");
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1); // vsync
    checkOrExit(glewInit() == GLEW_OK, "Failed while initializing GLEW");

    // Buffers OpenGL
    constexpr int N = 120000;
    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, N * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    GLuint glProgram = makeProgram(vertexShader, fragmentShader);

    // Inicializing OpenCL ////////////////////////////////////////////////////
    cl_int err = 0;
    cl_uint numPlatforms = 0;
    clGetPlatformIDs(0, nullptr, &numPlatforms);
    std::vector<cl_platform_id> plats(numPlatforms);
    clGetPlatformIDs(numPlatforms, plats.data(), nullptr);

    checkOrExit(numPlatforms > 0, "No OpenCL platform encountered");
    cl_platform_id chosenPlat = plats[0]; // takes the first platform
    cl_uint numDevices = 0;
    clGetDeviceIDs(chosenPlat, CL_DEVICE_TYPE_CPU, 0, nullptr, &numDevices);
    checkOrExit(numDevices > 0, "No OpenCL GPU device found");
    std::vector<cl_device_id> devs(numDevices);
    clGetDeviceIDs(chosenPlat, CL_DEVICE_TYPE_CPU, numDevices, devs.data(), nullptr);
    auto dev = devs[0];

    auto clContext = clCreateContext(nullptr,
                                                1,
                                                &dev,
                                                nullptr,
                                                nullptr,
                                                &err);
    checkCLError(err, "clCreateContext");

    auto clQueue = clCreateCommandQueueWithProperties(clContext, dev, nullptr, &err);
    checkCLError(err, "clCreateCommandQueueWithProperties");


    // Building Kernel ////////////////////////////////////////////////////////
    auto clProgram = clCreateProgramWithSource(clContext, 1, &kernelOpenCL, nullptr, &err);
    checkCLError(err, "clCreateProgramWithSource");
    err = clBuildProgram(clProgram, 1, &dev, "", nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t logSize = 0;
        clGetProgramBuildInfo(clProgram, dev, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::string log(logSize, '\0');
        clGetProgramBuildInfo(clProgram, dev, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
        std::cerr << "[OpenCL build log]\n" << log << std::endl;
        std::exit(EXIT_FAILURE);
    }
    auto clKernel = clCreateKernel(clProgram, "fill_points", &err);
    checkCLError(err, "clCreateKernel");


    // Buffer OpenCL CPU //////////////////////////////////////////////////////
    std::vector<cl_float2> cpu_buffer(N);
    auto t0 = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        auto t = std::chrono::duration<float>(std::chrono::steady_clock::now() - t0).count();

        // Executes kernel in CPU buffer
        cl_mem clbuf = clCreateBuffer(clContext, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                      sizeof(cl_float2)*N, cpu_buffer.data(), &err);
        checkCLError(err, "clCreateBuffer");

        clSetKernelArg(clKernel, 0, sizeof(cl_mem), &clbuf);
        clSetKernelArg(clKernel, 1, sizeof(int), &N);
        clSetKernelArg(clKernel, 2, sizeof(float), &t);
        size_t gsz = N;
        clEnqueueNDRangeKernel(clQueue,
                               clKernel, 
                               1,
                               nullptr,
                               &gsz,
                               nullptr,
                               0,
                               nullptr,
                               nullptr);
        clFinish(clQueue);

        // Copying data for OpenGL
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*N*2, cpu_buffer.data());

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(glProgram);
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, N);
        glfwSwapBuffers(win);

        clReleaseMemObject(clbuf);
    }
}
