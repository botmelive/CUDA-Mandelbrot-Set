#include "kernel.h"
#include <stdio.h>

#define TX 32
#define TY 32
#define MAX_ITER 6

typedef double Float;

__device__ unsigned char clip(int n) { return n > 255 ? 255 : (n < 0 ? 0 : n); }

__device__ float shiftrange(float c, float d, float t)
{
   return c + (float)(d - c) * t;
}

/*
x2:= 0
y2:= 0

while (x2 + y2 ≤ 4 and iteration < max_iteration) do
    y:= 2 * x * y + y0
    x:= x2 - y2 + x0
    x2:= x * x
    y2:= y * y
    iteration:= iteration + 1
*/

__device__ int mandelbrot(Float x0, Float y0, int MAX)
{
   /*https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set*/
   Float x = 0.0f;
   Float y = 0.0f;
   Float x2 = 0.0f;
   Float y2 = 0.0f;
   int iteration = 0;

   while (x * x + y * y <= 4 && iteration < MAX)
   {
      y = (x + x) * y + y0;
      x = x2 - y2 + x0;
      x2 = x * x;
      y2 = y * y;
      iteration++;
   }

   return iteration;
}

__device__ void HSV2RGB(float H, float S, float V, int &R, int &G, int &B)
{
   float nNormalizedH = (float)H * 0.003921569F; // / 255.0F
   float nNormalizedS = (float)S * 0.003921569F;
   float nNormalizedV = (float)V * 0.003921569F;
   float nR;
   float nG;
   float nB;
   if (nNormalizedS == 0.0F)
   {
      nR = nG = nB = nNormalizedV;
   }
   else
   {
      if (nNormalizedH == 1.0F)
         nNormalizedH = 0.0F;
      else
         nNormalizedH = nNormalizedH * 6.0F; // / 0.1667F
   }
   float nI = floorf(nNormalizedH);
   float nF = nNormalizedH - nI;
   float nM = nNormalizedV * (1.0F - nNormalizedS);
   float nN = nNormalizedV * (1.0F - nNormalizedS * nF);
   float nK = nNormalizedV * (1.0F - nNormalizedS * (1.0F - nF));
   if (nI == 0.0F)
   {
      nR = nNormalizedV;
      nG = nK;
      nB = nM;
   }
   else if (nI == 1.0F)
   {
      nR = nN;
      nG = nNormalizedV;
      nB = nM;
   }
   else if (nI == 2.0F)
   {
      nR = nM;
      nG = nNormalizedV;
      nB = nK;
   }
   else if (nI == 3.0F)
   {
      nR = nM;
      nG = nN;
      nB = nNormalizedV;
   }
   else if (nI == 4.0F)
   {
      nR = nK;
      nG = nM;
      nB = nNormalizedV;
   }
   else if (nI == 5.0F)
   {
      nR = nNormalizedV;
      nG = nM;
      nB = nN;
   }
   R = (int)(nR * 255.0F);
   G = (int)(nG * 255.0F);
   B = (int)(nB * 255.0F);
}

__global__ void mandelbrotKernel(uchar4 *d_out, int w, int h, int MAX, Bounds bounds)
{
   const int c = blockIdx.x * blockDim.x + threadIdx.x;
   const int r = blockIdx.y * blockDim.y + threadIdx.y;

   if ((c >= w) || (r >= h))
      return; // Check if within image bounds

   const int i = c + r * w; // 1D indexing

   Float x = (Float)c / w;
   Float y = (Float)r / h;

   Float x_scaled = shiftrange(bounds.B1.x, bounds.B2.x, x);
   Float y_scaled = shiftrange(bounds.B1.y, bounds.B2.y, y);

   int iter = mandelbrot(x_scaled, y_scaled, MAX);

   int color = 0;
   int R = 0;
   int G = 0;
   int B = 0;
   if (iter < MAX)
   {
      int H = 255 * ((float)iter / MAX);
      int S = 255;
      int L = iter < MAX ? 255 : 0;

      HSV2RGB(H, S, L, R, G, B);
   }

   d_out[i].x = R;
   d_out[i].y = G;
   d_out[i].z = B;
   d_out[i].w = 255;
}

void kernelLauncher(uchar4 *d_out, int w, int h, int color, Bounds bounds)
{
   const dim3 gridSize = dim3((w + TX - 1) / TX, (h + TY - 1) / TY);
   const dim3 blockSize(TX, TY);
   mandelbrotKernel<<<gridSize, blockSize>>>(d_out, w, h, color, bounds);
}