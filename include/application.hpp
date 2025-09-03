#pragma once
#include <memory>
#include <chrono>
#include <print>

#include <GL/glew.h>

#include "common.hpp"
#include "window.hpp"
#include "compute.hpp"
#include "renderer.hpp"
//#include <GLFW/glfw3.h>



class Application {
    private:
        std::unique_ptr<Window> window;
        std::unique_ptr<Renderer> renderer;
        std::unique_ptr<Compute> compute;
        

        // Timing
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::time_point lastTime;

        float deltaTime;
        float elapsedTime;


        // Methods
        void initialize();
        void updateTiming();
        void printStats();


    public:
        
        // Configuration
        struct Config {
            int windowWidth;
            int windowHeight;
            std::string windowTitle;
            size_t pointCount;
            bool vsync;
            std::string kernelPath;
            std::string vertexPath;
            std::string fragmentPath;
        } config;


        Application(const Config& cfg);
        void run(); // main loop
        ~Application();
};



