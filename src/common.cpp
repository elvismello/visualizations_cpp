#include <string>
#include <stdexcept>
#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>


// Exception handling
// Util
// template<typename T>

void checkOrExit(bool condition, const char* msg) {
    if (!condition) {
        throw std::runtime_error(std::string("[error] ")\
                                 + msg);
    }
}

void checkCLError(cl_int err, const char* where) {
    if (err != CL_SUCCESS) {
        throw std::runtime_error(std::string("[OpenCL error] ")\
                                 + where\
                                 + std::string(" -> ")\
                                 + std::to_string(err));
    }
}