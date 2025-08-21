__kernel void fill_points(global float2* points, const int N, const float t) {
    int i = get_global_id(0);
    if (i >= N) return;
    float u = (float)i / (float)(N - 1);
    float a = 2.0f;
    float b = 3.0f;
    // float x = 0.8f * cos(2.0f * 3.1415926f * (a*u + 0.10f*t));
    float x = 0.8f * cos(4.0f * 3.1415926f * (a*u + 0.10f*t));
    // float y = 0.8f * sin(2.0f * 3.1415926f * (b*u + 0.13f*t));
    float y = 0.8f * sin(2.0f * 3.1415926f * (b*u + 0.2f*t));
    float nx = x;
    float ny = y;
    points[i] = (float2)(nx, ny);
}