#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <cstddef>
//#include <exception>
#include <iostream>
//#include <vector>
#include <math.h>
#include <print>

#include "compute.hpp"
#include "common.hpp"
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
    createBuffers();
    setInitialConditions();
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

    kernel = clCreateKernel(program, "direct_sum_gravity", &err);
    checkCLError(err, "clCreateKernel");
}



void OpenCLCompute::createBuffers()
{
    cl_int err;

    clBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float2) * pointCount, nullptr, &err);

    velocityBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float2) * pointCount, nullptr, &err);

    positionOutputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float2) * pointCount, nullptr, &err);

    checkCLError(err, "clCreateBuffer");
}



void OpenCLCompute::setInitialConditions()
{
    std::vector<cl_float2> initialPositions(pointCount);
    std::vector<cl_float2> initialVelocities(pointCount);


    // create vector values

    for (size_t i = 0; i < pointCount; ++i)
    {        
        initialVelocities[i].s[0] = 0.0f;
        initialVelocities[i].s[1] = 0.0f;
    }

    float current_x = -0.5f;
    float current_y = -0.5f;
    for (size_t i = 0; i < pointCount; i++)
    {

        initialPositions[i].s[0] = i * 1.0f / pointCount - 0.5;
        initialPositions[i].s[1] = pow(-1, i) * 0.2f;

        // for (size_t j = 0; j < (pointCount / 10); j++)
        // {
            // size_t current_index = i * 10 + j;
            // if (current_index > pointCount) break;
// 
            // initialPositions[current_index].s[0] = current_x;
            // initialPositions[current_index].s[1] = current_y;
// 
            // current_y += 1.0f / (pointCount / 10);
        // }
        // current_x += 1.0f / 10.0f;
    }

    // writing buffers
    cl_int err;
    err = clEnqueueWriteBuffer(queue, clBuffer, CL_TRUE, 0, sizeof(cl_float2) * pointCount, initialPositions.data(), 0, nullptr, nullptr);
    checkCLError(err, "clEnqueueWriteBuffer (positions)");

    err = clEnqueueWriteBuffer(queue, velocityBuffer, CL_TRUE, 0, sizeof(cl_float2) * pointCount, initialVelocities.data(), 0, nullptr, nullptr);
    checkCLError(err, "clEnqueueWriteBuffer (velocities)");

    // ensuring that write was done
    clFinish(queue);
}




void OpenCLCompute::updateBufferData(float time, std::vector<cl_float2> * tempBuffer)
{
    int N = static_cast<int>(pointCount);
    cl_int err;
    size_t globalSize = pointCount;
    
    // configuring kernel parameters
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuffer);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &velocityBuffer);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &positionOutputBuffer);
    clSetKernelArg(kernel, 3, sizeof(int), &N);
    clSetKernelArg(kernel, 4, sizeof(float), &time);

    // executing kernel
    err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);
    checkCLError(err, "clEnqueueNDRangeKernel");
    clFinish(queue);

    std::swap(clBuffer, positionOutputBuffer);

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



