#pragma once
#include <vector>
#include <string>
#include <CL/cl.h>
#include <GL/glew.h>

// Shaders
GLuint makeShader(GLenum type, const char* src);
GLuint makeProgram(const char* vs, const char* fs);


class Renderer {
    private:
        GLuint vao, vbo;
        GLuint glProgram;
        size_t pointCount;

        void setupBuffers();
        void setupVertexAttributes();
        GLuint makeShader(GLenum type, const char* src);
        GLuint makeProgram(const char* vs, const char* fs);


    public:
        Renderer(size_t numPoints);
        
        // Shader management
        void loadAndCompileShaders(const std::string& vertexPath, const std::string& fragmentPath);
        
        // Buffer operations
        void updateBuffer(const std::vector<cl_float2>& data);
        
        // Rendering
        // void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f);
        void clear();
        void render(bool drawLines = false);
        
        // Getters
        size_t getPointCount() const { return pointCount; }
        
        ~Renderer();
};
