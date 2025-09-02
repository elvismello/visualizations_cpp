#version 330 core
// in vec4 gl_FragCoord;
out vec4 FragColor;
void main()
{
    // float radius = length(gl_PointCoord.xy - gl_FragCoord.xy);
// 
    // if (radius > 1.0)
        // discard;

    FragColor = vec4(0.15, 0.8, 1.0, 1.0);
    // vec2 screenCenter = vec2(1366, 768) / 2;
    // float dist = length(gl_FragCoord.xy - screenCenter) / 400.0;
    // vec3 color = mix(vec3(0.15, 0.8, 1.0), vec3(1.0, 0.3, 0.8), dist);
    // FragColor = vec4(color, 1.0);
}