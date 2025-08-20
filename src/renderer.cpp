//#include <string>
//#include <GL/glew.h>
//#include <GL/glext.h>
#include <cstddef>
#include <stdexcept>
#include "common.hpp"
#include "renderer.hpp"
// #include <string_view>


// const std::string_view kVS = R"(
// #version 330 core
// layout (location = 0) in vec2 aPos;
// void main() {
//     gl_PointSize = 2.5;
//     gl_Position = vec4(aPos, 0.0, 1.0);
// }
// )";

// const std::string_view kFS = R"(
// #version 330 core
// out vec4 FragColor;
// void main() {
//     FragColor = vec4(0.15, 0.8, 1.0, 1.0);
// }
// )";



// Shader Functions
GLuint Renderer::makeShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0; 
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(s, len, nullptr, log.data());
        
        throw std::runtime_error(std::string("[GL] shader compile error:\n")\
                                 + log);
        }
    return s;
}



GLuint Renderer::makeProgram(const char* vs, const char* fs) {
    GLuint v = makeShader(GL_VERTEX_SHADER, vs);
    GLuint f = makeShader(GL_FRAGMENT_SHADER, fs);
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(p, len, nullptr, log.data());        
        throw std::runtime_error(std::string("[GL] program link error:\n")\
                                 + log);
    }
    return p;
}



Renderer::Renderer(size_t numPoints) : pointCount(numPoints)
{
    setupBuffers();
    setupVertexAttributes();
}



void Renderer::setupBuffers()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, pointCount * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
}



void Renderer::setupVertexAttributes()
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
}



void Renderer::loadAndCompileShaders(const std::string& vertexPath, const std::string& fragmentPath)
{
    auto vertexSrc = loadKernelSource(vertexPath);
    auto fragmentSrc = loadKernelSource(fragmentPath);

    glProgram = makeProgram(vertexSrc.c_str(), fragmentSrc.c_str());
    glUseProgram(glProgram);
}



void Renderer::updateBuffer (const std::vector<cl_float2>& data)
{
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * pointCount * 2, data.data());
}



//void Renderer::clear(float r, float g, float b)
void Renderer::clear()
{
    //glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}



void Renderer::render(bool drawLines){
    if (drawLines)
    {
        glLineWidth(2.0f);
        glDrawArrays(GL_LINE_STRIP, 0, pointCount);
    }
    else
    {
        glDrawArrays(GL_POINTS, 0, pointCount);
    }
}



Renderer::~Renderer()
{
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (glProgram) glDeleteProgram(glProgram);
}