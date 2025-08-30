#pragma once
#include <cstddef>
#include <string>
#include <vector>

#include "common.hpp"
// #include "renderer.hpp"


// #define CL_TARGET_OPENCL_VERSION 220
// #include <CL/cl.h>

class OpenCLCompute {
    private:
        // opencl variables
        std::vector<cl_platform_id> allPlatforms;
        cl_device_id device;
        cl_context context;
        cl_command_queue queue;
        cl_kernel kernel;
        cl_program program;

        // io data
        std::string kernelPath;
        
        // buffers
        cl_mem clBuffer; // opencl sees this buffer
        //GLuint glVBO; // opengl sees this buffer

        cl_mem velocityBuffer; // for gravity computation
        cl_mem positionOutputBuffer;
        
        // kernel data for computation
        size_t pointCount;
        bool useSharedBuffer;
        
        // initialization functions
        // creates shared buffer after the opengl context
        void initializePlatform();
        void createContext(); // also selects device
        void createQueue();
        void createKernel();
        void createBuffers();
        void setInitialConditions();


    public:
        OpenCLCompute(size_t numPoints, std::string kernelPath);
        
        
        // updates points directly in the opengl buffer
        void updateBufferData(float time, std::vector<cl_float2> * tempBuffer);
        
        //GLuint getVBO() const {return glVBO;};
        
        size_t getPointCount() const {return pointCount;};
        
        ~OpenCLCompute();
};