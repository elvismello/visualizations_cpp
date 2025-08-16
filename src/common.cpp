#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <print>

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


// Helper functions
std::string loadKernelSource(const std::string& path){
    std::ifstream file(path);
    if (!file.is_open()){
        throw std::runtime_error("Couldn't load " + path);
    }
    std::ostringstream stringBuffer;
    stringBuffer << file.rdbuf();
    auto source = stringBuffer.str();

    if (source.size() >= 3 && 
        (unsigned char)source[0] == 0xEF &&
        (unsigned char)source[1] == 0xBB &&
        (unsigned char)source[2] == 0xBF)
    {
        std::println("BOM format encounterd...");
        source = source.substr(3);
    }

    return source;
}