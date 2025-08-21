#include <CL/cl.h>
#include <cstddef>
#include <iostream>
//#include <vector>
#include <print>

#include "compute.hpp"
//#include <CL/cl.h>
//#include <CL/cl_platform.h>
//#include <GLFW/glfw3.h>
//#include <GL/glx.h>



OpenCLCompute::OpenCLCompute(size_t numPoints, std::string kernelPath) : pointCount(numPoints), kernelPath(kernelPath)
{
    initializePlatform();
    createContext();
    createQueue();
    createKernel();
    createBuffer();
}



void OpenCLCompute::initializePlatform()
{
    // finding platform
    cl_uint numPlatforms = 0;
    clGetPlatformIDs(0, nullptr, &numPlatforms);
    //std::vector<cl_platform_id> allPlatforms(numPlatforms);
    allPlatforms.resize(numPlatforms);
    clGetPlatformIDs(numPlatforms, allPlatforms.data(), nullptr);
    
    checkOrExit(numPlatforms > 0, "No OpenCL platform encountered");
}



void OpenCLCompute::createContext()
{
    // finding device
    cl_uint numDevices = 0;
    clGetDeviceIDs(allPlatforms[0], CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
    std::vector<cl_device_id> allDevices(numDevices);
    clGetDeviceIDs(allPlatforms[0], CL_DEVICE_TYPE_GPU, numDevices, allDevices.data(), nullptr);
    
    checkOrExit(numDevices > 0, "No OpenCL device encountered");
    device = allDevices[0]; // possible implementation of an automatic device selection

    // creating context
    cl_int err = 0;

    std::print("Using regular OpenCL (no shared buffer)...\n");
    context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    checkCLError(err, "clCreateContext");
    useSharedBuffer = false;

}



void OpenCLCompute::createQueue()
{
    cl_int err = 0;

    // creating queue
    queue = clCreateCommandQueueWithProperties(context, device, nullptr, &err);
    checkCLError(err, "clCreateCommandQueueWithProperties");

}



void OpenCLCompute::createKernel()
{
    // loading and compiling kernel
 
    cl_int err;

    auto kernelSrc = loadKernelSource(kernelPath);
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
}



void OpenCLCompute::createBuffer()
{
    cl_int err;

    clBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float2) * pointCount, nullptr, &err);

    checkCLError(err, "clCreateBuffer");
}



void OpenCLCompute::updateBufferData(float time, std::vector<cl_float2> * tempBuffer)
{
    int N = static_cast<int>(pointCount);
    cl_int err;
    size_t globalSize = pointCount;
    
    // configuring kernel parameters
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuffer);
    clSetKernelArg(kernel, 1, sizeof(int), &N);
    clSetKernelArg(kernel, 2, sizeof(float), &time);

    // executing kernel
    err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);
    checkCLError(err, "clEnqueueNDRangeKernel");
    clFinish(queue);

    // read data back to CPU
    clEnqueueReadBuffer(queue, clBuffer, CL_TRUE, 0, sizeof(cl_float2) * pointCount, (*tempBuffer).data(), 0, nullptr, nullptr);
}



OpenCLCompute::~OpenCLCompute()
{
    if (clBuffer) clReleaseMemObject(clBuffer);
    //if (glVBO) glDeleteBuffers(1, &glVBO);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    // clReleaseDevice(device); // Removed: device was not retained or created via clCreateSubDevices
}



