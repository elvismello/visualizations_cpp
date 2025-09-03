#include "application.hpp"
#include "compute.hpp"
#include <chrono>
#include <memory>
#include <print>



Application::Application (const Config& cfg) : config(cfg)
{
    initialize();
}



void Application::initialize()
{
    // create window
    window = std::make_unique<Window>(config.windowWidth,
                                      config.windowHeight,
                                      config.vsync,
                                      config.windowTitle);

    // create opencl compute object
    compute = std::make_unique<Compute>(config.pointCount,
                                              config.kernelPath);

    // renderer
    renderer = std::make_unique<Renderer>(config.pointCount);
    renderer->loadAndCompileShaders(config.vertexPath, config.fragmentPath);

    startTime = std::chrono::steady_clock::now();
    lastTime = startTime;
}



void Application::updateTiming()
{
    auto currentTime = std::chrono::steady_clock::now();
    deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    elapsedTime = std::chrono::duration<float>(currentTime - startTime).count();
    lastTime = currentTime;
}



void Application::printStats()
{
    static int frameCount = 0;
    static float timeAccum = 0.0f;
    
    frameCount++;
    timeAccum += deltaTime;
    
    // Print stats every 60 frames
    if (frameCount >= 60) {
        float avgDelta = timeAccum / frameCount;
        float fps = 1.0f / avgDelta;
        
        std::print("Time: {:6.2f}s | FPS: {:6.0f} | Frame: {:6.3f}ms\r", 
                   elapsedTime, fps, avgDelta * 1000.0f);
        std::fflush(stdout);
        
        frameCount = 0;
        timeAccum = 0.0f;
    }
}



void Application::run()
{
    std::vector<cl_float2> tempBuffer(compute->getPointCount());

    while (!window->shouldClose())
    {
        window->pollEvents();
        // handleInput();
        updateTiming();

        //compute->updateBufferData(elapsedTime, &tempBuffer);
        compute->updateBufferData(deltaTime, &tempBuffer);
        renderer->updateBuffer(tempBuffer);

        renderer->clear();
        renderer->render();
        
        window->swapBuffers();

        printStats();
    }
}






Application::~Application()
{
    std::print("\nBye!\n");
}