#pragma once
#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>

// template<typename T>
//void checkOrExit(T condition, const char* msg);
void checkOrExit(bool condition, const char* msg);
void checkCLError(cl_int err, const char* where);

constexpr size_t N = 120000; // Point Number