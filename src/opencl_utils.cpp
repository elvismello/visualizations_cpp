#include <fstream>
#include <sstream>
#include <string>
// #include "../include/opencl_utils.h"

// Kernel OpenCL
std::string loadKernelSource(const std::string& path){
    std::ifstream file(path);
    if (!file.is_open()){
        throw std::runtime_error("Couldn't load " + path);
    }
    std::ostringstream stringBuffer;
    stringBuffer << file.rdbuf();
    return stringBuffer.str();
}
