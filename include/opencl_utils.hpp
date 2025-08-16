#pragma once
#include <string>
#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>

std::string loadKernelSource(const std::string& path);