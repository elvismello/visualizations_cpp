#include "compute.hpp"
#include "renderer.hpp"

#include <GL/glx.h>
#include <cstddef>
#include <iostream>
#include <vector>
#include <print>

#include <CL/cl.h>
#include <CL/cl_platform.h>
#include "CL/cl_gl.h"
#include <GLFW/glfw3.h>
//#include <GL/glx.h>



OpenCLCompute::OpenCLCompute(size_t numPoints) : pointCount(numPoints)
{
    // finding platform
    cl_uint numPlatforms = 0;
    clGetPlatformIDs(0, nullptr, &numPlatforms);
    std::vector<cl_platform_id> allPlatforms(numPlatforms);
    clGetPlatformIDs(numPlatforms, allPlatforms.data(), nullptr);
    
    checkOrExit(numPlatforms > 0, "No OpenCL platform encountered");
    
    // finding device
    cl_uint numDevices = 0;
    clGetDeviceIDs(allPlatforms[0], CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
    std::vector<cl_device_id> allDevices(numDevices);
    clGetDeviceIDs(allPlatforms[0], CL_DEVICE_TYPE_GPU, numDevices, allDevices.data(), nullptr);
    
    checkOrExit(numDevices > 0, "No OpenCL device encountered");
    device = allDevices[0]; // possible implementation of an automatic device selection
    
    // creating context
    cl_int err = 0;

    try{
        cl_context_properties properties[] = {
            CL_CONTEXT_PLATFORM,
            (cl_context_properties)allPlatforms[0],
            CL_GL_CONTEXT_KHR,
            (cl_context_properties)glXGetCurrentContext(),
            CL_WGL_HDC_KHR,
            (cl_context_properties)glXGetCurrentDisplay(),
            0
        };

        context = clCreateContext(properties, 1, &device, nullptr, nullptr, &err);
        //checkCLError(err, "clCreateContext");
        if (err == CL_SUCCESS)
        {
            useSharedBuffer = true;
        }
        else
        {
            checkCLError(err, "clCreateContext");
        }

    }
    catch (...)
    {
        std::print("Using regular OpenCL (no shared buffer)...\n");
        context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
        checkCLError(err, "clCreateContext");
        useSharedBuffer = false;
    }
    // creating queue
    queue = clCreateCommandQueueWithProperties(context, device, nullptr, &err);
    checkCLError(err, "clCreateCommandQueueWithProperties");

    // loading and compiling kernel
    auto kernelSrc = loadKernelSource("../kernels/fill_points.cl");
    const char* kernelCode = kernelSrc.c_str();

    program = clCreateProgramWithSource(context, 1, &kernelCode, nullptr, &err);
    checkCLError(err, "clCreateProgramWithSource");

    err = clBuildProgram(program, 1, &device, "", nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t logSize = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::string log(logSize, '\0');
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
        std::cerr << "[OpenCL build log]\n" << log << std::endl;
        throw std::runtime_error("OpenCL build failed");
    }

    kernel = clCreateKernel(program, "fill_points", &err);
    checkCLError(err, "clCreateKernel");

    // // creating buffer
    // buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float2) * pointCount, nullptr, &err);
    // checkCLError(err, "clCreateBuffer");
}



void OpenCLCompute::initSharedBuffer()
{
    glGenBuffers(1, &glVBO);
    glBindBuffer(GL_ARRAY_BUFFER, glVBO);
    glBufferData(GL_ARRAY_BUFFER, pointCount * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    cl_int err;

    if (useSharedBuffer)
    {

        clBuffer = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY, glVBO, &err);
        if (err == CL_SUCCESS)
            std::print("Shared buffer created\n");
        else
        {
            clBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float2) * pointCount, nullptr, &err);
            useSharedBuffer = false;
        }
    }
    else
    {
        clBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float2) * pointCount, nullptr, &err);
    }

    checkCLError(err, "clCreateBuffer");
}



void OpenCLCompute::updateBufferData(float time)
{
    int N = static_cast<int>(pointCount);
    cl_int err;
    size_t globalSize = pointCount;
    if (useSharedBuffer)
    {
        clEnqueueAcquireGLObjects(queue, 1, &clBuffer, 0, nullptr, nullptr);

        clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuffer);
        clSetKernelArg(kernel, 1, sizeof(int), &N);
        clSetKernelArg(kernel, 2, sizeof(float), &time);

        err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);
        checkCLError(err, "clEnqueueNDRangeKernel");
        
        clEnqueueReleaseGLObjects(queue, 1, &clBuffer, 0, nullptr, nullptr);
        clFinish(queue);
    }
    else
    {    
        // configuring kernel parameters
        clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuffer);
        clSetKernelArg(kernel, 1, sizeof(int), &N);
        clSetKernelArg(kernel, 2, sizeof(float), &time);

        // executing kernel
        err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);
        checkCLError(err, "clEnqueueNDRangeKernel");
        clFinish(queue);

        // read data back to CPU
        std::vector<cl_float2> tempBuffer(pointCount);
        clEnqueueReadBuffer(queue, clBuffer, CL_TRUE, 0, sizeof(cl_float2) * pointCount, tempBuffer.data(), 0, nullptr, nullptr);

        // updating VBO
        glBindBuffer(GL_ARRAY_BUFFER, glVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * pointCount * 2, tempBuffer.data());
    }
}



OpenCLCompute::~OpenCLCompute()
{
    if (clBuffer) clReleaseMemObject(clBuffer);
    if (glVBO) glDeleteBuffers(1, &glVBO);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}

// const float* OpenCLCompute::getBufferData() const {
    // return reinterpret_cast<const float*>(hostBuffer.data());
// }




