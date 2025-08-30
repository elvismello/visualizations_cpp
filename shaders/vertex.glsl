#version 330 core
layout (location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    gl_PointSize = 10.0;

    // float dist = length(aPos); 
    // gl_PointSize = 1.1 * (2.0 + 8.0 * (1.0 - dist)); 
    // gl_Position = vec4(aPos, 0.0, 1.0);
}