#pragma once
#include "common.hpp"
#include <GLFW/glfw3.h>


class Window {
    private:
        GLFWwindow* window;
        int width, height;
        bool vsync;

    public:
        Window(int w, int h, bool vsync, const std::string& title);
        bool shouldClose() const;
        void pollEvents();
        void swapBuffers();

        ~Window();

};