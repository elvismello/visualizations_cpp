#include "window.hpp"
//#include <GLFW/glfw3.h>
// #include <GLFW/glfw3.h>


Window::Window (int w, int h, const std::string& title) : width(w), height(h)
{
    glfwInit();
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    checkOrExit(window != nullptr, "Failed while creating window");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync
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