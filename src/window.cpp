#include "window.hpp"
//#include <GLFW/glfw3.h>
// #include <GLFW/glfw3.h>


Window::Window (int w, int h, bool vsync, const std::string& title) : width(w), height(h), vsync(vsync)
{
    glfwInit();
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    checkOrExit(window != nullptr, "Failed while creating window");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(vsync); // vsync
}



bool Window::shouldClose() const
{
    return glfwWindowShouldClose(window);
}



void Window::pollEvents()
{
    glfwPollEvents();
}



void Window::swapBuffers()
{
    glfwSwapBuffers(window);
}



Window::~Window()
{
    if (window)
        glfwDestroyWindow(window);
    glfwTerminate();
}