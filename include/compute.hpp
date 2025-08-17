#pragma once
#include "common.hpp"
#include "renderer.hpp"
#include <cstddef>
#include <vector>

// #define CL_TARGET_OPENCL_VERSION 220
// #include <CL/cl.h>

class OpenCLCompute {
    private:
        cl_device_id device;
        cl_context context;
        cl_command_queue queue;
        cl_kernel kernel;
        cl_program program;
        
        cl_mem clBuffer; // opencl sees this buffer
        //GLuint glVBO; // opengl sees this buffer
        
        size_t pointCount;
        bool useSharedBuffer;

    public:
        OpenCLCompute(size_t numPoints);
        ~OpenCLCompute();
        
        // creates shared buffer after the opengl context
        void initBuffer();

        // updates points directly in the opengl buffer
        void updateBufferData(float time, std::vector<cl_float2> * tempBuffer);
        
        //GLuint getVBO() const {return glVBO;};

        size_t getPointCount() const {return pointCount;};


};