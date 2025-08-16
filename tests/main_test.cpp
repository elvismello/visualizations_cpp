#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <threads.h>
#include <vector>
#include <chrono>
#include <cmath>
#include <cstdlib>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define CL_TARGET_OPENCL_VERSION 200
#include <CL/cl.h>

// Util
template<typename T>
void check_or_exit(T condition, const char* msg) {
    if (!condition) {
        std::cerr << "[error] " << msg << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

inline void checkCLError(cl_int err, const char* where) {
    if (err != CL_SUCCESS) {
        std::cerr << "[OpenCL erro] " << where << " -> " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

// Shaders
const char* kVS = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
void main() {
    gl_PointSize = 2.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

const char* kFS = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.15, 0.8, 1.0, 1.0);
}
)";

// Kernel OpenCL
std::string loadKernelSource(const std::string& path){
    std::ifstream file(path);
    if (!file.is_open()){
        throw std::runtime_error("Couldn't load " + path);
    }
    std::ostringstream string_buffer;
    string_buffer << file.rdbuf();
    return string_buffer.str();
}



// Shader Functions
GLuint makeShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(s, len, nullptr, log.data());
        std::cerr << "[GL] shader compile error:\n" << log << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return s;
}

GLuint makeProgram(const char* vs, const char* fs) {
    GLuint v = makeShader(GL_VERTEX_SHADER, vs);
    GLuint f = makeShader(GL_FRAGMENT_SHADER, fs);
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(p, len, nullptr, log.data());
        std::cerr << "[GL] program link error:\n" << log << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return p;
}

int main() {

    // Getting kernels
    auto src = loadKernelSource("./fill_points.cl");
    const char* kOpenCL = src.c_str();



    // Inicialize GLFW e GLEW
    check_or_exit(glfwInit(), "Failed while initializing GLFW");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    auto* win = glfwCreateWindow(960, 720, "OpenCL + OpenGL CPU copy", nullptr, nullptr);
    check_or_exit(win != nullptr, "Failed while creating window");
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1); // vsync
    check_or_exit(glewInit() == GLEW_OK, "Failed while initializing GLEW");

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

    GLuint prog = makeProgram(kVS, kFS);

    // Inicializing OpenCL
    cl_int err = 0;
    cl_uint numPlatforms = 0;
    clGetPlatformIDs(0, nullptr, &numPlatforms);
    std::vector<cl_platform_id> plats(numPlatforms);
    clGetPlatformIDs(numPlatforms, plats.data(), nullptr);

    check_or_exit(numPlatforms > 0, "No OpenCL platform encountered");
    cl_platform_id chosenPlat = plats[0]; // takes the first platform
    cl_uint numDevices = 0;
    clGetDeviceIDs(chosenPlat, CL_DEVICE_TYPE_CPU, 0, nullptr, &numDevices);
    check_or_exit(numDevices > 0, "No OpenCL GPU device found");
    std::vector<cl_device_id> devs(numDevices);
    clGetDeviceIDs(chosenPlat, CL_DEVICE_TYPE_CPU, numDevices, devs.data(), nullptr);
    auto dev = devs[0];

    auto clctx = clCreateContext(nullptr, 1, &dev, nullptr, nullptr, &err);
    checkCLError(err, "clCreateContext");

    auto queue = clCreateCommandQueueWithProperties(clctx, dev, nullptr, &err);
    checkCLError(err, "clCreateCommandQueueWithProperties");


    // Building Kernel
    auto prg = clCreateProgramWithSource(clctx, 1, &kOpenCL, nullptr, &err);
    checkCLError(err, "clCreateProgramWithSource");
    err = clBuildProgram(prg, 1, &dev, "", nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t logSize = 0;
        clGetProgramBuildInfo(prg, dev, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::string log(logSize, '\0');
        clGetProgramBuildInfo(prg, dev, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
        std::cerr << "[OpenCL build log]\n" << log << std::endl;
        std::exit(EXIT_FAILURE);
    }
    auto krn = clCreateKernel(prg, "fill_points", &err);
    checkCLError(err, "clCreateKernel");


    // Buffer OpenCL CPU
    std::vector<cl_float2> cpu_buffer(N);
    auto t0 = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        auto t = std::chrono::duration<float>(std::chrono::steady_clock::now() - t0).count();

        // Executes kernel in CPU buffer
        cl_mem clbuf = clCreateBuffer(clctx, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                      sizeof(cl_float2)*N, cpu_buffer.data(), &err);
        checkCLError(err, "clCreateBuffer");

        clSetKernelArg(krn, 0, sizeof(cl_mem), &clbuf);
        clSetKernelArg(krn, 1, sizeof(int), &N);
        clSetKernelArg(krn, 2, sizeof(float), &t);
        size_t gsz = N;
        clEnqueueNDRangeKernel(queue, krn, 1, nullptr, &gsz, nullptr, 0, nullptr, nullptr);
        clFinish(queue);

        // Copying data for OpenGL
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*N*2, cpu_buffer.data());

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, N);
        glfwSwapBuffers(win);

        clReleaseMemObject(clbuf);
    }
}
