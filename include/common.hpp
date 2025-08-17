#pragma once
#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>
#include <string>


// template<typename T>
//void checkOrExit(T condition, const char* msg);
void checkOrExit(bool condition, const char* msg);
void checkCLError(cl_int err, const char* where);
std::string loadKernelSource(const std::string& path);

constexpr size_t N = 1200000; // Point Number
constexpr size_t WINDOW_WIDTH = 1366;
constexpr size_t WINDOW_HEIGHT = 768;