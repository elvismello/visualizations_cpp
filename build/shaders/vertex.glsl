#version 330 core
layout (location = 0) in vec2 aPos;
void main() {
    gl_PointSize = 2.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}