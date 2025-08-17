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
        
        cl_mem cl_buffer; // opencl sees this buffer
        GLuint gl_vbo; // opengl sees this buffer
        
        size_t pointCount;

    public:
        OpenCLCompute(size_t numPoints);
        ~OpenCLCompute();
        
        // creates shared buffer after the opengl context
        void initSharedBuffer();

        // updates points directly in the opengl buffer
        void updateBufferData(float time);
        
        size_t getPointCount() const;


};