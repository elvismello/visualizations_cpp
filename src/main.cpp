#include "application.hpp"
#include "common.hpp"


int main() {
    Application::Config config;

    config.windowWidth = WINDOW_WIDTH;
    config.windowHeight = WINDOW_HEIGHT;
    config.windowTitle = "visProject";
    // config.pointCount = DEFAULT_POINT_COUNT;
    config.pointCount = 10000;
    // config.pointCount = D;
    config.vsync = true;
    config.kernelPath = "../kernels/gravity.cl";    
    config.vertexPath = "../shaders/vertex.glsl";
    config.fragmentPath = "../shaders/fragment.glsl";

    Application app(config);
    
    app.run();

    return 0;
}

