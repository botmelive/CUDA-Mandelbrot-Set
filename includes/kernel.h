#pragma once

#include <curand_kernel.h>
#include <cuComplex.h>

struct uchar4;
struct float2;
struct int2;

struct Bounds{
    float2 B1 = {-1.5f, -1.0f};
    float2 B2 = { 0.6f,  1.0f};
};

void kernelLauncher(uchar4*, int, int, int, Bounds);