#version 330 core
in vec4 gl_FragCoord;
out vec4 FragColor;
void main()
{
    vec2 screenCenter = vec2(480, 360);
    float dist = length(gl_FragCoord.xy - screenCenter) / 400.0;
    vec3 color = mix(vec3(0.15, 0.8, 1.0), vec3(1.0, 0.3, 0.8), dist);
    FragColor = vec4(color, 1.0);
} 