#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <cstddef>
//#include <exception>
#include <iostream>
//#include <vector>
#include <math.h>
#include <print>
#include <string>

#include "compute.hpp"
#include "common.hpp"
//#include <CL/cl.h>
//#include <CL/cl_platform.h>
//#include <GLFW/glfw3.h>
//#include <GL/glx.h>



Compute::Compute(size_t numPoints, std::string kernelPath) : pointCount(numPoints), kernelPath(kernelPath)
{
    initializePlatform();
    createContext();
    createQueue();
    createKernel();
    createBuffers();
    setInitialConditions();
}



void Compute::initializePlatform()
{
    // finding platform
    cl_uint numPlatforms = 0;
    clGetPlatformIDs(0, nullptr, &numPlatforms);
    //std::vector<cl_platform_id> allPlatforms(numPlatforms);
    allPlatforms.resize(numPlatforms);
    clGetPlatformIDs(numPlatforms, allPlatforms.data(), nullptr);
    
    checkOrExit(numPlatforms > 0, "No OpenCL platform encountered");
}



void Compute::createContext()
{
    // finding device
    cl_uint numDevices = 0;
    clGetDeviceIDs(allPlatforms[0], CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices);
    std::vector<cl_device_id> allDevices(numDevices);
    clGetDeviceIDs(allPlatforms[0], CL_DEVICE_TYPE_ALL, numDevices, allDevices.data(), nullptr);
    
    checkOrExit(numDevices > 0, "No OpenCL device encountered");
    
    // simple estimation of the most capable device
    std::print("Finding best device and listing FLOPS estimatimation...\n\n");
    std::print("{:<10s} {:<50s}\n", "TFLOPS", "Device name");
    for (int i = 0; i < 60; i++)
    {
        std::print("-");
    }
    std::print("\n");
    cl_device_id bestDevice = allDevices[0];
    std::string bestDeviceName;
    int maxFlops = 0;
    for (auto dev: allDevices)
    {
        cl_uint computeUnits = 0;
        cl_uint clockFreq = 0;
        cl_uint deviceType = 0;
        char name[256];

        // TODO add way to identify cores per computation unit for GPUs
        // https://github.com/ProjectPhysX/OpenCL-Wrapper does it

        clGetDeviceInfo(dev, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &computeUnits, nullptr);
        clGetDeviceInfo(dev, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &clockFreq, nullptr);
        clGetDeviceInfo(dev, CL_DEVICE_TYPE, sizeof(cl_uint), &deviceType, nullptr);
        clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(name), name, nullptr);
        

        cl_uint ipc = deviceType == CL_DEVICE_TYPE_GPU ? 2u : 32u; // instructions per cycle
        auto flops = computeUnits * clockFreq * ipc;
        std::print("{:<10.6f} {:<50s}\n", (float)flops * 1e-6, name);
        if (flops > maxFlops)
        {
            maxFlops = flops;
            bestDevice = dev;
            bestDeviceName = name;
        }
    }
    for (int i = 0; i < 60; i++)
    {
        std::print("-");
    }
    std::print("\n\n");
    device = bestDevice;

    std::print("Best device is {}\n", bestDeviceName);
    

    // creating context
    cl_int err = 0;
    context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    checkCLError(err, "clCreateContext");
    useSharedBuffer = false;

}



void Compute::createQueue()
{
    cl_int err = 0;

    // creating queue
    queue = clCreateCommandQueueWithProperties(context, device, nullptr, &err);
    checkCLError(err, "clCreateCommandQueueWithProperties");

}



void Compute::createKernel()
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



void Compute::createBuffers()
{
    cl_int err;

    clBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float2) * pointCount, nullptr, &err);

    velocityBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float2) * pointCount, nullptr, &err);

    positionOutputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float2) * pointCount, nullptr, &err);

    checkCLError(err, "clCreateBuffer");
}



void Compute::setInitialConditions()
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
        initialPositions[i].s[0] = i * 1.0f / pointCount * 1.8f - 0.9f;
        initialPositions[i].s[1] = i % 100 / 100.0f * 1.8f - 0.9f;
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




void Compute::updateBufferData(float time, std::vector<cl_float2> * tempBuffer)
{
    int N = static_cast<int>(pointCount);
    cl_int err;
    size_t globalSize = pointCount;

    // Tree computation


    
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



Compute::~Compute()
{
    if (clBuffer) clReleaseMemObject(clBuffer);
    //if (glVBO) glDeleteBuffers(1, &glVBO);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    // clReleaseDevice(device); // Removed: device was not retained or created via clCreateSubDevices
}



