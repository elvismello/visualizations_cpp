// #include <opencl-c-base.h>
__kernel void direct_sum_gravity(global float2* points,
                                 global float2* pointsVel,
                                 global float2* pointsOutput,
                                 const int N, const float dt)
{
    int i = get_global_id(0);
    if (i >= N) return; // Needed?

    float2 accel = (float2)(0.0, 0.0);
    for (int j = 0; j < N; j++)
    {
        if (i==j) continue;
        
        float dist = distance(points[i], points[j]) + 1e-5f;

        float G = 4.0f * 3.1415f / 1e7;
        accel = accel + G * 1.0f / pow(dist, 3)
                * (points[j] - points[i]);
    }

    // Integrating velocity and postion
    // needs another float2 vec for the previous accel
    float2 velHalf = pointsVel[i] + accel * dt / 2; // use previous accel
    pointsOutput[i] = points[i] + velHalf * dt;
    // if (pointsOutput[i].x < -1.0f) {
        // pointsOutput[i].x = -1.0f;
        // velHalf.x = -velHalf.x;
    // }
    // if (pointsOutput[i].x > 1.0f) {
        // pointsOutput[i].x = 1.0f;
        // velHalf.x = -velHalf.x;
    // }
    // if (pointsOutput[i].y < -1.0f) {
        // pointsOutput[i].y = -1.0f;
        // velHalf.y = -velHalf.y;
    // }
    // if (pointsOutput[i].y > 1.0f) {
        // pointsOutput[i].y = 1.0f;
        // velHalf.y = -velHalf.y;
    // }

    if (length(pointsVel[i]) > 100)
    {
        pointsVel[i].x = 5;
        pointsVel[i].y = 5;
    }
    // calculate new accel here
    pointsVel[i] = velHalf + accel * dt / 2;
}