#include "compute.hpp"
#include "renderer.hpp"
#include <CL/cl.h>
// #include <CL/cl_platform.h>
// #include <cstddef>
#include <CL/cl_platform.h>
#include <cstddef>
#include <iostream>
#include <vector>



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
    device = allDevices[0];// possible implementation of a automatic device selection
    
    // creating context
    cl_int err = 0;
    context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    checkCLError(err, "clCreateContext");

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



OpenCLCompute::~OpenCLCompute()
{
    if (cl_buffer) clReleaseMemObject(cl_buffer);
    if (gl_vbo) glDeleteBuffers(1, &gl_vbo);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}



void OpenCLCompute::initSharedBuffer()
{
    glGenBuffers(1, &gl_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
    glBufferData(GL_ARRAY_BUFFER, pointCount * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    cl_int err;
    cl_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float2) * pointCount, nullptr, &err);
    checkCLError(err, "clCreateBuffer");
}



void OpenCLCompute::updateBufferData(float time)
{
    // configuring kernel parameters
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &cl_buffer);
    clSetKernelArg(kernel, 1, sizeof(size_t), &pointCount);
    clSetKernelArg(kernel, 2, sizeof(float), &time);

    // executing kernel
    size_t globalSize = pointCount;
    clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);

    // read data back to CPU
    std::vector<cl_float2> tempBuffer(pointCount);
    clEnqueueReadBuffer(queue, cl_buffer, CL_TRUE, 0, sizeof(cl_float2) * pointCount, tempBuffer.data(), 0, nullptr, nullptr);

    // updating VBO
    glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * pointCount * 2, tempBuffer.data());
}



size_t OpenCLCompute::getPointCount() const {
    return pointCount;
}



// const float* OpenCLCompute::getBufferData() const {
    // return reinterpret_cast<const float*>(hostBuffer.data());
// }




