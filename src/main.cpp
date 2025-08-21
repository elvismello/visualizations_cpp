#include "application.hpp"
#include "common.hpp"


int main() {
    Application::Config config;

    config.windowWidth = WINDOW_WIDTH;
    config.windowHeight = WINDOW_HEIGHT;
    config.windowTitle = "visProject";
    config.pointCount = DEFAULT_POINT_COUNT;
    // config.pointCount = D;
    config.vsync = true;
    config.kernelPath = "../kernels/fill_points.cl";    
    config.vertexPath = "../shaders/vertex_1.glsl";
    config.fragmentPath = "../shaders/fragment_1.glsl";

    Application app(config);
    
    app.run();

    return 0;
}

