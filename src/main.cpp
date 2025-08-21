//#include <iostream>
//#include <vector>
// #include <chrono>
// #include <cmath>
// #include <cstdlib>

// #include <print>

// #include "common.hpp"
// #include "renderer.hpp"
// #include "compute.hpp"
// #include "window.hpp"
#include "application.hpp"
#include "common.hpp"

// #include <GL/glew.h>


//#define CL_TARGET_OPENCL_VERSION 220
// #include <CL/cl.h>


int main() {


    Application::Config config;

    config.windowWidth = WINDOW_WIDTH;
    config.windowHeight = WINDOW_HEIGHT;
    config.windowTitle = "Aew, krl";
    config.pointCount = DEFAULT_POINT_COUNT;
    config.vsync = true;
    config.kernelPath = "../kernels/fill_points.cl";    
    config.vertexPath = "../shaders/vertex_1.glsl";
    config.fragmentPath = "../shaders/fragment_1.glsl";

    Application app(config);
    
    app.run();

    return 0;
}

