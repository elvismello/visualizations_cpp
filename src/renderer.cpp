#include <GL/glew.h>
#include <string>
#include <stdexcept>
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
GLuint makeShader(GLenum type, const char* src) {
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



GLuint makeProgram(const char* vs, const char* fs) {
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